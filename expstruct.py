#!/usr/bin/env python3
"""Structural fingerprint of a SIGEL .exp, for cross-machine diffing.

PORTING.md D26 and section 7: a fitness number is NOT a cross-machine
reference in either direction -- a 1-ULP change in start height moves fitness
45%, and the reference box is i386/x87 against this one's aarch64/IEEE. So
this dumps what compares EXACTLY -- the same individuals present, the same
names, the same program text, the same ordering, the same file shape -- and
drops every float.

The dropping is deliberate and checked, not incidental: --audit prints every
line this refused to look at, so "no float reached the fingerprint" is a
claim the reader can verify rather than take on trust.

    expstruct.py FILE            the fingerprint
    expstruct.py --audit FILE    the excluded lines, to prove what was dropped
"""
import hashlib, re, sys

# Anything matching these never reaches the fingerprint. FITNESS= is the
# obvious one; the HISTORY block carries "Fitness (Elter 1): 3.9e-05" too, and
# a date is wall-clock rather than structure, so a fresh run would differ on it
# for a reason that says nothing about the port.
DROP = re.compile(rb'FITNESS\s*=|Fitness\s*\(|Date of|^\s*$')
# A bare float anywhere is dropped as a belt-and-braces second net.
FLOAT = re.compile(rb'\d+\.\d+([eE][-+]?\d+)?')

IND   = re.compile(rb'INDIVIDUAL\((\d+)\) BEGIN')
NAME  = re.compile(rb"NAME\s*=\s*'([^']*)'")
KV    = re.compile(rb'^\s*(POOLPOS|AGE|POPULATIONSIZE|NEXTIDENTIFIER|POOLGENERATION)\s*=\s*(-?\d+)')
# A program line: an opcode in caps then integer operands. Integers only --
# there is no float in a program, which is why program text is the strongest
# surface available here.
OPC   = re.compile(rb'^\s*([A-Z]+)((\s+-?\d+(,-?\d+)*)?)\s*$')
# One experiment-history entry per generation, written by
# SIG_GPExperimentHistoryEntry::print() as
#   <generationNo> <year> <month> <day> <hour> <min> <sec> <3 floats>
# The three floats are the fitness curve and are NOT compared; the COUNT of
# entries and the generation numbers are integers and are. This has to be
# matched before the float filter below, or the floats on the line would drop
# the whole entry and the curve would look empty.
HIST  = re.compile(rb'^(\d+) (?:19|20)\d{2} \d{1,2} \d{1,2} \d{1,2} \d{1,2} \d{1,2} ')

def parse(path):
    hdr, inds, shape, dropped, hist = {}, [], [], [], []
    cur = None
    for raw in open(path, 'rb'):
        line = raw.rstrip(b'\n')
        m = HIST.match(line)
        if m:
            hist.append(int(m.group(1)))   # the generation number only
            continue
        if DROP.search(line) or FLOAT.search(line):
            if line.strip():
                dropped.append(line)
            continue
        m = IND.search(line)
        if m:
            cur = {'idx': int(m.group(1)), 'name': None, 'prog': [], 'kv': {}}
            inds.append(cur)
            continue
        m = NAME.search(line)
        if m and cur is not None:
            cur['name'] = m.group(1).decode('latin-1'); continue
        m = KV.match(line)
        if m:
            k, v = m.group(1).decode(), int(m.group(2))
            (cur['kv'] if cur is not None else hdr)[k] = v
            continue
        m = OPC.match(line)
        if m and cur is not None:
            cur['prog'].append(line.strip().decode('latin-1')); continue
        s = line.strip()
        if s and not s.startswith(b'#'):
            shape.append(s.decode('latin-1'))
    return hdr, inds, shape, dropped, hist

def sha(parts):
    h = hashlib.sha1()
    for p in parts:
        h.update(p.encode('latin-1') if isinstance(p, str) else p)
        h.update(b'\0')
    return h.hexdigest()[:16]

def field(rep, name):
    """One named field out of the report, so a probe asserts on the field it is
    about rather than on the whole blob."""
    for line in rep:
        if line.startswith(name):
            return line[len(name):].strip()
    return None

