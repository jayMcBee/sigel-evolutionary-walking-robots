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
QTLIBDIR=$(qmake6 -query QT_INSTALL_LIBS)

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
# to parse every `signals:' block in the tree and intersect with the names
# below; against the CORRECTED list that intersection is now EMPTY, because
# lostFocus is exactly the name it removed.
#
# selectionChanged was anchored to `( )' and so missed the QListViewItem*
# overload, which Qt 2's QListView also declared and which is equally dead.
# Both errors were found by review, and they cancelled in the total.
#
# KNOWN FALSE-POSITIVE SURFACE, since a regex cannot know the sender's class:
#   selectionChanged()  is LIVE on QLineEdit, QTextEdit and QPlainTextEdit.
#     Zero such senders today (all 4 sites are QListView/QListBox), but the
#     tree has 6 QMultiLineEdit -> QTextEdit, so C6/C7 could introduce one.
#     The QItemSelection overload is deliberately NOT matched, so converting a
#     view to selectionModel()->selectionChanged(sel, desel) passes cleanly.
#   selected(), currentChanged(), clicked(int), activated() are unambiguous
#     in this tree; each was checked sender-by-sender.
# The gate PRINTS every offending line precisely so a false positive is
# visible rather than silently believed.
DEAD_SIGNALS='SIGNAL\( *(activated *\( *\)|activated *\( *const *QString'\
'|clicked *\( *int|selected *\(|selectionChanged *\( *(\)|Q(ListView|TreeWidget)Item)'\
'|currentChanged *\( *Q(ListView|ListBox|TreeWidget|ListWidget)Item|rightButtonClicked'\
'|doubleClicked *\( *Q(ListView|TreeWidget)Item)'

MODULES="${*:-SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP SIGEL_Visualisation SIGEL_CommonGUI SIGEL_SlaveGUI MT_GUI SIGEL_MasterGUI}"
pass=0; fail=0; warn=0; skipped=0; winskip=0

# The shim self-check was here: it built and RAN q2compat_check.cpp under
# ASan+UBSan, and was the only mechanical check that could see an ownership
# error. Deleted with the shim -- it only ever tested the compatibility
# layer, so nothing is left for it to check. It was reported separately and
# never counted in the module totals, so 105/4 is unchanged by its removal.

# The gate's own self-test. $DEAD_SIGNALS is the only check here that is a
# REGEX rather than a compiler, so it is the only one that can silently stop
# matching -- which it did: anchoring selectionChanged to `( )' hid the
# QListViewItem* overload for a whole commit. Every row below is a spelling
# that appears in this tree or a near-miss that must NOT match.
rt_p=0; rt_f=0
while IFS='|' read -r want line; do
    [ -z "$want" ] && continue
    # grep -c exits 1 when the count is 0, which under `set -e' kills the whole
    # script before it prints anything. Same trap C1 hit with a sed.
    got=$(printf '%s\n' "$line" | command grep -cE "$DEAD_SIGNALS" || true)
    if [ "$got" = "$want" ]; then rt_p=$((rt_p+1)); else
        rt_f=$((rt_f+1)); echo "  regex self-test: want $want got $got for: $line"
    fi
done <<'RXEOF'
1|	connect(a, SIGNAL(activated()), b);
1|	connect(a, SIGNAL( activated() ), b);
1|	connect(a, SIGNAL(activated(const QString &)), b);
1|	connect(a, SIGNAL(clicked(int)), b);
1|	connect(a, SIGNAL(selected(QListBoxItem*)), b);
1|	connect(a, SIGNAL(selected(QAction *)), b);
1|	connect(a, SIGNAL(selectionChanged()), b);
1|	connect(a, SIGNAL(selectionChanged(QListViewItem*)), b);
1|		    SIGNAL( selectionChanged( QListViewItem * ) ),
1|	connect(a, SIGNAL(currentChanged(QListViewItem*)), b);
1|	connect(a, SIGNAL(currentChanged(QListBoxItem*)), b);
1|	connect(a, SIGNAL(currentChanged(QTreeWidgetItem*)), b);
1|	connect(a, SIGNAL(currentChanged(QListWidgetItem*)), b);
1|	connect(a, SIGNAL(doubleClicked(QTreeWidgetItem*)), b);
1|	connect(a, SIGNAL(selectionChanged(QTreeWidgetItem*)), b);
0|	connect(a, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), b);
0|	connect(a, SIGNAL(customContextMenuRequested(const QPoint&)), b);
1|	connect(a, SIGNAL(doubleClicked( QListViewItem * )), b);
1|	connect(a, SIGNAL(rightButtonClicked(QListBoxItem*, const QPoint&)), b);
1|		    SIGNAL( rightButtonClicked ( QListViewItem *, const QPoint &, int ) ),
0|	connect(a, SIGNAL(clicked()), b);
0|	connect(a, SIGNAL(activated(int)), b);
0|	connect(a, SIGNAL(valueChanged(int)), b);
0|	connect(a, SIGNAL(textChanged(const QString&)), b);
0|	connect(a, SIGNAL(stateChanged(int)), b);
0|	connect(a, SIGNAL(toggled(bool)), b);
0|	connect(a, SIGNAL(pressed()), b);
0|	connect(a, SIGNAL(timeout()), b);
0|	connect(editor, SIGNAL(lostFocus()), SLOT(slotResetFocus()));
0|	connect(a, SIGNAL(newText(const QString &)), b);
0|	connect(a, SIGNAL(triggered()), b);
0|	connect(a, SIGNAL(textActivated(const QString &)), b);
0|	connect(a, SIGNAL(idClicked(int)), b);
0|	connect(a, SIGNAL(itemSelectionChanged()), b);
0|	connect(a, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), b);
0|	connect(a, SIGNAL(currentChanged(int)), b);
0|	connect(a, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), b);
0|	connect(sel, SIGNAL(selectionChanged(QItemSelection,QItemSelection)), b);
0|	connect(sel, SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), b);
RXEOF
printf '%-22s %2d pass  %2d fail\n' "dead-signal regex" "$rt_p" "$rt_f"
pass=$((pass+rt_p)); fail=$((fail+rt_f))

