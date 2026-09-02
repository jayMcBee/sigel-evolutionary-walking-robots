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

def fingerprint(path):
    """The WHOLE report as one string -- what --selfcheck asserts on.
    Deliberately the whole thing, SHAPE included: an earlier version compared
    only the counts and the two content hashes, and it therefore could not have
    caught fitness leaking into SHAPE. A tool that must be blind to fitness has
    to be blind to it in every line it prints, not merely in the ones that get
    diffed most often."""
    return "\n".join(report(path))

def selfcheck(path):
    """Teeth for the one property everything here rests on: this must be blind
    to fitness and sighted on structure. Both halves are asserted, because a
    tool that saw NOTHING would pass the fitness half on its own."""
    import re as _re, tempfile, os
    base = open(path, 'rb').read()
    ref  = fingerprint(path)
    fails = []

    def variant(data, label, must_change):
        fd, tmp = tempfile.mkstemp(suffix='.exp'); os.close(fd)
        open(tmp, 'wb').write(data)
        got = fingerprint(tmp)
        os.unlink(tmp)
        changed = (got != ref)
        if changed != must_change:
            fails.append("%s: fingerprint %s, expected it to %s" % (
                label, "moved" if changed else "held",
                "move" if must_change else "hold"))
        return changed

    # 1. a changed FITNESS value must be INVISIBLE
    m = _re.search(rb'FITNESS=([0-9.eE+-]+);', base)
    if m:
        variant(base.replace(m.group(0), b'FITNESS=0.123456;', 1),
                "fitness value changed", False)
    else:
        fails.append("no FITNESS= line found -- selfcheck cannot run")

    # 2. a changed program OPERAND must be VISIBLE
    m = _re.search(rb'\n(\s+)(MOVE|ADD|SUB|CMP) (-?\d+)', base)
    if m:
        alt = b'\n' + m.group(1) + m.group(2) + b' ' + str(int(m.group(3)) + 1).encode()
        variant(base.replace(m.group(0), alt, 1), "program operand changed", True)
    else:
        fails.append("no program line found -- selfcheck cannot run")

    # 3. a swapped pair of individuals must be VISIBLE (ordering is compared)
    blocks = _re.findall(rb'  INDIVIDUAL\(\d+\) BEGIN\{.*?\}INDIVIDUAL\(\d+\) END;',
                         base, _re.S)
    if len(blocks) >= 2:
        sw = base.replace(blocks[0], b'@@A@@', 1).replace(blocks[1], b'@@B@@', 1)
        sw = sw.replace(b'@@A@@', blocks[1], 1).replace(b'@@B@@', blocks[0], 1)
        variant(sw, "two individuals swapped", True)
    else:
        fails.append("fewer than 2 individuals -- selfcheck cannot run")

    for f in fails:
        print("  FAIL " + f)
    print("expstruct selfcheck  %d pass  %d fail" % (3 - len(fails), len(fails)))
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
