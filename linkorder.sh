#!/bin/sh
# Link and joint order for every experiment -- PORTING.md Phase D.
#
# Q2Dict's hash order is what numbers the links and joints, and DynaMechs
# numbers its bodies from that. Phase D deletes the shim, so the order has to
# move into the data files instead. This prints the order the current build
# produces; linkorder.txt is that output, committed. The check is a diff:
#
#   ./linkorder.sh | diff -u linkorder.txt -
#
# Serial on purpose: SIG_Environment::generateTerrain rewrites
# $SIGEL_ROOT/Terrain.ter on every evaluation, so parallel workers sharing a
# root read it half-written (PORTING.md §7).
set -e
ROOT=$(cd "$(dirname "$0")" && pwd)
B=${1:-build-fast}
SIGEL_ROOT=$ROOT/x/kdesigelSources.1.3/kdesigel/kdesigel
export SIGEL_ROOT
for exp in "$ROOT"/data/Experiments/*.exp; do
	echo "== $(basename "$exp")"
	# Individual 0 only: the robot is a property of the experiment, not of the
	# individual, so every individual gives the same order.
	"$ROOT/$B/sigel_eval" -v "$exp" 0 2>/dev/null |
		sed -n 's/^  \(link\|joint\) /\1 /p'
done
