#!/bin/sh
# Replicate the published SIGEL experiments and compare against the 2001 record.
#
#   ./replicate.sh              build/sigel_eval, under ASan and UBSan
#   ./replicate.sh build-fast   an unsanitised build, ~15x faster
#
# THE ORACLE IS SIGEL 1.0, NOT THE 1.3 SOURCE BEING PORTED. The published
# .exp files are dated August-September 2001; 1.0 was rolled 2001-09-06 and
# 1.3's ChangeLog starts 2001-12-18. Every 1.0 -> 1.3 change is a confound in
# this comparison -- see PORTING.md §7.
#
# Byte-identical files are detected and reported once.
#
# EACH WORKER GETS ITS OWN SIGEL_ROOT. SIG_Environment::generateTerrain rewrites
# $SIGEL_ROOT/Terrain.ter on every single evaluation and reads it straight back.
# That write is atomic, so sharing one root no longer corrupts a reader; a root
# each still keeps every worker independent of the others.
#
# A crashed evaluation is an ERROR, never a fitness of 0. Scoring crashes as 0
# made an earlier version of this script load-dependent.
#
# Two numbers are reported per experiment. `best` compares best-of-population,
# which is a loose test: the more individuals share the recorded best value, the
# more chances max() has to hit it. `match` counts individuals whose own value
# reproduces to 0.1%, which is far sharper -- but the stored per-individual
# FITNESS fields are partly inherited from parents rather than measured, so a
# low match count is not by itself a defect. Read them together.
set -e
ROOT=$(cd "$(dirname "$0")" && pwd)
BUILD=${1:-build}
case $BUILD in /*) EVAL=$BUILD/sigel_eval ;; *) EVAL=$ROOT/$BUILD/sigel_eval ;; esac
[ -x "$EVAL" ] || { echo "no $EVAL -- run: make${1:+ B=$1 SAN= SIGSAN=}"; exit 1; }
[ -d "$ROOT/data/Experiments" ] || { echo "no data/ -- see PORTING.md §9"; exit 1; }

ASAN_OPTIONS=detect_leaks=0
export ASAN_OPTIONS

python3 - "$EVAL" "$ROOT" <<'PY'
import concurrent.futures as cf, glob, hashlib, os, queue, re, shutil, subprocess, sys, tempfile
ev, root = sys.argv[1], sys.argv[2]
src = os.path.join(root, 'x/kdesigelSources.1.3/kdesigel/kdesigel')

# One SIGEL_ROOT per worker, each with its own Terrain.ter to rewrite.
workers = min(os.cpu_count() or 1, 4)
tmp = tempfile.mkdtemp(prefix='sigel-replicate-')
roots = queue.Queue()
for i in range(workers):
    d = os.path.join(tmp, str(i))
    os.makedirs(d)
    shutil.copy(os.path.join(src, 'Terrain.ter'), d)
    roots.put(d)

class Crashed(Exception):
    pass

def one(args):
    exp, i = args
    d = roots.get()
    try:
        env = dict(os.environ, SIGEL_ROOT=d)
        p = subprocess.run([ev, exp, str(i)], capture_output=True, text=True, env=env)
    finally:
        roots.put(d)
    m = re.search(r'this run\s+([-0-9.e+]+)', p.stdout)
    if p.returncode != 0 or not m:
        raise Crashed(f"{os.path.basename(exp)} individual {i}: exit {p.returncode}\n"
                      f"{p.stdout[-400:]}{p.stderr[-2000:]}")
    return float(m.group(1))

seen, rows, wide = {}, [], 0
for exp in sorted(glob.glob(os.path.join(root, 'data/Experiments/*.exp'))):
    name = os.path.basename(exp)[:-4]
    text = open(exp, encoding='latin-1').read()
    digest = hashlib.md5(text.encode('latin-1')).hexdigest()
    if digest in seen:
        print(f"{name:<34}   byte-identical to {seen[digest]}, skipped")
        continue
    seen[digest] = name
    rec = [float(x) for x in re.findall(r'FITNESS=([-0-9.e+]+);', text)]
    if not rec:
        continue
    with cf.ThreadPoolExecutor(max_workers=workers) as pool:
        got = list(pool.map(one, [(exp, i) for i in range(len(rec))]))
    hit = sum(1 for r, g in zip(rec, got)
              if abs(g - r) <= 1e-3 * abs(r) or (r == 0 and g == 0))
    r, g = max(rec), max(got)
    ratio = g / r if r else float('nan')
    rows.append((name, len(rec), r, g, ratio, hit))
    if not (0.9 <= ratio <= 1.1):
        wide += 1
shutil.rmtree(tmp, ignore_errors=True)

print(f"\n{'experiment':<34}{'n':>5}{'2001 best':>12}{'ours':>12}{'best':>8}{'match':>8}")
for name, n, r, g, ratio, hit in rows:
    flag = '' if 0.9 <= ratio <= 1.1 else '   <<<'
    print(f"{name:<34}{n:>5}{r:>12.5g}{g:>12.5g}{ratio:>8.3f}{hit:>6}/{n:<3}{flag}")
print(f"\n{len(rows) - wide} of {len(rows)} distinct experiments within 10% on best-of-population")
sys.exit(1 if wide else 0)
PY
