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
# QOpenGLWidget arrived with C3's SIG_VisualisationWidget.
INCS="$INCS -isystem $QTINC/QtOpenGL -isystem $QTINC/QtOpenGLWidgets"
for d in newmat09 dynamechs/dm Dynamo/Src/Inc fparser cv97 SOLID-2.0/include pvm3/include; do
    INCS="$INCS -isystem $SL/$d"
done

# SIGEL_Visualisation joined at C5, SIGEL_CommonGUI at C3 and SIGEL_SlaveGUI at
# C4 -- a GUI module joins this list only when EVERY file in it compiles.
# The converted forms generate ui_<Form>.h into build/ui, and GUI module headers
# include them -- so they must exist and be on the include path BEFORE the module
# and header passes, not only inside the forms section. C4 found this: five
# SIGEL_SlaveGUI headers failed the standalone pass for want of the flag.
FORMS_FAILED=
# Regenerate unconditionally. make only re-runs uic when a .ui is newer than its
# ui_*.h, so on a warm tree `make forms' prints nothing and the uic-warning check
# below sees an empty log -- green with the defect still in the .ui. Review
# proved it: injecting an <images> block failed the FIRST run and passed the
# second. Deleting the output directory costs one uic pass over 20 forms.
rm -rf "$ROOT/build/ui"
if make -s -C "$ROOT" forms >/tmp/mkforms.$$ 2>&1; then
    INCS="$INCS -I$ROOT/build/ui"
else
    FORMS_FAILED=1
    echo "  make forms FAILED -- every GUI module check below is unreliable:"
    cat /tmp/mkforms.$$
fi

# Signals that existed in Qt 2 and do NOT exist in Qt 6, in SIGNAL() spelling.
# Verified one by one with QMetaObject::indexOfSignal against Qt 6.10.2, not
# read off a porting guide. Deliberately NOT here because they are still live:
# clicked() (moc clones the default argument of clicked(bool)), activated(int)
# on a QComboBox, valueChanged(int), textChanged(...), stateChanged(int),
# toggled(bool), pressed(), timeout().
#
# lostFocus() was here and was WRONG: MT_Editor declares its own lostFocus()
# signal (include/MT_GUI/MT_Editor.h:32) and Qt 2.3's QLineEdit never had one.
# A name shared with a framework signal is a false positive in EITHER
# direction, so the class has to be established per site. The counter-check is
# to grep every `signals:' block in the tree for these names; lostFocus is the
# only collision.
#
# selectionChanged was anchored to `( )' and so missed the QListViewItem*
# overload, which Qt 2's QListView also declared and which is equally dead.
# Both errors were found by review, and they cancelled in the total.
DEAD_SIGNALS='SIGNAL\( *(activated *\( *\)|activated *\( *const *QString'\
'|clicked *\( *int|selected *\(|selectionChanged *\('\
'|currentChanged *\( *Q(ListView|ListBox)Item|rightButtonClicked'\
'|doubleClicked *\( *QListViewItem)'

MODULES="${*:-SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP SIGEL_Visualisation SIGEL_CommonGUI SIGEL_SlaveGUI}"
pass=0; fail=0; warn=0

# The shim self-check was here: it built and RAN q2compat_check.cpp under
# ASan+UBSan, and was the only mechanical check that could see an ownership
# error. Deleted with the shim -- it only ever tested the compatibility
# layer, so nothing is left for it to check. It was reported separately and
# never counted in the module totals, so 105/4 is unchanged by its removal.

