#!/bin/sh
# The main check of the Qt 6 port (PORTING.md §7). It compiles every module
# and header, checks the forms, runs the probes and guidrive scenarios below,
# and prints one row per section and a total.
#
#   ./checks/check.sh                 every module
#   ./checks/check.sh MT_GPSystem     one module in the compile passes; the
#                                     other sections still run
#
# It exits non-zero if any check fails or any section is skipped.
#
# Vendored headers use -isystem, so their warnings do not hide SIGEL's own.
# The warning count on the total line is part of the result; read it.
#
# -DMINMAX_H empties vendored Dynamo's minmax.h, which defines min/max as
# macros and breaks libstdc++. No SIGEL code calls unqualified min/max.
set -e
ROOT=$(cd "$(dirname "$0")/.." && pwd)
# ROOT comes from the script's path, so check it: run through a symlink or a
# copy elsewhere, it would name the wrong directory, and every path below is
# built from it.
[ -f "$ROOT/Makefile" ] && [ -d "$ROOT/checks" ] || {
	echo "$0: $ROOT is not the repo root -- run the script by its real path,"\
	     "not through a symlink or a copy" >&2; exit 1; }

# Work from the repo root, whatever the caller's directory: guidrive opens
# robots/twoBases/twoBases.rrb relative to its working directory.
cd "$ROOT" || exit 1
SRC=$ROOT/sigel
# The folder SIGEL is started from -- the Makefile's sigelApp target.
APP=$ROOT/sigelApp
SL=$ROOT/downloads/supportingLibs
QTINC=$(qmake6 -query QT_INSTALL_HEADERS)
QTLIBDIR=$(qmake6 -query QT_INSTALL_LIBS)

FLAGS="-fsyntax-only -std=c++17 -Wall -Wextra -DMINMAX_H"
# The include paths match the Makefile's. A path missing here makes a file
# fail that the real build compiles.
INCS="-I$ROOT/shim -I$SRC/include -isystem $QTINC -isystem $QTINC/QtCore"
INCS="$INCS -isystem $QTINC/QtGui -isystem $QTINC/QtWidgets"
# QtOpenGL and QtOpenGLWidgets, for SIG_VisualisationWidget's QOpenGLWidget.
INCS="$INCS -isystem $QTINC/QtOpenGL -isystem $QTINC/QtOpenGLWidgets"
for d in newmat09 dynamechs/dm Dynamo/Src/Inc fparser cv97 pvm3/include; do
    INCS="$INCS -isystem $SL/$d"
done

# The forms generate ui_<Form>.h, and GUI module headers include them, so they
# are generated here, BEFORE the module and header passes. They go to a scratch
# build folder of this script's own: regenerating the real build/ui would leave
# every program built from it out of date during this run.
FORMS_FAILED=
# Regenerated every time, into an empty folder. make re-runs uic only when a
# .ui is newer than its ui_*.h, so on a warm tree `make forms' prints nothing,
# and the uic-warning check in the forms section would pass on an empty log
# with a defect still in the .ui.
FORMSB=${TMPDIR:-/tmp}/forms.$$
rm -rf "$FORMSB"
if make -s -C "$ROOT" B="$FORMSB" forms >/tmp/mkforms.$$ 2>&1; then
    INCS="$INCS -I$FORMSB/ui"
else
    FORMS_FAILED=1
    echo "  make forms FAILED -- every GUI module check below is unreliable:"
    cat /tmp/mkforms.$$
fi

MODULES="${*:-SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP SIGEL_Visualisation SIGEL_CommonGUI SIGEL_SlaveGUI MT_GUI SIGEL_MasterGUI}"
pass=0; fail=0; warn=0; skipped=0; winskip=0

for m in $MODULES; do
    mp=0; mf=0; mw=0
    for f in "$SRC/src/$m"/*.cpp; do
        [ -e "$f" ] || continue
        # WIN_* files are Windows-only and permanently out of scope:
        # WIN_SIG_GPRemoteZORCFitnessFunction needs HANDLE and OVERLAPPED
        # from windows.h and cannot compile on Linux. They are counted as a
        # known exclusion, not as a failure, so that "0 fail" and the exit
        # status stay usable.
        case "${f##*/}" in WIN_*) winskip=$((winskip+1)); continue ;; esac
        if g++ $FLAGS $INCS "$f" 2>/tmp/chk.$$; then mp=$((mp+1)); else mf=$((mf+1)); fi
        mw=$((mw + $(command grep -ac "$SRC.*warning:" /tmp/chk.$$ || true)))
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
    case "${h##*/}" in WIN_*) winskip=$((winskip+1)); continue ;; esac
    printf '#include "%s"\nint main(){return 0;}\n' "$rel" > /tmp/hdr.$$.cpp
    if g++ $FLAGS $INCS /tmp/hdr.$$.cpp 2>/dev/null; then hp=$((hp+1)); else hf=$((hf+1)); echo "  header FAIL: $rel"; fi
 done
done
printf '%-22s %2d pass  %2d fail\n' "headers standalone" "$hp" "$hf"
pass=$((pass+hp)); fail=$((fail+hf))

# ---------------------------------------------------------------------------
# Line endings. The tree is LF only: no tracked text file may contain CRLF.
# This fails as soon as CRLF comes back through a Windows editor, an unpacked
# archive, a patch or a clone with core.autocrlf=true. It reads every tracked
# file, so adding or deleting a tracked text file moves the pass count by one.
#
# Git decides what is binary, not this script. \r\n inside a PNG or a tarball
# is data, not a line ending, so binaries are skipped. A test for a NUL in the
# first 8 KB gets that wrong: sigel/textures/Hippie.pnm has no NUL at all, and
# sigel/textures/UniDo_LSXI.pnm has its first NUL at offset 15,456. Both are
# raw raster data. `git ls-files --eol' calls all five .pnm binary, because git
# falls back to a printable-byte ratio when there is no NUL.
#
# Lone CRs are not tested. Every tracked file that holds one is binary
# (archives, images, .blend models), where 0x0d is data.
#
# Written in Python: this is byte counting against git, and in shell it is
# easy to skip a file silently while reporting a clean pass.
enc_out=$(cd "$ROOT" && python3 - <<'ENCPY'
import subprocess
def git(*a):
    r = subprocess.run(["git"] + list(a), capture_output=True)
    if r.returncode != 0:
        raise SystemExit("git %s failed" % " ".join(a))
    return r.stdout
# -z, NOT the default. Without it git C-quotes any path that holds a tab, a
# quote, a backslash or a byte above 127 -- "l\303\244tin.txt" -- and open()
# then fails on the quoted string.
files = [f for f in git("ls-files","-z").decode("utf-8","surrogateescape").split(chr(0)) if f]
# THE w/ COLUMN, NOT THE i/ COLUMN. `ls-files --eol' prints both: i/ is the
# blob in the index, w/ is the file on disk, and this check reads the file on
# disk. The two differ while a change is not yet staged (git add), and testing
# i/ could then drop a tracked text file out of the check. Do not change this to
# test the whole first column.
binary = set()
for line in git("ls-files","--eol","-z").decode("utf-8","surrogateescape").split(chr(0)):
    if not line: continue
    cols = line.split(chr(9))
    w = [t for t in cols[0].split() if t.startswith("w/")]
    if w and w[0] == "w/-text":
        binary.add(cols[-1])
ok = bad = binfiles = unreadable = 0
for rel in files:
    try: cur = open(rel,"rb").read()
    except OSError: cur = None
    if cur is None:
        unreadable += 1
        print("  %s: tracked but unreadable" % rel)
    elif rel in binary:
        binfiles += 1
    elif b"\r\n" in cur:
        bad += 1
        print("  %s: CRLF is back (%d pairs)" % (rel, cur.count(b"\r\n")))
    else:
        ok += 1
# Every tracked file must land in exactly one bucket. The if/elif chain above
# makes this true by construction. The test is there so that an edit which
# lets a file fall out of every bucket fails loudly. It is not coverage.
if ok + bad + binfiles + unreadable != len(files):
    raise SystemExit("encodings: %d files but %d + %d + %d + %d accounted" %
                     (len(files), ok, bad, binfiles, unreadable))
print("COUNTS %d %d %d %d" % (ok, bad, binfiles, unreadable))
ENCPY
)
enc_field() { echo "$enc_out" | sed -n "s/^COUNTS $1.*/\\1/p"; }
ep=$(enc_field '\([0-9]*\) ')
ef=$(enc_field '[0-9]* \([0-9]*\) ')
eb=$(enc_field '[0-9]* [0-9]* \([0-9]*\) ')
eu=$(enc_field '[0-9]* [0-9]* [0-9]* \([0-9]*\)')
# Zero. A non-zero count means CRLF has come back; never raise this to make a
# failure go away.
ENC_BASELINE=0
# A floor on the files SEEN, because zero failures is also what a check that
# read nothing reports. `git ls-files' returning empty gives COUNTS 0 0 0 0,
# whose four numbers are all non-empty, so the empty-result branch below does
# not catch it. The tree has about 600 tracked files, so fewer than 500 means
# the check did not run.
#
# It counts files seen, not files that passed, and it is tested last. A
# tree-wide CRLF regression drives the LF-only count to zero; tested first, or
# on that count, the floor would report "it did not run" and one failure
# instead of every CRLF file.
ENC_FLOOR=500
# Fail closed on an empty result. If python exits 0 but prints no COUNTS line,
# the fields come back empty, the numeric test errors, and because that is an
# `if' condition set -e does not fire.
#
# If python exits non-zero instead, set -e stops the script at the enc_out
# assignment above, with no encodings row and no total line. That is loud, so
# it is left as it is; this branch does not handle it.
if [ -z "$ep" ] || [ -z "$ef" ] || [ -z "$eb" ] || [ -z "$eu" ]; then
    echo "  encodings check produced no COUNTS line -- treating as FAILED"
    printf '%-22s %2d pass  %2d fail\n' "encodings" 0 1
    fail=$((fail+1))
    ep=0
elif [ "$ef" -gt "$ENC_BASELINE" ] || [ "$eu" -gt 0 ]; then
    echo "$enc_out" | command grep -v '^COUNTS ' || true
    printf '%-22s %2d pass  %2d fail  (%s with CRLF, %s unreadable -- the tree is LF only)\n' \
           "encodings" "$ep" "$((ef-ENC_BASELINE+eu))" "$ef" "$eu"
    fail=$((fail+ef-ENC_BASELINE+eu))
elif [ "$((ep+ef+eb+eu))" -lt "$ENC_FLOOR" ]; then
    # LAST, and on files SEEN. See ENC_FLOOR above for why both.
    echo "$enc_out" | command grep -v '^COUNTS ' || true
    printf '%-22s %2d pass  %2d fail  (saw only %d tracked files, floor is %d -- it did not run)\n' \
           "encodings" 0 1 "$((ep+ef+eb+eu))" "$ENC_FLOOR"
    fail=$((fail+1))
    # Not $ep: the row says 0 pass, so the total must get 0.
    ep=0
else
    printf '%-22s %2d LF-only  %2d CRLF, %s binary (git)\n' \
           "encodings" "$ep" "$ef" "$eb"
fi
pass=$((pass+ep))