dead=0; deadbase=0
for m in $MODULES; do
    mp=0; mf=0; mw=0
    for f in "$SRC/src/$m"/*.cpp; do
        [ -e "$f" ] || continue
        # WIN_* is Windows-only and PERMANENTLY out of scope:
        # WIN_SIG_GPRemoteZORCFitnessFunction needs HANDLE and OVERLAPPED
        # from windows.h and has no Qt 2 API left in it, so it cannot
        # compile on Linux at all -- section 7 records that its failure is
        # correct rather than debt. Counted as a KNOWN EXCLUSION rather
        # than a failure, so that "0 fail" means something and the exit
        # status this script now returns is usable. Skipping it is the
        # only honest alternative to a red run for ever.
        case "${f##*/}" in WIN_*) winskip=$((winskip+1)); continue ;; esac
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
    # every CONVERTED module. As of C8 there is no non-zero baseline left at all:
    # the whole tree is at zero and any new dead connect fails the gate wherever
    # it appears.
    #
    # -o|wc -l, not -c: grep -c counts matching LINES. No line carries two
    # SIGNAL() macros today, so the two agree -- but the baselines are exact
    # numbers and should not quietly drift if that ever stops being true.
    md=$(command grep -rhoE "$DEAD_SIGNALS" "$SRC/src/$m" "$SRC/include/$m" 2>/dev/null | wc -l)
    case "$m" in
        SIGEL_MasterGUI) base=0  ;;   # C7 repaired all 44
        MT_GUI)          base=0  ;;   # C6 repaired all 31
        MT_Control)      base=0  ;;   # C8 repaired all 15
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
    case "${h##*/}" in WIN_*) winskip=$((winskip+1)); continue ;; esac
    printf '#include "%s"\nint main(){return 0;}\n' "$rel" > /tmp/hdr.$$.cpp
    if g++ $FLAGS $INCS /tmp/hdr.$$.cpp 2>/dev/null; then hp=$((hp+1)); else hf=$((hf+1)); echo "  header FAIL: $rel"; fi
 done