dead=0; deadbase=0
for m in $MODULES; do
    mp=0; mf=0; mw=0
    for f in "$SRC/src/$m"/*.cpp; do
        [ -e "$f" ] || continue
        if g++ $FLAGS $INCS "$f" 2>/tmp/chk.$$; then mp=$((mp+1)); else mf=$((mf+1)); fi
        mw=$((mw + $(grep -c "$SRC.*warning:" /tmp/chk.$$ || true)))
    done
    # Dead string-based connects. A SIGNAL() naming a signal Qt 6 does not have
    # compiles, links, runs and never fires -- there is no other check in this
    # script, or in the compiler, that can see it. The list is not guessed: each
    # signature was run through QMetaObject::indexOfSignal on the real Qt 6.10.2
    # meta-object, which is exactly what connect() does at run time. C4 shipped
    # ten of these and only review caught them.
    # Counted against a per-module baseline, NOT folded into mf: an unconverted
    # module's dead connects are known debt (§2 has the table), and folding them
    # in would misreport them as compile failures and leave the script standing
    # red until C8. Any count ABOVE the baseline fails. The baseline is zero for
    # every CONVERTED module, so a new one cannot be introduced there; the three
    # non-zero ones below are MT_Control, MT_GUI and SIGEL_MasterGUI, which is
    # C8, C6 and C7 work. MT_Control IS compiled by this script today, so the
    # gate is not "zero everywhere it looks".
    #
    # -o|wc -l, not -c: grep -c counts matching LINES. No line carries two
    # SIGNAL() macros today, so the two agree -- but the baselines are exact
    # numbers and should not quietly drift if that ever stops being true.
    md=$(command grep -rhoE "$DEAD_SIGNALS" "$SRC/src/$m" "$SRC/include/$m" 2>/dev/null | wc -l)
    case "$m" in
        SIGEL_MasterGUI) base=44 ;;
        MT_GUI)          base=31 ;;
        MT_Control)      base=15 ;;
        *)               base=0  ;;
    esac
    dead=$((dead+md)); deadbase=$((deadbase+base))
    if [ "$md" -gt "$base" ]; then
        echo "  $m: $md connect(s) to a signal Qt 6 does not have, baseline $base:"
        command grep -rnE "$DEAD_SIGNALS" "$SRC/src/$m" "$SRC/include/$m" 2>/dev/null \
            | sed "s|$SRC/|    |" | cut -c1-140
        mf=$((mf+md-base))
    fi
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
# The module loop only reaches src/<Module>/ and include/<Module>/. sigel.cpp and
# sigel_slave.cpp sit at the top of src/ and the forms carry <connection> blocks,
# so both are outside every baseline above. Neither has a dead signal today and
# this keeps it that way.
stray=$(command grep -rhoE "$DEAD_SIGNALS" "$SRC"/src/*.cpp "$SRC"/ui 2>/dev/null | wc -l)
if [ "$stray" -gt 0 ]; then
    echo "  dead signal outside every module baseline:"
    command grep -rnE "$DEAD_SIGNALS" "$SRC"/src/*.cpp "$SRC"/ui 2>/dev/null \
        | sed "s|$SRC/|    |" | cut -c1-140
    fail=$((fail+stray))
fi
printf '%-22s %2d dead (baseline %d -- §2 has the per-signal table)\n' \
       "Qt 6 signals" "$((dead+stray))" "$deadbase"

# ---------------------------------------------------------------------------
# Parsers that no gate reads.
#
# Every other check here is a COMPILE check, and the four behaviour gates only
# exercise what a fitness evaluation touches. A file format the program parses
# but the gates never open is therefore covered by nothing at all -- which is
# exactly how Qt 2's QTextStream::operator>>(char&) skipping whitespace, and
# Qt 6's not, survived the whole of SIGEL_GP's conversion while every gate
# stayed green. It emptied the slave directory of every PVM host.
#
# Kept as a heredoc rather than a committed .cpp for the same reason the header
# pass is: it needs no source file of its own, and the expectations belong next
# to the reason they exist.
cat > /tmp/pvm.$$.cpp <<'PVMEOF'
#include "SIGEL_GP/SIG_GPPVMHost.h"
#include <QString>
#include <cstdio>
static int fails = 0;
static void eq( const char *what, QString got, QString want )
{
    if ( got != want ) {
        ++fails;
        printf( "  PVMHOST %s: got [%s] want [%s]\n",
                what, qPrintable(got), qPrintable(want) );
    }
}
int main()
{
    // 1.3's own data: data/Experiments/twoBasesSimpleFitness1.exp
    SIGEL_GP::SIG_GPPVMHost h( "eiche 2 1 \"/home/pg368b/ross/projects/sigel\"" );
    eq( "name",      h.name,                       "eiche" );
    eq( "maxSlaves", QString::number(h.maxSlaves), "2" );
    eq( "enabled",   QString::number(h.enabled),   "1" );
    eq( "dir",       h.executableDir.path(),       "/home/pg368b/ross/projects/sigel" );
    // Qt 2 skipped whitespace on EVERY char read, so it silently dropped spaces
    // inside the quoted path too. That is a 2003 defect and it is preserved.
    SIGEL_GP::SIG_GPPVMHost s( "herz 4 0 \"/tmp/with space/sigel\"" );
    eq( "dir (2003 defect)", s.executableDir.path(), "/tmp/withspace/sigel" );
    return fails ? 1 : 0;
}
PVMEOF
pp=0; pf=0
if g++ -std=c++17 -fPIC $(echo $INCS | sed 's/-fsyntax-only//') /tmp/pvm.$$.cpp \
       "$SRC/src/SIGEL_GP/SIG_GPPVMHost.cpp" -o /tmp/pvm.$$ \
       $(qmake6 -query QT_INSTALL_LIBS 2>/dev/null | sed 's|^|-L|') -lQt6Core 2>/tmp/pvmb.$$
then
    if /tmp/pvm.$$ 2>/dev/null; then pp=1; else pf=1; fi
else
    pf=1; echo "  PVMHOST check did not build:"; head -5 /tmp/pvmb.$$
fi
printf '%-22s %2d pass  %2d fail\n' "parsers" "$pp" "$pf"
pass=$((pass+pp)); fail=$((fail+pf))
rm -f /tmp/pvm.$$ /tmp/pvm.$$.cpp /tmp/pvmb.$$

# ---------------------------------------------------------------------------
# Phase C -- the converted Designer forms.
#
# MODULES above is the nine core modules. A GUI module can only join that list
# once EVERY file in it compiles, which is C3-C7; until then the forms converted
# so far would be covered by nothing at all, and §7 says extending this script
# is part of the first Phase C step rather than an afterthought.
#
# Six things per form, because they break independently:
#   1. Qt 6's uic accepts the converted .ui           (make forms)
#   2. the generated ui_<Form>.h compiles standalone
#   3. the committed QWidget-derived base class compiles
#   4. moc accepts that base class and its output compiles
#   5. the .qrc and the generated header agree, in BOTH directions
#   6. every Designer Line still carries an orientation
# (4) is here because nothing links SIGEL_MasterGUI yet, so a Q_OBJECT that moc
# chokes on would otherwise not be found until C7. (5) and (6) cover the two
# losses that are silent everywhere else -- a dropped image and a dropped
# separator orientation both compile, run, and just render wrong.
#
# Generation is delegated to the Makefile rather than repeated here: check.sh
# disagreeing with the Makefile about flags has already produced one phantom
# failure (the QtGui/QtWidgets include path, found by review).
FORM_LIST="MT_UI/MT_AddConstantsWidgetBase:MT_GUI \
            MT_UI/MT_AddIndividualsWidget:MT_GUI \
            MT_UI/MT_EstimationWidgetBase:MT_GUI \
            MT_UI/MT_ExperimentWidgetBase:MT_GUI \
            MT_UI/MT_IndividualWidgetBase:MT_GUI \
            MT_UI/MT_PopulationWidgetBase:MT_GUI \
            MT_UI/MT_SearchWidgetBase:MT_GUI \
            MT_UI/MT_SelectionWidgetBase:MT_GUI \
            MT_UI/MT_StatisticsWidgetBase:MT_GUI \
            SIGEL_MasterUI/SIG_EditHostDialogBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_EnvironmentBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_ExperimentViewBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_GPParameterBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_IndividualListBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_IndividualViewBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_LanguageParametersBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_RobotBase:SIGEL_MasterGUI \
            SIGEL_MasterUI/SIG_SimulationParameterBase:SIGEL_MasterGUI \
            SIGEL_SlaveUI/SIG_MovieSettingsDialogBase:SIGEL_SlaveGUI \
            SIGEL_SlaveUI/SIG_SimulationWidgetBase:SIGEL_SlaveGUI"

MOCBIN=$(qmake6 -query QT_INSTALL_LIBEXECS)/moc
fp=0; ff=0; fw=0
# `forms' already ran above, before the module passes, because GUI headers need
# its output. REUSE THAT LOG -- do not re-run make here. A second `make forms'
# is a no-op that emits nothing, so its (empty) output would replace the first
# run's warnings and the uic-warning check below would become unreachable. That
# is exactly what happened when this section was hoisted: the gate C1 added to
# catch a dropped <images> block stopped being able to fire. Found by the C4
# review, which proved it by injecting one.
cp /tmp/mkforms.$$ /tmp/uic.$$ 2>/dev/null || : > /tmp/uic.$$
if [ -n "$FORMS_FAILED" ]; then
    echo "  make forms FAILED -- checks 2-7 below did not run:"; cat /tmp/uic.$$; ff=$((ff+1))
else
    # uic writes warnings to stderr and still exits 0 -- a dropped <images>
    # block or a renamed duplicate widget is reported exactly this way, and
    # both change the rendered form. Treat any output as a failure, minus Qt's
    # locale banner, which every Qt tool prints under a non-UTF-8 LC_ALL and
    # which says nothing about the form.
    grep -v '^Detected locale \|^Qt depends on a UTF-8 locale\|^If this causes problems\|^for more information' \
         /tmp/uic.$$ > /tmp/uic2.$$ || true
    if [ -s /tmp/uic2.$$ ]; then echo "  uic WARNED:"; cat /tmp/uic2.$$; ff=$((ff+1)); fi
    for entry in $FORM_LIST; do
        # <uidir>/<Form>:<Module>[:blocked]  -- "blocked" means the form itself
        # is converted but a custom widget it embeds is not, so its generated
        # header cannot compile yet. Checks 1, 5 and 6 still run; 2-4 cannot.
        blocked=""
        case $entry in *:*:*) blocked=${entry##*:}; entry=${entry%:*};; esac
        form=${entry%:*}; mod=${entry#*:}; base=$(basename "$form")
        if [ -n "$blocked" ]; then
            echo "  form SKIP: $base compile/moc -- blocked on $blocked:" \
                 "it embeds a custom widget whose header is still Qt 2"
        else
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
        fi
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
        # `pfx=$(sed ...)' takes sed's exit status, and sed on a missing file
        # exits 2 -- so under `set -e' this killed the script before the
        # [ -f "$qrc" ] guard below, which exists for exactly that case, could
        # run. 19 of the 20 forms have no .qrc, so C2's first added form would
        # have aborted the gate with no forms line and no total printed.
        # Found by the C1 review.
        pfx=
        [ -f "$qrc" ] && pfx=$(sed -n 's/.*<qresource prefix="\([^"]*\)".*/\1/p' "$qrc")
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
        # 7. every view the form switches sorting on also pins the DIRECTION.
        # Qt 2's QListView sorted column 0 ascending by default; Qt 6's
        # setSortingEnabled(true) leaves the indicator descending, so the rows
        # come out reversed wherever column 0 holds text. C1 found this and
        # fixed one view; C2 re-created it in four more. This is why it is a
        # check and not a habit.
        if [ -f "$ROOT/build/ui/ui_$base.h" ]; then
            for v in $(grep -oE '^        [A-Za-z0-9_]+->setSortingEnabled\(true\)' \
                       "$ROOT/build/ui/ui_$base.h" | sed 's/->.*//;s/ *//' | sort -u); do
                if [ -n "$blocked" ] || grep -q "$v->sortByColumn(" "$SRC/src/$mod/$base.cpp"; then fp=$((fp+1))
                else ff=$((ff+1)); echo "  form FAIL: $base sorts $v but never pins the direction"; fi
            done
        fi
        # 6. every Designer separator still says which way it runs.
        # `uic3 -convert' DROPS a Line's `orientation', and that property is the
        # only thing Qt 6's uic reads to choose a frame shape: without it the
        # widget is a bare QFrame, i.e. NoFrame, and the separator paints
        # nothing. Invisible to every other check here. C2 measured which:
        # uic3 KEEPS an explicit frameShape and drops only the then-redundant
        # orientation, so 3 of the 6 Lines were affected, not 6 -- the ones
        # whose Qt 2 form set orientation ALONE. Either property satisfies it.
        nline=$(grep -c '<widget class="Line"' "$SRC/ui/$form.ui" || true)
        nshape=$(grep -A3 '<widget class="Line"' "$SRC/ui/$form.ui" \
                 | grep -cE '<property name="(orientation|frameShape)"' || true)
        if [ "$nline" -le "$nshape" ]; then fp=$((fp+1))
        else ff=$((ff+1)); echo "  form FAIL: $form.ui has $nline Line widgets but $nshape with a shape"; fi
    done
fi
printf '%-22s %2d pass  %2d fail  %3d warnings\n' "forms (Phase C)" "$fp" "$ff" "$fw"
pass=$((pass+fp)); fail=$((fail+ff)); warn=$((warn+fw))

rm -f /tmp/chk.$$ /tmp/hdr.$$.cpp /tmp/uic.$$ /tmp/uic2.$$ /tmp/moc.$$.cpp /tmp/mkforms.$$
echo "-----"
echo "total: $pass pass, $fail fail, $warn warnings in SIGEL code"