def selfcheck(path):
    """Teeth for the properties everything here rests on.

    An earlier version asserted only that the WHOLE report moved (or held). A
    fresh-eyes review demonstrated NINE of eleven tool-breakages passing that:
    SHAPE is a catch-all, so a matcher that silently STOPS matching still moves
    the report, and the probe reads that as success. Two changes follow:

      - sighted probes assert on the SPECIFIC field they are about (PROGRAMS,
        ORDER), never on the concatenation;
      - a structural floor compares the report against GROUND TRUTH recomputed
        from the raw bytes -- individual count, total program lines, history
        length against POOLGENERATION -- which is what catches a matcher that
        died and dumped its content into SHAPE.

    And the fitness probes are TWO. DROP does not match `Fitness Value:', which
    is 1720 of the fitness values in a shipped experiment against 100 `FITNESS='
    lines; FLOAT is their only net. A probe that changes a `FITNESS=' line alone
    cannot see FLOAT die, because its own injected value trips FLOAT."""
    import re as _re, tempfile, os
    base = open(path, 'rb').read()
    ref  = report(path)
    fails = []

    def variant(data, label, must_change, fieldname=None):
        fd, tmp = tempfile.mkstemp(suffix='.exp'); os.close(fd)
        open(tmp, 'wb').write(data)
        got = report(tmp)
        os.unlink(tmp)
        if fieldname:
            changed = field(got, fieldname) != field(ref, fieldname)
            what = fieldname
        else:
            changed = got != ref
            what = "report"
        if changed != must_change:
            fails.append("%s: %s %s, expected it to %s" % (
                label, what, "moved" if changed else "held",
                "move" if must_change else "hold"))

    # --- structural floor, recomputed from the raw bytes --------------------
    truth_inds = len(_re.findall(rb'INDIVIDUAL\(\d+\) BEGIN', base))
    truth_ops  = sum(1 for blk in _re.findall(rb'PROGRAM BEGIN\{(.*?)\}PROGRAM END;',
                                              base, _re.S)
                       for ln in blk.split(b'\n') if ln.strip())
    got_inds = int(field(ref, 'INDIVIDUALS') or -1)
    got_ops  = sum(int(m) for m in _re.findall(r'ops=(\d+)', "\n".join(ref)))
    if got_inds != truth_inds:
        fails.append("individuals: report says %d, the file has %d" % (got_inds, truth_inds))
    if got_ops != truth_ops:
        fails.append("program lines: report captured %d, the file has %d -- the "
                     "rest fell through to SHAPE" % (got_ops, truth_ops))
    # The three header fields must be PRESENT and numeric. Without this a dead
    # KV matcher prints them as "(absent)", the POOLGENERATION cross-check below
    # silently skips, and the whole breakage passes -- measured, not supposed.
    for k in ('POPULATIONSIZE', 'NEXTIDENTIFIER', 'POOLGENERATION'):
        v = field(ref, k)
        if v is None or not v.lstrip('-').isdigit():
            fails.append("header %s reads [%s], expected a number" % (k, v))
    # Per-individual detail must be present too, for the same reason.
    if _re.search(r'poolpos=-\s', "\n".join(ref)):
        fails.append("poolpos is absent on at least one individual")

    hist = (field(ref, 'HISTORY') or "0").split()[0]
    poolgen = field(ref, 'POOLGENERATION') or ""
    if not hist.isdigit() or int(hist) == 0:
        fails.append("history: report says [%s], expected a non-zero count" % hist)
    elif poolgen.isdigit() and int(hist) != int(poolgen):
        fails.append("history: %s entries but POOLGENERATION is %s -- one entry "
                     "per generation is the invariant" % (hist, poolgen))

    # --- fitness must be INVISIBLE, in BOTH spellings -----------------------
    m = _re.search(rb'FITNESS=([0-9.eE+-]+);', base)
    if m: variant(base.replace(m.group(0), b'FITNESS=0.123456;', 1),
                  "FITNESS= value changed", False)
    else: fails.append("no FITNESS= line found -- selfcheck cannot run")

    # An INTEGER-valued fitness, which FLOAT cannot see: it is the only probe
    # that can catch DROP losing its FITNESS= clause. With a float value the
    # belt-and-braces FLOAT net masks that breakage entirely.
    m = _re.search(rb'FITNESS=([0-9.eE+-]+);', base)
    if m: variant(base.replace(m.group(0), b'FITNESS=7;', 1),
                  "FITNESS= set to an INTEGER", False)

    m = _re.search(rb'Fitness Value: ([0-9.eE+-]+)', base)
    if m: variant(base.replace(m.group(0), b'Fitness Value: 0.987654', 1),
                  "Fitness Value: changed", False)
    else: fails.append("no 'Fitness Value:' line found -- selfcheck cannot run")

    # --- structure must be VISIBLE, in the field that owns it ---------------
    m = _re.search(rb'\n(\s+)(MOVE|ADD|SUB|CMP) (-?\d+)', base)
    if m:
        alt = b'\n' + m.group(1) + m.group(2) + b' ' + str(int(m.group(3)) + 1).encode()
        variant(base.replace(m.group(0), alt, 1), "program operand changed",
                True, 'PROGRAMS')
    else: fails.append("no program line found -- selfcheck cannot run")

    blocks = _re.findall(rb'  INDIVIDUAL\(\d+\) BEGIN\{.*?\}INDIVIDUAL\(\d+\) END;',
                         base, _re.S)
    if len(blocks) >= 2:
        sw = base.replace(blocks[0], b'@@A@@', 1).replace(blocks[1], b'@@B@@', 1)
        sw = sw.replace(b'@@A@@', blocks[1], 1).replace(b'@@B@@', blocks[0], 1)
        variant(sw, "two individuals swapped", True, 'ORDER')
    else: fails.append("fewer than 2 individuals -- selfcheck cannot run")

    for f in fails:
        print("  FAIL " + f)
    print("expstruct selfcheck  %d pass  %d fail" % (7 - len(fails), len(fails)))
    return 1 if fails else 0

