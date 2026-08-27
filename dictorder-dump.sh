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
# Serial on purpose: SIG_Environment::generateTerrain rewrites
# $SIGEL_ROOT/Terrain.ter on every evaluation, so parallel workers sharing a
# root read it half-written (PORTING.md §7).
set -eu
ROOT=$(cd "$(dirname "$0")" && pwd)
B=${1:-build-fast}
EVAL=$ROOT/$B/sigel_eval
[ -x "$EVAL" ] || { echo "no $EVAL -- make B=$B SAN= SIGSAN=" >&2; exit 1; }
SIGEL_ROOT=$ROOT/x/kdesigelSources.1.3/kdesigel/kdesigel
export SIGEL_ROOT

# A silently short dump is the dangerous failure: Phase D re-captures this file
# at every step, so an empty run that exited 0 would overwrite the baseline and
# report success. Count what we expect to find and refuse to run if it is off.
exps=$(find "$ROOT/data/Experiments" -name '*.exp' | sort)
rrbs=$(find "$ROOT/data" -name '*.rrb' | sort)
ne=$(echo "$exps" | grep -c . || true); nr=$(echo "$rrbs" | grep -c . || true)
[ "$ne" -eq 14 ] && [ "$nr" -eq 7 ] || {
	echo "expected 14 .exp and 7 .rrb under data/, found $ne and $nr." >&2
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
	err=$(mktemp)
	if ! "$EVAL" -v "$f" 0 2>"$err" |
	     sed -n 's/^  \(loaded\|copy\|rrb\) /\1 /p'; then
		echo "$(basename "$f"): sigel_eval failed" >&2
		cat "$err" >&2; rm -f "$err"; exit 1
	fi
	if grep -qE 'AddressSanitizer|LeakSanitizer|runtime error:' "$err"; then
		echo "$(basename "$f"): sanitizer report" >&2
		cat "$err" >&2; rm -f "$err"; exit 1
	fi
	rm -f "$err"
done
