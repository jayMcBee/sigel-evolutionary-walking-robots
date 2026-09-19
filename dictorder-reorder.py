#!/usr/bin/env python3
"""Rewrite robot data so declaration order IS the order the simulation uses.

PORTING.md Phase D. The Qt 2 build iterated links, joints, sensors and drives in
Q2Dict's hash order. The Qt 6 containers keep insertion order, so that order has
to come from the file. This permutes each file's declarations into the order
dictorder-baseline.txt records: its `rrb` lines for a .rrb, its `copy` lines for
a .exp.

Reads data/, writes data-reordered/. Never writes data/.

Bodies and materials are not permuted: nothing numbers them (loadGeometries is
order-free, materials are looked up by name), and body order follows link order.

Each target must name exactly the entities the file declares, and each rewrite
must be a pure permutation of the file's lines. One file failing either stops
the run before anything is written.
"""
import collections, pathlib, re, sys

ROOT = pathlib.Path(__file__).resolve().parent
SRC, DST = ROOT / "data", ROOT / "data-reordered"
NUMBERED = ("link", "joint", "sensor", "drive")


def read_baseline(path):
    """{filename: {tag: {kind: [names]}}}, plus ('point', link) keys."""
    out, cur = {}, None
    for line in path.read_text().splitlines():
        if line.startswith("== "):
            cur = out.setdefault(line[3:], {})
            continue
        m = re.match(r"(\w+)\s+point (\S+)\s+\d+\s+(\S+)$", line)
        if m:
            tag, link, name = m.groups()
            cur.setdefault(tag, {}).setdefault(("point", link), []).append(name)
            continue
        m = re.match(r"(\w+)\s+(\w+)\s+\d+\s+#-?\d+\s+(\S+)$", line)
        if m:
            tag, kind, name = m.groups()
            cur.setdefault(tag, {}).setdefault(kind, []).append(name)
    return out


def permutation_ok(before, after):
    return collections.Counter(before) == collections.Counter(after)


def reorder(seq, key_of, target):
    """Order seq by target. Both must name the same entities."""
    names = [key_of(x) for x in seq]
    if sorted(names) != sorted(target) or len(set(target)) != len(target):
        raise ValueError(f"file declares {names}, baseline orders {target}")
    return sorted(seq, key=lambda x: target.index(key_of(x)))


# --- .rrb ------------------------------------------------------------------
# material X { }, link X { }, joint <subtype> X { }, drive X { }, sensor X { }.
# Verified across all 7 files: no other top-level construct, no stray lines.
RRB_HEAD = re.compile(r"^(material|link|joint|drive|sensor)\s+(?:\w+\s+)?(\S+)\s*\{")


def split_rrb(text):
    """[(kind, name, lines)] blocks, with gaps kept as (None, None, lines)."""
    out, buf, lines = [], [], text.splitlines(keepends=True)
    i = 0
    while i < len(lines):
        m = RRB_HEAD.match(lines[i])
        if not m:
            buf.append(lines[i]); i += 1; continue
        if buf:
            out.append((None, None, buf)); buf = []
        blk = [lines[i]]
        while not blk[-1].startswith("}"):
            i += 1
            blk.append(lines[i])
        out.append((m.group(1), m.group(2), blk))
        i += 1
    if buf:
        out.append((None, None, buf))
    return out


def rewrite_rrb(text, target):
    blocks = split_rrb(text)
    for kind in NUMBERED:
        idx = [i for i, b in enumerate(blocks) if b[0] == kind]
        chosen = reorder([blocks[i] for i in idx], lambda b: b[1], target.get(kind, []))
        for i, b in zip(idx, chosen):
            blocks[i] = b
    # point lines inside each link block
    out = []
    for kind, name, lines in blocks:
        if kind == "link":
            pidx = [i for i, l in enumerate(lines) if l.lstrip().startswith("point ")]
            pts = [lines[i] for i in pidx]
            got = reorder(pts, lambda l: l.split("point ", 1)[1].split("=")[0].strip(),
                          target.get(("point", name), []))
            for i, l in zip(pidx, got):
                lines[i] = l
        out.append(lines)
    return "".join(l for lines in out for l in lines)