# ---------------------------------------------------------------------------
# DISpinBox's decimal text. `gui behaviour' steps and types whole numbers into
# DISpinBoxes; only this probe types a fraction or a comma.
#
# DISpinBox overrides Qt's text/value mapping, and code that compiles can still
# lose what the user typed. Qt 6's QSpinBox::validate() and fixup() parse an
# INTEGER and run BEFORE the virtual valueFromText, so "0.375" can become 0
# before the override sees the fraction.
cat > /tmp/dsp.$$.cpp <<'DSPEOF'
#include "MT_GUI/DoubleSpinBox.h"
#include <QApplication>
#include <QLineEdit>
#include <QLocale>
#include <QLineEdit>
#include <QValidator>
#include <cstdio>
struct P : DISpinBox {
    P(int d) : DISpinBox(d) {}
    void type(const char *s) { lineEdit()->setText(s); interpretText(); }
    // lineEdit() and validate() are both non-public in the classes above; this
    // is the one place that can reach the installed validator.
    const QValidator *installedValidator() const { return lineEdit()->validator(); }
};
static int fails = 0;
static void eq(const char *what, QString got, QString want)
{
    if (got != want) {
        ++fails;
        printf("  DISpinBox %s: got [%s] want [%s]\n",
               what, qPrintable(got), qPrintable(want));
    }
}
static void vcomma(const char *where, QLocale system)
{
    QLocale::setDefault(system);
    P k(3); k.setRange(3, 0.0, 100.0);
    const QValidator *v = k.installedValidator();
    if (!v) { ++fails; printf("  DISpinBox comma %s: no validator installed\n", where); return; }
    QString comma = "0,375"; int pos = 0;
    QValidator::State st = v->validate(comma, pos);
    if (st != QValidator::Invalid) {
        ++fails;
        printf("  DISpinBox comma %s: \"0,375\" gave %s, Qt 2 gave Invalid\n",
               where, st == QValidator::Acceptable ? "Acceptable" : "Intermediate");
    }
}
int main(int c, char **v)
{
    QApplication a(c, v);
    P d(3); d.setRange(3, 0.0, 100.0); d.type("0.375");
    eq("3dp text",  d.text(),                  "0.375");
    eq("3dp value", QString::number(d.value()), "375");
    // 1.3 was locale-INDEPENDENT: QApplication forced LC_NUMERIC=C and Qt 2's
    // QDoubleValidator hard-coded '.'. Qt 6's validators follow the system
    // locale while QString::toDouble does not, so without QLocale::c() on the
    // validator a comma-decimal locale reads 0.375 as 0. This row runs the same
    // input under de_DE.
    QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));
    P g(3); g.setRange(3, 0.0, 100.0); g.type("0.375");
    eq("3dp text de_DE",  g.text(),                  "0.375");
    eq("3dp value de_DE", QString::number(g.value()), "375");
    // Pinning to QLocale::c() is NOT enough on its own, and these rows exist
    // for that trap. C's GROUP separator is ',', so a bare QLocale::c()
    // validator accepts "0,375" as 375 with a group separator, and
    // QString::toDouble(), which never takes group separators, then returns 0.
    // Qt 2's validator ran the whole string through strtod and required all of
    // it to be consumed, so validate() returned Invalid and the keystroke was
    // refused.
    //
    // Asserted through lineEdit()->validator(), the object the locale is
    // pinned on: DISpinBox::validate() is private (DoubleSpinBox.h), and a
    // probe that calls it through a derived struct does not compile.
    //
    // Invalid, not merely "not Acceptable": without RejectGroupSeparator the
    // state is Intermediate, so a test for Acceptable passes either way.
    vcomma("de_DE", QLocale(QLocale::German, QLocale::Germany));
    vcomma("C", QLocale::c());
    QLocale::setDefault(QLocale::c());
    P e(1); e.setRange(1, 0.0, 100.0); e.type("2.5");
    eq("1dp text",  e.text(),                  "2.5");
    eq("1dp value", QString::number(e.value()), "25");
    P i(0); i.setRange(0, 100); i.type("42");
    eq("int text",  i.text(),                  "42");
    return fails ? 1 : 0;
}
DSPEOF
wp=0; wf=0
if g++ -std=c++17 -fPIC $INCS /tmp/dsp.$$.cpp "$SRC/src/MT_GUI/DoubleSpinBox.cpp" \
       -o /tmp/dsp.$$ $(qmake6 -query QT_INSTALL_LIBS 2>/dev/null | sed 's|^|-L|') \
       -lQt6Widgets -lQt6Gui -lQt6Core 2>/tmp/dspb.$$
then
    if QT_QPA_PLATFORM=offscreen /tmp/dsp.$$ 2>/dev/null; then wp=1; else wf=1; fi
else
    wf=1; echo "  DISpinBox check did not build:"; head -5 /tmp/dspb.$$
fi
printf '%-22s %2d pass  %2d fail\n' "widgets" "$wp" "$wf"
pass=$((pass+wp)); fail=$((fail+wf))
rm -f /tmp/dsp.$$ /tmp/dsp.$$.cpp /tmp/dspb.$$

# ---------------------------------------------------------------------------
# SIG_GPPVMHost's parser.
#
# The other gates read only the experiments' own host line, `. 1 1 "."'. This
# probe adds a named host, a path with several parts, and a path with a space.
# Qt 2's QTextStream::operator>>(char&) skipped whitespace and Qt 6's does
# not, which changes how a PVM host line is read; with it wrong, every host
# loses its slave directory.
#
# A heredoc rather than a committed .cpp: it needs no source file of its own,
# and the expectations belong next to the reason they exist.
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
    // The experiments' own line: "." is the local machine, and PVM's search path.
    SIGEL_GP::SIG_GPPVMHost h( ". 1 1 \".\"" );
    eq( "name",      h.name,                       "." );
    eq( "maxSlaves", QString::number(h.maxSlaves), "1" );
    eq( "enabled",   QString::number(h.enabled),   "1" );
    eq( "dir",       h.executableDir.path(),       "." );
    // A named host with a multi-part path, which "." does not cover.
    SIGEL_GP::SIG_GPPVMHost m( "otherbox 2 0 \"/opt/sigel/bin\"" );
    eq( "name (named)",      m.name,                       "otherbox" );
    eq( "maxSlaves (named)", QString::number(m.maxSlaves), "2" );
    eq( "enabled (named)",   QString::number(m.enabled),   "0" );
    eq( "dir (named)",       m.executableDir.path(),       "/opt/sigel/bin" );
    // Qt 2 skipped whitespace on EVERY char read, so it silently dropped spaces
    // inside the quoted path too. That is a 2003 defect and it is preserved.
    SIGEL_GP::SIG_GPPVMHost s( "otherbox 4 0 \"/tmp/with space/sigel\"" );
    eq( "dir (2003 defect)", s.executableDir.path(), "/tmp/withspace/sigel" );
    return fails ? 1 : 0;
}
PVMEOF
pp=0; pf=0
# $INCS is include flags only -- -fsyntax-only lives in $FLAGS and is not used
# here, because this probe must LINK and RUN, not just parse.
if g++ -std=c++17 -fPIC $INCS /tmp/pvm.$$.cpp \
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
# A freed pointer kept behind a guard that still passes.
#
# SIG_Simulation's constructor throws when SIMULATIONLIBRARY names the removed
# Dynamo backend. In visualizeThis() that throw leaves BETWEEN
# `delete visualisation;' and the assignment that replaces it, so without an
# explicit null the member keeps the pointer it just freed. The
# `if (visualisation)' guards then pass, and because the base destructor
# (SIGEL_CommonGUI/SIG_VisualisationWidget.cpp, ~SIG_VisualisationWidget) also
# deletes `visualisation', destroying the widget afterwards is a double free.
#
# Checked because the null LOOKS redundant two lines above
# `visualisation = new ...' and reads like tidying.
#
# Rule: exactly ONE delete of that member, in any spelling the regex can reach,
# followed by a null. Not visible to this check: a delete through an ALIAS
# (`SIG_SimulationVisualisation *v = visualisation; delete v;'). No grep can
# see that; it needs the compiler or a human.
#
# A `delete visualisation' in this widget's OWN destructor would fail here, and
# that is correct rather than a false alarm: the base destructor already frees
# it, so a second one is a double free, not a leak fix.
vp=0; vf=0
vsrc=$SRC/src/SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.cpp
# `//' comments are stripped and newlines flattened before matching: the
# delete and the null sit on separate lines, and a line-based grep would not
# see the pair.
# LC_ALL=C, so that a byte that is not valid in the caller's locale cannot make
# grep treat the one flattened line as binary and skip it.
vflat=$(LC_ALL=C sed 's://.*::' "$vsrc" | tr '\n' ' ' | tr -s ' ')
vdel=$(printf '%s' "$vflat" | LC_ALL=C command grep -oE \
       'delete *\(* *(this-> *)?visualisation *\)* *;' | wc -l)
vnul=$(printf '%s' "$vflat" | LC_ALL=C command grep -oE \
       'delete *\(* *(this-> *)?visualisation *\)* *; *(this-> *)?visualisation *= *(nullptr|0|NULL) *;' | wc -l)
if [ "$vdel" -eq 1 ] && [ "$vnul" -eq 1 ]; then
    vp=1
else
    vf=1
    echo "  SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.cpp:"
    echo "    $vdel delete(s) of 'visualisation', $vnul of them followed by a null."
    echo "    Expected exactly 1 of each. SIG_Simulation's constructor throws for the"
    echo "    removed Dynamo backend, so without the null the member keeps a freed"
    echo "    pointer: 14 'if (visualisation)' guards pass, and the base-class"
    echo "    destructor frees it a second time."
fi
printf '%-22s %2d pass  %2d fail\n' "freed-pointer null" "$vp" "$vf"
pass=$((pass+vp)); fail=$((fail+vf))

# ---------------------------------------------------------------------------
# Controls the user cannot see or reach.
#
# A container too small for its children hides them, while every child still
# reports itself present, enabled and correctly sized. No widget-level probe
# sees that. The clipcheck scenario walks every View page and every tab and
# reports any widget whose rect leaves its parent.
#
# The scenario carries its own positive control and FAILS if the control does
# not fire, because "0 clipped" from a check that cannot detect clipping is
# worth nothing. Shrinking the window cannot be the control: the pages carry
# real layouts and reflow instead of clipping, where 1.3 is absolutely
# positioned and does clip. So it moves one real widget outside its parent and
# requires the report.
#
# guidrive is BUILT HERE, before its first use. A failed build leaves the
# previous binary in place, so each section below that uses the build first
# tests that it is current, and fails if it is not.
# `make -q' or `make' with no target answers for `all', which does NOT depend
# on guidrive, so always name the target.
if ! make -s -C "$ROOT" guidrive sigelApp >/tmp/gdb.$$ 2>&1; then
    echo "  guidrive does not build:"
    tail -6 /tmp/gdb.$$ | sed 's/^/    /'
fi
rm -f /tmp/gdb.$$
guidrive_current() {
    make -q --no-print-directory -C "$ROOT" guidrive sigelApp 2>/dev/null && return 0
    echo "  build/guidrive or sigelApp/ is missing or out of date -- this section did NOT"
    echo "  run. Build it with 'make guidrive sigelApp'."
    return 1
}

cp=0; cf=0
if ! guidrive_current; then
    cf=1
elif SIGEL_ROOT="$APP" QT_QPA_PLATFORM=offscreen \
       SIGEL_EXP="$ROOT/experiments/twoBases.exp" \
       timeout 300 "$ROOT/build/guidrive" clipcheck >/tmp/clip.$$ 2>/dev/null; then
    cp=1
else
    cf=1
    sed -n '/CLIPPED\|selftest/p' /tmp/clip.$$ | sed 's/^/  /'
fi
rm -f /tmp/clip.$$
printf '%-22s %2d pass  %2d fail\n' "no clipped controls" "$cp" "$cf"
pass=$((pass+cp)); fail=$((fail+cf))

# ---------------------------------------------------------------------------
# Forms that can be dragged smaller than Qt 6 can lay them out.
#
# clipcheck walks the widgets at one size. This asks what the smallest
# permitted size IS. An explicit <minimumSize> in a .ui becomes
# setMinimumSize(), which overrides minimumSizeHint(), so a form whose declared
# minimum is below what its layout needs can be dragged down until its
# children compress, and a walk at the default size sees nothing wrong.
# MT_StatisticsWidgetBase sits in the MetaTrainer's QSplitter, where a user can
# really drag it that small.
#
# All twenty forms are measured, and the scenario asserts that there are 20,
# so a shortened list cannot pass by testing nothing.
#
# The scenario's selftest forces one form's minimum below its hint and
# requires the same comparison to report it, so "TOO SMALL: 0" comes from a
# check that can fire. To test this row: set MT_StatisticsWidgetBase's
# minimum to 220x390; the row then fails with "TOO SMALL: 1".
#
# A stale binary, a timeout, a killed process, a failed run and a wrong form
# count each get their own message. Stderr is kept, because a crash or a Qt
# fatal shows up there and nowhere else.
mp=0; mf=0
if ! guidrive_current; then
    mf=1
else
    # `|| mrc=$?', not a bare run followed by `mrc=$?': under set -e a
    # non-zero formsize would stop the whole script here, with no row and no
    # total.
    #
    # guidrive returns 0 only when it measured 20 forms, so the `ngot != nui'
    # branch below fires on a 21st .ui that the scenario's table lacks.
    mrc=0
    SIGEL_ROOT="$APP" QT_QPA_PLATFORM=offscreen \
      SIGEL_EXP="$ROOT/experiments/twoBases.exp" \
      SIGEL_SCRATCH="${TMPDIR:-/tmp}" \
      timeout 300 "$ROOT/build/guidrive" formsize >/tmp/fmin.$$ 2>/tmp/fmerr.$$ \
      || mrc=$?
    # The .ui files on disk are the independent source. The scenario's form
    # table is a HAND list, so "20 forms" asserted against itself catches only
    # shrinkage; against the directory it also catches a 21st form that nobody
    # added to the table.
    nui=$(find "$SRC/ui" -name '*.ui' | wc -l)
    ngot=$(sed -n 's/.*of \([0-9]*\) forms.*/\1/p' /tmp/fmin.$$ | tail -1)
    if [ "$mrc" = 124 ]; then
        mf=1
        echo "  formsize was killed at 300 s -- it did not finish, so its result"
        echo "  means nothing:"
        tail -4 /tmp/fmin.$$ | sed 's/^/    /'
    elif [ "$mrc" -gt 128 ]; then
        # A killed process is not a form defect: 137 is SIGKILL, often the OOM
        # killer, and 139 is SIGSEGV.
        mf=1
        echo "  formsize was KILLED by signal $((mrc-128)) -- it did not finish,"
        echo "  so its result means nothing. 137 is SIGKILL: CHECK FREE MEMORY"
        echo "  FIRST, a check.sh run has already been OOM-killed on this box."
        [ -s /tmp/fmerr.$$ ] && { echo "  its stderr said:"
                                  tail -5 /tmp/fmerr.$$ | sed 's/^/    /'; }
    elif [ "$mrc" != 0 ]; then
        mf=1
        echo "  a form declares a minimum below what Qt 6 needs to lay it out,"
        echo "  or too few forms reached the comparison, or the check cannot"
        echo "  detect either (exit $mrc):"
        sed -n '/TOO SMALL:/p;/selftest/p;/^!!/p' /tmp/fmin.$$ | sed 's/^/    /'
        [ -s /tmp/fmerr.$$ ] && { echo "  and its stderr said:";
                                  tail -5 /tmp/fmerr.$$ | sed 's/^/    /'; }
    elif [ -z "$ngot" ] || [ "$ngot" != "$nui" ]; then
        mf=1
        echo "  formsize measured [$ngot] forms but $nui .ui files exist --"
        echo "  a form was added to ui/ and not to the scenario's table, or the"
        echo "  scenario printed nothing this section could read."
    else
        mp=1
    fi