def report(path):
    hdr, inds, shape, dropped, hist = parse(path)
    out = []
    for k in ('POPULATIONSIZE', 'NEXTIDENTIFIER', 'POOLGENERATION'):
        out.append(f"{k:16s} {hdr.get(k, '(absent)')}")
    out.append(f"{'INDIVIDUALS':16s} {len(inds)}")
    for d in inds:
        out.append("  [%4d] name=%-8s poolpos=%-5s age=%-4s ops=%-4d prog=%s" % (
            d['idx'], d['name'], d['kv'].get('POOLPOS', '-'),
            d['kv'].get('AGE', '-'), len(d['prog']), sha(d['prog'])))
    out.append(f"{'ORDER':16s} {sha([str(d['name']) for d in inds])}")
    out.append(f"{'PROGRAMS':16s} {sha([q for d in inds for q in d['prog']])}")
    out.append(f"{'SHAPE':16s} {sha(shape)}  ({len(shape)} structural lines)")
    if hist:
        contiguous = (hist == list(range(hist[0], hist[0] + len(hist))))
        out.append(f"{'HISTORY':16s} {len(hist)} entries, gen {hist[0]}..{hist[-1]}, "
                   f"contiguous={contiguous}")
    else:
        out.append(f"{'HISTORY':16s} 0 entries")
    return out

def main():
    if '--selfcheck' in sys.argv:
        args = [a for a in sys.argv[1:] if not a.startswith('--')]
        sys.exit(selfcheck(args[0]))
    audit = '--audit' in sys.argv
    args = [a for a in sys.argv[1:] if not a.startswith('--')]
    hdr, inds, shape, dropped, hist = parse(args[0])
    if audit:
        print(f"== EXCLUDED {len(dropped)} lines (never reach the fingerprint) ==")
        for d in dropped:
            print("  " + d.strip().decode('latin-1'))
        return
    print("\n".join(report(args[0])))

main()
