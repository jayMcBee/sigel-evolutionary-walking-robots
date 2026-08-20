#!/bin/sh
# Exit criterion for the Qt 6 port (PORTING.md D11).
#
#   ./check.sh                 all converted modules + the shim self-check
#   ./check.sh MT_GPSystem     one module
#
# Vendored headers use -isystem so their warnings do not drown SIGEL's own:
# with -I the tree produces ~12,979 warnings, with -isystem ~300, all of them
# in code we are responsible for. WARNINGS ARE PART OF THE CRITERION -- the
# A3 Qt::endl regression was reported by this command at the step that
# introduced it and went unread.
#
# -Wno-error=template-body replaces the blanket -fpermissive used up to A8. It
# suppresses exactly one vendored defect (cv97/JVector.h:29 calls a base-class
# member unqualified from a class template) instead of downgrading errors
# everywhere. Verified identical pass/fail across all 90 files.
#
# -DMINMAX_H empties vendored Dynamo's minmax.h, which defines min/max as
# macros and poisons libstdc++. No SIGEL code calls unqualified min/max.
set -e
ROOT=$(cd "$(dirname "$0")" && pwd)
SRC=$ROOT/x/kdesigelSources.1.3/kdesigel/kdesigel
SL=$ROOT/x/supportingLibs/supportingLibs
QTINC=$(qmake6 -query QT_INSTALL_HEADERS)

FLAGS="-fsyntax-only -std=c++17 -Wall -Wextra -Wno-error=template-body -DMINMAX_H"
INCS="-I$ROOT/shim -I$SRC/include -isystem $QTINC -isystem $QTINC/QtCore"
for d in newmat09 dynamechs/dm Dynamo/Src/Inc fparser cv97 SOLID-2.0/include pvm3/include; do
    INCS="$INCS -isystem $SL/$d"
done

MODULES="${1:-SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP}"
pass=0; fail=0; warn=0

# The shim self-check instantiates every Q2* member; 56 of them are reached by
# no converted .cpp, so without this half the shim is never type-checked.
# The self-check has a main(), a live-object counter and ~40 assertions. Syntax
# checking it proves nothing: LINK AND RUN IT. This is the only mechanical check
# that can see an ownership error, which is the whole risk in Phase B.
printf '%-22s ' "compat (shim)"
QTLIB=$(qmake6 -query QT_INSTALL_LIBS)
if g++ -std=c++17 -fPIC -g -fsanitize=address,undefined -Wall -Wextra \
       -I$SRC/include -isystem "$QTINC" -isystem "$QTINC/QtCore" \
       "$SRC/include/compat/q2compat_check.cpp" -L"$QTLIB" -lQt6Core \
       -o /tmp/q2chk.$$ 2>/tmp/chk.$$; then
    if /tmp/q2chk.$$ >/tmp/run.$$ 2>&1; then
        printf 'ok   (built and RUN under ASan+UBSan)\n'
    else
        printf 'FAIL at runtime\n'; cat /tmp/run.$$; fail=$((fail+1))
    fi
    rm -f /tmp/q2chk.$$ /tmp/run.$$
else
    printf 'FAIL to build\n'; cat /tmp/chk.$$; fail=$((fail+1))
fi

for m in $MODULES; do
    mp=0; mf=0; mw=0
    for f in "$SRC/src/$m"/*.cpp; do
        [ -e "$f" ] || continue
        if g++ $FLAGS $INCS "$f" 2>/tmp/chk.$$; then mp=$((mp+1)); else mf=$((mf+1)); fi
        mw=$((mw + $(grep -c "$SRC.*warning:" /tmp/chk.$$ || true)))
    done
    printf '%-22s %2d pass  %2d fail  %3d warnings\n' "$m" "$mp" "$mf" "$mw"
    pass=$((pass+mp)); fail=$((fail+mf)); warn=$((warn+mw))
done

# Headers are otherwise only checked when some .cpp happens to include them.
hp=0; hf=0
for m in $MODULES; do
 for h in "$SRC/include/$m"/*.h; do
    [ -e "$h" ] || continue
    rel=${h#$SRC/include/}
    printf '#include "%s"\nint main(){return 0;}\n' "$rel" > /tmp/hdr.$$.cpp
    if g++ $FLAGS $INCS /tmp/hdr.$$.cpp 2>/dev/null; then hp=$((hp+1)); else hf=$((hf+1)); echo "  header FAIL: $rel"; fi
 done
done
printf '%-22s %2d pass  %2d fail\n' "headers standalone" "$hp" "$hf"
rm -f /tmp/chk.$$ /tmp/hdr.$$.cpp
echo "-----"
echo "total: $pass pass, $fail fail, $warn warnings in SIGEL code"
