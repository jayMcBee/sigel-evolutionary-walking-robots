#!/bin/sh
# End-to-end gate for Phase D -- PORTING.md.
#
#   ./fitness-check.sh | diff -u fitness-baseline.txt -
#
# Three individuals of each of the 7 experiments. This is the check that the
# shim removal did not move a simulated number: dictorder-dump.sh proves the
# ordering is preserved, this proves the physics that comes out of it is.
#
# NOT a cross-machine reference. Fitness is chaotic -- §7 measures a 1-ULP
# change in start height moving an individual by 45% -- and the 2003 build was
# i386 x87. These values pin this machine against itself, nothing more.
set -eu
ROOT=$(cd "$(dirname "$0")" && pwd)
B=${1:-build-fast}
[ $# -le 1 ] || { echo "usage: $0 [build-dir] -- it reads experiments/ and robots/" >&2; exit 1; }
[ -x "$ROOT/$B/sigel_eval" ] || { echo "no $ROOT/$B/sigel_eval" >&2; exit 1; }

# A failed `make` stops at the first bad compile and leaves the PREVIOUS
# sigel_eval in place, so a test-for-existence passes and the gate silently
# scores a stale binary. That happened during D13: the build failed on two
# sites, the gates were run straight after, and both came back green against
# the binary from before the change. Gate results mean nothing unless the
# build that produced them succeeded. pvm-check.sh has carried this guard
# from the start; these two did not.
# The hint has to match the build dir: SAN=/SIGSAN= empty is right for the
# unsanitised build-fast, but passing it for the sanitised `build` links
# sanitiser-compiled objects without the runtime and fails on
# __ubsan_handle_type_mismatch_v1. Found by following this message's own advice.
make -q B="$B" 2>/dev/null || {
	if [ "$B" = build ]; then hint="make B=$B"; else hint="make B=$B SAN= SIGSAN="; fi
	echo ""$ROOT/$B/sigel_eval" is out of date -- run '$hint'" >&2; exit 1; }

SIGEL_ROOT=$ROOT/x/kdesigelSources.1.3/kdesigel/kdesigel
export SIGEL_ROOT
# Duplicate-key tie-breaking, which no amount of shipped data can exercise --
# no robot has a duplicate name, so both baselines stay empty when it breaks.
# Run the self-check twice. The second run turns LeakSanitizer ON, which the
# evaluations below cannot afford -- they carry a documented 41 KB baseline
# leak (PORTING.md D18). The self-check frees everything it allocates, so it
# CAN be leak-checked, and that is what stands between a dropped qDeleteAll in
# ~SIG_Material and a clean run. Added after the D11 review found that hole.
#
# IT ONLY WORKS ON A SANITIZED BUILD, and the default B is build-fast, which is
# built with SAN= and has no sanitizer at all. The first version of this ran
# there anyway: ASAN_OPTIONS was an ignored environment variable, the "second"
# run was the first one again with its output thrown away, and deleting the
# qDeleteAll left ./fitness-check.sh exiting 0 with a byte-identical baseline.
# It was inert in the exact invocation PORTING.md prescribes. Found by the D12
# review. So test the binary rather than assume, and SAY when it is skipped --
# a check that quietly does nothing is worse than no check, because the
# operator believes it ran.
"$ROOT/$B/sigel_eval" -selfcheck >&2 || exit 1
if nm -C "$ROOT/$B/sigel_eval" 2>/dev/null | grep -q __asan_init; then
	leaks=$(ASAN_OPTIONS=detect_leaks=1 "$ROOT/$B/sigel_eval" -selfcheck 2>&1 >/dev/null) || {
		echo "selfcheck LEAKED under LeakSanitizer:" >&2
		echo "$leaks" >&2
		exit 1
	}
else
	echo "note: $B has no sanitizer, so the self-check leak test was SKIPPED." >&2
	echo "      run 'ASAN_OPTIONS=detect_leaks=0 ./fitness-check.sh build' for it." >&2
fi

n=$(find "$ROOT/experiments" -name '*.exp' | wc -l)
[ "$n" -eq 7 ] || { echo "expected 7 .exp under experiments/, found $n" >&2; exit 1; }
# CAPTURE, TEST THE STATUS, THEN FILTER -- do NOT pipe sigel_eval straight into
# tail. This line used to read
#
#   v=$("$ROOT/$B/sigel_eval" "$f" "$i" 2>/dev/null | tail -1 | awk '{print $3}')
#
# which loses BOTH halves of the evidence. A pipeline's status is its LAST
# command's, so awk's 0 hid a segfaulting, aborting or OOM-killed sigel_eval;
# and 2>/dev/null threw away the stderr a sanitizer reports on. That matters
# most in the one invocation this gate exists for: under
# `ASAN_OPTIONS=detect_leaks=0 ./fitness-check.sh build' a UBSan
# `runtime error:' went to /dev/null and the gate read green with every number
# identical to the baseline. `[ -n "$v" ]' below is not a substitute -- it only
# catches a crash that printed NOTHING, and a crash after the last fitness line
# still leaves one to read.
#
# dictorder-dump.sh:62-77 closed this exact hole and says so; this script did
# not. Found by review 2026-09-07.
out=$(mktemp); err=$(mktemp)
for f in $(find "$ROOT/experiments" -name '*.exp' | sort); do
	for i in 0 1 2; do
		rc=0; "$ROOT/$B/sigel_eval" "$f" "$i" >"$out" 2>"$err" || rc=$?
		if [ "$rc" -ne 0 ]; then
			echo "$(basename "$f") $i: sigel_eval exited $rc" >&2
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
