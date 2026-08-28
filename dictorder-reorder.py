#!/usr/bin/env python3
"""Rewrite robot data so declaration order IS the order the simulation uses.

PORTING.md Phase D. Q2Dict's hash order currently numbers the links, joints,
sensors and drives (SIG_DynaMechsSimulationData.cpp). Deleting the shim makes
the containers insertion-ordered, so that numbering has to come from the file
instead. This permutes each file's declarations into the order the shim produces
today, taken from dictorder-baseline.txt.

Reads data/, writes data-reordered/. Never writes data/.

Bodies and materials are deliberately left alone: nothing numbers them
(loadGeometries is order-free, materials are looked up by name), and for octopus
the target body order is not even reachable by permuting links.

Every rewrite is checked to be a pure permutation -- same multiset of lines in
and out. A reorderer that quietly drops a line is worse than one that fails.
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
        m = re.match(r"(\w+)\s+(\w+)\s+\d+\s+(\S+)$", line)
        if m:
            tag, kind, name = m.groups()
            cur.setdefault(tag, {}).setdefault(kind, []).append(name)
    return out


def permutation_ok(before, after):
    return collections.Counter(before) == collections.Counter(after)


def reorder(seq, key_of, target):
    """Order seq by target; anything not named in target keeps its place."""
    rank = {n: i for i, n in enumerate(target)}
    known = sorted((x for x in seq if key_of(x) in rank), key=lambda x: rank[key_of(x)])
    it = iter(known)
    return [next(it) if key_of(x) in rank else x for x in seq]


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
        if kind not in target:
            continue
        idx = [i for i, b in enumerate(blocks) if b[0] == kind]
        chosen = reorder([blocks[i] for i in idx], lambda b: b[1], target[kind])
        for i, b in zip(idx, chosen):
            blocks[i] = b
    # point lines inside each link block
    out = []
    for kind, name, lines in blocks:
        key = ("point", name)
        if kind == "link" and key in target:
            pidx = [i for i, l in enumerate(lines) if l.lstrip().startswith("point ")]
            pts = [lines[i] for i in pidx]
            got = reorder(pts, lambda l: l.split("point ", 1)[1].split("=")[0].strip(),
                          target[key])
            for i, l in zip(pidx, got):
                lines[i] = l
        out.append(lines)
    return "".join(l for lines in out for l in lines)


# --- .exp ------------------------------------------------------------------
# The robot lives between "StreamedRobot" and "RobotComplete" as one entity per
# line, except joints and sensors, which take two. Only one joint type appears
# in the 14 files: RotationalJoint. Census over all 14: Link 87, RotationalJoint
# 73 (+73 continuation), Drive 73, JointSensor 66 (+66), Material 31, Body 31,
# Geometry 31, Polygon 488.
#
# Stored numbers are read straight back by SIG_Link's stream constructor
# (SIG_Link.cpp:65 "tx >> name >> number"), so permuting lines leaves every
# number exactly as 2001 wrote it. Only the order changes, which is the thing
# SIG_DynaMechsSimulationData turns into the DynaMechs body index.
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
        return text                      # no robot block, nothing to do

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
        if kind not in target:
            continue
        idx = [n for n, u in enumerate(units) if u[0] == kind]
        chosen = reorder([units[n] for n in idx], lambda u: u[1], target[kind])
        for n, u in zip(idx, chosen):
            units[n] = u

    return "".join(lines[:lo + 1]) + "".join(l for _, _, b in units for l in b) \
         + "".join(lines[hi:])


def main():
    base = read_baseline(ROOT / "dictorder-baseline.txt")
    if not DST.exists():
        sys.exit(f"{DST} does not exist -- cp -a data data-reordered first")
    changed = failed = 0
    for src in sorted(SRC.rglob("*.rrb")):
        target = base.get(src.name, {}).get("rrb")
        if target is None:
            sys.exit(f"no baseline entry for {src.name}")
        text = src.read_text()
        new = rewrite_rrb(text, target)
        if not permutation_ok(text.splitlines(), new.splitlines()):
            print(f"NOT A PERMUTATION: {src.name}", file=sys.stderr); failed += 1; continue
        dst = DST / src.relative_to(SRC)
        dst.write_text(new)
        changed += text != new
        print(f"  {'reordered' if text != new else 'unchanged'}  {src.name}")
    for src in sorted(SRC.glob("Experiments/*.exp")):
        target = base.get(src.name, {}).get("copy")
        if target is None:
            sys.exit(f"no baseline entry for {src.name}")
        text = src.read_text(errors="surrogateescape")
        new = rewrite_exp(text, target)
        if not permutation_ok(text.splitlines(), new.splitlines()):
            print(f"NOT A PERMUTATION: {src.name}", file=sys.stderr); failed += 1; continue
        dst = DST / src.relative_to(SRC)
        dst.write_text(new, errors="surrogateescape")
        changed += text != new
        print(f"  {'reordered' if text != new else 'unchanged'}  {src.name}")

    print(f"{changed} rewritten, {failed} failed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