done
# hp/hf were reported but never folded into the totals, so the headline "N fail"
# silently excluded the standalone-header failure. Found by review.
printf '%-22s %2d pass  %2d fail\n' "headers standalone" "$hp" "$hf"
pass=$((pass+hp)); fail=$((fail+hf))
# The module loop only reaches src/<Module>/ and include/<Module>/ for the
# modules in MODULES. Three things sit outside every baseline above:
#   - sigel.cpp and sigel_slave.cpp, at the top of src/ (C8's files);
#   - SIGEL_RealInterface, a module directory in no list (a stub today);
#   - the forms, whose <connection> blocks are XML and carry the bare signal
#     name, NOT a SIGNAL() macro -- so $DEAD_SIGNALS structurally cannot match
#     them and a separate pattern is needed. All 49 are live today
#     (clicked, valueChanged, toggled, sliderReleased).
# An earlier version of this scan pointed $DEAD_SIGNALS at the .ui directory and
# so claimed a coverage it could never have had.
stray=$(command grep -rhoE "$DEAD_SIGNALS" \
        "$SRC"/src/*.cpp "$SRC"/src/SIGEL_RealInterface "$SRC"/include/SIGEL_RealInterface \
        2>/dev/null | wc -l)
uistray=$(command grep -rhoE '<signal>(activated|selected|rightButtonClicked|doubleClicked|selectionChanged|currentChanged) *\(' \
          "$SRC"/ui 2>/dev/null | wc -l)
stray=$((stray+uistray))
if [ "$stray" -gt 0 ]; then
    echo "  dead signal outside every module baseline:"
    command grep -rnE "$DEAD_SIGNALS" \
        "$SRC"/src/*.cpp "$SRC"/src/SIGEL_RealInterface "$SRC"/include/SIGEL_RealInterface \
        2>/dev/null | sed "s|$SRC/|    |" | cut -c1-140
    command grep -rnE '<signal>(activated|selected|rightButtonClicked|doubleClicked|selectionChanged|currentChanged) *\(' \
        "$SRC"/ui 2>/dev/null | sed "s|$SRC/|    |" | cut -c1-140
    fail=$((fail+stray))
fi
printf '%-22s %2d dead (baseline %d -- §2 has the per-signal table)\n' \
       "Qt 6 signals" "$((dead+stray))" "$deadbase"

# ---------------------------------------------------------------------------
# Encoding and line-ending fidelity.
#
# 46 files in this tree are Latin-1 and many are CRLF or MIXED CRLF/LF. Both
# survive every other check here -- a file whose CRLF has been stripped compiles
# identically and passes all four behaviour gates -- and both are destroyed by
# the ordinary way of editing a file from a script: Python text mode reads with
# universal newlines and writes back LF. That happened during C6 and silently
# rewrote 26 files, turning a 436-line conversion into a 3,227-line diff.
#
# Written in Python rather than shell: this is byte counting against git, and
# the first, shell version skipped files silently while reporting a clean pass.
#
# The invariant is deliberately weak so legitimately added lines do not trip it:
# a file that HAD a CR must still have one, and a file that had non-ASCII bytes
# must still have them. Wholesale conversion is what it catches.
enc_out=$(cd "$ROOT" && python3 - <<'ENCPY'
import subprocess
base = subprocess.run(["git","rev-list","--max-parents=0","HEAD"],
                      capture_output=True, text=True).stdout.split()[0]
files = subprocess.run(["git","ls-files"], capture_output=True, text=True).stdout.split(chr(10))
ok = bad = skip = translated = 0
for rel in files:
    if not rel.endswith((".cpp",".h",".ui",".exp",".mt")): continue
    r = subprocess.run(["git","show","%s:%s" % (base, rel)], capture_output=True)
    if r.returncode != 0: skip += 1; continue      # added after the root commit
    try: cur = open(rel,"rb").read()
    except OSError: skip += 1; continue
    old = r.stdout
    # Losing the non-ASCII bytes is EXPECTED and deliberate: Phase 0b translated
    # the German comments to English, which is where the umlauts went. Counted
    # and reported, never failed.
    if any(x > 127 for x in old) and not any(x > 127 for x in cur):
        translated += 1
    if b"\r" in old and b"\r" not in cur:
        bad += 1
        print("  %s: CRLF stripped (%d CRs -> 0)" % (rel, old.count(b"\r")))
    else:
        ok += 1
print("COUNTS %d %d %d %d" % (ok, bad, skip, translated))
ENCPY
)
ep=$(echo "$enc_out" | sed -n 's/^COUNTS \([0-9]*\) .*/\1/p')
ef=$(echo "$enc_out" | sed -n 's/^COUNTS [0-9]* \([0-9]*\) .*/\1/p')
es=$(echo "$enc_out" | sed -n 's/^COUNTS [0-9]* [0-9]* \([0-9]*\) .*/\1/p')
et=$(echo "$enc_out" | sed -n 's/^COUNTS [0-9]* [0-9]* [0-9]* \([0-9]*\)/\1/p')
# 25 files lost their CRLF in ONE commit, 762c87f "D6: delete Q2Array, its 180
# sites are plain QList" (2026-08-27) -- verified by walking each file's history
# for its first CR-free revision, not inferred. An earlier version of this
# comment blamed Phase 0's comment passes, which is where the German umlauts
# went but not the carriage returns. Pre-existing damage, carried as a baseline
# so the gate fails on a NEW one rather than standing permanently red. Lower it
# when they are restored, never raise it.
ENC_BASELINE=25
# Fail CLOSED on an empty result. The failure this catches is "python exited 0
# but printed no COUNTS line": $ep/$ef come back empty, `[ "$ef" -gt 25 ]' errors,
# and because that is an `if' CONDITION set -e does not fire -- so the gate used
# to print blanks and score 0/0 while claiming to have run.
#
# If python exits NON-zero instead, set -e trips on the enc_out assignment above
# and the script stops there, printing no encodings row and no total: line. That
# is loud rather than silent, so it is left alone -- but a reader should not
# expect this branch to be what handles it. An earlier version of this comment
# said set -e does not fire at all, which is wrong.
if [ -z "$ep" ] || [ -z "$ef" ]; then
    echo "  encodings check produced no COUNTS line -- treating as FAILED"
    printf '%-22s %2d pass  %2d fail\n' "encodings" 0 1
    fail=$((fail+1))
elif [ "$ef" -gt "$ENC_BASELINE" ]; then
    echo "$enc_out" | command grep -v '^COUNTS ' || true
    printf '%-22s %2d pass  %2d fail  (baseline %d -- a NEW file lost its CRLF)\n' \
           "encodings" "$ep" "$((ef-ENC_BASELINE))" "$ENC_BASELINE"
    fail=$((fail+ef-ENC_BASELINE))
else
    printf '%-22s %2d pass  %2d known CRLF losses (D6), %s translated, %s postdate root\n' \
           "encodings" "$ep" "$ef" "$et" "$es"
fi
pass=$((pass+ep))

# ---------------------------------------------------------------------------
# Widgets whose behaviour no gate reads.
#
# Same blind spot as `parsers' below, one layer up: DISpinBox overrides Qt's
# text/value mapping, and a conversion that compiles perfectly can still throw
# away what the user typed. Qt 6's QSpinBox::validate()/fixup() are an INTEGER
# parser and run BEFORE the virtual valueFromText, so "0.375" became 0 and the
# override never saw the fraction. Nothing else here could see that.
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
    // input under de_DE and is the only thing here that can see that.
    QLocale::setDefault(QLocale(QLocale::German, QLocale::Germany));
    P g(3); g.setRange(3, 0.0, 100.0); g.type("0.375");
    eq("3dp text de_DE",  g.text(),                  "0.375");
    eq("3dp value de_DE", QString::number(g.value()), "375");
    // ...and pinning to QLocale::c() is NOT sufficient on its own, which is the
    // trap these rows exist for. C's GROUP separator is ',', so a bare
    // QLocale::c() validator accepts "0,375" as 375-with-a-group-separator, and
    // QString::toDouble() -- which never takes group separators -- then returns
    // 0. Qt 2 could not: its validator ran the whole string through strtod and
    // demanded it be consumed to the NUL (qstring.cpp toDouble), so ok was false
    // and validate() returned Invalid outright -- the keystroke was refused.
    //
    // Asserted through lineEdit()->validator(), which is the object the locale
    // is pinned on and is reachable with public API: DISpinBox::validate() is
    // private (DoubleSpinBox.h), and a probe calling it through a derived struct
    // does not compile -- which silently took the whole widgets section, this
    // block and the C6 rows above it, out of the build when first written.
    //
    // Invalid, not merely "not Acceptable": without RejectGroupSeparator the
    // state is Intermediate, so only checking for Acceptable passes either way
    // and the row cannot see the bug it exists for.
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
# Qt 2 item virtuals that Qt 6 does not call any more.
#
# QListViewItem::key( int, bool ) drove QListView's sort (qlistview.cpp:802).
# QTreeWidgetItem has no key() at all -- it sorts through operator< -- so the
# moment C7 renamed the base class, SIG_IndividualListItem::key() became dead
# code that still compiles, still looks right, and is never called. The
# individuals list silently fell back to sorting column 0 as raw TEXT:
# 0, 1, 10, 100, 11 where 1.3 shows 0, 1, 2, ... 10. The 1.3 binary confirmed
# the numeric order. Nothing in a compiler or in the dead-signal gate can see
# this: it is a virtual that stopped being virtual.
#
# Rule: a class that still declares one of these must also declare the Qt 6
# member that replaced it.
kp=0; kf=0
# Flattened to one line before matching: review demonstrated that a declaration
# split as "QString\nkey(int, bool) const;" was invisible to a line-based grep,
# and that a decoy "operator<( QTreeWidgetItem * )" -- a pointer parameter, which
# overrides nothing -- was accepted. The signature below is the one that actually
# overrides QTreeWidgetItem::operator<. find, not a fixed-depth glob.
for h in $(find "$SRC/include" -name '*.h' | sort); do
    flat=$(tr '\n' ' ' < "$h")
    printf '%s' "$flat" | command grep -qE 'QString[[:space:]]+key[[:space:]]*\([[:space:]]*int' || continue
    if printf '%s' "$flat" | command grep -qE 'operator<[[:space:]]*\([[:space:]]*const[[:space:]]+QTreeWidgetItem[[:space:]]*&'; then
        kp=$((kp+1))
    else
        kf=$((kf+1))
        echo "  ${h#$SRC/}: declares Qt 2's key(int,bool) but no"
        echo "    operator<( const QTreeWidgetItem & ) -- Qt 6 sorts through operator<,"
        echo "    so key() is never called and the column sorts as raw text."
    fi
done
printf '%-22s %2d pass  %2d fail\n' "dead item virtuals" "$kp" "$kf"
pass=$((pass+kp)); fail=$((fail+kf))

# ---------------------------------------------------------------------------
# A freed pointer kept behind a guard that still passes.
#
# SIG_Simulation's constructor throws whenever SIMULATIONLIBRARY names the
# Dynamo backend, which physics_backends.md deleted on 2026-08-28. That throw
# unwinds out of visualizeThis() BETWEEN the `delete visualisation;' and the
# assignment meant to replace it, so without an explicit null the member keeps
# the pointer it just freed. Twenty-one sites dereference it behind fourteen
# `if (visualisation)' guards -- the guard passes -- and, because `visualisation'
# is a BASE-class member whose base destructor does `delete visualisation'
# (SIGEL_CommonGUI/SIG_VisualisationWidget.cpp:60), destroying the widget after
# such a throw is a double free as well.
#
# Guarded rather than trusted because the null LOOKS redundant three lines above
# `visualisation = new ...' and reads like tidying.
#
# WHAT THIS CAN AND CANNOT SEE. An earlier version counted only the exact
# spelling `delete visualisation;' and required delete-count == null-count; a
# fresh-eyes review then showed THREE false passes, because a second delete in
# any other spelling was invisible to the count and so kept the two equal.
# It now requires exactly ONE delete of that member, in any spelling a regex can
# reach, and that it be followed by a null. Still invisible, and stated rather
# than papered over: a delete through an ALIAS
# (`SIG_SimulationVisualisation *v = visualisation; delete v;'). No grep closes
# that; it needs the compiler or a human.
#
# A `delete visualisation' in this widget's OWN destructor would fail here, and
# that is correct rather than a false alarm: the base destructor already frees
# it, so a second one is a double free, not a leak fix.
vp=0; vf=0
vsrc=$SRC/src/SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.cpp
# Comments stripped and newlines flattened before matching, for the same reason
# the item-virtuals check above flattens: the delete, its explanation and the
# null sit on separate lines and a line-based grep would see none of the pair.
# LC_ALL=C because this file is Latin-1 (4 of its 781 lines carry non-ASCII
# bytes) and flattening puts them all on ONE line: under a UTF-8 locale that
# line is invalid multibyte, which is the silent-skip TRAP in PORTING.md.
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
# The one real user-facing defect SIGEL_SlaveGUI turned up was a QGroupBox too
# small to hold its own children: seven 50x50 navigation buttons and three
# position readouts, all present, enabled and correctly sized, all clipped out
# of existence by a container that had collapsed to 90x37. No widget-level probe
# saw it -- every child reported healthy -- and it took a screenshot of the
# running program to notice. This is that defect made mechanical: walk every
# View page and every tab and report any widget whose rect leaves its parent.
#
# The scenario carries its own positive control and FAILS if the control does
# not fire, because "0 clipped" from a check that cannot detect clipping is
# worth nothing. Shrinking the window is NOT usable as that control here -- the
# converted pages carry real layouts and reflow instead of clipping, where 1.3
# is absolutely positioned and does clip. So it displaces a real widget instead
# and requires the report.
cp=0; cf=0
if SIGEL_ROOT="$SRC" QT_QPA_PLATFORM=offscreen \
       SIGEL_EXP="$ROOT/data-reordered/Experiments/twoBasesSimpleFitness2.exp" \
       timeout 300 "$ROOT/build-fast/guidrive" clipcheck >/tmp/clip.$$ 2>/dev/null; then
    cp=1
else
    cf=1
    sed -n '/CLIPPED\|selftest/p' /tmp/clip.$$ | sed 's/^/  /'
fi
rm -f /tmp/clip.$$
printf '%-22s %2d pass  %2d fail\n' "no clipped controls" "$cp" "$cf"
pass=$((pass+cp)); fail=$((fail+cf))

# ---------------------------------------------------------------------------
# The structural fingerprint tool's own teeth.
#
# expstruct.py is what compares an evolved .exp across the two architectures
# (PORTING.md 9, the evolution path). Everything it concludes rests on ONE
# property: it must be blind to fitness and sighted on structure. Fitness is
# not a cross-machine reference in either direction -- D26 and section 7
# measure a 1-ULP change in start height moving fitness 45%, and the reference
# box is i386/x87 against this one's aarch64 -- while the tournament that picks
# survivors is a bare `>=' between two of those doubles
# (SIG_GPSimpleTournament.cpp:77). So a tool that let one fitness value reach
# its output would report an unavoidable divergence as a regression.
#
# --selfcheck asserts BOTH halves, because a tool that saw nothing at all would
# pass the fitness half on its own: a changed FITNESS value must not move the
# report, a changed program operand must, and two swapped individuals must.
# It compares the WHOLE report including SHAPE -- an earlier version compared
# only the counts and the content hashes, and could not have caught fitness
# leaking into SHAPE. Teeth-tested by disabling both fitness filters (caught)
# and by blinding the program matcher (caught).
ep=0; ef=0
if python3 "$ROOT/expstruct.py" --selfcheck \
       "$ROOT/data/Experiments/twoBasesSimpleFitness1.exp" >/tmp/eps.$$ 2>&1; then
    ep=1
else
    ef=1
    sed 's/^/  /' /tmp/eps.$$
fi
rm -f /tmp/eps.$$
printf '%-22s %2d pass  %2d fail\n' "expstruct selfcheck" "$ep" "$ef"
pass=$((pass+ep)); fail=$((fail+ef))

# ---------------------------------------------------------------------------
# The two programs. They are src/*.cpp, so no entry in MODULES reaches them and
# nothing compiled them until C8 -- which is how a QMotifPlusStyle that Qt 6
# does not have, and a pthread_create cast C++17 rejects, both survived this
# far. C9 needs them to build, link and run; this covers the first of the three.
pp=0; pf=0
for prog in sigel sigel_slave; do
    if g++ $FLAGS $INCS "$SRC/src/$prog.cpp" 2>/tmp/prog.$$; then
        pp=$((pp+1))
    else
        pf=$((pf+1)); echo "  $prog.cpp does not compile:"; head -5 /tmp/prog.$$
    fi
done
rm -f /tmp/prog.$$

# ...but compiling was never the hard part. Every C9 defect -- a missing moc, an
# unemitted vtable, a resource dropped from a static archive, the master
# SIG_GPExperiment reaching the slave -- is invisible to -fsyntax-only and shows
# up only at link. So the gate also requires the two binaries to be BUILT and
# CURRENT, and runs the one of the two that can smoke-test itself headlessly.
#
# Fails rather than skips when they are absent: a gate that quietly passes when
# the thing it checks is missing is the failure mode this port has already hit
# twice (the C7 comma probe, the C6 spin-box rows).
if make -q B=build-fast SAN= SIGSAN= programs 2>/dev/null; then
    for prog in sigel sigel_slave; do
        if [ -x "$ROOT/build-fast/$prog" ]; then pp=$((pp+1)); else
            pf=$((pf+1)); echo "  build-fast/$prog missing"; fi
    done
    # sigel_slave with no PVM daemon must reach its own guard and exit cleanly.
    # sigel needs a display and starts a pvmd, so its run is driven by hand and
    # against the 1.3 oracle -- see PORTING.md C9.
    out=$(SIGEL_ROOT="$SRC" QT_QPA_PLATFORM=offscreen \
          timeout 60 "$ROOT/build-fast/sigel_slave" 2>&1 </dev/null || true)
    case $out in
        *"hasn't been started as a PVM slave"*) pp=$((pp+1)) ;;
        *) pf=$((pf+1))
           echo "  sigel_slave did not reach its no-PVM guard; it printed:"
           printf '%s\n' "$out" | tail -3 | sed 's/^/    /' ;;
    esac
