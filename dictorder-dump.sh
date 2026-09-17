#!/bin/sh
# Every Q2Dict iteration order that reaches the simulation -- PORTING.md Phase D.
#
# Q2Dict's hash order numbers the links, joints, bodies, materials, drives,
# sensors and each link's significant points. DynaMechs numbers its bodies from
# that. Phase D deletes the shim, so the order has to move into the data files.
# This prints the order the current build produces; dictorder-baseline.txt is that
# output, committed. The check is a diff:
#
#   ./dictorder-dump.sh | diff -u dictorder-baseline.txt -
#
# Watches all three load paths, because they give three different orders:
#   loaded   the robot as the .exp deserialises it
#   copy     after SIG_Robot's copy ctor, which round-trips through
#            writeToFileTransfer and so reverses every colliding chain
#   rrb      the standalone model, read in declaration order by SIGEL_RobotIO
#
# Serial on purpose. SIG_Environment::generateTerrain rewrites
# $SIGEL_ROOT/Terrain.ter on every evaluation; parallel workers sharing a root
# used to read it half-written, and the write is now atomic. Serial stays,
# because the dump is compared line by line and a worker pool would reorder it.
set -eu
ROOT=$(cd "$(dirname "$0")" && pwd)
B=${1:-build-fast}
DATA=${2:-data-reordered}   # data/ is the pristine download, never written
EVAL=$ROOT/$B/sigel_eval
[ -x "$EVAL" ] || { echo "no $EVAL -- make B=$B SAN= SIGSAN=" >&2; exit 1; }

# A failed `make` stops at the first bad compile and leaves the PREVIOUS
# sigel_eval in place, so a test-for-existence passes and the gate silently
# scores a stale binary. That happened during D13: the build failed on two
# sites, the gates were run straight after, and both came back green against
# the binary from before the change. Gate results mean nothing unless the
# build that produced them succeeded. pvm-check.sh has carried this guard
# from the start; these two did not.
make -q B="$B" 2>/dev/null || {
	echo ""$EVAL" is out of date -- run 'make B=$B SAN= SIGSAN='" >&2; exit 1; }

SIGEL_ROOT=$ROOT/x/kdesigelSources.1.3/kdesigel/kdesigel
export SIGEL_ROOT

# A silently short dump is the dangerous failure: Phase D re-captures this file
# at every step, so an empty run that exited 0 would overwrite the baseline and
# report success. Count what we expect to find and refuse to run if it is off.
exps=$(find "$ROOT/$DATA/Experiments" -name '*.exp' | sort)
rrbs=$(find "$ROOT/$DATA" -name '*.rrb' | sort)
ne=$(echo "$exps" | grep -c . || true); nr=$(echo "$rrbs" | grep -c . || true)
[ "$ne" -eq 14 ] && [ "$nr" -eq 7 ] || {
	echo "expected 14 .exp and 7 .rrb under $DATA/, found $ne and $nr." >&2
	echo "PORTING.md §7: experiments.tar.gz carries only 12 -- the two runner" >&2
	echo ".exp come from data/results/runner*Experiment.tar.gz." >&2
	exit 1
}

for f in $exps $rrbs; do
	echo "== $(basename "$f")"
	# Individual 0 only: the dump happens before experiment.population is
	# touched, so every individual gives the same order.
	# stderr is kept and inspected rather than discarded: the simulation
	# prints routine diagnostics there ("attempt to read invalid sensor"),
	# but a sanitizer report would land there too, and an earlier version of
	# this script sent all of it to /dev/null.
	# Do NOT pipe sigel_eval straight into sed: the pipeline's status is
	# sed's, so a segfaulting sigel_eval gave exit 0 and a header-only file.
	# An earlier version of this script had exactly that hole while claiming
	# to have closed it. Capture, test the status, then filter.
	out=$(mktemp); err=$(mktemp)
	rc=0; "$EVAL" -v "$f" 0 >"$out" 2>"$err" || rc=$?
	if [ "$rc" -ne 0 ]; then
		echo "$(basename "$f"): sigel_eval exited $rc" >&2
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