fi
rm -f /tmp/fmin.$$ /tmp/fmerr.$$
printf '%-22s %2d pass  %2d fail\n' "form minimums" "$mp" "$mf"
pass=$((pass+mp)); fail=$((fail+mf))

# The same check for the slave's simulation window and its movie-settings
# dialog. clipcheck cannot reach either: they belong to the slave, not to
# anything the master's menus open, so the `slavegui' scenario walks them. The
# two containers that clipped there are GroupBox6 "Navigation" and
# groupboxDirectory "File conventions"; their <minimumSize> blocks prevent it.
#
# This reads the two totals and does not diff the whole scenario. The rest of
# slavegui's output covers a GL view that does not render offscreen, and would
# put noise in a baseline. To test this row: delete either <minimumSize> block
# from its .ui and rebuild; the matching line becomes non-zero and the row
# fails.
#
# Stderr is KEPT, because this is the only run of SIGEL_SlaveGUI in the gate,
# and so the only runtime check of its string-based connects. guidrive makes
# one deliberately bogus connect at startup; if its warning is absent, the
# connect logging was suppressed and an empty stderr proves nothing. The
# `gui behaviour' section explains this control in full.
sp=0; sf=0
if ! guidrive_current; then
    sf=1
elif SIGEL_ROOT="$APP" QT_QPA_PLATFORM=offscreen \
       SIGEL_EXP="$ROOT/experiments/twoBases.exp" \
       SIGEL_SCRATCH="${TMPDIR:-/tmp}" \
       timeout 300 "$ROOT/build/guidrive" slavegui >/tmp/sclip.$$ 2>/tmp/serr.$$; then
    # Both lines must be PRESENT and read 0. A missing line is a walk that did
    # not happen, which is the "0 clipped from a check that never ran" shape.
    got=$(sed -n 's/.*\(slave window\|movie settings dialog\) *[0-9]* visible widgets, \([0-9]*\) clipped.*/\1=\2/p' /tmp/sclip.$$)
    if [ "$(printf '%s\n' "$got" | wc -l)" -ne 2 ] || printf '%s\n' "$got" | command grep -qv '=0$'; then
        sf=1
        echo "  the slave window or the movie dialog clips a control, or the walk"
        echo "  did not run -- both lines must be present and read 0:"
        printf '%s\n' "$got" | sed 's/^/    /'
    elif ! command grep -q guidriveStderrControl /tmp/serr.$$; then
        sf=1
        echo "  Qt's connect logging is SUPPRESSED -- SIGEL_SlaveGUI's connects"
        echo "  were not checked. Unset QT_LOGGING_RULES and re-run."
    elif command grep -E 'No such (signal|slot)' /tmp/serr.$$ \
             | command grep -qv guidriveStderrControl; then
        sf=1
        echo "  a connect in SIGEL_SlaveGUI names a signal or slot that does not exist:"
        command grep -E 'No such (signal|slot)' /tmp/serr.$$ \
            | command grep -v guidriveStderrControl | sort -u | head -4 | sed 's/^/    /'
    else
        sp=1
    fi
else
    sf=1
    echo "  slavegui did not finish; its clip walk proves nothing:"
    sed -n '/clipped\|!!/p' /tmp/sclip.$$ | sed 's/^/    /'
fi
rm -f /tmp/sclip.$$ /tmp/serr.$$
printf '%-22s %2d pass  %2d fail\n' "slave gui" "$sp" "$sf"
pass=$((pass+sp)); fail=$((fail+sf))

# ---------------------------------------------------------------------------
# expstruct.py's self-test.
#
# expstruct.py compares evolved .exp files across machines by structure. It
# must be blind to fitness and see structure. Fitness is not comparable across
# machines: a 1-ULP change in start height moves fitness by 45 %, and the
# reference machine is i386/x87 where this one is aarch64. The tournament that
# picks survivors is a bare `>=' between two such doubles
# (SIG_GPSimpleTournament.cpp, run). A tool that let fitness into its report
# would report an unavoidable difference as a regression.
#
# --selfcheck asserts BOTH halves, because a tool that saw nothing would pass
# the fitness half alone: a changed FITNESS value must not move the report,
# while a changed program operand and two swapped individuals must. It
# compares the WHOLE report, SHAPE included.
ep=0; ef=0
if python3 "$ROOT/checks/programs/expstruct.py" --selfcheck \
       "$ROOT/experiments/twoBases.exp" >/tmp/eps.$$ 2>&1; then
    ep=1
else
    ef=1
    sed 's/^/  /' /tmp/eps.$$
fi
rm -f /tmp/eps.$$
printf '%-22s %2d pass  %2d fail\n' "expstruct selfcheck" "$ep" "$ef"
pass=$((pass+ep)); fail=$((fail+ef))

# ---------------------------------------------------------------------------
# The two programs, sigel and sigel_slave. They are src/*.cpp, so the module
# loop does not reach them.
pp=0; pf=0
for prog in sigel sigel_slave; do
    if g++ $FLAGS $INCS "$SRC/src/$prog.cpp" 2>/tmp/prog.$$; then
        pp=$((pp+1))
    else
        pf=$((pf+1)); echo "  $prog.cpp does not compile:"; head -5 /tmp/prog.$$
    fi
done
rm -f /tmp/prog.$$

# Compiling is not enough: a missing moc, an unemitted vtable or a resource
# dropped from a static archive shows up only at link. So the two binaries
# must also be BUILT and CURRENT, and sigel_slave, which can test itself
# headlessly, is run.
#
# Missing binaries fail rather than skip: a check that passes when the thing
# it checks is missing tests nothing.
if make -q --no-print-directory -C "$ROOT" programs sigelApp 2>/dev/null; then
    for prog in sigel sigel_slave; do
        f=$ROOT/build/$prog
        if [ ! -x "$f" ]; then
            pf=$((pf+1)); echo "  build/$prog missing"
        elif [ "$(head -c 4 "$f" | tail -c 3)" != ELF ]; then
            # A SHELL WRAPPER in place of the binary would pass everything
            # else. It is -x, and it can exec the real binary, so the smoke
            # test below still sees the no-PVM guard. `make -q' calls the
            # target current, because the wrapper is newer than every
            # prerequisite, so the link recipe and its ctor_size assertion
            # never run.
            pf=$((pf+1)); echo "  build/$prog is not an ELF binary"
        else pp=$((pp+1)); fi
    done
    # sigel_slave with no PVM daemon must reach its own guard and print its
    # no-PVM message.
    # sigel needs a display and starts a pvmd, so it is not run here.
    out=$(SIGEL_ROOT="$APP" QT_QPA_PLATFORM=offscreen \
          timeout 60 "$ROOT/build/sigel_slave" 2>&1 </dev/null || true)
    case $out in
        *"hasn't been started as a PVM slave"*) pp=$((pp+1)) ;;
        *) pf=$((pf+1))
           echo "  sigel_slave did not reach its no-PVM guard; it printed:"
           printf '%s\n' "$out" | tail -3 | sed 's/^/    /' ;;
    esac
else
    pf=$((pf+1))
    echo "  the two programs are not built or are out of date --"
    echo "    run 'make programs sigelApp'"
fi
printf '%-22s %2d pass  %2d fail\n' "programs" "$pp" "$pf"
pass=$((pass+pp)); fail=$((fail+pf))

# ---------------------------------------------------------------------------
# The master GUI's structure, against what the running SIGEL 1.3 shows.
#
# Each menu entry and toolbar button, with its shortcut, enabled state and
# check state, against guidump-baseline.txt. The 42 menu entries read off the
# running 1.3 binary match that file, apart from one deliberate divergence
# named in its header. So a difference here is a difference from 1.3.
#
# It sees what compiles, links and runs but is still wrong: a lost
# accelerator, an action that is no longer greyed, a toolbar button that shows
# the long menu label instead of the short one, or an action that is no longer
# checkable.
gp=0; gf=0
# It links the libraries, moc and resource objects that guidrive links, and
# reads its data from sigelApp/ through SIGEL_ROOT, so guidrive_current is
# the test that they are current.
if guidrive_current; then
    cat > /tmp/gui.$$.cpp <<'GUIEOF'
// Headless dump of the GUI's structure, in the shape of guidump-baseline.txt.
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QAction>
#include <cstdio>
#include "SIGEL_MasterGUI/SIG_MainWindow.h"
#include "SIGEL_MasterGUI/SIG_IndividualListItem.h"
#include <QTreeWidget>

// sigel.cpp defines this true, sigel_slave.cpp false; MT_Controller reads it.
bool guiEnabled = true;

static const char *en(const QAction *a) { return a->isEnabled() ? "enabled" : "greyed"; }
static const char *tk(const QAction *a)
{
    if (!a->isCheckable()) return "n/a";
    return a->isChecked() ? "tick" : "no-tick";
}

