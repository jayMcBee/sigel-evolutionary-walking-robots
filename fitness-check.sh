#!/bin/sh
# End-to-end gate for Phase D -- PORTING.md.
#
#   ./fitness-check.sh | diff -u fitness-baseline.txt -
#
# Three individuals of each of the 14 experiments. This is the check that the
# shim removal did not move a simulated number: dictorder-dump.sh proves the
# ordering is preserved, this proves the physics that comes out of it is.
#
# NOT a cross-machine reference. Fitness is chaotic -- §7 measures a 1-ULP
# change in start height moving an individual by 45% -- and the 2003 build was
# i386 x87. These values pin this machine against itself, nothing more.
set -eu
ROOT=$(cd "$(dirname "$0")" && pwd)
B=${1:-build-fast}
DATA=${2:-data-reordered}
[ -x "$ROOT/$B/sigel_eval" ] || { echo "no $ROOT/$B/sigel_eval" >&2; exit 1; }
SIGEL_ROOT=$ROOT/x/kdesigelSources.1.3/kdesigel/kdesigel
export SIGEL_ROOT
n=$(find "$ROOT/$DATA/Experiments" -name '*.exp' | wc -l)
[ "$n" -eq 14 ] || { echo "expected 14 .exp under $DATA/, found $n" >&2; exit 1; }
for f in $(find "$ROOT/$DATA/Experiments" -name '*.exp' | sort); do
	for i in 0 1 2; do
		v=$("$ROOT/$B/sigel_eval" "$f" "$i" 2>/dev/null | tail -1 | awk '{print $3}')
		[ -n "$v" ] || { echo "$(basename "$f") $i produced no fitness" >&2; exit 1; }
		printf '%-34s %d  %s\n' "$(basename "$f" .exp)" "$i" "$v"
	done
done
