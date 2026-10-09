#!/bin/sh
# Fitness check.
#
#   ./checks/fitness-check.sh | diff -u checks/baselines/fitness-baseline.txt -
#   ASAN_OPTIONS=detect_leaks=0 ./checks/fitness-check.sh build-asan \
#       | diff -u checks/baselines/fitness-baseline.txt -
#
# Evaluates three individuals of each of the 7 experiments and prints their
# fitness. It is the only check that compares simulated numbers. Before the
# evaluations it runs the MetaGP mating check, and on a sanitized build the
# unit tests.
#
# The baseline holds for this machine only. Fitness is chaotic: a change of
# one unit in the last place of a start height moved one individual by 45 %.
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
[ -x "$ROOT/$B/coredrive" ] || { echo "no $ROOT/$B/coredrive" >&2; exit 1; }

# A failed build leaves the previous coredrive in place. Without this test
# the check would pass on a program from before the change.
make -q --no-print-directory -C "$ROOT" B="$B" coredrive >/dev/null 2>&1 || {
	echo ""$ROOT/$B/coredrive" is out of date -- run 'make B=$B coredrive'" >&2; exit 1; }

# SIGEL rewrites $SIGEL_ROOT/Terrain.ter on every evaluation, so run it in the
# folder SIGEL is started from, never in the tracked source.
SIGEL_ROOT=$ROOT/sigelApp
[ -f "$SIGEL_ROOT/Terrain.ter" ] || { echo "no $SIGEL_ROOT/Terrain.ter -- run 'make'" >&2; exit 1; }
export SIGEL_ROOT
# The mating code of MetaGP. Its rules hold for every seed, so it has no
# baseline; it passes or fails.
"$ROOT/$B/coredrive" -metamating >&2 || exit 1
# The mating code again and the unit tests, with LeakSanitizer on: every
# object they make must be freed. The evaluations below cannot run this way,
# because a simulation does not free all it allocates. A build without a
# sanitizer ignores ASAN_OPTIONS, so test the program and say when the leak
# tests are skipped.
if nm -C "$ROOT/$B/coredrive" 2>/dev/null | grep -q __asan_init; then
	leaks=$(ASAN_OPTIONS=detect_leaks=1 make -s --no-print-directory -C "$ROOT" B="$B" test 2>&1) || {
		echo "make test FAILED under the sanitizers -- the build, a test or a leak:" >&2
		echo "$leaks" >&2
		exit 1
	}
	# UndefinedBehaviorSanitizer reports and goes on, so the run still passes.
	if printf '%s\n' "$leaks" | grep -q 'runtime error:'; then
		echo "the unit tests have a sanitizer report:" >&2
		echo "$leaks" >&2
		exit 1
	fi
	leaks=$(ASAN_OPTIONS=detect_leaks=1 "$ROOT/$B/coredrive" -metamating 2>&1 >/dev/null) || {
		echo "metamating LEAKED under LeakSanitizer:" >&2
		echo "$leaks" >&2
		exit 1
	}
else
	echo "note: $B has no sanitizer, so the leak test of the mating code and the" >&2
	echo "      sanitized run of the unit tests were SKIPPED." >&2
	echo "      run 'ASAN_OPTIONS=detect_leaks=0 ./checks/fitness-check.sh build-asan' for it." >&2
fi

# Tracked files only: an untracked file there must not change what this reads.
tracked() { git -C "$ROOT" ls-files -- "$1" | sed "s|^|$ROOT/|" | sort; }
n=$(tracked 'experiments/*.exp' | wc -l)
[ "$n" -eq 7 ] || { echo "expected 7 .exp under experiments/, found $n" >&2; exit 1; }
# Capture the output, test the exit status, then filter. A pipe into tail
# would report tail's status and hide a crash of coredrive. stderr is kept
# and searched, because a sanitizer reports there.
out=$(mktemp); err=$(mktemp)
for f in $(tracked 'experiments/*.exp'); do
	for i in 0 1 2; do
		rc=0; "$ROOT/$B/coredrive" "$f" "$i" >"$out" 2>"$err" || rc=$?
		if [ "$rc" -ne 0 ]; then
			echo "$(basename "$f") $i: coredrive exited $rc" >&2
			cat "$err" >&2; rm -f "$out" "$err"; exit 1
		fi
		if grep -qE 'AddressSanitizer|LeakSanitizer|runtime error:' "$err"; then
			echo "$(basename "$f") $i: sanitizer report" >&2
			cat "$err" >&2; rm -f "$out" "$err"; exit 1
		fi
		v=$(tail -1 "$out" | awk '{print $3}')
		[ -n "$v" ] || { echo "$(basename "$f") $i produced no fitness" >&2
		                 cat "$err" >&2; rm -f "$out" "$err"; exit 1; }
		printf '%-34s %d  %s\n' "$(basename "$f" .exp)" "$i" "$v"
	done
done
rm -f "$out" "$err"