else
    pf=$((pf+1))
    echo "  the two programs are not built or are out of date --"
    echo "    run 'make B=build-fast SAN= SIGSAN= programs'"
fi
printf '%-22s %2d pass  %2d fail\n' "programs" "$pp" "$pf"
pass=$((pass+pp)); fail=$((fail+pf))

# ---------------------------------------------------------------------------
# The master GUI's structure, against what the running SIGEL 1.3 actually shows.
#
# Every other check here proves code COMPILES or LINKS. This one checks what the
# user sees: each menu entry and toolbar button with its shortcut, its enabled
# state and its check state. guidump-baseline.txt is not a snapshot of whatever
# the port emitted -- the 42 menu entries the 1.3 oracle read off the RUNNING
# 1.3 binary were diffed against it mechanically, zero mismatches, so a
# difference here is a regression against 1.3 itself.
#
# It is the only thing that can see an accelerator that went missing, an action
# that stopped being greyed, a toolbar button showing the long menu label
# instead of the short one, or a checkable action that quietly stopped being
# checkable. All of those compile, link and run perfectly.
gp=0; gf=0
if [ -d "$ROOT/build-fast/lib" ] && [ -d "$ROOT/build-fast/obj/moc" ]; then
    cat > /tmp/gui.$$.cpp <<'GUIEOF'
