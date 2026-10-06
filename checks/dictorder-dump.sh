#!/bin/sh
# Order check.
#
#   ./checks/dictorder-dump.sh | diff -u checks/baselines/dictorder-baseline.txt -
#
# Prints the parts of each robot in the order of SIG_Robot's lists, with the
# number each part carries: links, joints, drives, sensors, bodies, materials,
# commands, the points of each link, and a digest of each geometry.
# SIG_Robot::writeToFileTransfer writes a robot in that order, to an experiment
# file and to a PVM slave. A change of container in SIG_Robot shows here.
#
# Three passes:
#   loaded   the robot as an experiment file gives it
#   copy     the copy that SIG_Robot's copy constructor makes, on which the
#            simulation runs
#   rrb      the robot as SIGEL_RobotIO builds it from a robot file
#
# It evaluates nothing. It is the only check that builds all 7 robot files.
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
# Every path below is built from ROOT, so it must be the repository.
[ -f "$ROOT/Makefile" ] && [ -d "$ROOT/checks" ] || {
	echo "$0: $ROOT is not the repo root -- run the script by its real path,"\
	     "not through a symlink or a copy" >&2; exit 1; }

# Run from the repository root, whatever the caller's folder is.
cd "$ROOT" || exit 1
B=${1:-build}
[ $# -le 1 ] || { echo "usage: $0 [build-dir] -- it reads experiments/ and robots/" >&2; exit 1; }
EVAL=$ROOT/$B/coredrive
[ -x "$EVAL" ] || { echo "no $EVAL -- make B=$B coredrive" >&2; exit 1; }

# A failed build leaves the previous coredrive in place. Without this test
# the check would pass on a program from before the change.
make -q --no-print-directory -C "$ROOT" B="$B" coredrive >/dev/null 2>&1 || {
	echo ""$EVAL" is out of date -- run 'make B=$B coredrive'" >&2; exit 1; }

# SIGEL rewrites $SIGEL_ROOT/Terrain.ter on every evaluation, so run it in the
# folder SIGEL is started from, never in the tracked source.
SIGEL_ROOT=$ROOT/sigelApp
[ -f "$SIGEL_ROOT/Terrain.ter" ] || { echo "no $SIGEL_ROOT/Terrain.ter -- run 'make'" >&2; exit 1; }
export SIGEL_ROOT

# A short dump must not pass: count the input files and refuse to run if the
# count is wrong.
# Tracked files only: an untracked file there must not change what this reads.
tracked() { git -C "$ROOT" ls-files -- "$1" | sed "s|^|$ROOT/|" | sort; }
exps=$(tracked 'experiments/*.exp')
rrbs=$(tracked 'robots/*.rrb')
ne=$(echo "$exps" | grep -c . || true); nr=$(echo "$rrbs" | grep -c . || true)
[ "$ne" -eq 7 ] && [ "$nr" -eq 7 ] || {
	echo "expected 7 .exp under experiments/ and 7 .rrb under robots/," >&2
	echo "found $ne and $nr." >&2
	exit 1
}

for f in $exps $rrbs; do
	echo "== $(basename "$f")"
	# Capture the output, test the exit status, then filter. A pipe into sed
	# would report sed's status and hide a crash of coredrive. stderr is kept
	# and searched, because a sanitizer reports there.
	out=$(mktemp); err=$(mktemp)
	rc=0; "$EVAL" -order "$f" >"$out" 2>"$err" || rc=$?
	if [ "$rc" -ne 0 ]; then
		echo "$(basename "$f"): coredrive exited $rc" >&2
		cat "$err" >&2; rm -f "$out" "$err"; exit 1
	fi
	if grep -qE 'AddressSanitizer|LeakSanitizer|runtime error:' "$err"; then
		echo "$(basename "$f"): sanitizer report" >&2
		cat "$err" >&2; rm -f "$out" "$err"; exit 1
	fi
	# A run that produced no order lines at all is a failure, not an empty diff.
	if ! sed -n 's/^  \(loaded\|copy\|rrb\) /\1 /p' "$out" | grep -q .; then
		echo "$(basename "$f"): no order lines -- did it load?" >&2
		cat "$err" >&2; rm -f "$out" "$err"; exit 1
	fi
	sed -n 's/^  \(loaded\|copy\|rrb\) /\1 /p' "$out"
	rm -f "$out" "$err"
done