# --- .exp ------------------------------------------------------------------
# The robot lives between "StreamedRobot" and "RobotComplete" as one entity per
# line, except joints and sensors, which take two. Only one joint type appears
# in the 7 files: RotationalJoint. Census over all 7: Link 60, RotationalJoint
# 53 (+53 continuation), Drive 53, JointSensor 46 (+46), Material 20, Body 20,
# Geometry 20, Polygon 341.
#
# Stored numbers are read straight back by SIG_Link's stream constructor
# (SIG_Link.cpp:65 "tx >> name >> number"), so permuting lines leaves every
# number exactly as 2001 wrote it. Only the order changes: the order the next
# reader of the robot's stream sees, and so each link's joint order there.
EXP_UNITS = {          # leading token -> (name field, extra continuation lines)
    "Link": (1, 0),
    "RotationalJoint": (2, 1),
    "Drive": (2, 0),
    "JointSensor": (2, 1),
}
EXP_KIND = {"Link": "link", "RotationalJoint": "joint",
            "Drive": "drive", "JointSensor": "sensor"}


def rewrite_exp(text, target):
    lines = text.splitlines(keepends=True)
    try:
        lo = lines.index("StreamedRobot\n")
        hi = lines.index("RobotComplete\n", lo)
    except ValueError:
        raise ValueError("no line that is exactly StreamedRobot or RobotComplete")

    units, i = [], lo + 1                # [(kind, name, [lines])], gaps as (None,..)
    while i < hi:
        tok = lines[i].split()[:1]
        spec = EXP_UNITS.get(tok[0]) if tok else None
        if spec is None:
            units.append((None, None, [lines[i]])); i += 1; continue
        field, extra = spec
        blk = lines[i:i + 1 + extra]
        units.append((EXP_KIND[tok[0]], lines[i].split()[field], blk))
        i += 1 + extra

    for kind in NUMBERED:
        idx = [n for n, u in enumerate(units) if u[0] == kind]
        chosen = reorder([units[n] for n in idx], lambda u: u[1], target.get(kind, []))
        for n, u in zip(idx, chosen):
            units[n] = u

    return "".join(lines[:lo + 1]) + "".join(l for _, _, b in units for l in b) \
         + "".join(lines[hi:])


def main():
    base = read_baseline(ROOT / "dictorder-baseline.txt")
    if not DST.exists():
        sys.exit(f"{DST} does not exist -- cp -a data data-reordered first")
    jobs = [(p, "rrb", rewrite_rrb) for p in sorted(SRC.rglob("*.rrb"))] \
         + [(p, "copy", rewrite_exp) for p in sorted(SRC.glob("Experiments/*.exp"))]
    names = sorted(p.name for p, _, _ in jobs)
    if names != sorted(base):
        sys.exit(f"data/ holds {names}, the baseline names {sorted(base)} -- nothing written")
    done = []
    for src, tag, rewrite in jobs:
        target = base.get(src.name, {}).get(tag)
        if target is None:
            sys.exit(f"no baseline entry for {src.name} -- nothing written")
        text = src.read_bytes().decode(errors="surrogateescape")    # keeps CRLF
        try:
            new = rewrite(text, target)
        except ValueError as e:
            sys.exit(f"{src.name}: {e} -- nothing written")
        if not permutation_ok(text.splitlines(), new.splitlines()):
            sys.exit(f"NOT A PERMUTATION: {src.name} -- nothing written")
        if not (DST / src.relative_to(SRC)).parent.is_dir():
            sys.exit(f"no directory for {src.relative_to(SRC)} in {DST} -- nothing written")
        done.append((src, text, new))
    for src, text, new in done:
        (DST / src.relative_to(SRC)).write_bytes(new.encode(errors="surrogateescape"))
        print(f"  {'reordered' if text != new else 'unchanged'}  {src.name}")
    print(f"{sum(text != new for _, text, new in done)} rewritten")
    return 0


if __name__ == "__main__":
    sys.exit(main())