// Headless structural dump of the ported GUI, in the same shape the 1.3 oracle
// reports, so the two can be diffed mechanically.
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
    // zero-pads so the compare is NUMERIC. The 1.3 oracle read these exact
    // Fitness values off the running binary and reported the e-05 values
    // sorting first; as raw text 1.14825 would lead. This also pins the
    // exponent branch of key(), where Qt 2's unsigned truncate() silently
    // became a signed one that cleared the string.
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
    if g++ -std=c++17 -O1 -DMINMAX_H $INCS -I"$ROOT/build-fast/ui" /tmp/gui.$$.cpp \
         $(find "$ROOT/build-fast/obj/moc" -name '*.o') \
         "$ROOT/build-fast/obj/qrc/SIG_GPParameterBase.o" -o /tmp/gui.$$ \
         -Wl,--start-group "$ROOT/build-fast"/lib/*.a -Wl,--end-group \
         "$SL/pvm3/lib/LINUX64/libpvm3.a" -ltirpc \
         -L"$QTLIBDIR" -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui \
         -lQt6Core -lGL -lGLU -lm 2>/tmp/guib.$$; then
        SIGEL_ROOT="$SRC" QT_QPA_PLATFORM=offscreen timeout 120 /tmp/gui.$$ \
            > /tmp/guio.$$ 2>/dev/null || true
        if command grep -v '^#' "$ROOT/guidump-baseline.txt" | diff -u - /tmp/guio.$$ > /tmp/guid.$$; then
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
    echo "  no built GUI libraries -- run 'make B=build-fast SAN= SIGSAN= gui'"
fi
printf '%-22s %2d pass  %2d fail\n' "gui vs 1.3" "$gp" "$gf"
pass=$((pass+gp)); fail=$((fail+gf))

# ---------------------------------------------------------------------------
# Phase C, step C10 -- what the GUI DOES, not merely what it shows.
#
# `gui vs 1.3' above is static: menus, toolbars, and the values a freshly
# loaded experiment displays. It cannot see a wrong answer to a click, because
# nothing in it ever clicks. This section drives the real SIG_MainWindow with
# real Qt input events -- QTest posts QMouseEvent, QKeyEvent and
# QContextMenuEvent through QApplication::notify, so hit-testing, menu popups,
# item-view selection and the slots behind them all run. It is NOT the same as
# a mouse -- bypassing QWindowSystemInterface changes activation, grabs and
# double-click synthesis -- so this proves the application's logic, not the
# platform layer's. guidrive.cpp's header says so at more length.
#
# The baseline was diffed against the RUNNING 1.3 binary, so a failure here is
# a regression against 1.3. guibehaviour-baseline.txt says which fact came from
# where. The load-bearing line is `nameIsASurvivor=0', NOT the word SURVIVED:
# after the fitness sort the stale index stays in range, so reverting the fix
# does not crash this scenario -- it repoints the detail pane at a survivor,
# and that flip is what the teeth test measured. SURVIVED is a liveness check.
#
# It needs an experiment to open, so it is skipped rather than failed when the
# reference data is absent -- the data ships separately from the tarballs.
bp=0; bf=0
BEXP=$ROOT/data-reordered/Experiments/twoBasesSimpleFitness2.exp
if [ ! -f "$BEXP" ]; then
    # data-reordered/ is gitignored, so a fresh clone lands here. Say SKIPPED
    # loudly and count it: reporting `0 pass 0 fail' made the section vanish
    # from the total and left the exit status clean, which is the same shape as
    # the three silent-short-run holes this file has already been bitten by.
    echo "  SKIPPED: no $BEXP -- the data ships separately from the tarballs;"
    echo "  provision data/ as section 9 describes, then re-run. THIS SECTION"
    echo "  TESTED NOTHING."
    skipped=$((skipped+1))
elif make -s -C "$ROOT" B=build-fast SAN= SIGSAN= guidrive >/tmp/bdb.$$ 2>&1; then
    # SIGEL_ROOT must be the SOURCE tree: the driver loads pixmaps and terrain
    # from it. Neither scenario spawns a sigel_slave, so neither needs one.
    #
    # SIX scenarios make up the baseline, concatenated in this order:
    #   gate       C10 -- the tree, sorting, add/delete/reset, the dialogs,
    #              the context menus, the MetaGP warning
    #   pages      C11a -- the five View pages C10 never opened, every spin
    #              box, slider, combo, checkbox and validator on them
    #   exportall  C11b -- all eight File > Export children, each file's
    #              sha256, size, line count and ends. SEVEN of the eight are
    #              byte-identical to what the 2003 i386 binary writes, checked
    #              by the oracle, so a move here is a regression against 1.3
    #              and not merely against yesterday. The eighth, .lap, differs
    #              for a reason the baseline records.
    #   overwrite  C11b -- 1.3 asks "Do you want to overwrite?" and then
    #              writes the file whichever way you answer, because the second
    #              write is not inside the switch. Confirmed on the running
    #              binary. This pins the DEFECT: sentinelSurvived=1 would mean
    #              the port had started honouring the prompt.
    #   metagui    MT_GUI -- the MetaGP window, which nothing had ever
    #              opened. Its ten validators had never been given C7's C-locale
    #              treatment, so an unpinned QIntValidator(0,1000) called
    #              "1,000" ACCEPTABLE under en_US while text().toInt() returned
    #              0: a user types one thousand and zero reaches the system.
    #              1.3 rejects both separators -- measured on the binary -- so
    #              the fix restores Qt 2 rather than improving on it.
    #   dialogs    C11c -- the six dialogs, C7's 21st validator, and the
    #              select-on-focus defect. The load-bearing lines are the two
    #              `typing "5" gives [0.015]' / `typing "2" gives [12]' ones:
    #              Qt 6 selects a pre-filled field when a dialog hands it focus
    #              and Qt 2 did not, so before the fix a typed digit REPLACED
    #              the value instead of appending to it -- 2 individuals added
    #              where 1.3 adds 12. Both figures are the oracle's, off the
    #              running binary.
    #
    # `roundtrip' is NOT run here. It exports, imports and re-exports each
    # format, which takes 97 seconds against 30 for exportall, and what it
    # uniquely covers -- writer/reader/writer symmetry -- is largely covered by
    # exportall (a broken reader moves the export) and by dictorder. Run it by
    # hand when touching a readFromFile or writeToFile:
    #   SIGEL_ROOT=... SIGEL_EXP=... build-fast/guidrive roundtrip
    # $1 scenario, $2 outfile, $3.. extra NAME=VALUE for the child only.
    # The extras go through env rather than being written as a prefix on the
    # function call: a prefix would also apply to the SHELL, and bash then
    # prints "warning: setlocale: LC_ALL: cannot change locale" on any box
    # where the locale is not generated -- which is most of them, and is
    # exactly the box this check is designed to run on.
    # stderr is KEPT, appended to /tmp/berr.$$, and checked below. It used to
    # go to /dev/null, which threw away the only thing that can catch a dead
    # connect the $DEAD_SIGNALS regex has never heard of: Qt's own runtime
    # "No such signal". That regex is a closed list of the nine Qt 2 spellings
    # in §2 and matches SIGNAL( only, so a tenth kind -- or any SLOT() naming a
    # slot that no longer exists -- was invisible to the whole gate. Injecting
    # SIGNAL(highlighted(int)), which QTreeWidget does not have, passed every
    # section green while Qt printed the warning into /dev/null. Found by
    # review 2026-09-03.
    guidrive_run() {
        local sc="$1" out="$2"; shift 2
        env "$@" SIGEL_ROOT="$SRC" SIGEL_EXP="$BEXP" \
            SIGEL_SCRATCH="${TMPDIR:-/tmp}" QT_QPA_PLATFORM=offscreen \
            timeout 300 "$ROOT/build-fast/guidrive" "$sc" > "$out" 2>>/tmp/berr.$$
    }
    : > /tmp/berr.$$
    if guidrive_run gate /tmp/bo.$$ && guidrive_run pages /tmp/bp.$$ \
       && guidrive_run exportall /tmp/bx.$$ && guidrive_run overwrite /tmp/bw.$$ \
       && guidrive_run dialogs /tmp/bg.$$ && guidrive_run metagui /tmp/bm.$$; then
        cat /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ > /tmp/ball.$$
        # The driver prints `!!' when it could not do what it was asked -- a
        # dialog that would not accept, a file that never appeared. Such a run
        # must not pass, and must not be diffed into a baseline either: one
        # DID get committed that way, a population export that silently wrote
        # nothing, and only a later diff caught it. Checked before the diff so
        # the message is about the right thing.
        if command grep -q '^ *!!' /tmp/ball.$$; then
            bf=1
            echo "  the driver could not carry out part of a scenario:"
            command grep -n '^ *!!' /tmp/ball.$$ | head -6 | sed 's/^/    /'
            echo "  (a run containing these must never be committed as a baseline)"
        elif command grep -qE 'No such (signal|slot)' /tmp/berr.$$; then
            # Qt says this at RUNTIME when a string-based connect names
            # something that does not exist. It compiles, it links, and the
            # slot simply never fires -- which is the whole failure class C4
            # found and $DEAD_SIGNALS only partly covers. This check needs no
            # list of Qt 2 spellings because Qt does the matching.
            bf=1
            echo "  a string-based connect names a signal or slot that does not exist:"
            command grep -E 'No such (signal|slot)' /tmp/berr.$$ \
                | sort -u | head -6 | sed 's/^/    /'
            echo "  (it compiles and links; the slot never fires -- see §2's table)"
        elif command grep -q '^ *!!' "$ROOT/guibehaviour-baseline.txt"; then
            bf=1
            echo "  the BASELINE itself contains a failure marker -- it was"
            echo "  captured from a run that did not complete. Regenerate it:"
            command grep -n '^ *!!' "$ROOT/guibehaviour-baseline.txt" | head -4 | sed 's/^/    /'
        elif command grep -v '^#' "$ROOT/guibehaviour-baseline.txt" | diff -u - /tmp/ball.$$ > /tmp/bd.$$; then
            bp=1
        else
            bf=1; echo "  the GUI no longer BEHAVES the way SIGEL 1.3 does:"
            head -16 /tmp/bd.$$ | sed 's/^/    /'
        fi
    else
        bf=1
        echo "  the driver did not finish -- it exits(1) on an out-of-range pool"
        echo "  position, which is how the Qt 6 clear() regression showed up:"
        tail -6 /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ 2>/dev/null | sed 's/^/    /'
    fi

    # C7 pinned 21 validators to QLocale::c() with RejectGroupSeparator because
    # Qt 2 forced LC_NUMERIC="C" process-wide and its QDoubleValidator hard-coded
    # '.', while Qt 6 validators follow the system locale and the read-back is
    # QString::toDouble(), which does not. Left to disagree, a typed "9,81"
    # validates under a comma locale and reads back as ZERO -- silent data loss
    # into the saved experiment. The 1.3 oracle measured the 2003 binary under a
    # de_DE built with woody's own localedef and found it locale-independent, so
    # this is 1.3 behaviour to preserve and not a Qt 6 nicety.
    #
    # This needs no baseline of its own: the runs must simply be IDENTICAL.
    #
    # TWO locales, and the reason is that one of them alone was a tautology.
    # de_DE exercises QT's half: QLocale reads the environment directly, so it
    # reports de_DE with a comma decimal even where no such locale is
    # GENERATED -- measured -- which is what C7's validator pinning is tested
    # against. But setlocale() then fails and LC_NUMERIC stays "C", so libc's
    # half was never touched. en_DK is a comma-decimal locale that IS installed
    # here (`locale -a`), so under it libc really does switch: a stray
    # sprintf("%f") or strtod in a reader or writer shows up only in this one.
    # Found by review, which measured the difference between them.
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
rm -f /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ \
      /tmp/bl.$$ /tmp/bl2.$$ /tmp/ball.$$ \
      /tmp/bd.$$ /tmp/bdb.$$ /tmp/berr.$$
# exportall and overwrite WRITE FILES, 2.7 MB of them, the population export
# being most of it. Fixed names, so they are overwritten rather than
# accumulated, but leaving them in TMPDIR is untidy.
rm -f "${TMPDIR:-/tmp}"/x11b-gpp.gpp "${TMPDIR:-/tmp}"/x11b-sip.sip \
      "${TMPDIR:-/tmp}"/x11b-lap.lap "${TMPDIR:-/tmp}"/x11b-env.env \
      "${TMPDIR:-/tmp}"/x11b-pop.pop "${TMPDIR:-/tmp}"/x11b-prg.prg \
      "${TMPDIR:-/tmp}"/x11b-ind.ind "${TMPDIR:-/tmp}"/x11b-dat.dat \
      "${TMPDIR:-/tmp}"/x11b-ow.sip "${TMPDIR:-/tmp}"/x11b-ow \
      "${TMPDIR:-/tmp}"/c11c-lap.lap
printf '%-22s %2d pass  %2d fail\n' "gui behaviour" "$bp" "$bf"
pass=$((pass+bp)); fail=$((fail+bf))

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
[ "$winskip" = 0 ] || echo "$winskip Windows-only WIN_* file(s) excluded -- permanent, §7"
[ "$skipped" = 0 ] || echo "$skipped SECTION(S) SKIPPED -- see above; they tested nothing"
# EXIT NON-ZERO WHEN ANYTHING FAILED. There was no exit here at all, so the
# script always returned 0 and `./check.sh && ...' proceeded through a red run.
# A skipped section counts as a failure for the exit status: it is the shape
# this file has been bitten by three times -- a section that quietly tests
# nothing and reports no failures. Found by review.
[ "$fail" = 0 ] && [ "$skipped" = 0 ]
