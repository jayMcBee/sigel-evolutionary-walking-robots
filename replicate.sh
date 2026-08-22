#!/bin/sh
# Replicate the 14 published SIGEL experiments and compare against 2003.
#
#   ./replicate.sh              build/sigel_eval, under ASan and UBSan
#   ./replicate.sh build-fast   an unsanitised build, ~15x faster
#
# Each .exp stores every individual's program next to the fitness it scored in
# 2003, so the comparison is best-of-population against best-of-population.
# Individual fitness fields are partly stale -- five individuals in
# twoBasesHighMutationRate share FITNESS=1.02726 with five different programs --
# so never compare a single individual.
#
# Fitness is metres per second. The project's endbericht gives two independent
# figures to check against: 0.9 m/s average for twoBasesHighMutationRate (§5.2)
# and 0.26 m/s for walkerNiceWalkingFitness (§5.10).
set -e
ROOT=$(cd "$(dirname "$0")" && pwd)
BUILD=${1:-build}
EVAL=$ROOT/$BUILD/sigel_eval
[ -x "$EVAL" ] || { echo "no $EVAL -- run: make${1:+ B=$1 SAN= SIGSAN=}"; exit 1; }
[ -d "$ROOT/data/Experiments" ] || { echo "no data/ -- see PORTING.md §9"; exit 1; }

SIGEL_ROOT=$ROOT/x/kdesigelSources.1.3/kdesigel/kdesigel
export SIGEL_ROOT
ASAN_OPTIONS=detect_leaks=0
export ASAN_OPTIONS

python3 - "$EVAL" "$ROOT" <<'PY'
import concurrent.futures as cf, glob, os, re, subprocess, sys
ev, root = sys.argv[1], sys.argv[2]

def one(args):
    exp, i = args
    out = subprocess.run([ev, exp, str(i)], capture_output=True, text=True).stdout
    m = re.search(r'this run\s+([-0-9.e+]+)', out)
    return float(m.group(1)) if m else 0.0

rows, wide = [], 0
for exp in sorted(glob.glob(os.path.join(root, 'data/Experiments/*.exp'))):
    name = os.path.basename(exp)[:-4]
    text = open(exp, encoding='latin-1').read()
    rec = [float(x) for x in re.findall(r'FITNESS=([-0-9.e+]+);', text)]
    if not rec:
        continue
    with cf.ThreadPoolExecutor(max_workers=os.cpu_count()) as pool:
        got = list(pool.map(one, [(exp, i) for i in range(len(rec))]))
    r, g = max(rec), max(got)
    ratio = g / r if r else float('nan')
    rows.append((name, len(rec), r, g, ratio))
    if not (0.9 <= ratio <= 1.1):
        wide += 1

print(f"{'experiment':<34}{'n':>5}{'2003 best':>12}{'ours':>12}{'ratio':>8}")
for name, n, r, g, ratio in rows:
    flag = '' if 0.9 <= ratio <= 1.1 else '   <<<'
    print(f"{name:<34}{n:>5}{r:>12.5g}{g:>12.5g}{ratio:>8.3f}{flag}")
print(f"\n{len(rows) - wide} of {len(rows)} within 10% of the 2003 result")
sys.exit(1 if wide else 0)
PY
