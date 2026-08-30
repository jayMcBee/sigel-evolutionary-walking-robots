#!/bin/sh
# Exit criterion for the Qt 6 port (PORTING.md D11).
#
#   ./check.sh                 all converted modules, syntax only
#   ./check.sh MT_GPSystem     one module
#
# Vendored headers use -isystem so their warnings do not drown SIGEL's own:
# with -I the tree produces ~12,979 warnings, with -isystem ~300, all of them
# in code we are responsible for. WARNINGS ARE PART OF THE CRITERION -- the
# A3 Qt::endl regression was reported by this command at the step that
# introduced it and went unread.
#
# replaces the blanket -fpermissive used up to A8. It
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

FLAGS="-fsyntax-only -std=c++17 -Wall -Wextra -DMINMAX_H"
# QtGui and QtWidgets are here because the Makefile has them: without them
# SIG_GPPopulation.cpp fails on <QApplication> and this script reported a
# "failure" the real build does not have. Found by review.
INCS="-I$ROOT/shim -I$SRC/include -isystem $QTINC -isystem $QTINC/QtCore"
INCS="$INCS -isystem $QTINC/QtGui -isystem $QTINC/QtWidgets"
for d in newmat09 dynamechs/dm Dynamo/Src/Inc fparser cv97 SOLID-2.0/include pvm3/include; do
    INCS="$INCS -isystem $SL/$d"
done

MODULES="${1:-SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP}"
pass=0; fail=0; warn=0

# The shim self-check was here: it built and RAN q2compat_check.cpp under
# ASan+UBSan, and was the only mechanical check that could see an ownership
# error. Deleted with the shim -- it only ever tested the compatibility
# layer, so nothing is left for it to check. It was reported separately and
# never counted in the module totals, so 105/4 is unchanged by its removal.

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

# ---------------------------------------------------------------------------
# Phase C -- the converted Designer forms.
#
# MODULES above is the nine core modules. A GUI module can only join that list
# once EVERY file in it compiles, which is C3-C7; until then the forms converted
# so far would be covered by nothing at all, and §7 says extending this script
# is part of the first Phase C step rather than an afterthought.
#
# Four things per form, because three of them can break independently:
#   1. Qt 6's uic accepts the converted .ui           (make forms)
#   2. the generated ui_<Form>.h compiles standalone
#   3. the committed QWidget-derived base class compiles
#   4. moc accepts that base class and its output compiles
# (4) is here because nothing links SIGEL_MasterGUI yet, so a Q_OBJECT that moc
# chokes on would otherwise not be found until C7.
#
# Generation is delegated to the Makefile rather than repeated here: check.sh
# disagreeing with the Makefile about flags has already produced one phantom
# failure (the QtGui/QtWidgets include path, found by review).
FORM_LIST="SIGEL_MasterUI/SIG_GPParameterBase:SIGEL_MasterGUI"

MOCBIN=$(qmake6 -query QT_INSTALL_LIBEXECS)/moc
fp=0; ff=0; fw=0
if ! make -s -C "$ROOT" forms >/tmp/uic.$$ 2>&1; then
    echo "  uic FAIL:"; cat /tmp/uic.$$; ff=$((ff+1))
else
    # uic writes warnings to stderr and still exits 0 -- a dropped <images>
    # block or a renamed duplicate widget is reported exactly this way, and
    # both change the rendered form. Treat any output as a failure, minus Qt's
    # locale banner, which every Qt tool prints under a non-UTF-8 LC_ALL and
    # which says nothing about the form.
    grep -v '^Detected locale \|^Qt depends on a UTF-8 locale\|^If this causes problems\|^for more information' \
         /tmp/uic.$$ > /tmp/uic2.$$ || true
    if [ -s /tmp/uic2.$$ ]; then echo "  uic WARNED:"; cat /tmp/uic2.$$; ff=$((ff+1)); fi
    INCS="$INCS -I$ROOT/build/ui"
    for entry in $FORM_LIST; do
        form=${entry%:*}; mod=${entry#*:}; base=$(basename "$form")
        # 2. the generated header, standalone
        printf '#include "ui_%s.h"\nint main(){return 0;}\n' "$base" > /tmp/hdr.$$.cpp
        if g++ $FLAGS $INCS /tmp/hdr.$$.cpp 2>/dev/null; then fp=$((fp+1))
        else ff=$((ff+1)); echo "  form FAIL: ui_$base.h"; fi
        # 3. the committed base class
        if g++ $FLAGS $INCS "$SRC/src/$mod/$base.cpp" 2>/tmp/chk.$$; then fp=$((fp+1))
        else ff=$((ff+1)); echo "  form FAIL: $mod/$base.cpp"; fi
        fw=$((fw + $(grep -c "$SRC.*warning:" /tmp/chk.$$ || true)))
        # 4. moc, and its output
        if $MOCBIN $(echo "$INCS" | sed 's/-isystem /-I/g') \
               "$SRC/include/$mod/$base.h" -o /tmp/moc.$$.cpp 2>/tmp/chk.$$ \
           && g++ $FLAGS $INCS /tmp/moc.$$.cpp 2>>/tmp/chk.$$; then fp=$((fp+1))
        else ff=$((ff+1)); echo "  form FAIL: moc $mod/$base.h"; cat /tmp/chk.$$; fi
        # 5. the form's images, checked in BOTH directions.
        #
        # Qt 2 embedded them in the .ui; uic3 -extract pulled them into a .qrc
        # beside the form. Qt 6's uic drops any leftover <images> block with
        # nothing but a warning and emits NO icon at all -- the button or combo
        # item just renders blank, and every other check here stays green.
        #
        # forward : every ":/..." the header asks for is in the .qrc and on disk
        # reverse : every file in the .qrc is actually asked for
        # The reverse direction is the one that matters. Without it a form that
        # LOST its images passes trivially, because there is then no ":/..." to
        # check -- which is exactly the failure this exists to catch.
        qrc="$SRC/ui/$form.qrc"
        pfx=$(sed -n 's/.*<qresource prefix="\([^"]*\)".*/\1/p' "$qrc" 2>/dev/null)
        for want in $(grep -o ':/[A-Za-z0-9_/.-]*' "$ROOT/build/ui/ui_$base.h" | sort -u); do
            [ -f "$qrc" ] || { ff=$((ff+1)); echo "  form FAIL: $want but no $form.qrc"; continue; }
            rel=${want#:$pfx/}
            if [ "$rel" != "$want" ] && grep -q "<file>$rel</file>" "$qrc" \
               && [ -f "$SRC/ui/$(dirname "$form")/$rel" ]
            then fp=$((fp+1))
            else ff=$((ff+1)); echo "  form FAIL: $want not backed by $form.qrc"; fi
        done
        if [ -f "$qrc" ]; then
            for have in $(sed -n 's|.*<file>\(.*\)</file>.*|\1|p' "$qrc"); do
                if grep -q ":$pfx/$have" "$ROOT/build/ui/ui_$base.h"; then fp=$((fp+1))
                else ff=$((ff+1)); echo "  form FAIL: $form.qrc carries $have, ui_$base.h never uses it"; fi
            done
        fi
    done
fi
printf '%-22s %2d pass  %2d fail  %3d warnings\n' "forms (Phase C)" "$fp" "$ff" "$fw"
pass=$((pass+fp)); fail=$((fail+ff)); warn=$((warn+fw))

rm -f /tmp/chk.$$ /tmp/hdr.$$.cpp /tmp/uic.$$ /tmp/uic2.$$ /tmp/moc.$$.cpp
echo "-----"
echo "total: $pass pass, $fail fail, $warn warnings in SIGEL code"
