#!/bin/sh
# Does PVM work?  PORTING.md Phase P, steps P3 and P4.
#
#   ./checks/pvm-check.sh [builddir]        default build
#
# Run `make pvm && make pvm-link' first.  Compiles pvm_smoke.c, then starts its
# own pvmd3 and runs two round trips against it before stopping it again:
#
#   pvm_smoke   PVM alone, C, no SIGEL and no sanitizers      -- step P3
#   pvm_link    SIGEL's SIG_GPPVMData calling real PVM        -- step P4
#
# Two programs so a failure says which half broke, and BOTH always run: a P3
# failure must not hide P4's result. Exit status is non-zero if either failed.
#
# It refuses to run rather than skipping when <builddir>/pvm_link is missing or
# out of date. An earlier version skipped it with a note and exited 0, so the
# documented `make && make pvm && ./checks/pvm-check.sh' reported success having
# proved only P3 -- `make' does not build pvm_link.
#
# Not one of the three checks that must stay green -- it has no baseline to
# diff against, it is pass/fail.  It exists because the Phase P research
# reported PVM "verified by building and running it" while the binary it
# verified still carried a bug, and because P4 links against this library.
#
# PVM_TMP MUST BE SHORT.  mksocs() does
#
#     (void)PVMTMPNAMFUN(spath);        /* pvmd.c:5066, char spath[128]  */
#     strcpy(uns.sun_path, spath);      /* pvmd.c:5067, sun_path[108]    */
#
# and pvmtmpnam() builds "$PVM_TMP/pvmtmp<pid>.<n>" (pvmcruft.c:760).  A
# sockaddr_un holds 107 characters plus the NUL, so PVM_TMP has 107 minus that
# suffix.  Measured: 92 characters starts, 93 aborts with "*** buffer overflow
# detected ***" before the daemon prints anything -- that is the 6-digit-pid,
# 1-digit-counter case, 107 - 7 - 6 - 1 - 1.
#
# The limit below is 87, and it was 88 until 2026-09-05.  Two things make the
# worst case one character shorter than that.  pvmtmpnam's counter is
# `n = 1 + (f++ % 10000)' assigned AFTER the sprintf that uses it, so n runs
# 0,1,...,10000 and reaches FIVE digits, not four.  And %06d pads without
# truncating, so a pid above 999999 -- /proc/sys/kernel/pid_max is 4194304 on
# this machine -- contributes seven.  107 - 7 - 7 - 1 - 5 = 87.  The old guard
# admitted exactly one length that could still abort the daemon.  Found by
# review.  The same strcpy is at lpvm.c:1249 and :2465 on the task side.
#
# gcc warns about a DIFFERENT overflow in the same function -- pvmd.c:5178
# sprintf's into a char buf[128] -- but nothing ever reaches it, because the
# strcpy above aborts first.  None of the 26 Debian patches that apply to 3.4.6
# fixes either one; the 8 that touch what we compile are in vendor/patches/.
set -u

ROOT=$(cd "$(dirname "$0")/.." && pwd)
# ROOT is derived, not the script's own directory, so check it: a symlink or
# a copy left at the old path would point it at the PARENT of the repo, and
# check.sh removes $ROOT/build/ui before any other guard runs.
[ -f "$ROOT/Makefile" ] && [ -d "$ROOT/checks" ] || {
	echo "$0: $ROOT is not the repo root -- run the script by its real path,"\
	     "not through a symlink or a copy" >&2; exit 1; }
PVM=$ROOT/x/supportingLibs/supportingLibs/pvm3
BIN=$PVM/lib/LINUX64
B=${1:-build}
LINK=$ROOT/$B/pvm_link

[ -f "$BIN/libpvm3.a" ] && [ -x "$BIN/pvmd3" ] || {
	echo "no libpvm3.a or pvmd3 -- run 'make pvm' first" >&2; exit 1; }
[ -x "$LINK" ] || {
	echo "no $LINK -- run 'make B=$B pvm-link' first" >&2; exit 1; }
make -q --no-print-directory -C "$ROOT" B="$B" pvm-link >/dev/null 2>&1 || {
	echo "$LINK is out of date -- run 'make B=$B pvm-link'" >&2; exit 1; }

# Own directory by default.  An override is honoured but never rm -rf'd: this
# script deletes the files PVM makes, not whatever directory it was pointed at.
PVM_TMP=${PVM_TMP:-/tmp/pvm-sigel-$(id -u)}
case $PVM_TMP in
	/*) ;;
	 *) echo "PVM_TMP must be an absolute path" >&2; exit 1 ;;
esac
[ ${#PVM_TMP} -le 87 ] || {
	echo "PVM_TMP is ${#PVM_TMP} characters; pvmd.c:5067 allows 87" >&2
	exit 1; }

PVM_ROOT=$PVM
PVM_ARCH=LINUX64
export PVM_ROOT PVM_ARCH PVM_TMP

mkdir -p "$PVM_TMP"
rm -f "$PVM_TMP/pvmd.$(id -u)" "$PVM_TMP/pvml.$(id -u)" "$PVM_TMP"/pvmtmp*

# Kill the daemon we started, by pid.  `pkill pvmd3' would take out a real
# evolution run sharing this machine.
pvmd_pid=
cleanup() {
	[ -n "$pvmd_pid" ] && kill "$pvmd_pid" 2>/dev/null || true
}
trap cleanup EXIT INT TERM

cc -I"$PVM/include" "$ROOT/checks/programs/pvm_smoke.c" "$BIN/libpvm3.a" -ltirpc \
	-o "$PVM_TMP/pvm_smoke" || exit 1

# pvm_link constructs a SIG_Environment, whose default constructor loads
# terrain through vendored DynaMechs and leaks 20,400 bytes in 51 allocations
# (dmEnvironment.cpp:110, via SIG_Environment.cpp, loadDynaMechsEnvironment). Suppress that one
# function rather than turning leak detection off: a leak on the PVM path is
# exactly what this check should still catch.
cat > "$PVM_TMP/lsan.supp" <<'SUPP'
leak:dmEnvironment::loadTerrainData
SUPP
SIGEL_ROOT=$ROOT/sigel
LSAN_OPTIONS=suppressions=$PVM_TMP/lsan.supp
export SIGEL_ROOT LSAN_OPTIONS

"$BIN/pvmd3" > "$PVM_TMP/pvmd.out" 2>&1 &
pvmd_pid=$!

# Ready means the address file exists, not that a process exists: a daemon that
# is about to abort in mksocs() is still a running process for a moment.
i=0
while [ "$i" -lt 100 ]; do
	[ -f "$PVM_TMP/pvmd.$(id -u)" ] && break
	kill -0 "$pvmd_pid" 2>/dev/null || break
	i=$((i + 1)); sleep 0.1
done
[ -f "$PVM_TMP/pvmd.$(id -u)" ] || {
	echo "pvmd3 never wrote $PVM_TMP/pvmd.$(id -u):"
	cat "$PVM_TMP/pvmd.out"
	exit 1; }
echo "pvmd3          running, pid $pvmd_pid"

echo
echo "P3  PVM alone"
"$PVM_TMP/pvm_smoke"; p3=$?

echo
echo "P4  SIGEL's SIG_GPPVMData against real PVM"
"$LINK"; p4=$?

echo
[ "$p3" -eq 0 ] && [ "$p4" -eq 0 ] || {
	echo "FAILED: P3 exit $p3, P4 exit $p4" >&2; exit 1; }
echo "both PASS"