static void dumpMenu(const QString &path, QMenu *m)
{
    for (QAction *a : m->actions()) {
        if (a->isSeparator()) { printf("%s | --- | - | - | -\n", qPrintable(path)); continue; }
        QString sc = a->shortcut().isEmpty() ? QString("-") : a->shortcut().toString();
        printf("%s | %s | %s | %s | %s\n", qPrintable(path), qPrintable(a->text()),
               qPrintable(sc), en(a), tk(a));
        if (a->menu()) dumpMenu(path + ">" + a->text(), a->menu());
    }
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    SIGEL_MasterGUI::SIG_MainWindow w(nullptr, "MainWindow");
    printf("== WINDOW TITLE ==\n[%s]\n", qPrintable(w.windowTitle()));
    printf("== ICON SIZE ==\n%dx%d\n", w.iconSize().width(), w.iconSize().height());
    printf("== MENUS ==\n");
    for (QAction *top : w.menuBar()->actions())
        if (top->menu()) dumpMenu(top->text(), top->menu());
    printf("== TOOLBARS ==\n");
    for (QToolBar *tb : w.findChildren<QToolBar *>()) {
        printf("-- %s (title [%s]) --\n", qPrintable(tb->objectName()), qPrintable(tb->windowTitle()));
        for (QAction *a : tb->actions()) {
            if (a->isSeparator()) { printf("  ---\n"); continue; }
            printf("  %s | iconText[%s] | tip[%s] | %s | %s\n",
                   qPrintable(a->text()), qPrintable(a->iconText()),
                   qPrintable(a->toolTip()), en(a), tk(a));
        }
    }
    // The individuals list sorts through SIG_IndividualListItem::key(), which
    // zero-pads so that the compare is NUMERIC. The running 1.3 binary sorts
    // these Fitness values with the e-05 values first; as raw text 1.14825
    // would lead. This also covers the exponent branch of key(): Qt 2's
    // truncate() took an unsigned length, Qt 6's takes a signed one, and a
    // negative length clears the string.
    printf("== FITNESS SORT ==\n");
    {
        QTreeWidget t; t.setColumnCount(3);
        const char *fit[] = {"1.14825","2.34536e-05","3.09259e-05","4.19675e-05",
                             "4.42652e-05","5.153e-05","5.6388e-05","0.0029747"};
        int n = 0;
        for (const char *f : fit) {
            SIGEL_MasterGUI::SIG_IndividualListItem *it =
                new SIGEL_MasterGUI::SIG_IndividualListItem(&t);
            it->setText(0, QString::number(50000 + n++));
            it->setText(1, f); it->setText(2, "1");
        }
        t.setSortingEnabled(true); t.sortByColumn(1, Qt::AscendingOrder);
        for (int i = 0; i < t.topLevelItemCount(); ++i)
            printf("%s\n", qPrintable(t.topLevelItem(i)->text(1)));
    }
    return 0;
}
GUIEOF
    if g++ -std=c++17 -O1 -DMINMAX_H $INCS -I"$ROOT/build/ui" /tmp/gui.$$.cpp \
         $(find "$ROOT/build/obj/moc" -name '*.o') \
         "$ROOT/build/obj/qrc/SIG_GPParameterBase.o" -o /tmp/gui.$$ \
         -Wl,--start-group "$ROOT/build"/lib/*.a -Wl,--end-group \
         "$SL/pvm3/lib/LINUX64/libpvm3.a" -ltirpc \
         -L"$QTLIBDIR" -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui \
         -lQt6Core -lGL -lGLU -lm 2>/tmp/guib.$$; then
        SIGEL_ROOT="$APP" QT_QPA_PLATFORM=offscreen timeout 120 /tmp/gui.$$ \
            > /tmp/guio.$$ 2>/dev/null || true
        if command grep -v '^#' "$ROOT/checks/baselines/guidump-baseline.txt" | diff -u - /tmp/guio.$$ > /tmp/guid.$$; then
            gp=1
        else
            gf=1; echo "  the GUI no longer matches what SIGEL 1.3 shows:"
            head -14 /tmp/guid.$$ | sed 's/^/    /'
        fi
    else
        gf=1; echo "  GUI structure probe did not build:"; head -5 /tmp/guib.$$ | sed 's/^/    /'
    fi
    rm -f /tmp/gui.$$ /tmp/gui.$$.cpp /tmp/guib.$$ /tmp/guid.$$ /tmp/guio.$$
else
    gf=1
fi
printf '%-22s %2d pass  %2d fail\n' "gui vs 1.3" "$gp" "$gf"
pass=$((pass+gp)); fail=$((fail+gf))

# ---------------------------------------------------------------------------
# What the GUI DOES, not only what it shows.
#
# `gui vs 1.3' above is static and cannot see a wrong answer to a click. This
# section drives the real SIG_MainWindow with Qt input events: QTest posts
# QMouseEvent, QKeyEvent and QContextMenuEvent through QApplication::notify, so
# hit-testing, menu popups, item-view selection and the slots behind them all
# run. It bypasses QWindowSystemInterface, so window activation, grabs and
# double-click synthesis are not tested here; `real clicks' below covers them.
#
# Where a 1.3 side exists, the behaviour matches the running 1.3 binary;
# roundtrip, metadrive, openfocus and rngseed have none. The values are the
# port's own for twoBases.exp, and guibehaviour-baseline.txt says which fact
# came from where. `gate' exits 1 on an out-of-range pool index after a
# deletion, and its `nameIsASurvivor=0' line covers a stale index that stays
# in range. SURVIVED shows only that the run did not crash.
#
# It needs an experiment to open, so a missing experiment file SKIPS it,
# loudly, and the skip fails the exit status.
bp=0; bf=0
BEXP=$ROOT/experiments/twoBases.exp
if [ ! -f "$BEXP" ]; then
    # experiments/ is tracked, so only a damaged checkout lands here. The skip
    # is counted: `0 pass 0 fail' would drop the section from the total and
    # leave the exit status clean.
    echo "  SKIPPED: no $BEXP -- restore experiments/ from git, then re-run."
    echo "  THIS SECTION TESTED NOTHING."
    skipped=$((skipped+1))
elif make -s -C "$ROOT" guidrive sigelApp >/tmp/bdb.$$ 2>&1; then
    # SIGEL_ROOT is sigelApp/, the folder SIGEL is started from: the driver
    # loads pixmaps and terrain from it. No scenario here spawns a sigel_slave.
    #
    # ELEVEN scenarios make up the baseline, concatenated in this order:
    #   gate       the tree, sorting, add/delete/reset, the dialogs, the
    #              context menus, the MetaGP warning
    #   pages      the five View pages: every spin box, slider, combo,
    #              checkbox and validator on them
    #   exportall  all eight File > Export children: each file's sha256, size,
    #              line count and ends. Seven of the eight were shown
    #              byte-identical to what the 2003 i386 binary writes; the
    #              baseline records why the eighth, .lap, differs.
    #   overwrite  an export over an existing file. A name without the
    #              extension gives a date-stamped file and no prompt. The name
    #              with it raises the file dialog's own confirmation, as a
    #              child of the dialog, and No leaves the file alone.
    #   dialogs    the six dialogs, their validators, and select-on-focus.
    #              Qt 6 selects a pre-filled field when a dialog gives it focus
    #              and Qt 2 did not, so a typed digit would REPLACE the value
    #              instead of appending to it. The load-bearing lines are
    #              `typing "5" gives [0.015]' and `typing "2" gives [12]':
    #              1.3 adds 12 individuals there, not 2.
    #   metagui    the MetaGP window. Its validators are pinned to the C
    #              locale: unpinned, a QIntValidator(0,1000) accepts "1,000"
    #              under en_US while text().toInt() returns 0. 1.3 rejects
    #              both separators.
    #   roundtrip  export, import, export on one machine. The READER must undo
    #              a change made between the two exports, so a reader that
    #              does nothing cannot pass. Breaking any of the five readers
    #              moves roundtrip and not exportall, which never reads back.
    #   metadrive  the MetaGP controls that metagui only lists: MT_Editor,
    #              MT_AddConstantsWidget, `update statistics' and MT_GUI's
    #              toolbar actions, each pressed. Every line is a boolean or a
    #              count, because the constants are random.
    #   runlock    the lock on the parameter pages while an evolution runs,
    #              in its locked state. Nothing else executes it.
    #   rngseed    a new experiment, a GP seed, N individuals added and saved:
    #              pins the random stream with no PVM, simulation or fitness.
    #   openfocus  the flag guard in SIG_ExperimentListView::slotLoadExperiment.
    #              It sends the focus event a window manager sends while the
    #              file is being read. Reverting the guard alone moves
    #              `orphan'; reverting the null checks too makes it segfault,
    #              which fails the run rather than the diff.
    #
    # $1 scenario, $2 outfile, $3.. extra NAME=VALUE for the child only.
    # The extras go through env, not as a prefix on the function call: a
    # prefix would also apply to the SHELL, and bash then prints "warning:
    # setlocale: LC_ALL: cannot change locale" wherever the locale is not
    # generated.
    # Stderr is appended to /tmp/berr.$$ and checked below for Qt's runtime
    # "No such signal" and "No such slot" warnings.
    guidrive_run() {
        local sc="$1" out="$2"; shift 2
        env "$@" SIGEL_ROOT="$APP" SIGEL_EXP="$BEXP" \
            SIGEL_SCRATCH="${TMPDIR:-/tmp}" QT_QPA_PLATFORM=offscreen \
            timeout 300 "$ROOT/build/guidrive" "$sc" > "$out" 2>>/tmp/berr.$$
    }
    # `true', not `:'. In dash a redirection error on the special built-in `:'
    # ends the shell even inside `||', with no row and no total. On `true' it
    # is an ordinary failure that `||' catches.
    true > /tmp/berr.$$ || { bf=1; echo "  cannot create /tmp/berr.$$"; }
    if guidrive_run gate /tmp/bo.$$ && guidrive_run pages /tmp/bp.$$ \
       && guidrive_run exportall /tmp/bx.$$ && guidrive_run overwrite /tmp/bw.$$ \
       && guidrive_run dialogs /tmp/bg.$$ && guidrive_run metagui /tmp/bm.$$ \
       && guidrive_run roundtrip /tmp/br.$$ \
       && guidrive_run metadrive /tmp/bv.$$ \
       && guidrive_run runlock /tmp/bk.$$ \
       && guidrive_run rngseed /tmp/bz.$$ \
       && guidrive_run openfocus /tmp/bq.$$; then
        # The GUI shows a slave directory by its physical path; strip the repo's.
        cat /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ \
            /tmp/br.$$ /tmp/bv.$$ /tmp/bk.$$ /tmp/bz.$$ /tmp/bq.$$ \
            | awk -v r="$(cd "$ROOT" && pwd -P)" '{ while ((i = index($0, r)) > 0)
                  $0 = substr($0, 1, i - 1) "<REPOROOT>" substr($0, i + length(r)); print }' \
            > /tmp/ball.$$
        # THE RUNTIME-CONNECT CHECK AND ITS POSITIVE CONTROL.
        #
        # Qt says "No such signal" or "No such slot" at RUNTIME when a
        # string-based connect names something that does not exist. It
        # compiles and links, and the slot never fires. The compiler cannot see
        # it. This check sees only the connects that the scenarios execute.
        #
        # Qt logs the warning under the category qt.core.qobject.connect, and
        # categories can be switched off: QT_LOGGING_RULES='*=false' in the
        # environment (guidrive_run uses env without -i, so the whole
        # environment passes through) or a qtlogging.ini. An empty stderr then
        # looks exactly like a clean run.
        #
        # So guidrive makes ONE deliberately bogus connect at startup, and this
        # requires its warning. It uses the same category and mechanism; a
        # control in `default' would not do, because qt.core.qobject.connect
        # can be switched off on its own. If the control did not fire, this
        # check could not have fired either, and that is a failure on its own.
        CTL=guidriveStderrControl
        if ! command grep -q "$CTL" /tmp/berr.$$; then
            bf=1
            echo "  Qt's connect logging is SUPPRESSED -- the runtime-connect check"
            echo "  could not have fired, so this run proves nothing about connects."
            echo "  Unset QT_LOGGING_RULES (or remove a qtlogging.ini) and re-run."
        fi
        # NOT an elif chain with the `!!' check below. A dead connect is the
        # most likely CAUSE of a `!!': the driver clicks, the slot never
        # fires, and the driver reports that it could not do the thing.
        # Reporting only the symptom would hide the cause.
        if command grep -E 'No such (signal|slot)' /tmp/berr.$$ \
               | command grep -qv "$CTL"; then
            bf=1
            echo "  a string-based connect names a signal or slot that does not exist:"
            command grep -E 'No such (signal|slot)' /tmp/berr.$$ \
                | command grep -v "$CTL" | sort -u | head -6 | sed 's/^/    /'
            echo "  (it compiles and links; the slot never fires -- see §2's table)"
        fi
        # The driver prints `!!' when it could not do what it was asked -- a
        # dialog that would not accept, a file that never appeared. Such a run
        # must not pass and must never become a baseline. Checked before the
        # diff, so that the message names the right thing.
        if command grep -q '^ *!!' /tmp/ball.$$; then
            bf=1
            echo "  the driver could not carry out part of a scenario:"
            command grep -n '^ *!!' /tmp/ball.$$ | head -6 | sed 's/^/    /'
            echo "  (a run containing these must never be committed as a baseline)"
        elif command grep -q '^ *!!' "$ROOT/checks/baselines/guibehaviour-baseline.txt"; then
            bf=1
            echo "  the BASELINE itself contains a failure marker -- it was"
            echo "  captured from a run that did not complete. Regenerate it:"
            command grep -n '^ *!!' "$ROOT/checks/baselines/guibehaviour-baseline.txt" | head -4 | sed 's/^/    /'
        elif [ "$bf" -ne 0 ]; then
            :   # already failed above; the diff would only add noise
        elif command grep -v '^#' "$ROOT/checks/baselines/guibehaviour-baseline.txt" | diff -u - /tmp/ball.$$ > /tmp/bd.$$; then
            bp=1
        else
            bf=1; echo "  the GUI no longer behaves the way the baseline records:"
            head -16 /tmp/bd.$$ | sed 's/^/    /'
        fi
    else
        bf=1
        echo "  the driver did not finish -- it exits(1) on an out-of-range pool"
        echo "  position, which is how the Qt 6 clear() regression showed up:"
        tail -6 /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ \
             /tmp/br.$$ /tmp/bv.$$ /tmp/bk.$$ /tmp/bz.$$ /tmp/bq.$$ 2>/dev/null | sed 's/^/    /'
        # Stderr is most likely to say why on this path: a crash or a Qt fatal
        # lands there.
        if [ -s /tmp/berr.$$ ]; then
            echo "  and the driver's stderr said:"
            tail -8 /tmp/berr.$$ | sed 's/^/    /'
        fi
    fi

    # The pages again, under comma-decimal locales. The output must be
    # IDENTICAL to the run above, so no baseline of its own is needed.
    #
    # Qt 2 forced LC_NUMERIC="C" process-wide and its QDoubleValidator
    # hard-coded '.'. Qt 6 validators follow the system locale, while the
    # read-back is QString::toDouble(), which does not. So the validators are
    # pinned to QLocale::c() with RejectGroupSeparator; unpinned, a typed
    # "9,81" validates under a comma locale and reads back as ZERO, and the
    # saved experiment silently loses it. The 2003 binary was measured under
    # de_DE and is locale-independent, so this is 1.3 behaviour to keep.
    #
    # TWO locales, because each covers one half. de_DE tests Qt's half:
    # QLocale reads the environment directly and reports a comma decimal even
    # where no de_DE locale is generated. But setlocale() then fails and
    # LC_NUMERIC stays "C", so libc's half is not touched. en_DK is a
    # comma-decimal locale that is installed here (`locale -a`), and under it
    # libc really does switch: a stray sprintf("%f") or strtod in a reader or
    # writer shows up only there. Without en_DK only Qt's half runs, and the
    # script says so.
    LOCTEST=en_DK.utf8
    locale -a 2>/dev/null | command grep -qx "$LOCTEST" || LOCTEST=
    if [ "$bf" = 0 ]; then
        if guidrive_run pages /tmp/bl.$$ LANG=de_DE.UTF-8 LC_ALL=de_DE.UTF-8 \
           && diff -q /tmp/bp.$$ /tmp/bl.$$ >/dev/null \
           && { [ -z "$LOCTEST" ] || { guidrive_run pages /tmp/bl2.$$ \
                  LANG="$LOCTEST" LC_ALL="$LOCTEST" \
                  && diff -q /tmp/bp.$$ /tmp/bl2.$$ >/dev/null; }; }; then
            [ -n "$LOCTEST" ] || echo "  note: no installed comma-decimal locale;" \
                "only Qt's half of the locale check ran"
        else
            bf=1; bp=0
            echo "  the pages BEHAVE DIFFERENTLY under a comma-decimal locale;"
            echo "  C7's validator locale pinning is what stops that, and 1.3"
            echo "  was measured locale-independent on the running binary:"
            { diff /tmp/bp.$$ /tmp/bl.$$; diff /tmp/bp.$$ /tmp/bl2.$$; } 2>/dev/null \
                | head -12 | sed 's/^/    /'
        fi
    fi
else
    bf=1; echo "  guidrive did not build:"; head -5 /tmp/bdb.$$ | sed 's/^/    /'
fi
rm -f /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ /tmp/br.$$ \
      /tmp/bv.$$ /tmp/bk.$$ /tmp/bz.$$ /tmp/bq.$$ \
      /tmp/bl.$$ /tmp/bl2.$$ /tmp/ball.$$ \
      /tmp/bd.$$ /tmp/bdb.$$ /tmp/berr.$$
# exportall, overwrite, roundtrip, dialogs and rngseed write files to TMPDIR,
# several MB in all. They are removed here.
rm -f "${TMPDIR:-/tmp}"/x11b-gpp.gpp "${TMPDIR:-/tmp}"/x11b-sip.sip \
      "${TMPDIR:-/tmp}"/x11b-lap.lap "${TMPDIR:-/tmp}"/x11b-env.env \
      "${TMPDIR:-/tmp}"/x11b-pop.pop "${TMPDIR:-/tmp}"/x11b-prg.prg \
      "${TMPDIR:-/tmp}"/x11b-ind.ind "${TMPDIR:-/tmp}"/x11b-dat.dat \
      "${TMPDIR:-/tmp}"/x11b-ow.sip "${TMPDIR:-/tmp}"/x11b-ow "${TMPDIR:-/tmp}"/x11b-ow-*.sip \
      "${TMPDIR:-/tmp}"/c11c-lap.lap "${TMPDIR:-/tmp}"/rngseed.exp
for e in gpp sip lap env pop; do
    rm -f "${TMPDIR:-/tmp}/rt-a-$e.$e" "${TMPDIR:-/tmp}/rt-b-$e.$e" \
          "${TMPDIR:-/tmp}/rt-a-$e"    "${TMPDIR:-/tmp}/rt-b-$e"
done
rm -f "${TMPDIR:-/tmp}"/rt-prg-a.prg "${TMPDIR:-/tmp}"/rt-prg-b.prg \
      "${TMPDIR:-/tmp}"/rt-prg-a     "${TMPDIR:-/tmp}"/rt-prg-b \
      "${TMPDIR:-/tmp}"/rt-ind-a.ind "${TMPDIR:-/tmp}"/rt-ind-a \
      "${TMPDIR:-/tmp}"/rt-rob-before.lap "${TMPDIR:-/tmp}"/rt-rob-after.lap \
      "${TMPDIR:-/tmp}"/rt-rob-before     "${TMPDIR:-/tmp}"/rt-rob-after
printf '%-22s %2d pass  %2d fail\n' "gui behaviour" "$bp" "$bf"
pass=$((pass+bp)); fail=$((fail+bf))

# ---------------------------------------------------------------------------
# REAL X INPUT -- the only section that clicks through the X server.
#
# The guidrive scenarios above post QMouseEvent through QApplication::notify.
# That reaches every slot SIGEL has, but never goes through
# QWindowSystemInterface. So three things are untested above: window
# activation, the pointer grab a popup takes, and Qt's synthesis of a double
# click from two presses. Enter and leave ARE covered above: QTest::mouseMove
# on a widget calls QCursor::setPos(), a real pointer warp.
#
# This section runs guidrive as a REAL X11 CLIENT on a separate Xvfb server
# with QT_QPA_PLATFORM=xcb, and drives it with XTEST through xdotool.
#
# ITS POSITIVE CONTROL IS INSIDE THE SCENARIO AND IS CHECKED HERE. If the
# clicks are not real -- a broken xdotool, a display that never came up, or a
# fallback to a synthetic path -- every count is zero, which looks exactly like
# "the port ignores real clicks". So the scenario compares one real click with
# one QTest::mouseClick at the same point through a native event filter, and
# prints DISCRIMINATES only when the real one produced native ButtonPress
# events and QTest produced none. Without that line this row fails.
#
# A missing Xvfb or xdotool FAILS this row rather than skipping it: a skip
# would be a section that tested nothing and reported no failure.
xtp=0; xtf=0
XTDISP=:97
if [ ! -f "$ROOT/checks/baselines/xtest-baseline.txt" ]; then
    xtf=1; echo "  checks/baselines/xtest-baseline.txt is missing -- this gate tested NOTHING"
elif [ ! -f "$BEXP" ]; then
    # Same data dependency as `gui behaviour': without the file the run hangs
    # in the modal Load dialog until the timeout, and the message would blame
    # the driver for a missing file.
    echo "  SKIPPED: no $BEXP -- restore experiments/ from git. THIS SECTION"
    echo "  TESTED NOTHING."
    skipped=$((skipped+1))
elif ! command -v Xvfb >/dev/null 2>&1 || ! command -v xdotool >/dev/null 2>&1 \
     || ! command -v xdpyinfo >/dev/null 2>&1; then
    xtf=1
    echo "  Xvfb, xdotool or xdpyinfo is not installed, so no real click could be"
    echo "  delivered and the only section that tests the platform layer did"
    echo "  not run. Install x11-utils/xvfb and xdotool, or delete this section"
    echo "  deliberately -- do not leave it passing silently."
elif [ ! -x "$ROOT/build/guidrive" ]; then
    xtf=1; echo "  guidrive is not built -- the real-input section tested NOTHING"
elif ! (cd "$ROOT" && make -q guidrive sigelApp) 2>/dev/null; then
    # NAME THE TARGET: `make -q' with no target answers for `all', which does
    # not depend on guidrive. `gui behaviour' builds guidrive but fails only
    # its own row if that build fails, so without this test a compile failure
    # leaves the PREVIOUS binary in place and this row would score it.
    xtf=1
    echo "  build/guidrive or sigelApp/ is out of date, so this section would have"
    echo "  measured a binary that is not the source in the tree."
elif true; then
    # A display already in use would make every click land in someone else's
    # session, so refuse rather than share one.
    #
    # DISPLAY=, NOT --display. xdotool has no --display option: it answers
    # "getdisplaygeometry: unrecognized option" and exits 1 whatever the state
    # of the server. With --display this guard could never fire, and the
    # readiness poll below could never succeed.
    if DISPLAY="$XTDISP" xdotool getdisplaygeometry >/dev/null 2>&1; then
        xtf=1
        echo "  display $XTDISP is already in use -- refusing to drive it."
    else
        Xvfb "$XTDISP" -screen 0 1400x1000x24 -nolisten tcp >/tmp/xtv.$$ 2>&1 &
        xtpid=$!
        # Poll rather than sleep a fixed time: too short is a flake and too
        # long is dead time on every run. xdotool is already required above,
        # so this adds no new dependency.
        xtup=0; xti=0
        while [ "$xti" -lt 60 ]; do
            if DISPLAY="$XTDISP" xdotool getdisplaygeometry >/dev/null 2>&1; then
                xtup=1; break
            fi
            xti=$((xti+1)); command sleep 0.25
        done
        if [ "$xtup" -eq 0 ]; then
            xtf=1
            echo "  Xvfb never came up on $XTDISP:"
            head -5 /tmp/xtv.$$ 2>/dev/null | sed 's/^/    /'
        else
            # SCRUB THE SCALING VARIABLES. This is the only section whose
            # result depends on Qt's coordinate scaling. mapToGlobal() returns
            # logical pixels and xdotool takes device pixels, so at a ratio of
            # 1.25 every click is real but lands 20 per cent away, and the run
            # reports a false difference. env is used without -i, so the
            # caller's whole environment passes through. Offscreen pins the
            # ratio, which is why no other section needs this. guidrive also
            # refuses a devicePixelRatio other than 1, so a caller who runs it
            # directly is covered too.
            if env DISPLAY="$XTDISP" SIGEL_ROOT="$APP" SIGEL_EXP="$BEXP" \
                   SIGEL_SCRATCH="${TMPDIR:-/tmp}" QT_QPA_PLATFORM=xcb \
                   QT_SCALE_FACTOR=1 QT_SCREEN_SCALE_FACTORS= \
                   QT_ENABLE_HIGHDPI_SCALING=0 QT_AUTO_SCREEN_SCALE_FACTOR=0 \
                   QT_FONT_DPI= QT_SCALE_FACTOR_ROUNDING_POLICY=Round \
                   timeout 300 "$ROOT/build/guidrive" xtest \
                   > /tmp/xt.$$ 2>/tmp/xterr.$$; then
                # The control, before the diff: if the run could not tell a real
                # click from a QTest one, the numbers below mean nothing and the
                # baseline would match a run in which nothing was
                # clicked at all.
                if ! command grep -q 'DISCRIMINATES' /tmp/xt.$$; then
                    xtf=1
                    echo "  the real/QTest control did NOT fire, so this run proves"
                    echo "  nothing about real input -- no click here was real:"
                    command grep -E 'native ButtonPress|platform' /tmp/xt.$$ \
                        | head -3 | sed 's/^/    /'
                elif command grep -q '^ *!!' /tmp/xt.$$; then
                    xtf=1
                    echo "  the driver could not carry out part of the scenario:"
                    command grep -n '^ *!!' /tmp/xt.$$ | head -6 | sed 's/^/    /'
                elif command grep -q '^ *!!' "$ROOT/checks/baselines/xtest-baseline.txt"; then
                    xtf=1
                    echo "  the BASELINE itself contains a failure marker:"
                    command grep -n '^ *!!' "$ROOT/checks/baselines/xtest-baseline.txt" | head -4 | sed 's/^/    /'
                elif command grep -v '^#' "$ROOT/checks/baselines/xtest-baseline.txt" \
                        | diff -u - /tmp/xt.$$ > /tmp/xtd.$$; then
                    xtp=1
                else
                    xtf=1
                    echo "  the port no longer responds to REAL input the way it did:"
                    head -16 /tmp/xtd.$$ | sed 's/^/    /'
                fi
            else
                xtf=1
                echo "  the xtest run did not finish:"
                tail -6 /tmp/xt.$$ 2>/dev/null | sed 's/^/    /'
                if [ -s /tmp/xterr.$$ ]; then
                    echo "  and its stderr said:"
                    tail -6 /tmp/xterr.$$ | sed 's/^/    /'
                fi
            fi
        fi
        # By pid, never `pkill Xvfb': a real session on this machine may have
        # one of its own, and the gates are not allowed to take it down.
        kill "$xtpid" 2>/dev/null || true
        wait "$xtpid" 2>/dev/null || true
        rm -f /tmp/xtv.$$ /tmp/xt.$$ /tmp/xtd.$$ /tmp/xterr.$$
    fi
fi
printf '%-22s %2d pass  %2d fail\n' "real clicks" "$xtp" "$xtf"
pass=$((pass+xtp)); fail=$((fail+xtf))

# ---------------------------------------------------------------------------
# The widget-to-file path: File > Save Experiment.
#
# `pages' proves typing reaches the widgets. `exportall' proves widgets reach a
# file, in one direction, for the eight export formats. Neither writes the
# experiment file, so this section covers what the pages put into a saved
# .exp.
#
# pagesave-baseline.txt holds the port's own save of twoBases.exp. On another
# experiment, the port's save matched the 2003 i386 binary's byte for byte.
# The EDITED half is the base plus eleven page edits; the file's header says
# which.
#
# LanguageParameters is checked separately because it is NOT in the block: it
# sits far below POPULATION BEGIN{ in the saved file, so a check over the block
# alone would miss the registers edit.
pp=0; pf=0
PSD="${TMPDIR:-/tmp}"
if [ ! -f "$ROOT/checks/baselines/pagesave-baseline.txt" ]; then
    pf=1; echo "  checks/baselines/pagesave-baseline.txt is missing -- this gate tested NOTHING"
elif [ ! -f "$BEXP" ]; then
    # Same data dependency and the same policy as `gui behaviour'. Without
    # this the two runs HANG in the modal Load dialog until the 300 s timeout,
    # and the message would blame the driver for a missing file.
    echo "  SKIPPED: no $BEXP -- restore experiments/ from git. THIS SECTION"
    echo "  TESTED NOTHING."
    skipped=$((skipped+1))
elif guidrive_current; then
    # Same env as guidrive_run above, minus the locale extras. The two runs
    # differ only in SIGEL_PAGEEDIT, which selects the eleven-edit set.
    psrun() {
        env ${2:+SIGEL_PAGEEDIT=1} SIGEL_ROOT="$APP" SIGEL_EXP="$BEXP" \
            SIGEL_SCRATCH="$PSD" QT_QPA_PLATFORM=offscreen \
            timeout 300 "$ROOT/build/guidrive" pagesave > "$1" 2>>/tmp/pserr.$$
    }
    rm -f "$PSD/pagesave-base.exp" "$PSD/pagesave-edited.exp"
    true > /tmp/pserr.$$ || { pf=1; echo "  cannot create /tmp/pserr.$$"; }
    if psrun /tmp/ps1.$$ && psrun /tmp/ps2.$$ 1; then
        if [ ! -s "$PSD/pagesave-base.exp" ] || [ ! -s "$PSD/pagesave-edited.exp" ]; then
            pf=1
            echo "  a pagesave run wrote no experiment file -- nothing was compared:"
            ls -l "$PSD/pagesave-base.exp" "$PSD/pagesave-edited.exp" 2>&1 | sed 's/^/    /'
        elif command grep -q '^ *!!' /tmp/ps1.$$ /tmp/ps2.$$; then
            pf=1
            echo "  the driver could not carry out part of the save:"
            command grep -h '^ *!!' /tmp/ps1.$$ /tmp/ps2.$$ | head -4 | sed 's/^/    /'
        elif ! command grep -q guidriveStderrControl /tmp/pserr.$$; then
            # Same positive control as the gui behaviour section: without it an
            # empty stderr is indistinguishable from a clean run.
            pf=1
            echo "  Qt's connect logging is SUPPRESSED -- the save path's connects"
            echo "  were not checked. Unset QT_LOGGING_RULES and re-run."
        elif command grep -E 'No such (signal|slot)' /tmp/pserr.$$ \
                 | command grep -qv guidriveStderrControl; then
            # A string-based connect on the save path that names something Qt 6
            # does not have. It is checked on this run's own stderr.
            pf=1
            echo "  a connect on the save path names a signal or slot that does not exist:"
            command grep -E 'No such (signal|slot)' /tmp/pserr.$$ \
                | command grep -v guidriveStderrControl | sort -u | head -4 | sed 's/^/    /'
        else
            # Everything before POPULATION BEGIN{ is the parameter block the
            # five View pages own. Raw bytes, not key/value pairs. Two keys in
            # this data (FLOORPICTUREFILE, TEXTUREFILE) have an EMPTY value
            # line after them. Anything that skips blank lines reads the next
            # key as a value and goes out of step from there on.
            # `|| true' on both greps is LOAD-BEARING. Under set -e with no
            # trap, a grep that matches NOTHING exits 1 and stops the whole
            # script here, with no row and no total -- and a missing
            # LanguageParameters line is exactly the regression this looks
            # for. The empty output then fails the diff, which is right.
            {
                echo "== BASE BLOCK =="
                awk '/^POPULATION BEGIN\{/{exit} {print}' "$PSD/pagesave-base.exp"
                echo "== BASE LanguageParameters =="
                command grep -h '^LanguageParameters' "$PSD/pagesave-base.exp" || true
                echo "== EDITED BLOCK =="
                awk '/^POPULATION BEGIN\{/{exit} {print}' "$PSD/pagesave-edited.exp"
                echo "== EDITED LanguageParameters =="
                command grep -h '^LanguageParameters' "$PSD/pagesave-edited.exp" || true
            } > /tmp/psall.$$
            # NOT `grep -v ^#': the DATA contains `#####' separator lines.
            # Header comments are `# text' or a bare `#', so this removes
            # those and nothing else.
            if sed '/^# /d; /^#$/d' "$ROOT/checks/baselines/pagesave-baseline.txt" \
                   | diff -u - /tmp/psall.$$ > /tmp/psd.$$; then
                pp=1
            else
                pf=1
                echo "  what the pages WRITE no longer matches the baseline:"
                head -14 /tmp/psd.$$ | sed 's/^/    /'
            fi
        fi
    else
        pf=1
        # SAY WHY: sizes, the tail of each output and the stderr, so that an
        # intermittent failure leaves evidence.
        echo "  a pagesave run did not finish. Exit codes and sizes:"
        for f in /tmp/ps1.$$ /tmp/ps2.$$; do
            if [ -f "$f" ]; then
                printf '    %s: %s bytes\n' "$f" "$(wc -c < "$f")"
            else
                printf '    %s: MISSING\n' "$f"
            fi
        done
        tail -4 /tmp/ps1.$$ /tmp/ps2.$$ 2>/dev/null | sed 's/^/    /'
        if [ -s /tmp/pserr.$$ ]; then
            echo "  the driver's stderr said:"
            tail -12 /tmp/pserr.$$ | sed 's/^/    /'
        else
            echo "  and its stderr was EMPTY -- no flush, no watchdog line, nothing."
            echo "  CHECK FREE MEMORY FIRST: this box has 7.2 GiB and a desktop on"
            echo "  it, and a check.sh run has already been OOM-killed here. SIGKILL"
            echo "  gives exactly these symptoms. Only if memory was fine is the"
            echo "  other reading worth chasing -- the watchdog is a QTimer, so its"
            echo "  silence would then mean the event loop was blocked and the 300 s"
            echo "  timeout did the killing."
        fi
    fi
    rm -f "$PSD/pagesave-base.exp" "$PSD/pagesave-edited.exp"
else
    pf=1
fi
rm -f /tmp/ps1.$$ /tmp/ps2.$$ /tmp/psall.$$ /tmp/psd.$$ /tmp/pserr.$$
printf '%-22s %2d pass  %2d fail\n' "pagesave" "$pp" "$pf"
pass=$((pass+pp)); fail=$((fail+pf))


# ---------------------------------------------------------------------------
# The truncated pi, and the constants the sensor path is built on.
#
# SIGEL 1.3 converts radians to degrees with pi TRUNCATED TO EIGHT DECIMALS:
# not 180/pi but 180/3.14159265, which is 57.29577957855229 against the true
# 57.295779513082323, a relative error of 1.14e-09.
#
# This is not a defect to fix; it is behaviour to keep. Every evolved program
# in the shipped experiments was selected against sensor values that carry
# this error, and the values feed a chaotic simulation, so "correcting" it
# changes what the robots do. The change is a one-word edit that looks like
# tidying: write M_PI and this row is the only thing that notices.
#
# 1.3 invites the edit, because it is not consistent with itself: it uses the
# true M_PI in SIGEL_Robot/IFunctions.cpp, calculateAnyJoint, and the
# truncated literal in the two simulation files. Both are kept as 1.3 has
# them. A reader who finds the M_PI first will read 3.14159265 as an
# oversight.
#
# The shipped 1.3 binaries do not contain the true constant at all:
#     sigel_slave   404ca5dc1af05a77 (truncated)   1 occurrence
#                   404ca5dc1a63c1f8 (true 180/pi) 0 occurrences
#     sigel         both                           0 occurrences
# The full capture, with the .rodata table and the disassembly, is in
# verification-against-sigel-1.3/v5-1.3-mdh-compared.txt.
#
# Two checks, because neither covers the other.
#   SOURCE  catches an edit at one of the four sites even when another site
#           still supplies the same constant, which a binary search cannot
#           see. It does not depend on the compiler.
#   BINARY  catches any SPELLING that produces the true value -- M_PI,
#           4*atan(1), a longer literal, a header constant -- which a grep for
#           `M_PI' would miss.
#
# Only the radian factor is checked in the binary. The sensor path uses eight
# constants, and all eight match 1.3 as values in the source. In our aarch64
# image, -DBL_MAX, +DBL_MAX, 360.0 and -90.0 appear zero times as 8-byte
# doubles, and 90.0 and 180.0 appear only in debug sections. 2.0 appears so
# often that it is noise. The compiler folds these into immediates or larger
# expressions. So the radian factor is the only one of the eight with a
# .rodata entry to compare, and it is also the only one anybody would edit.
#
# Spelling: our sense writes `360.0 / (2.0*3.14159265)' where 1.3's image
# holds the folded 180/3.14159265. Different expression, IDENTICAL bits:
# 2.0*x is exact, and 360/2x is the same correctly rounded quotient as 180/x.
v5p=0; v5f=0
V5SRC=$SRC/src/SIGEL_Simulation
V5Q=$V5SRC/SIG_DynaMechsSimulationQueries.cpp
V5C=$V5SRC/SIG_DynaMechsCommandInterface.cpp
V5BIN=$ROOT/build/sigel_eval
if [ ! -f "$V5Q" ] || [ ! -f "$V5C" ]; then
    v5f=1; echo "  the two simulation sources are missing -- nothing was checked"
else
    # 4 sites: three in sense (rad->deg) and one in moveDrive (deg->rad).
    # ANCHORED. A bare `3\.14159265' is a PREFIX match. Lengthening one site
    # to 3.14159265358979 would keep the count at 4 but change the factor to
    # 404ca5dc1a63c200. That is neither the kept constant nor a forbidden
    # one, so the binary half would miss it too. The trailing class closes
    # that.
    # `//' comments are stripped before both counts. A note saying "do not
    # change this to M_PI" is documentation, not a defect, and a comment that
    # quotes the literal is not a fifth site.
    v5t=$(sed 's://.*::' "$V5Q" "$V5C")
    v5n=$(printf '%s\n' "$v5t" | command grep -Ec '3\.14159265([^0-9]|$)' || true)
    v5m=$(printf '%s\n' "$v5t" | command grep -c 'M_PI' || true)
    # M_PI FIRST. Changing a site to M_PI also drops the count, so both tests
    # fire; the substitution is the specific message and must be the one
    # printed.
    if [ "$v5m" != 0 ]; then
        v5f=1
        echo "  M_PI has appeared in the simulation sources, where 1.3 uses 3.14159265:"
        command grep -n 'M_PI' "$V5Q" "$V5C" | sed 's/^/    /'
        echo "  This is the one-word edit this section exists to catch. See above."
    elif [ "$v5n" != 4 ]; then
        v5f=1
        echo "  the truncated pi is at $v5n sites in the simulation sources, expected 4:"
        command grep -n '3\.14159265' "$V5Q" "$V5C" | sed 's/^/    /'
        echo "  If a site was legitimately added or removed, move the count"
        echo "  deliberately and say why."
    elif [ ! -x "$V5BIN" ]; then
        v5f=1; echo "  no $V5BIN -- run 'make sigel_eval'"
    elif ! make -q -C "$ROOT" --no-print-directory sigel_eval 2>/dev/null; then
        v5f=1; echo "  $V5BIN is out of date -- run 'make sigel_eval'"
    else
        # .rodata ONLY, and that bound is load-bearing. The Makefile compiles
        # with -g, so the constant also appears in .debug_loclists. Searched
        # over the whole file, the debug copies would keep the count above
        # zero, and the "it is missing" branch could never fire. Debug
        # sections are not what the program computes with.
        v5out=$(python3 - "$V5BIN" <<'V5PY'
import struct, sys, math
d = open(sys.argv[1], 'rb').read()
if d[:4] != b'\x7fELF':
    sys.exit("not an ELF file")
shoff = struct.unpack_from('<Q', d, 0x28)[0]
shent = struct.unpack_from('<H', d, 0x3a)[0]
shnum = struct.unpack_from('<H', d, 0x3c)[0]
shstr = struct.unpack_from('<H', d, 0x3e)[0]
hdr = [struct.unpack_from('<IIQQQQIIQQ', d, shoff + i * shent) for i in range(shnum)]
stro = hdr[shstr][4]
def name(off):
    end = d.index(b'\0', stro + off)
    return d[stro + off:end].decode()
ro = [h for h in hdr if name(h[0]) == '.rodata']
if not ro:
    sys.exit("no .rodata section")
off, size = ro[0][4], ro[0][5]
blob = d[off:off + size]
n = lambda v: blob.count(struct.pack('<d', v))
print("%d %d %d" % (n(180.0/3.14159265), n(180.0/math.pi), n(math.pi/180.0)))
V5PY
) || v5out=""
        if [ -z "$v5out" ]; then
            v5f=1; echo "  the constant search did not run -- nothing was checked"
        else
            v5keep=$(echo "$v5out" | awk '{print $1}')
            v5bad=$(echo "$v5out" | awk '{print $2 + $3}')
            # FORBIDDEN FIRST, for the same reason M_PI is tested before the
            # site count. Replacing the kept constant with the true one does
            # BOTH: it removes the kept value and adds the forbidden one.
            # "The true pi has reached the binary" names the change; "is NOT
            # in" names only the symptom.
            if [ "$v5bad" != 0 ]; then
                v5f=1
                echo "  the TRUE pi has reached the binary: 180/M_PI or M_PI/180 is"
                echo "  present $v5bad time(s) in $V5BIN, and 1.3's own binaries"
                echo "  contain neither. Every joint-sensor reading would shift by"
                echo "  1.14e-09 -- invisible in print, and fed into a chaotic"
                echo "  simulation the shipped programs were evolved against."
            elif [ "$v5keep" -lt 1 ]; then
                v5f=1
                echo "  180/3.14159265 (404ca5dc1af05a77) is NOT in $V5BIN's"
                echo "  .rodata. 1.3's sensor path is built on it."
            else
                v5p=1
            fi
        fi
    fi
fi
printf '%-22s %2d pass  %2d fail\n' "truncated pi (V5)" "$v5p" "$v5f"
pass=$((pass+v5p)); fail=$((fail+v5f))

# ---------------------------------------------------------------------------
# Whole experiments through File > Save Experiment.
#
# The hammer half of the expected report comes from a capture of the running
# 2003 binary: verification-against-sigel-1.3/v8-1.3-gp-blocks.txt. A failure
# in the hammer half's 1.3 lines is a difference from 1.3; the list of kinds
# below says which lines those are. The octopus half is the port's own output.
# What 1.3 does with that robot is in
# verification-against-sigel-1.3/v1-1.3-roundtrip.txt, captured on the same
# robot stored in the other order, which 1.3 writes back as the order
# octopus.exp stores.
#
# Two experiments, because hammer alone proves less than it seems.
#   hammer has 5 links, 4 joints, 4 drives and no sensors, too few to share a
#   hash slot, and 1.3 does not permute its link, joint or drive containers.
#   So agreement there is no evidence. hammer tests material
#   order, `Body' emission order and `middle3''s axis points, plus everything
#   outside the robot: the section line counts, the experiment history, the
#   HISTORY growth defect and the ten first-save keys.
#   octopus supplies the rest. Its joint, drive and sensor containers DO
#   collide: 1.3 permutes all three on every save, and the body order and the
#   command list too, while the port keeps the stored order.
#
# Two saves each. Pass 0 to 1 shows the ten keys arrive; pass 1 to 2 shows
# they hold and gives the steady-state growth.
#
# The input cannot be compared with pass 1: the shipped .exp are a 2001 format
# revision, and the 2003 binary adds ten keys with defaults on the first save.
# So pass 1 is compared with pass 2, and the ten keys are checked by name and
# value.
#
# The two divergences from 1.3 are in the expected text on purpose, not
# filtered out:
#
#   robot block   Ours does not change on a save: byte-identical in all
#                 three passes, on both robots. 1.3's flips between two
#                 orders on every save. The port writes insertion order by
#                 decision: Q2Dict was made insertion-ordered (PORTING.md, D3
#                 under "the shim's users"). A single save cannot see this: it
#                 shows the order written, not whether a second save moves it.
#                 That is why there are two saves and why octopus is here.
#
#   TEXALPHA      99 here, 255 in the v8 capture. Not a port defect: 1.3
#                 writes BOTH values, from its two save paths. Its GUI save
#                 and its headless save of hammer differ in exactly this one
#                 line. SIG_Environment's constructors default texAlpha to
#                 0xFF; SIG_EnvironmentView::getOutOfExperiment pushes it into
#                 sliderAlpha, and putIntoExperiment reads it back. 1.3's
#                 form gives that slider no maximum, so Qt 2 caps it at
#                 QRangeControl's default of 99. Qt 6 has no QRangeControl;
#                 its 0-99 default comes from QAbstractSlider. The headless
#                 path has no slider and keeps 255. Ours is a GUI save, so 99
#                 is 1.3's own GUI value.
#
# Not every line below is 1.3's, and the diff labels say so. Four kinds:
#
#   1.3's own numbers, from the hammer capture: the experiment-history line
#   count and its first and last entry, the first block's character counts,
#   the HISTORY growth and the ten first-save keys.
#
#   1.3's DATA, read back. The individual names are the shipped file's own
#   bytes, so pinning them pins this build against 1.3's file.
#
#   NOT 1.3's: `markers', `pvmhost' and the hammer robot-block hash. The
#   experiment's host block and Body directories are not the shipped ones, so
#   these pin this build against the file in experiments/. The file has one
#   host, so host order is not tested; v8-1.3-gp-blocks.txt keeps the order of
#   1.3's 20 hosts for the file as shipped.
#
#   OURS only: the `expstruct' hash and the whole octopus half. expstruct is
#   kept because it covers the population, which nothing else here reaches.
#   The octopus rows are the stored order, which the port writes back
#   unchanged; v1-1.3-roundtrip.txt quotes the order 1.3 writes.
#
# One diff rather than a dozen ifs, so that a generator which writes the wrong
# text, or stops early, fails on the whole report and not only on the tests
# someone remembered to write. The generator runs inside `|| genok=0', which
# suspends set -e for it. Without that, one failing command in it would stop
# the whole script, with no row and no total.
#
# No line may compare this run only with itself. Three passes change together
# when the INPUT changes, and three hashes of an empty extract are equal when
# the extractor dies. So every "identical in all three" line also prints
# content: a size, a hash, or a first and last entry.
v2p=0; v2f=0
V2HAM=$ROOT/experiments/hammer.exp
V2OCT=$ROOT/experiments/octopus.exp
V2D=${TMPDIR:-/tmp}/v2.$$
if [ ! -f "$V2HAM" ] || [ ! -f "$V2OCT" ]; then
    # Same data dependency and the same policy as the sections above: say
    # SKIPPED loudly and count it, never report 0 pass 0 fail.
    echo "  SKIPPED: no $V2HAM or $V2OCT -- restore experiments/ from git,"
    echo "  then re-run. THIS SECTION TESTED NOTHING."
    skipped=$((skipped+1))
elif [ ! -x "$ROOT/build/guidrive" ]; then
    v2f=1; echo "  build/guidrive is missing -- run 'make guidrive sigelApp'"
elif ! make -q -C "$ROOT" --no-print-directory guidrive sigelApp 2>/dev/null; then
    # A failed make leaves the previous binary in place, so a test for
    # existence passes on a stale one.
    v2f=1; echo "  build/guidrive or sigelApp/ is out of date -- run 'make guidrive sigelApp'"
elif ! mkdir -p "$V2D"; then
    v2f=1; echo "  cannot create $V2D"
else
    # Guarded: a bare `cp && chmod' under `set -e' would abort the whole
    # script rather than fail this section.
    cpok=1
    { cp "$V2HAM" "$V2D/ham0.exp" && chmod u+w "$V2D/ham0.exp" \
      && cp "$V2OCT" "$V2D/oct0.exp" && chmod u+w "$V2D/oct0.exp"; } || cpok=0
    true > "$V2D/err" || cpok=0
    # pagesave with SIGEL_PAGEEDIT unset is exactly File > Open then File >
    # Save Experiment.
    # $1 = stem, $2 = input pass number.
    v2run() {
        rm -f "$V2D/pagesave-base.exp"
        env SIGEL_ROOT="$APP" SIGEL_EXP="$V2D/$1$2.exp" SIGEL_SCRATCH="$V2D" \
            QT_QPA_PLATFORM=offscreen timeout 300 "$ROOT/build/guidrive" \
            pagesave > "$V2D/$1$2.out" 2>>"$V2D/err" \
        && mv "$V2D/pagesave-base.exp" "$V2D/$1$(($2 + 1)).exp"
    }
    v2marks() { command grep -n '^#####' "$1" | cut -d: -f1 | tr '\n' ' ' | sed 's/ $//'; }
    v2hosts() { command grep -A1 '^PVMHOST$' "$1" | command grep -v '^PVMHOST$' \
                | command grep -v '^--$' | awk '{h=h" "$1} END{printf "%d%s", NR, h}'; }
    v2s6()    { awk '/^#####/{n++; next} n==5' "$1" | md5sum | cut -c1-16; }
    v2s6n()   { awk '/^#####/{n++; next} n==5' "$1" | wc -l | tr -d ' '; }
    # $2 = 1 for the first entry, $ for the last.
    v2s6e()   { awk '/^#####/{n++; next} n==5' "$1" | command grep -v '^$' | sed -n "$2p"; }
    v2s5()    { awk '/^#####/{n++; next} n==4' "$1" | md5sum | cut -c1-16; }
    v2s5n()   { awk '/^#####/{n++; next} n==4' "$1" | wc -l | tr -d ' '; }
    v2blocks(){ awk '/HISTORY BEGIN\{/{c=0} c!=""{c++} /}HISTORY END;/{if(c!=""){print c; c=""}}' "$1"; }
    # Byte offsets rather than awk's RS="\0": mawk reads a NUL record separator
    # as one whole-file record, busybox awk reads it as RS="" and returns one
    # number per paragraph. grep -b is the same answer without the dialect.
    # Prefixed names, because a function's variables are global unless
    # declared local: a bare s= and e= here would be visible to every later
    # section.
    v2first() { v2fs=$(command grep -abo 'HISTORY BEGIN{' "$1" | head -1 | cut -d: -f1)
                v2fe=$(command grep -abo '}HISTORY END'  "$1" | head -1 | cut -d: -f1)
                echo $((v2fe - v2fs - 14)); }
    v2names() { command grep -o "NAME='[^']*'" "$1" | md5sum | cut -c1-16; }
    v2namee() { command grep -o "NAME='[^']*'" "$1" | sed -n "$2p"; }
    v2robot() { awk '/^StreamedRobot/{p=1} p{print} /^RobotComplete/{exit}' "$1" | md5sum | cut -c1-16; }
    v2robotn(){ awk '/^StreamedRobot/{p=1} p{print} /^RobotComplete/{exit}' "$1" | wc -l | tr -d ' '; }
    # $2 = the line prefix, $3 = which field on it carries the name.
    v2list()  { command grep "$2" "$1" | awk -v k="$3" '{printf "%s ", $k}' | sed 's/ $//'; }
    v2body()  { command grep -o 'Body [^ ]*' "$1" | awk '{printf "%s ", $2}' | sed 's/ $//'; }
    v2cmds()  { command grep -o '^[A-Z]* CommandParameters' "$1" | awk '{printf "%s ", $1}' | sed 's/ $//'; }
    # A key's number of value lines is declared, not discovered:
    # FLOORDIMENSION has two (X and Z), the other nine have one. Without the
    # cap a key takes every line down to the next ALL-CAPS one, and
    # AUTOSAVETIME would report GPSconst, GNSconst and more as its value.
    v2keys()  { awk '/^POPULATION BEGIN\{/{exit}
                     BEGIN{split("FLOORDIMENSION FLOORFUNCTION FLOORPICTUREFILE FLOORFUNCSELECTED TEXTUREFILE TEXALPHA WITHTEXTURE AUTOSAVETIME RESEVGEN WITHHISTORY",K," ");
                           for(i=1;i<=10;i++){want[K[i]]=1}; want["FLOORDIMENSION"]=2}
                     $0 in want {k=$0; c[k]++; got=0; next}
                     k!="" && got<want[k] {v[k]=v[k]" "($0==""?"<blank>":$0); got++; next}
                     {k=""}
                     END{for(i=1;i<=10;i++){printf "%-18s x%d%s\n", K[i], c[K[i]]+0, v[K[i]]}}' "$1"; }
    v2eq3()   { [ "$2" = "$3" ] && [ "$3" = "$4" ] && echo "$1" || echo "NO"; }
    if [ "$cpok" = 0 ]; then
        v2f=1; echo "  could not stage the two experiments into $V2D"
    elif ! v2run ham 0 || ! v2run ham 1 || ! v2run oct 0 || ! v2run oct 1; then
        v2f=1
        echo "  a save did not finish. Sizes of what it did write:"
        ls -l "$V2D" 2>&1 | sed 's/^/    /'
        if [ -s "$V2D/err" ]; then
            echo "  the driver's stderr said:"
            tail -12 "$V2D/err" | sed 's/^/    /'
        else
            echo "  and its stderr was EMPTY. CHECK FREE MEMORY FIRST -- a"
            echo "  check.sh run has already been OOM-killed on this box, and"
            echo "  SIGKILL gives exactly these symptoms."
        fi
    elif command grep -q '^ *!!' "$V2D"/ham0.out "$V2D"/ham1.out \
                                "$V2D"/oct0.out "$V2D"/oct1.out; then
        v2f=1
        echo "  the driver could not carry out part of a save:"
        command grep -h '^ *!!' "$V2D"/ham*.out "$V2D"/oct*.out | head -4 | sed 's/^/    /'
    elif ! command grep -q guidriveStderrControl "$V2D/err"; then
        # Its own positive control, on its own stderr: `pagesave' checks the
        # same scenario, but from a different run. Without it an empty stderr
        # cannot be told from a suppressed one.
        v2f=1
        echo "  Qt's connect logging is SUPPRESSED -- the save path's connects"
        echo "  were not checked. Unset QT_LOGGING_RULES and re-run."
    elif command grep -E 'No such (signal|slot)' "$V2D/err" \
             | command grep -qv guidriveStderrControl; then
        v2f=1
        echo "  a connect on the save path names a signal or slot that does not exist:"
        command grep -E 'No such (signal|slot)' "$V2D/err" \
            | command grep -v guidriveStderrControl | sort -u | head -4 | sed 's/^/    /'
    else
        genok=1
        {
        echo "== V2 round trip, against v8-1.3-gp-blocks.txt (hammer) and the port's own output (octopus)"
        for p in 0 1 2; do echo "markers ham$p        $(v2marks "$V2D/ham$p.exp")"; done
        for p in 0 1 2; do echo "pvmhost ham$p        $(v2hosts "$V2D/ham$p.exp")"; done
        echo "exp history lines    $(v2s6n "$V2D/ham0.exp") $(v2s6n "$V2D/ham1.exp") $(v2s6n "$V2D/ham2.exp")"
        echo "exp history stable   $(v2eq3 yes "$(v2s6 "$V2D/ham0.exp")" "$(v2s6 "$V2D/ham1.exp")" "$(v2s6 "$V2D/ham2.exp")")"
        # CONTENT, not only stability: without these two, section 6 is
        # compared only with ITSELF across the three passes, never with 1.3.
        # The v8 capture quotes both lines from the 1.3 run, so they are
        # 1.3's bytes. The floats here are read and written as text, never
        # recomputed, so comparing them byte for byte is legitimate where a
        # fitness value would not be.
        echo "exp history first    $(v2s6e "$V2D/ham2.exp" 1)"
        echo "exp history last     $(v2s6e "$V2D/ham2.exp" '$')"
        echo "first block chars    $(v2first "$V2D/ham0.exp") $(v2first "$V2D/ham1.exp") $(v2first "$V2D/ham2.exp")"
        for p in 0 1; do
            q=$((p + 1))
            v2blocks "$V2D/ham$p.exp" > "$V2D/b$p.txt"
            v2blocks "$V2D/ham$q.exp" > "$V2D/b$q.txt"
            echo "history growth $p->$q  $(paste "$V2D/b$p.txt" "$V2D/b$q.txt" \
                | awk '{d[$2-$1]++} END{n=0; for(k in d){printf "%s%d blocks %+d", (n++?" ":""), d[k], k} if(n==0) printf "NO BLOCKS"}')"
        done
        # The names and the population come from the shipped 1.3 data, so
        # pinning their bytes pins them against 1.3. The robot block does
        # not: its Body directories are not the shipped ones.
        echo "names stable         $(v2eq3 yes "$(v2names "$V2D/ham0.exp")" "$(v2names "$V2D/ham1.exp")" "$(v2names "$V2D/ham2.exp")") $(command grep -c "NAME='" "$V2D/ham0.exp" || true) $(v2namee "$V2D/ham2.exp" 1) $(v2namee "$V2D/ham2.exp" '$')"
        # THE SIZE IS PART OF THE ASSERTION: three hashes of an empty extract
        # are equal, and cmp calls two empty expstruct reports identical. The
        # line count fails on that.
        echo "hammer robot block   $(v2eq3 'identical in all three' "$(v2robot "$V2D/ham0.exp")" "$(v2robot "$V2D/ham1.exp")" "$(v2robot "$V2D/ham2.exp")") $(v2robotn "$V2D/ham2.exp") lines $(v2robot "$V2D/ham2.exp")"
        "$ROOT/checks/programs/expstruct.py" "$V2D/ham1.exp" > "$V2D/e1.txt"
        "$ROOT/checks/programs/expstruct.py" "$V2D/ham2.exp" > "$V2D/e2.txt"
        echo "expstruct ham1==ham2 $(cmp -s "$V2D/e1.txt" "$V2D/e2.txt" && echo yes || echo NO) $(wc -l < "$V2D/e2.txt" | tr -d ' ') lines $(md5sum < "$V2D/e2.txt" | cut -c1-16)"
        for p in 0 1 2; do echo "-- first-save keys, ham$p"; v2keys "$V2D/ham$p.exp"; done
        # octopus: the containers that actually collide. Printed from the LAST
        # save, so a writer that only got the first one right cannot pass.
        echo "octopus section 5    $(v2eq3 'identical in all three' "$(v2s5 "$V2D/oct0.exp")" "$(v2s5 "$V2D/oct1.exp")" "$(v2s5 "$V2D/oct2.exp")") $(v2s5n "$V2D/oct2.exp") lines $(v2s5 "$V2D/oct2.exp")"
        echo "octopus material     $(v2list "$V2D/oct2.exp" '^Material ' 2)"
        echo "octopus link         $(v2list "$V2D/oct2.exp" '^Link ' 2)"
        echo "octopus joint        $(v2list "$V2D/oct2.exp" '^RotationalJoint ' 3)"
        echo "octopus drive        $(v2list "$V2D/oct2.exp" '^Drive ' 3)"
        echo "octopus sensor       $(v2list "$V2D/oct2.exp" '^JointSensor ' 3)"
        echo "octopus body         $(v2body "$V2D/oct2.exp")"
        echo "octopus commands     $(v2cmds "$V2D/oct2.exp")"
        } > "$V2D/report.txt" || genok=0
        # The octopus rows are the port's order, which is the stored order. On
        # this robot 1.3 keeps material and link but permutes joint, drive,
        # sensor, body and the command list on every save -- see
        # v1-1.3-roundtrip.txt. That is the divergence, recorded, not filtered.
        cat > "$V2D/expect.txt" <<'V2EXPECT'
== V2 round trip, against v8-1.3-gp-blocks.txt (hammer) and the port's own output (octopus)
markers ham0        37 65 152 75204 75291 75453
markers ham1        37 82 171 75325 75412 75574
markers ham2        37 82 171 75425 75512 75674
pvmhost ham0        1 .
pvmhost ham1        1 .
pvmhost ham2        1 .
exp history lines    161 161 161
exp history stable   yes
exp history first    1 2001 8 8 21 13 32 0.0821164 0 0.00939138
exp history last     160 2001 8 9 7 30 45 0.45972 0 0.3833
first block chars    10574 10581 10588
history growth 0->1  100 blocks +1
history growth 1->2  100 blocks +1
names stable         yes 100 NAME='10443' NAME='10194'
hammer robot block   identical in all three 70 lines dde685bd78c14a9d
expstruct ham1==ham2 yes 108 lines 51c948661a826da6
-- first-save keys, ham0
FLOORDIMENSION     x0
FLOORFUNCTION      x0
FLOORPICTUREFILE   x0
FLOORFUNCSELECTED  x0
TEXTUREFILE        x0
TEXALPHA           x0
WITHTEXTURE        x0
AUTOSAVETIME       x0
RESEVGEN           x0
WITHHISTORY        x0
-- first-save keys, ham1
FLOORDIMENSION     x1 50 50
FLOORFUNCTION      x1 0
FLOORPICTUREFILE   x1 <blank>
FLOORFUNCSELECTED  x1 1
TEXTUREFILE        x1 <blank>
TEXALPHA           x1 99
WITHTEXTURE        x1 0
AUTOSAVETIME       x1 0
RESEVGEN           x1 0
WITHHISTORY        x1 1
-- first-save keys, ham2
FLOORDIMENSION     x1 50 50
FLOORFUNCTION      x1 0
FLOORPICTUREFILE   x1 <blank>
FLOORFUNCSELECTED  x1 1
TEXTUREFILE        x1 <blank>
TEXALPHA           x1 99
WITHTEXTURE        x1 0
AUTOSAVETIME       x1 0
RESEVGEN           x1 0
WITHHISTORY        x1 1
octopus section 5    identical in all three 127 lines ecf08203808e2687
octopus material     greenPlastic bluePlastic redPlastic
octopus link         thirdFootLink firstFootLink base firstLegLink1 firstLegLink2 secondFootLink secondLegLink1 secondLegLink2 thirdLegLink1 thirdLegLink2
octopus joint        secondLegJoint1 secondLegJoint2 thirdLegJoint1 secondLegJoint3 thirdLegJoint2 thirdLegJoint3 firstLegJoint1 firstLegJoint2 firstLegJoint3
octopus drive        firstLegJoint2Drive firstLegJoint1Drive secondLegJoint1Drive secondLegJoint3Drive thirdLegJoint2Drive secondLegJoint2Drive thirdLegJoint3Drive thirdLegJoint1Drive firstLegJoint3Drive
octopus sensor       secondLegJoint1Sensor thirdLegJoint3Sensor secondLegJoint2Sensor firstLegJoint3Sensor secondLegJoint3Sensor thirdLegJoint2Sensor thirdLegJoint1Sensor firstLegJoint1Sensor firstLegJoint2Sensor
octopus body         legLink.wrl footLink.wrl tripleBase.wrl
octopus commands     MUL MOVE CMP COPY LOAD SENSE SUB DIV MIN DELAY ADD MOD MAX
V2EXPECT
        if [ "$genok" = 0 ]; then
            v2f=1
            echo "  the report generator could not write its report at all:"
            ls -l "$V2D/report.txt" 2>&1 | sed 's/^/    /'
        elif diff -u --label "1.3's numbers for hammer, two known divergences, the port's for octopus" \
                     --label "what this build wrote" \
                     "$V2D/expect.txt" "$V2D/report.txt" > "$V2D/diff.txt"; then
            v2p=1
        else
            v2f=1
            echo "  our round trip no longer matches the expected report:"
            # The CHANGED lines, not the first 20 lines of the diff: the report
            # is 58 lines, so a plain head shows context and can stop before
            # it reaches the difference.
            command grep -E '^([-+]|@@)' "$V2D/diff.txt" | head -20 | sed 's/^/    /'
        fi
    fi
    rm -rf "$V2D" || true
fi
printf '%-22s %2d pass  %2d fail\n' "v2 round trip vs 1.3" "$v2p" "$v2f"
pass=$((pass+v2p)); fail=$((fail+v2f))

# ---------------------------------------------------------------------------
# The Designer forms: two checks that nothing else makes.
#
#   uic      Qt 6's form compiler accepts every .ui with no warning. uic
#            prints a warning and still exits 0, for example when it renames
#            a duplicate widget name or drops an <images> block. The build
#            prints the warning and carries on, so this is the only place
#            that fails on it.
#   images   every ":/..." that a generated header asks for is in the form's
#            .qrc and on disk, and every file in the .qrc is asked for. A
#            missing image shows as a blank button and fails nothing else.
#            The second direction matters most: a form that LOST its images
#            asks for nothing, so the first direction passes.
#
# Every .ui under sigel/ui is checked. A .ui that is not in the Makefile's
# FORMS has no generated header, and fails here.
fp=0; ff=0
# `make forms' ran at the top, before the module passes, because GUI headers
# need its output. Its log is read here; do not run make again. A second
# `make forms' does nothing and prints nothing, so its empty log would hide
# the first run's warnings.
if [ -n "$FORMS_FAILED" ]; then
    echo "  make forms FAILED -- the form checks did not run"; ff=$((ff+1))
else
    # Any output is a failure, minus Qt's locale banner, which every Qt tool
    # prints under a non-UTF-8 LC_ALL and which says nothing about the form.
    grep -v '^Detected locale \|^Qt depends on a UTF-8 locale\|^If this causes problems\|^for more information' \
         /tmp/mkforms.$$ > /tmp/uic2.$$ || true
    if [ -s /tmp/uic2.$$ ]; then echo "  uic WARNED:"; cat /tmp/uic2.$$; ff=$((ff+1)); fi
    for ui in $(find "$SRC/ui" -name '*.ui' | sort); do
        form=${ui#$SRC/ui/}; form=${form%.ui}; base=$(basename "$form")
        [ -f "$FORMSB/ui/ui_$base.h" ] || { ff=$((ff+1))
            echo "  form FAIL: no ui_$base.h -- $form.ui is not in the Makefile's FORMS"; continue; }
        qrc="$SRC/ui/$form.qrc"
        # `pfx=$(sed ...)' takes sed's exit status, and sed on a missing file
        # exits 2, which under set -e would stop the script. Most forms have no
        # .qrc, hence the [ -f "$qrc" ] test first.
        pfx=
        [ -f "$qrc" ] && pfx=$(sed -n 's/.*<qresource prefix="\([^"]*\)".*/\1/p' "$qrc")
        for want in $(command grep -ao ':/[A-Za-z0-9_/.-]*' "$FORMSB/ui/ui_$base.h" | sort -u); do
            [ -f "$qrc" ] || { ff=$((ff+1)); echo "  form FAIL: $want but no $form.qrc"; continue; }
            rel=${want#:$pfx/}
            if [ "$rel" != "$want" ] && command grep -aq "<file>$rel</file>" "$qrc" \
               && [ -f "$SRC/ui/$(dirname "$form")/$rel" ]
            then fp=$((fp+1))
            else ff=$((ff+1)); echo "  form FAIL: $want not backed by $form.qrc"; fi
        done
        if [ -f "$qrc" ]; then
            for have in $(sed -n 's|.*<file>\(.*\)</file>.*|\1|p' "$qrc"); do
                if command grep -aq ":$pfx/$have" "$FORMSB/ui/ui_$base.h"; then fp=$((fp+1))
                else ff=$((ff+1)); echo "  form FAIL: $form.qrc carries $have, ui_$base.h never uses it"; fi
            done
        fi
    done
fi
printf '%-22s %2d pass  %2d fail\n' "forms" "$fp" "$ff"
pass=$((pass+fp)); fail=$((fail+ff))

rm -f /tmp/chk.$$ /tmp/hdr.$$.cpp /tmp/uic2.$$ /tmp/mkforms.$$
rm -rf "$FORMSB"
echo "-----"
echo "total: $pass pass, $fail fail, $warn warnings in SIGEL code"
[ "$winskip" = 0 ] || echo "$winskip Windows-only WIN_* file(s) excluded -- permanent, §7"
[ "$skipped" = 0 ] || echo "$skipped SECTION(S) SKIPPED -- see above; they tested nothing"
# Exit non-zero when anything failed, so that `./checks/check.sh && ...' stops
# on a red run. A skipped section counts as a failure too: it tested nothing
# and reported no failure.
[ "$fail" = 0 ] && [ "$skipped" = 0 ]
