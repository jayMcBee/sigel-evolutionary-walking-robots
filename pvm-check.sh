#!/bin/sh
# Does PVM work?  PORTING.md Phase P, step P3.
#
#   ./pvm-check.sh
#
# Builds nothing: run `make pvm' first.  Starts its own pvmd3, runs one round
# trip through pvm_smoke.c, stops it again.  Prints PASS or FAIL.
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
# detected ***" before the daemon prints anything.  The limit below is 88, the
# worst case with a 7-digit pid and a 4-digit sequence number.
#
# gcc warns about a DIFFERENT overflow in the same function -- pvmd.c:5178
# sprintf's into a char buf[128] -- but nothing ever reaches it, because the
# strcpy above aborts first.  None of the 26 Debian patches that apply to 3.4.6
# fixes either one; the 8 that touch what we compile are in patches/.
set -eu

ROOT=$(cd "$(dirname "$0")" && pwd)
PVM=$ROOT/x/supportingLibs/supportingLibs/pvm3
BIN=$PVM/lib/LINUX64

[ -f "$BIN/libpvm3.a" ] && [ -x "$BIN/pvmd3" ] || {
	echo "no libpvm3.a or pvmd3 -- run 'make pvm' first" >&2; exit 1; }

# Own directory by default.  An override is honoured but never rm -rf'd: this
# script deletes the files PVM makes, not whatever directory it was pointed at.
PVM_TMP=${PVM_TMP:-/tmp/pvm-sigel-$(id -u)}
case $PVM_TMP in
	/*) ;;
	 *) echo "PVM_TMP must be an absolute path" >&2; exit 1 ;;
esac
[ ${#PVM_TMP} -le 88 ] || {
	echo "PVM_TMP is ${#PVM_TMP} characters; pvmd.c:5067 allows 88" >&2
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

cc -I"$PVM/include" "$ROOT/pvm_smoke.c" "$BIN/libpvm3.a" -ltirpc \
	-o "$PVM_TMP/pvm_smoke"

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

"$PVM_TMP/pvm_smoke"
