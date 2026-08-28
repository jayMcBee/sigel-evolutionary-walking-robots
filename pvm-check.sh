#!/bin/sh
# Does PVM work?  PORTING.md Phase P, step P3.
#
#   ./pvm-check.sh
#
# Builds nothing: run `make pvm' first.  Starts pvmd3, runs one round trip
# through pvm_smoke.c, stops the daemon.  Prints PASS or FAIL.
#
# Not one of the three gates -- it has no baseline to diff against, it is
# pass/fail.  It exists because the Phase P research reported PVM "verified by
# building and running it" while the binary it verified still carried a bug,
# and because P4 links against this library.
#
# PVM_TMP MUST BE SHORT.  pvmd.c:5178 does
#
#     sprintf(buf, "PVMSOCK=%s", p);        /* mksocs(), char buf[128] */
#
# so the socket path has 120 bytes and gcc says so at compile time
# (-Wformat-overflow=).  Give it the repo's scratchpad and glibc aborts the
# daemon with "*** buffer overflow detected ***" before it prints anything.
# No Debian patch fixes this; all 26 that apply are already in patches/.
set -eu
ROOT=$(cd "$(dirname "$0")" && pwd)
PVM=$ROOT/x/supportingLibs/supportingLibs/pvm3
BIN=$PVM/lib/LINUX64

[ -f "$BIN/libpvm3.a" ] && [ -x "$BIN/pvmd3" ] || {
	echo "no libpvm3.a or pvmd3 -- run 'make pvm' first" >&2; exit 1; }

PVM_TMP=${PVM_TMP:-/tmp/pvm-sigel}
n=$(printf '%s/pvmd.%s' "$PVM_TMP" "$(id -u)" | wc -c)
[ "$n" -lt 120 ] || {
	echo "PVM_TMP too long: socket path is $n bytes, pvmd.c:5178 allows 120" >&2
	exit 1; }

PVM_ROOT=$PVM PVM_ARCH=LINUX64
export PVM_ROOT PVM_ARCH PVM_TMP

pkill -x pvmd3 2>/dev/null || true
rm -rf "$PVM_TMP"; mkdir -p "$PVM_TMP"
trap 'pkill -x pvmd3 2>/dev/null || true' EXIT

cc -I"$PVM/include" "$ROOT/pvm_smoke.c" "$BIN/libpvm3.a" -ltirpc -o "$PVM_TMP/pvm_smoke"

"$BIN/pvmd3" > "$PVM_TMP/pvmd.out" 2>&1 &
i=0
while [ $i -lt 50 ]; do
	pgrep -x pvmd3 > /dev/null && break
	i=$((i + 1)); sleep 0.1
done
pgrep -x pvmd3 > /dev/null || { echo "pvmd3 did not start:"; cat "$PVM_TMP/pvmd.out"; exit 1; }
echo "pvmd3      running"

"$PVM_TMP/pvm_smoke"
