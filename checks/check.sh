#!/bin/sh
# Exit criterion for the Qt 6 port (PORTING.md D11).
#
#   ./checks/check.sh                 all converted modules, syntax only
#   ./checks/check.sh MT_GPSystem     one module
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
ROOT=$(cd "$(dirname "$0")/.." && pwd)
# ROOT is derived, not the script's own directory, so check it: a symlink or
# a copy left at the old path would point it at the PARENT of the repo, and
# check.sh removes $ROOT/build/ui before any other guard runs.
[ -f "$ROOT/Makefile" ] && [ -d "$ROOT/checks" ] || {
	echo "$0: $ROOT is not the repo root -- run the script by its real path,"\
	     "not through a symlink or a copy" >&2; exit 1; }
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
for d in newmat09 dynamechs/dm Dynamo/Src/Inc fparser cv97 pvm3/include; do
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
# signal (include/MT_GUI/MT_Editor.h, MT_Editor) and Qt 2.3's QLineEdit never had one.
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
        mw=$((mw + $(command grep -ac "$SRC.*warning:" /tmp/chk.$$ || true)))
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
# THE LINE-ENDING RULE CHANGED ON 2026-09-09, BY DECISION: the tree is LF only.
# Every file under x/kdesigelSources.1.3 was converted -- 100 files, 17,750
# CRLF pairs -- and this half of the check was turned round to match. It used to
# say "a file that HAD a CR must still have one", with a baseline of 25 for the
# files 03ac805 had already stripped. It now says NO TRACKED TEXT FILE MAY
# CARRY CRLF, expected count zero, and it fails the moment one comes back
# through a Windows editor, an unpacked archive or a patch.
#
# Turning it round widened it as well. The old form could only judge files that
# existed in the root commit and had one of five extensions, so a NEW file
# arriving with CRLF was invisible to it. The rule below reads every tracked
# file instead.
#
# WHAT IS BINARY IS GIT'S ANSWER, NOT A HOME-MADE ONE. \r\n inside a PNG or a
# tarball is pixel data, not a line ending -- three tracked binaries hold 12
# such pairs between them -- so binaries must be skipped. The first version of
# this check tested for a NUL byte in the first 8 KB and GOT TWO FILES WRONG:
# textures/Hippie.pnm has no NUL anywhere in its 196,668 bytes, and
# textures/UniDo_LSXI.pnm has its first NUL at offset 15,456. Both are P6 raw
# raster and both were judged as text. They passed only because neither happens
# to contain a 0d 0a pair; a re-rendered texture that did would have failed this
# gate, and the obvious way to make it green again corrupts the image.
# `git ls-files --eol' gets all five .pnm right, because git falls back to a
# printable-byte ratio when there is no NUL. Ask git. Found by review 2026-09-09.
#
# Lone CRs are left alone, and NOT because they are Mac-classic line endings --
# an earlier version of this comment said that and it was wrong. 27 tracked
# files hold lone CRs and git calls ALL 27 binary: supportingLibs.tar.gz 26002,
# pvm3.4.6.tgz 3859,
# noExperiment.png 691, JustGreen.pnm 2848, altLogo.png 208, Hippie.pnm 208,
# Stone.pnm 68, and the 20 robot .blend files, 22 to 40 each. Those bytes are
# pixel values, archive and model data that happen to equal 0x0d. They were never line endings, and nothing here treats them as any
# -- the binary skip below means this gate never even reads them.
#
# THE NON-ASCII HALF IS UNCHANGED, in value AND in scope: it still runs over the
# five extensions that carried the German comments, judged against the root
# commit, and the binary skip above is deliberately NOT upstream of it. A file
# that had bytes above 127 and now has none is COUNTED and reported, never
# failed. That is Phase 0 turning the German comments into English.
#
# Written in Python rather than shell: this is byte counting against git, and
# the first, shell version skipped files silently while reporting a clean pass.
enc_out=$(cd "$ROOT" && python3 - <<'ENCPY'
import subprocess
def git(*a):
    r = subprocess.run(["git"] + list(a), capture_output=True)
    if r.returncode != 0:
        raise SystemExit("git %s failed" % " ".join(a))
    return r.stdout
base = git("rev-list","--max-parents=0","HEAD").decode().split()[0]
# -z, NOT the default. Without it git C-quotes any path holding a space, a
# tab, a quote or a byte above 127 -- "l\303\244tin.txt" -- and open() then
# fails on the literal quoted string, so four readable files were reported as
# unreadable and the whole gate went red. No such path exists in this tree
# today; -z means one never can. Found by review 2026-09-09.
files = [f for f in git("ls-files","-z").decode("utf-8","surrogateescape").split(chr(0)) if f]
# THE w/ COLUMN, NOT THE i/ COLUMN. `ls-files --eol' prints both: i/ is the
# blob in the index, w/ is the file on disk, and this check reads the file on
# disk. Testing both together got sigel_slave.mak wrong: while D31 was being
# made, its index blob read i/-text -- the version still in HEAD held two
# \r\r\n, which git's own heuristic calls binary -- against a working file of
# plain w/lf. That would have dropped a tracked text file out of the check
# entirely. BOTH COLUMNS READ lf ONCE D31 IS COMMITTED, so the demonstration is
# gone and only the rule survives; do not "simplify" this back to cols[0].
# Found by testing, 2026-09-09.
binary = set()
for line in git("ls-files","--eol","-z").decode("utf-8","surrogateescape").split(chr(0)):
    if not line: continue
    cols = line.split(chr(9))
    w = [t for t in cols[0].split() if t.startswith("w/")]
    if w and w[0] == "w/-text":
        binary.add(cols[-1])
ok = bad = skip = translated = binfiles = unreadable = 0
for rel in files:
    try: cur = open(rel,"rb").read()
    except OSError: cur = None
    # --- line endings ---
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
    # --- the non-ASCII half, same files and same order as before the inversion ---
    if not rel.endswith((".cpp",".h",".ui",".exp",".mt")): continue
    r = subprocess.run(["git","show","%s:%s" % (base, rel)], capture_output=True)
    if r.returncode != 0: skip += 1; continue      # added after the root commit
    if cur is None: skip += 1; continue
    old = r.stdout
    if any(x > 127 for x in old) and not any(x > 127 for x in cur):
        translated += 1
# EVERY TRACKED FILE MUST LAND IN EXACTLY ONE BUCKET. AS THE LOOP IS WRITTEN
# ABOVE THIS CANNOT FAIL -- the four branches are one if/elif chain over `files',
# so the identity holds by construction. It is kept anyway, and the honest
# reason is not that it catches something today: the FIRST version of this gate
# had a bare `continue' for binaries that landed in no bucket at all, and a file
# acquiring a NUL then dropped out of the check and out of the totals with no
# number saying so. This line makes that shape fail loudly if anyone writes it
# again. Do not quote it as coverage. Found by review 2026-09-09.
if ok + bad + binfiles + unreadable != len(files):
    raise SystemExit("encodings: %d files but %d + %d + %d + %d accounted" %
                     (len(files), ok, bad, binfiles, unreadable))
print("COUNTS %d %d %d %d %d %d" % (ok, bad, skip, translated, binfiles, unreadable))
ENCPY
)
enc_field() { echo "$enc_out" | sed -n "s/^COUNTS $1.*/\\1/p"; }
ep=$(enc_field '\([0-9]*\) ')
ef=$(enc_field '[0-9]* \([0-9]*\) ')
es=$(enc_field '[0-9]* [0-9]* \([0-9]*\) ')
et=$(enc_field '[0-9]* [0-9]* [0-9]* \([0-9]*\) ')
eb=$(enc_field '[0-9]* [0-9]* [0-9]* [0-9]* \([0-9]*\) ')
eu=$(enc_field '[0-9]* [0-9]* [0-9]* [0-9]* [0-9]* \([0-9]*\)')
# ZERO, and it stays zero. The tree is LF only since 2026-09-09, so there is no
# pre-existing damage left to carry: the 25 files 03ac805 stripped are no longer
# a special case, they are simply what every file looks like now. Never raise
# this to make a diff go away -- a non-zero count means CRLF has come back.
ENC_BASELINE=0
# A FLOOR, because zero failures is also what a check that ran over nothing
# reports. `git ls-files' returning empty gives COUNTS 0 0 0 0 0 0, whose six
# numbers are all NON-EMPTY, so the empty-result branch below does not catch it:
# the gate printed a green row having read no files at all. The helper above now
# aborts on a non-zero git status, and this floor is the second half -- the tree
# holds 665 tracked files, so anything under 500 means the check did not run.
#
# IT COUNTS FILES SEEN, NOT FILES THAT PASSED, AND IT IS TESTED LAST. Both
# matter, and the first version got both wrong. `ep' alone is the LF-only count,
# which a tree-wide CRLF regression drives to ZERO -- exactly the case D31 says
# this gate exists for, a clone with core.autocrlf=true -- so the floor fired
# first and reported "it did not run", blaming the harness, and recorded ONE
# failure for 611 broken files. Found by review 2026-09-09.
ENC_FLOOR=500
# Fail CLOSED on an empty result. The failure this catches is "python exited 0
# but printed no COUNTS line": $ep/$ef come back empty, the numeric test errors,
# and because that is an `if' CONDITION set -e does not fire -- so the gate used
# to print blanks and score 0/0 while claiming to have run.
#
# If python exits NON-zero instead, set -e trips on the enc_out assignment above
# and the script stops there, printing no encodings row and no total: line. That
# is loud rather than silent, so it is left alone -- but a reader should not
# expect this branch to be what handles it. An earlier version of this comment
# said set -e does not fire at all, which is wrong.
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
    # NOT $ep. The row says 0 pass, so 0 pass is what the total must get: the
    # first version printed 0 and added up to 499 phantom passes to `total:'.
    ep=0
else
    printf '%-22s %2d LF-only  %2d CRLF, %s binary (git), %s translated, %s postdate root\n' \
           "encodings" "$ep" "$ef" "$eb" "$et" "$es"
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
    // 1.3's own data: experiments/twoBases.exp
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
# (SIGEL_CommonGUI/SIG_VisualisationWidget.cpp, ~SIG_VisualisationWidget), destroying the widget after
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
# BUILD IT FIRST. This section and the one below it run 240 lines BEFORE the
# `gui behaviour' section that builds guidrive, so until 2026-09-05 the first
# ./checks/check.sh after editing guidrive.cpp -- or after editing anything guidrive
# links -- scored a STALE binary here and a fresh one there. That is the exact
# shape fitness-check.sh and pvm-check.sh already guard against, applied to the
# wrong end of this script. Note also that `make -q B=build-fast SAN= SIGSAN='
# with no target answers for `all', which does NOT depend on guidrive: it
# reports up to date while build-fast/guidrive is stale. Name the target.
if ! make -s -C "$ROOT" B=build-fast SAN= SIGSAN= guidrive >/tmp/gdb.$$ 2>&1; then
    echo "  guidrive does not build; the two clip checks below prove nothing:"
    tail -6 /tmp/gdb.$$ | sed 's/^/    /'
fi
rm -f /tmp/gdb.$$

cp=0; cf=0
if SIGEL_ROOT="$SRC" QT_QPA_PLATFORM=offscreen \
       SIGEL_EXP="$ROOT/experiments/twoBases.exp" \
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
# Forms that can be dragged smaller than Qt 6 can lay them out.
#
# The question clipcheck structurally CANNOT ask. clipcheck walks widgets at
# one size and reports any child that leaves its parent; this asks what the
# smallest permitted size IS. An explicit <minimumSize> in a .ui becomes
# setMinimumSize(), which overrides minimumSizeHint() -- so a form whose
# declared minimum is below what its layout needs can be dragged down until its
# children compress, and a walk at the default size sees nothing wrong. Six
# forms were in that state until 2026-09-05; MT_StatisticsWidgetBase was the
# worst, 220x390 against a needed 402x555, and it sits in the MetaTrainer's
# QSplitter where a user can really drag it there.
#
# All TWENTY forms are measured, not the six that were wrong -- the scenario
# asserts the corpus is 20, so a shortened list cannot pass by testing nothing,
# which is the shape this file has been bitten by three times.
#
# The teeth: guidrive's own selftest forces one form's minimum below its hint
# and requires the same comparison to report it, so "TOO SMALL: 0" is not a
# clean result from a check that cannot fire. Measured the other way too --
# putting MT_StatisticsWidgetBase back to 220x390 makes this section fail by
# name. See PORTING.md.
#
# FOUR CAUSES, FOUR MESSAGES. The first version collapsed "a form is too small",
# "guidrive is stale", "timeout killed it" and "it crashed" into one sentence
# asserting the first -- and on the stale path /tmp/fmin.$$ was never created,
# because the `&&' short-circuits before the redirect, so the operator got that
# assertion with an empty body and no hint that the binary was the problem.
# `programs' and `slave gui' both do better and this now follows them. Stderr is
# KEPT for the same reason `gui behaviour' keeps it: a crash, a Qt fatal or a
# timeout kill lands there and nowhere else. Found by review.
#
# `make -q' carries -C "$ROOT" because this script never cd's; without it the
# section depended on the caller's working directory.
mp=0; mf=0
: > /tmp/fmin.$$
if ! make -q --no-print-directory -C "$ROOT" B=build-fast SAN= SIGSAN= guidrive 2>/dev/null; then
    mf=1
    echo "  build-fast/guidrive is missing or out of date -- this section did NOT"
    echo "  run. Build it with 'make B=build-fast SAN= SIGSAN= guidrive'."
else
    # `|| mrc=$?', not a bare run followed by `mrc=$?'. This script sets -e and
    # this call is a plain command in an `else' branch, so a non-zero formsize
    # made the SHELL exit right here: no form-minimums line, no pagesave
    # section, no forms section, no total. The silent-short-run shape this
    # file guards against everywhere else, in the one place that could not
    # report it.
    #
    # TWO of the branches below, not three: `mrc = 124' and `mrc != 0' were
    # unreachable dead code, but the `ngot != nui' one sits on the mrc=0 path
    # and has been live throughout -- guidrive returns 0 only when it measured
    # 20 forms, so that branch fires on a 21st .ui nobody added to the
    # scenario's table, which is what it is for. Corrected by review.
    mrc=0
    SIGEL_ROOT="$SRC" QT_QPA_PLATFORM=offscreen \
      SIGEL_EXP="$ROOT/experiments/twoBases.exp" \
      SIGEL_SCRATCH="${TMPDIR:-/tmp}" \
      timeout 300 "$ROOT/build-fast/guidrive" formsize >/tmp/fmin.$$ 2>/tmp/fmerr.$$ \
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
        # A KILLED PROCESS IS NOT A FORM DEFECT. This branch used to lead with
        # "a form declares a minimum below what Qt 6 needs" for EVERY non-124
        # status, which includes 137 (SIGKILL, i.e. the OOM killer this file
        # warns about at the pagesave section) and 139 (SIGSEGV). It had never
        # run in production to show that, because until 2026-09-07 `set -e'
        # killed the script before it -- so the message was never read against
        # a real signal. Split out by review.
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

# The same defect, in the two containers where it was actually FOUND -- and
# clipcheck cannot see either of them. Its own comment says so: the slave's
# simulation window and its movie-settings dialog belong to the slave, not to
# anything the master's menus can open, so they are walked by the `slavegui'
# scenario instead. Nothing here ran `slavegui', so until this section existed
# BOTH fixes -- GroupBox6 "Navigation" 90x37 against a needed 220x331, and
# groupboxDirectory "File conventions" 465x37 against 401x99 -- were ungated,
# while PORTING.md said they were gated by the section above. Found by review.
#
# This greps the two totals rather than diffing the whole scenario, because the
# rest of slavegui's output covers a GL view that does not render offscreen and
# would baseline noise. The teeth are the fixes themselves: deleting either
# <minimumSize> block from its .ui and rebuilding makes the matching line
# non-zero and this section fail -- measured both ways, see PORTING.md.
#
# Its stderr is KEPT, not discarded, because this is also the only run of
# SIGEL_SlaveGUI in the whole gate: 44 SIGNAL( and 44 SLOT( sites that had no
# runtime coverage at all until now. Same positive control as the two sections
# below -- guidrive makes one deliberately bogus connect at startup, so an empty
# stderr means the logging was suppressed, not that the connects are sound.
sp=0; sf=0
if SIGEL_ROOT="$SRC" QT_QPA_PLATFORM=offscreen \
       SIGEL_EXP="$ROOT/experiments/twoBases.exp" \
       SIGEL_SCRATCH="${TMPDIR:-/tmp}" \
       timeout 300 "$ROOT/build-fast/guidrive" slavegui >/tmp/sclip.$$ 2>/tmp/serr.$$; then
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
# The structural fingerprint tool's own teeth.
#
# expstruct.py is what compares an evolved .exp across the two architectures
# (PORTING.md 9, the evolution path). Everything it concludes rests on ONE
# property: it must be blind to fitness and sighted on structure. Fitness is
# not a cross-machine reference in either direction -- D26 and section 7
# measure a 1-ULP change in start height moving fitness 45%, and the reference
# box is i386/x87 against this one's aarch64 -- while the tournament that picks
# survivors is a bare `>=' between two of those doubles
# (SIG_GPSimpleTournament.cpp, run). So a tool that let one fitness value reach
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
# -C "$ROOT" for the reason the `form minimums' section states above: this
# script never cd's, so without it this gate depended on the CALLER's working
# directory and, run from anywhere else, failed into "the two programs are not
# built or are out of date" -- blaming the build for the invocation. That
# comment was written 165 lines up and this line was missed. Found by review.
if make -q --no-print-directory -C "$ROOT" B=build-fast SAN= SIGSAN= programs 2>/dev/null; then
    for prog in sigel sigel_slave; do
        f=$ROOT/build-fast/$prog
        if [ ! -x "$f" ]; then
            pf=$((pf+1)); echo "  build-fast/$prog missing"
        elif [ "$(head -c 4 "$f" | tail -c 3)" != ELF ]; then
            # A SHELL WRAPPER LEFT IN PLACE OF THE BINARY PASSED EVERYTHING
            # ELSE. It is -x, it execs the real binary so the smoke test below
            # still prints the no-PVM guard, and `make -q' calls the target
            # current because the wrapper's mtime is newer than every
            # prerequisite -- so the link recipe never runs and neither does
            # the ctor_size assertion inside it. Measured: check.sh went fully
            # green against a 122-byte /bin/sh script standing in for
            # sigel_slave. Found by review.
            pf=$((pf+1)); echo "  build-fast/$prog is not an ELF binary"
        else pp=$((pp+1)); fi
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
# The behaviour matches the RUNNING 1.3 binary, compared on
# twoBasesSimpleFitness2.exp; the values are the port's own for twoBases.exp.
# guibehaviour-baseline.txt says which fact came from where. On twoBases.exp,
# reverting the deletion fix makes the scenario exit 1; `nameIsASurvivor=0'
# covers a stale index that stays in range. SURVIVED is a liveness check.
#
# It needs an experiment to open, so it is skipped rather than failed when the
# experiment file is absent.
bp=0; bf=0
BEXP=$ROOT/experiments/twoBases.exp
if [ ! -f "$BEXP" ]; then
    # experiments/ is tracked, so only a damaged checkout lands here. Say
    # SKIPPED loudly and count it: reporting `0 pass 0 fail' made the section
    # vanish from the total and left the exit status clean, which is the same
    # shape as the three silent-short-run holes this file has already been
    # bitten by.
    echo "  SKIPPED: no $BEXP -- restore experiments/ from git, then re-run."
    echo "  THIS SECTION TESTED NOTHING."
    skipped=$((skipped+1))
elif make -s -C "$ROOT" B=build-fast SAN= SIGSAN= guidrive >/tmp/bdb.$$ 2>&1; then
    # SIGEL_ROOT must be the SOURCE tree: the driver loads pixmaps and terrain
    # from it. Neither scenario spawns a sigel_slave, so neither needs one.
    #
    # ELEVEN scenarios make up the baseline, concatenated in this order
    # (`roundtrip' 2026-09-03, `metadrive' 2026-09-04, `runlock' 2026-09-05,
    # `openfocus' 2026-09-16; this said SIX until the first of those).
    # `runlock' is the ONLY thing that
    # executes D29's guard in its locked state -- a review measured that both
    # of D29's mechanisms could be reverted wholesale with all 846 checks
    # green, and that the emit it tests IS observable, so those passes were a
    # real absence of teeth rather than a blind probe:
    #   gate       C10 -- the tree, sorting, add/delete/reset, the dialogs,
    #              the context menus, the MetaGP warning
    #   pages      C11a -- the five View pages C10 never opened, every spin
    #              box, slider, combo, checkbox and validator on them
    #   exportall  C11b -- all eight File > Export children, each file's
    #              sha256, size, line count and ends. SEVEN of the eight
    #              match what the 2003 i386 binary writes byte for byte,
    #              checked by the oracle on twoBasesSimpleFitness2.exp. The
    #              eighth, .lap, differs for a reason the baseline records.
    #   overwrite  D35 -- an export over an existing file. A name without
    #              the extension must give a date-stamped file and no prompt.
    #              The name with it must raise the file dialog's own
    #              confirmation, as a child of the dialog, and No must leave
    #              the file alone.
    #   metagui    MT_GUI -- the MetaGP window, which nothing had ever
    #              opened. Its ten validators had never been given C7's C-locale
    #              treatment, so an unpinned QIntValidator(0,1000) called
    #              "1,000" ACCEPTABLE under en_US while text().toInt() returned
    #              0: a user types one thousand and zero reaches the system.
    #              1.3 rejects both separators -- measured on the binary -- so
    #              the fix restores Qt 2 rather than improving on it.
    #   openfocus  the flag guard in
    #              SIG_ExperimentListView::slotLoadExperiment. It sends the
    #              focus event a window manager sends while the file is being
    #              read. Reverting the guard alone moves `orphan'; reverting
    #              the null checks too makes it segfault, which fails the run
    #              rather than the diff.
    #   dialogs    C11c -- the six dialogs, C7's 21st validator, and the
    #              select-on-focus defect. The load-bearing lines are the two
    #              `typing "5" gives [0.015]' / `typing "2" gives [12]' ones:
    #              Qt 6 selects a pre-filled field when a dialog hands it focus
    #              and Qt 2 did not, so before the fix a typed digit REPLACED
    #              the value instead of appending to it -- 2 individuals added
    #              where 1.3 adds 12. Both figures are the oracle's, off the
    #              running binary.
    #
    # `roundtrip' IS run here as of 2026-09-03 -- PORTING.md's pagesave/roundtrip gap. This comment used
    # to say it was not, on the grounds that "what it uniquely covers is
    # largely covered by exportall (a broken reader moves the export)". THAT
    # ARGUMENT IS FALSE and gating it is what showed so: gutting each of the
    # five readers in turn moves roundtrip and NOT exportall, because exportall
    # never reads anything back. It costs 103 s measured, against 30 for
    # exportall -- the most expensive scenario here by 3.4x, and worth it.
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
    # `|| bf=1' because this runs under `set -e' with no trap: a bare `: > path'
    # that fails (unwritable or full /tmp) would abort the WHOLE script here,
    # with no summary line and no total -- the silent-short-run shape this file
    # has already been bitten by three times, but applied to everything rather
    # than one section.
    : > /tmp/berr.$$ || bf=1
    # roundtrip joined the list 2026-09-03, the other half of PORTING.md's pagesave/roundtrip gap. It is
    # export-import-export on ONE machine and needs no 1.3 reference: its point
    # is that the READER undoes a mutation made between the two exports, so a
    # no-op importer -- the likelier failure -- cannot pass it. C11c had to fix
    # exactly that: the probe used to serialise the same in-memory object twice
    # and printed STABLE regardless. It is deterministic across runs and costs
    # 103 s, the most expensive scenario here by a wide margin.
    if guidrive_run gate /tmp/bo.$$ && guidrive_run pages /tmp/bp.$$ \
       && guidrive_run exportall /tmp/bx.$$ && guidrive_run overwrite /tmp/bw.$$ \
       && guidrive_run dialogs /tmp/bg.$$ && guidrive_run metagui /tmp/bm.$$ \
       && guidrive_run roundtrip /tmp/br.$$ \
       && guidrive_run metadrive /tmp/bv.$$ \
       && guidrive_run runlock /tmp/bk.$$ \
       && guidrive_run rngseed /tmp/bz.$$ \
       && guidrive_run openfocus /tmp/bq.$$; then
        cat /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ \
            /tmp/br.$$ /tmp/bv.$$ /tmp/bk.$$ /tmp/bz.$$ /tmp/bq.$$ > /tmp/ball.$$
        # THE RUNTIME-CONNECT CHECK AND ITS POSITIVE CONTROL.
        #
        # Qt says "No such signal"/"No such slot" at RUNTIME when a
        # string-based connect names something that does not exist. It
        # compiles, it links, and the slot simply never fires -- the failure
        # class C4 found. $DEAD_SIGNALS cannot replace this: it is a closed
        # regex over the nine Qt 2 spellings in §2, matching SIGNAL( only, so a
        # tenth kind and every bad SLOT() are invisible to it. Nor is it the
        # other way round -- the regex is STATIC over all 14 modules while this
        # is runtime over only what these eleven scenarios execute. Partly
        # disjoint, so both are kept.
        #
        # But Qt emits it under the logging category qt.core.qobject.connect,
        # and categories are filterable. QT_LOGGING_RULES='*=false' in the
        # ambient environment (guidrive_run uses `env' without -i, so the whole
        # environment passes through) or a qtlogging.ini silences it, and an
        # empty stderr then looks exactly like a clean run. Review demonstrated
        # the full gate passing green with a genuinely dead connect injected.
        #
        # So guidrive makes ONE deliberately bogus connect at startup and this
        # requires its warning to be present. Same category, same mechanism --
        # a control in `default' would not do, because qt.core.qobject.connect
        # can be disabled on its own. An unfired control means this check could
        # not have fired either, which is worth a failure on its own: check.sh
        # already says of clipcheck that "0 clipped" from a check that cannot
        # detect clipping is worth nothing.
        CTL=guidriveStderrControl
        if ! command grep -q "$CTL" /tmp/berr.$$; then
            bf=1
            echo "  Qt's connect logging is SUPPRESSED -- the runtime-connect check"
            echo "  could not have fired, so this run proves nothing about connects."
            echo "  Unset QT_LOGGING_RULES (or remove a qtlogging.ini) and re-run."
        fi
        # NOT an elif chain with the `!!' check below: a dead connect is the
        # most likely CAUSE of a `!!' -- the driver clicks, the slot never
        # fires, the driver reports it could not do the thing -- so reporting
        # only the symptom hides the diagnosis exactly when it explains it.
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
        # must not pass, and must not be diffed into a baseline either: one
        # DID get committed that way, a population export that silently wrote
        # nothing, and only a later diff caught it. Checked before the diff so
        # the message is about the right thing.
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
        # stderr is captured now, and this is the path where it is most likely
        # to say why. Printing it here is the whole reason for capturing it
        # rather than discarding it: a crash, a Qt fatal, an ASan report or a
        # timeout kill all land here.
        if [ -s /tmp/berr.$$ ]; then
            echo "  and the driver's stderr said:"
            tail -8 /tmp/berr.$$ | sed 's/^/    /'
        fi
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
rm -f /tmp/bo.$$ /tmp/bp.$$ /tmp/bx.$$ /tmp/bw.$$ /tmp/bg.$$ /tmp/bm.$$ /tmp/br.$$ \
      /tmp/bv.$$ /tmp/bk.$$ /tmp/bz.$$ /tmp/bq.$$ \
      /tmp/bl.$$ /tmp/bl2.$$ /tmp/ball.$$ \
      /tmp/bd.$$ /tmp/bdb.$$ /tmp/berr.$$
# exportall, overwrite and roundtrip WRITE FILES -- 8.2 MB between them, the
# two population exports being most of it. Fixed names, so they are overwritten
# rather than accumulated, but leaving them in TMPDIR is untidy. roundtrip's
# fifteen were missed when it joined the list and left 5.5 MB per run behind.
rm -f "${TMPDIR:-/tmp}"/x11b-gpp.gpp "${TMPDIR:-/tmp}"/x11b-sip.sip \
      "${TMPDIR:-/tmp}"/x11b-lap.lap "${TMPDIR:-/tmp}"/x11b-env.env \
      "${TMPDIR:-/tmp}"/x11b-pop.pop "${TMPDIR:-/tmp}"/x11b-prg.prg \
      "${TMPDIR:-/tmp}"/x11b-ind.ind "${TMPDIR:-/tmp}"/x11b-dat.dat \
      "${TMPDIR:-/tmp}"/x11b-ow.sip "${TMPDIR:-/tmp}"/x11b-ow "${TMPDIR:-/tmp}"/x11b-ow-*.sip \
      "${TMPDIR:-/tmp}"/c11c-lap.lap
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
# REAL X INPUT -- the only section here that does not drive Qt through QTest.
#
# Everything above posts QMouseEvent through QApplication::notify. That reaches
# every slot SIGEL has, which is why those sections are worth something, but it
# never goes through QWindowSystemInterface. PORTING.md's C10 section named the
# four things that leaves untested: activation, the pointer grab a popup takes,
# Qt's synthesis of a double click out of two presses. Enter and leave was
# listed here too and that was wrong: QTest::mouseMove on a widget calls
# QCursor::setPos(), a real pointer warp. Offscreen could not deliver them,
# QTest can. See PORTING.md C13.
#
# This section runs guidrive as a REAL X11 CLIENT inside a nested Xvfb with
# QT_QPA_PLATFORM=xcb and drives it with XTEST through xdotool, so the port
# gets a genuine click where until now only the 1.3 oracle's side did.
#
# ITS POSITIVE CONTROL IS INSIDE THE SCENARIO AND IS CHECKED HERE. The whole
# section is worthless if the clicks are not real -- a broken xdotool, a display
# that never came up, or a fallback to a synthetic path would leave every count
# at zero, which looks exactly like "the port ignores real clicks". So the
# scenario compares one real click against one QTest::mouseClick at the same
# point through a native event filter and prints DISCRIMINATES only when the
# real one produced native ButtonPress events and QTest produced none. Its
# absence fails this section on its own.
#
# NOT skipped when Xvfb or xdotool is missing. A skip here is a section that
# tested nothing while reporting no failure, which is the shape check.sh has
# been bitten by before.
xtp=0; xtf=0
XTDISP=:97
if [ ! -f "$ROOT/checks/baselines/xtest-baseline.txt" ]; then
    xtf=1; echo "  checks/baselines/xtest-baseline.txt is missing -- this gate tested NOTHING"
elif [ ! -f "$BEXP" ]; then
    # Same data dependency as the two sections above: without the file the run
    # hangs in the modal Load dialog until the timeout and blames the driver
    # for a missing file.
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
elif [ ! -x "$ROOT/build-fast/guidrive" ]; then
    xtf=1; echo "  guidrive is not built -- the real-input section tested NOTHING"
elif ! (cd "$ROOT" && make -q B=build-fast SAN= SIGSAN= guidrive) 2>/dev/null; then
    # NAME THE TARGET. `make -q' with no target answers for `all', which does
    # not depend on guidrive. The section above builds it and only sets bf=1 if
    # that build fails, so without this check a compile failure leaves the
    # PREVIOUS binary in place and this section scores it -- printing
    # "1 pass 0 fail" for source it never compiled. Found by review. Section 7
    # of PORTING.md already states this rule for the three gate scripts; it was
    # missing here.
    xtf=1
    echo "  build-fast/guidrive is out of date, so this section would have"
    echo "  measured a binary that is not the source in the tree."
elif true; then
    # A display already in use would make every click land in someone else's
    # session, so refuse rather than share one.
    #
    # DISPLAY=, NOT --display. xdotool has no --display option: it answers
    # "getdisplaygeometry: unrecognized option" and exits 1 whatever the state
    # of the server. The first version of this section used --display in both
    # places, so this guard could never fire and the readiness poll below could
    # never succeed -- and the poll, written as `... && break', silently
    # degraded into a fixed 15 s sleep that happened to be long enough. The
    # gate caught it by failing on its first real run; a run by hand had not,
    # because nothing there checked the loop's outcome.
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
            # `|| xtf=1', not bare. This file runs under `set -e' with no
            # trap, so a bare `: > path' that fails on a full or unwritable
            # /tmp would abort the WHOLE script here. It would do that AFTER the
            # Xvfb above was started and BEFORE the kill below, leaving a server
            # on $XTDISP that makes every later run refuse the display. The
            # neighbouring section already writes `: > /tmp/berr.$$ || bf=1' for
            # the same reason. Found by review.
            : > /tmp/xterr.$$ || xtf=1
            # SCRUB THE SCALING VARIABLES. This is the only section here whose
            # result depends on Qt's coordinate scaling. mapToGlobal() returns
            # logical pixels and xdotool takes device pixels, so at a ratio of
            # 1.25 every click is real but lands 20 per cent away, and the run
            # reports a false difference from 1.3. `env' is used without `-i',
            # so the caller's whole environment passes through. Review
            # demonstrated it with QT_SCALE_FACTOR=1.25. Offscreen is immune,
            # because it pins the ratio, which is why no other section needs
            # this. guidrive also refuses a devicePixelRatio other than 1 on its
            # own, so a caller who runs it directly is covered too.
            if env DISPLAY="$XTDISP" SIGEL_ROOT="$SRC" SIGEL_EXP="$BEXP" \
                   SIGEL_SCRATCH="${TMPDIR:-/tmp}" QT_QPA_PLATFORM=xcb \
                   QT_SCALE_FACTOR=1 QT_SCREEN_SCALE_FACTORS= \
                   QT_ENABLE_HIGHDPI_SCALING=0 QT_AUTO_SCREEN_SCALE_FACTOR=0 \
                   QT_FONT_DPI= QT_SCALE_FACTOR_ROUNDING_POLICY=Round \
                   timeout 300 "$ROOT/build-fast/guidrive" xtest \
                   > /tmp/xt.$$ 2>/tmp/xterr.$$; then
                # The control, before the diff: if the run could not tell a real
                # click from a QTest one, the numbers below mean nothing and the
                # baseline would happily match a run in which nothing was
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
# PORTING.md's pagesave/roundtrip gap -- the widget-to-file path, which nothing covered until now.
#
# `pages' proves typing reaches the widgets. `exportall' proves widgets reach a
# file, in one direction, for the eight export formats. NEITHER of them runs
# putAllIntoExperiment(), so a regression between what a parameter page holds
# and what `File > Save Experiment' writes was caught by nothing at all. That
# was PORTING.md's pagesave/roundtrip gap, and it stood open because gating it needs a reference the port
# did not produce itself -- §7's rule that every gate here compares the port
# against itself.
#
# It has one: the port's save matches the 2003 i386 binary's own byte for
# byte, compared on twoBasesSimpleFitness2.exp. pagesave-baseline.txt holds the
# port's own save of twoBases.exp, on that proven ground. The EDITED half is
# the base plus C11a's eleven page edits; the file's header says why.
#
# LanguageParameters is checked separately because it is NOT in the block --
# it sits at line 71664 of the saved file, far below POPULATION BEGIN{ at 193,
# so a check over the block alone would silently miss the registers edit.
pp=0; pf=0
PSD="${TMPDIR:-/tmp}"
if [ ! -f "$ROOT/checks/baselines/pagesave-baseline.txt" ]; then
    pf=1; echo "  checks/baselines/pagesave-baseline.txt is missing -- this gate tested NOTHING"
elif [ ! -f "$BEXP" ]; then
    # Same data dependency and therefore the same policy as `gui behaviour'
    # above. Without this the two runs HANG in the modal Load dialog until the
    # 300 s timeout, twice, and report "did not finish", which blames the
    # driver for a missing file.
    echo "  SKIPPED: no $BEXP -- restore experiments/ from git. THIS SECTION"
    echo "  TESTED NOTHING."
    skipped=$((skipped+1))
elif [ -x "$ROOT/build-fast/guidrive" ]; then
    # Same env as guidrive_run above, minus the locale extras. The two runs
    # differ only in SIGEL_PAGEEDIT, which selects the eleven-edit set.
    psrun() {
        env ${2:+SIGEL_PAGEEDIT=1} SIGEL_ROOT="$SRC" SIGEL_EXP="$BEXP" \
            SIGEL_SCRATCH="$PSD" QT_QPA_PLATFORM=offscreen \
            timeout 300 "$ROOT/build-fast/guidrive" pagesave > "$1" 2>>/tmp/pserr.$$
    }
    rm -f "$PSD/pagesave-base.exp" "$PSD/pagesave-edited.exp"
    : > /tmp/pserr.$$
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
            # This is the only thing that catches a string-based connect naming
            # something Qt 6 does not have, and File > Save Experiment is not
            # exercised by any other scenario.
            pf=1
            echo "  a connect on the save path names a signal or slot that does not exist:"
            command grep -E 'No such (signal|slot)' /tmp/pserr.$$ \
                | command grep -v guidriveStderrControl | sort -u | head -4 | sed 's/^/    /'
        else
            # Everything before POPULATION BEGIN{ is the parameter block the
            # five View pages own. Raw bytes, not key/value pairs: two keys in
            # this data (FLOORPICTUREFILE, TEXTUREFILE) have an EMPTY value
            # line after them, so anything that skips blanks reads the next key
            # as a value and desynchronises silently from there on.
            # `|| true' on both greps is LOAD-BEARING, not tidiness. This runs
            # under `set -e' with no trap, so a grep that matches NOTHING exits
            # 1 and kills the WHOLE SCRIPT here -- no pagesave line, no forms
            # section, no total, no summary. And a missing LanguageParameters
            # line is EXACTLY the regression this check was added to catch, so
            # without these the gate would abort silently on its own quarry.
            # The empty output then fails the diff, which is the right outcome.
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
            # NOT `grep -v ^#': the DATA contains six `#####' separator
            # lines, and stripping every #-leading line ate them -- caught by
            # this gate failing on its own first run. Header comments are
            # `# text' or bare `#', so this strips those and nothing else.
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
        # SAY WHY. The first time this fired it printed this header and NOTHING
        # else -- both stdout files were empty and the captured stderr was
        # deleted unread, so an intermittent failure produced no evidence at
        # all. That is the shape this file keeps being bitten by, and the
        # `gui behaviour' section above had already been fixed for it.
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
    pf=1; echo "  build-fast/guidrive is missing -- run 'make B=build-fast SAN= SIGSAN= guidrive'"
fi
rm -f /tmp/ps1.$$ /tmp/ps2.$$ /tmp/psall.$$ /tmp/psd.$$ /tmp/pserr.$$
printf '%-22s %2d pass  %2d fail\n' "pagesave" "$pp" "$pf"
pass=$((pass+pp)); fail=$((fail+pf))


# ---------------------------------------------------------------------------
# V5 -- the truncated pi, and the constants the sensor path is built on.
#
# SIGEL 1.3 converts radians to degrees with pi TRUNCATED TO EIGHT DECIMALS.
# Not 180/pi but 180/3.14159265, which is 57.29577957855229 against the true
# 57.295779513082323 -- a relative error of 1.14e-09.
#
# THAT IS NOT A DEFECT TO FIX. IT IS THE BEHAVIOUR BEING PRESERVED. Every
# evolved program in the shipped experiments was selected against sensor
# values carrying that error, and the values feed a chaotic simulation, so
# "correcting" it changes what the robots do. The port's rule is convert, do
# not improve; PORTING.md lists this with the 1.3 behaviour preserved on
# purpose. This section exists because the change is a ONE-WORD EDIT that
# looks like tidying -- write M_PI and the gate is the only thing that
# notices.
#
# AND 1.3 INVITES THE EDIT, because it is inconsistent with itself. It
# uses the true M_PI in SIGEL_Robot/IFunctions.cpp, calculateAnyJoint and the truncated
# literal in the two simulation files. That is 1.3's own inconsistency,
# preserved verbatim -- IFunctions.cpp is untouched since the vendor drop
# apart from comment translation. A reader who finds the M_PI first will read
# the 3.14159265 as an oversight.
#
# MEASURED ON THE 1.3 BINARY, through the sigel-x86 session 2026-09-08, and
# the decisive part is an ABSENCE:
#     sigel_slave   404ca5dc1af05a77 (truncated)   1 occurrence
#                   404ca5dc1a63c1f8 (true 180/pi) 0 occurrences
#     sigel         both                           0 occurrences
# The correct constant is in NEITHER shipped 1.3 binary. So this is not "1.3
# happens to use a truncated pi somewhere"; the true value is absent from the
# image. Full capture, with the .rodata table and the disassembly, in
# verification-against-sigel-1.3/v5-1.3-mdh-compared.txt.
#
# TWO CHECKS, because neither covers the other.
#   SOURCE  catches an edit at one of the four sites even when another site
#           still supplies the same constant, which a binary search cannot
#           see. It is also independent of the compiler.
#   BINARY  catches any SPELLING that produces the true value -- M_PI,
#           4*atan(1), a longer literal, a header constant -- which a grep for
#           `M_PI' would miss.
#
# WHY ONLY THE RADIAN FACTOR IS GATED. The sensor path references eight
# constants and all eight agree with 1.3 AS VALUES, checked from the source.
# Only the radian factor can be gated in the binary, and the reason is
# measured, not assumed: on this machine -DBL_MAX, +DBL_MAX, 360.0 and -90.0
# appear ZERO times as 8-byte doubles in our image, and 90.0 and 180.0 appear
# only inside debug sections. aarch64 folds them into immediates or into
# larger expressions instead of emitting them. 2.0 does appear, 442 times,
# which is noise. So the radian factor is the only one of the eight with a
# .rodata entry to compare, and it is also the only one anybody would edit.
#
# NOTE ON SPELLING: our sense writes `360.0 / (2.0*3.14159265)' where 1.3's
# image holds the folded 180/3.14159265. Different expression, IDENTICAL
# bits -- 2.0*x is exact and 360/2x is the same correctly-rounded quotient as
# 180/x. Verified, not assumed.
v5p=0; v5f=0
V5SRC=$SRC/src/SIGEL_Simulation
V5Q=$V5SRC/SIG_DynaMechsSimulationQueries.cpp
V5C=$V5SRC/SIG_DynaMechsCommandInterface.cpp
V5BIN=$ROOT/build-fast/sigel_eval
if [ ! -f "$V5Q" ] || [ ! -f "$V5C" ]; then
    v5f=1; echo "  the two simulation sources are missing -- nothing was checked"
else
    # 4 sites: three in sense (rad->deg) and one in moveDrive (deg->rad).
    # ANCHORED. A bare `3\.14159265' is a PREFIX match, so lengthening one site
    # to 3.14159265358979 keeps the count at 4 while changing the factor to
    # 404ca5dc1a63c200 -- which is neither the kept constant nor either
    # forbidden one, so the binary half misses it too. Measured: the whole
    # section passed on that edit. The trailing class closes it.
    # COMMENTS STRIPPED BEFORE BOTH COUNTS. A note saying "do not change this
    # to M_PI" is documentation, not a defect, and a comment quoting the
    # literal is not a fifth site. Measured: without this, adding either kind
    # of comment failed the gate.
    v5t=$(sed 's://.*::' "$V5Q" "$V5C")
    v5n=$(printf '%s\n' "$v5t" | command grep -Ec '3\.14159265([^0-9]|$)' || true)
    v5m=$(printf '%s\n' "$v5t" | command grep -c 'M_PI' || true)
    # M_PI FIRST. Tidying a site to M_PI also drops the count, so both tests
    # fire; the substitution is the specific diagnosis and must be the one
    # printed. Measured -- with the count tested first, an M_PI edit reported
    # only "expected 4", which points at the wrong thing.
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
        v5f=1; echo "  no $V5BIN -- run 'make B=build-fast SAN= SIGSAN= all'"
    elif ! make -q -C "$ROOT" --no-print-directory B=build-fast SAN= SIGSAN= all 2>/dev/null; then
        v5f=1; echo "  $V5BIN is out of date -- run 'make B=build-fast SAN= SIGSAN= all'"
    else
        # .rodata ONLY, and that bound is load-bearing rather than tidiness.
        # The Makefile compiles with -g, so the constant also appears twice in
        # .debug_loclists. Searching the whole file made the "it is missing"
        # arm UNREACHABLE: patching the real constant out of .rodata still
        # left two debug copies, so the count never fell below one. Measured
        # on this binary -- 1 in .rodata at 0x141658, 2 in .debug_loclists.
        # Debug sections are not what the program computes with.
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
            # site count: replacing the kept constant with the true one does
            # BOTH -- it removes the kept value and introduces the forbidden
            # one. Measured with the .rodata word patched: tested the other
            # way round, the run reported only "is NOT in", which describes
            # the symptom and not the change.
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
# V2 -- whole experiments through File > Save Experiment.
#
# The hammer half of the expected report is copied from a capture of the
# running 2003 binary, taken before this conversion existed:
#
#   verification-against-sigel-1.3/v8-1.3-gp-blocks.txt   hammer, 2026-08-29
#
# So a failure in the hammer half is a regression against 1.3. The octopus half
# is the port's own output. What 1.3 does with this robot is in
# verification-against-sigel-1.3/v1-1.3-roundtrip.txt, captured on
# octopusSimpleFitness: the same robot stored in the other order, which 1.3
# writes back as the order octopus.exp stores.
#
# TWO EXPERIMENTS, BECAUSE ONE OF THEM PROVES LESS THAN IT LOOKS.
# hammer has 5 links, 4 joints, 4 drives and no sensors at all -- few enough
# that no hash bucket need collide. 1.3 does not permute its link, joint or
# drive containers either, so agreement there is not evidence. What hammer
# really tests is material order, `Body' emission order and `middle3''s axis
# points, plus everything outside the robot: the section line counts, PVMHOST
# order, the experiment history, the HISTORY growth defect and the ten
# first-save keys.
#   octopus supplies the rest. Its joint, drive and sensor containers DO
# collide -- 1.3 permutes all three on every save, plus the body order and the
# command list, and the port keeps the stored order. That is where "we
# reproduced the order" and "we never permute" come apart.
#
# TWO SAVES EACH, NOT V8'S THREE. Pass 0 to 1 shows the ten keys arrive; pass 1
# to 2 shows them hold and gives the steady-state growth. A third save only
# repeats the second.
#
# INPUT AGAINST PASS 1 CANNOT BE THE TEST -- V8 result 5. The shipped .exp are
# a 2001 format revision and the 2003 binary adds ten keys with defaults on the
# first save. A gate comparing a shipped file against its own round trip fails
# however correct the port is. So the test is pass 1 against pass 2, and the
# ten keys are asserted by name and value instead.
#
# THE TWO DIVERGENCES ARE IN THE EXPECTED TEXT ON PURPOSE, not filtered out:
#
#   robot block   Ours is a FIXED POINT -- byte-identical in all three passes,
#                 on both robots. 1.3's is an involution: state0 == state2 and
#                 state1 == state3. That is D3's flip to insertion order,
#                 decided deliberately, and PORTING.md's V2, V6 and D3 carry
#                 it. Measured on 1.3 for hammer through the sigel-x86 session
#                 2026-09-08, and for the octopus robot by V1 in 2026-08.
#                 A SINGLE SAVE CANNOT SEE THIS. It only shows the order we
#                 wrote, not whether a second save would move it. That is why
#                 there are two saves and why octopus is here.
#
#   TEXALPHA      99 here, 255 in the V8 capture. IT IS NOT A PORT DEFECT.
#                 1.3 writes BOTH values, from its two save paths. Measured on
#                 1.3 for hammer 2026-09-08: its GUI save and its headless save
#                 differ in EXACTLY ONE LINE and nothing else -- 255 against
#                 99, with the rest byte-identical including markers, PVMHOST,
#                 the experiment history and the HISTORY growth. The same
#                 split shows on a second robot: 1.3's own GUI save of
#                 twoBasesSimpleFitness2, captured 2026-09-03, reads 99.
#                 The mechanism, in 1.3's source and Qt 2's:
#                 SIG_Environment.cpp:47 defaults texAlpha to 0xFF;
#                 SIG_EnvironmentView.cpp, getOutOfExperiment pushes it into sliderAlpha and
#                 :112 reads it back out. The pristine form gives that slider
#                 no maximum -- `git show 0516d62:…/SIG_EnvironmentBase.ui',
#                 not the converted file at that path today -- and Qt 2 then
#                 caps it at QRangeControl's default of 99
#                 (qrangecontrol.cpp:111-119, reached from qslider.cpp:124).
#                 Qt 6 has no QRangeControl; its 0-99 default comes from
#                 QAbstractSlider. Different class, same number, same result.
#                 The headless path has no slider and keeps 255.
#                 Ours is a GUI save, so 99 is 1.3's own GUI value.
#
# NOT EVERY LINE BELOW IS 1.3's, and the diff labels say so. Three kinds:
#
#   1.3's own numbers, from the hammer capture -- the markers, PVMHOST, the
#   experiment-history line count and its first and last entry, the first
#   block's character counts, the HISTORY growth and the ten first-save keys.
#
#   1.3's DATA, read back out. The individual names and the hammer robot-block
#   hash are the shipped file's own bytes, so pinning them pins this build
#   against 1.3's artefact even though no capture quotes them.
#
#   OURS, and only ours: the `expstruct' hash, and the whole octopus half. V8
#   never ran expstruct; it is kept because it covers the population, which
#   nothing else here reaches. The octopus rows are the stored order, which the
#   port writes back unchanged; v1-1.3-roundtrip.txt quotes the other order,
#   the one 1.3 writes.
#
# WHY ONE DIFF RATHER THAN A DOZEN ifs: so that a generator which produces the
# wrong text, or stops early, fails on the whole report instead of on the one
# predicate someone remembered to write. The generator runs inside `|| v2f=1',
# which suspends `set -e' for it -- without that a single failing command in
# there would kill the WHOLE script, with no v2 line, no forms section and no
# total. That is the shape this file has been bitten by three times.
#
# TEETH-TESTED 2026-09-08, IN TWO ROUNDS, AND IT FOUND TWO HOLES IN THIS
# SECTION. Both are closed above and both are the same mistake in two
# directions -- a line that compares this run against itself and nothing else.
#
#   Round one mutated an intermediate pass file. It found that three lines
#   hashed an extract and compared three hashes, which says "identical" when
#   the EXTRACTOR dies, since three empty strings are equal. Replacing
#   expstruct.py with /bin/false passed the gate 1 pass 0 fail. Those lines
#   now print the size of what they hashed.
#
#   Round two mutated the INPUT experiment instead. It found that every
#   "identical in all three" line passes when the input changes, because all
#   three passes change with it. Renaming an individual, permuting hammer's
#   materials and editing an experiment-history entry all went unnoticed.
#   Those lines now print content too -- a hash, or a first and last entry.
#
# What the mutations show is one-way: each predicate CAN fail on a change of
# the kind it exists to catch. It is not that each mutation moves exactly one
# line, and an earlier version of this comment claimed that wrongly. A deleted
# key shifts every marker below it, and any edit to pass 1 or 2 also moves
# `expstruct'.
#
# The wrapper was tested too: missing data SKIPS and counts, a missing or
# stale binary FAILS, suppressed Qt connect logging FAILS, and the section was
# run from OUTSIDE the repo root to check the `make -q -C "$ROOT"' fix.
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
elif [ ! -x "$ROOT/build-fast/guidrive" ]; then
    v2f=1; echo "  build-fast/guidrive is missing -- run 'make B=build-fast SAN= SIGSAN= guidrive'"
elif ! make -q -C "$ROOT" --no-print-directory B=build-fast SAN= SIGSAN= guidrive 2>/dev/null; then
    # A failed make leaves the previous binary in place, so a test for
    # existence passes on a stale one. D13 was scored green that way.
    # -C "$ROOT" because this script never cd's -- without it the check
    # depends on the caller's working directory and reports a false failure
    # from anywhere but the repo root. The rule is stated at the top of the
    # `form minimums' section and two other sites already obey it.
    v2f=1; echo "  build-fast/guidrive is out of date -- run 'make B=build-fast SAN= SIGSAN= guidrive'"
elif ! mkdir -p "$V2D"; then
    v2f=1; echo "  cannot create $V2D"
else
    # Guarded: a bare `cp && chmod' under `set -e' would abort the whole
    # script rather than fail this section.
    cpok=1
    { cp "$V2HAM" "$V2D/ham0.exp" && chmod u+w "$V2D/ham0.exp" \
      && cp "$V2OCT" "$V2D/oct0.exp" && chmod u+w "$V2D/oct0.exp"; } || cpok=0
    : > "$V2D/err" || cpok=0
    # pagesave with SIGEL_PAGEEDIT unset is exactly File > Open then File >
    # Save Experiment. No new scenario was added.
    # $1 = stem, $2 = input pass number.
    v2run() {
        rm -f "$V2D/pagesave-base.exp"
        env SIGEL_ROOT="$SRC" SIGEL_EXP="$V2D/$1$2.exp" SIGEL_SCRATCH="$V2D" \
            QT_QPA_PLATFORM=offscreen timeout 300 "$ROOT/build-fast/guidrive" \
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
    # Prefixed names because dash has no function scope and this script is one
    # namespace: a bare s= and e= here would be visible to every later section.
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
    # A key's value line COUNT is part of the shape and is declared, not
    # discovered: FLOORDIMENSION has two (X and Z), the other nine have one.
    # Without the cap a key swallows every line down to the next ALL-CAPS one,
    # and AUTOSAVETIME then reports GPSconst, GNSconst and four more as its
    # own value -- measured while writing this, not supposed.
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
        # Its own positive control, on its own stderr. The `pagesave' section
        # above checks the same scenario, but on a DIFFERENT stream from a
        # different run -- `slave gui' carries a duplicate of this check for
        # exactly that reason. Without it an empty stderr cannot be told from
        # a suppressed one.
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
        # CONTENT, not only stability. Without these two the section compares
        # section 6 against ITSELF across the three passes and never against
        # 1.3 -- a mutated input passes, measured 2026-09-08. V8 result 4
        # quotes both lines from the 1.3 run, so they are 1.3's bytes.
        # The floats here are read and written as text, never recomputed, so
        # comparing them byte for byte is legitimate where a fitness value
        # would not be.
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
        # STABILITY IS NOT ENOUGH ON ITS OWN. Every "identical in all three"
        # line here passes when the INPUT changes, because all three passes
        # change together -- measured 2026-09-08 by renaming an individual in
        # the input, which the gate did not notice. So each such line also
        # carries content: a hash, or the first and last entry. The names,
        # the robot block and the population come from the shipped 1.3 data,
        # so pinning their bytes pins them against 1.3.
        echo "names stable         $(v2eq3 yes "$(v2names "$V2D/ham0.exp")" "$(v2names "$V2D/ham1.exp")" "$(v2names "$V2D/ham2.exp")") $(command grep -c "NAME='" "$V2D/ham0.exp" || true) $(v2namee "$V2D/ham2.exp" 1) $(v2namee "$V2D/ham2.exp" '$')"
        # THE SIZE IS PART OF THE ASSERTION, not decoration. Every line here
        # that hashes an extract and compares three hashes says "identical"
        # when the extractor DIES, because three empty strings are equal.
        # Measured: replacing expstruct.py with /bin/false left both reports
        # empty, cmp called them identical, and the gate passed 1/0 with the
        # tool gone. The line count fails on the same input.
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
markers ham0        37 65 190 75242 75329 75491
markers ham1        37 82 209 75363 75450 75612
markers ham2        37 82 209 75463 75550 75712
pvmhost ham0        20 herz pappel platane birke eiche wickie urobe sven hamlet honi kunibert moritz zehn koenig pik bube kreuz bolte laempel esche
pvmhost ham1        20 herz pappel platane birke eiche wickie urobe sven hamlet honi kunibert moritz zehn koenig pik bube kreuz bolte laempel esche
pvmhost ham2        20 herz pappel platane birke eiche wickie urobe sven hamlet honi kunibert moritz zehn koenig pik bube kreuz bolte laempel esche
exp history lines    161 161 161
exp history stable   yes
exp history first    1 2001 8 8 21 13 32 0.0821164 0 0.00939138
exp history last     160 2001 8 9 7 30 45 0.45972 0 0.3833
first block chars    10574 10581 10588
history growth 0->1  100 blocks +1
history growth 1->2  100 blocks +1
names stable         yes 100 NAME='10443' NAME='10194'
hammer robot block   identical in all three 70 lines 666f06787942ddb6
expstruct ham1==ham2 yes 108 lines 1ca79cc55948de76
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
octopus section 5    identical in all three 127 lines fbb7d019a2d69145
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
            # The CHANGED lines, not the first 20 lines of the diff. The
            # report is 58 lines, so a plain head shows context and can stop
            # before reaching the difference -- measured 2026-09-08, when a
            # changed TEXALPHA failed the gate and printed no TEXALPHA line.
            command grep -E '^([-+]|@@)' "$V2D/diff.txt" | head -20 | sed 's/^/    /'
        fi
    fi
    rm -rf "$V2D" || true
fi
printf '%-22s %2d pass  %2d fail\n' "v2 round trip vs 1.3" "$v2p" "$v2f"
pass=$((pass+v2p)); fail=$((fail+v2f))

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
fp=0; ff=0; fw=0; nlinetot=0; nsort=0
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
        fw=$((fw + $(command grep -ac "$SRC.*warning:" /tmp/chk.$$ || true)))
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
        for want in $(command grep -ao ':/[A-Za-z0-9_/.-]*' "$ROOT/build/ui/ui_$base.h" | sort -u); do
            [ -f "$qrc" ] || { ff=$((ff+1)); echo "  form FAIL: $want but no $form.qrc"; continue; }
            rel=${want#:$pfx/}
            if [ "$rel" != "$want" ] && command grep -aq "<file>$rel</file>" "$qrc" \
               && [ -f "$SRC/ui/$(dirname "$form")/$rel" ]
            then fp=$((fp+1))
            else ff=$((ff+1)); echo "  form FAIL: $want not backed by $form.qrc"; fi
        done
        if [ -f "$qrc" ]; then
            for have in $(sed -n 's|.*<file>\(.*\)</file>.*|\1|p' "$qrc"); do
                if command grep -aq ":$pfx/$have" "$ROOT/build/ui/ui_$base.h"; then fp=$((fp+1))
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
            for v in $(command grep -aoE '^        [A-Za-z0-9_]+->setSortingEnabled\(true\)' \
                       "$ROOT/build/ui/ui_$base.h" | sed 's/->.*//;s/ *//' | sort -u); do
                nsort=$((nsort+1))
                if [ -n "$blocked" ] || command grep -aq "$v->sortByColumn(" "$SRC/src/$mod/$base.cpp"; then fp=$((fp+1))
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
        nline=$(command grep -ac '<widget class="Line"' "$SRC/ui/$form.ui" || true)
        nlinetot=$((nlinetot+nline))
        nshape=$(command grep -aA3 '<widget class="Line"' "$SRC/ui/$form.ui" \
                 | command grep -acE '<property name="(orientation|frameShape)"' || true)
        if [ "$nline" -le "$nshape" ]; then fp=$((fp+1))
        else ff=$((ff+1)); echo "  form FAIL: $form.ui has $nline Line widgets but $nshape with a shape"; fi
    done
    # Both checks above are `-le' or a for-loop over a grep, so ZERO matches is
    # indistinguishable from a clean pass -- and a pattern that quietly stops
    # matching (uic changes its indentation, Designer renames the class) would
    # make every form pass with nothing checked. The corpus totals are known and
    # asserted here for that reason: 6 Line widgets across the 20 forms and 5
    # setSortingEnabled(true) in the generated headers. Raise them if a form
    # gains one; never lower them to make this quiet. Added 2026-09-05 after a
    # review pointed out that both checks pass on zero.
    if [ "$nlinetot" -lt 6 ]; then
        ff=$((ff+1))
        echo "  form FAIL: found $nlinetot Line widgets across the forms, expected at least 6 --"
        echo "             the separator check matched nothing and proves nothing"
    else fp=$((fp+1)); fi
    if [ "$nsort" -lt 5 ]; then
        ff=$((ff+1))
        echo "  form FAIL: found $nsort setSortingEnabled(true) sites, expected at least 5 --"
        echo "             the sort-direction check matched nothing and proves nothing"
    else fp=$((fp+1)); fi
fi
printf '%-22s %2d pass  %2d fail  %3d warnings\n' "forms (Phase C)" "$fp" "$ff" "$fw"
pass=$((pass+fp)); fail=$((fail+ff)); warn=$((warn+fw))

rm -f /tmp/chk.$$ /tmp/hdr.$$.cpp /tmp/uic.$$ /tmp/uic2.$$ /tmp/moc.$$.cpp /tmp/mkforms.$$
echo "-----"
echo "total: $pass pass, $fail fail, $warn warnings in SIGEL code"
[ "$winskip" = 0 ] || echo "$winskip Windows-only WIN_* file(s) excluded -- permanent, §7"
[ "$skipped" = 0 ] || echo "$skipped SECTION(S) SKIPPED -- see above; they tested nothing"
# EXIT NON-ZERO WHEN ANYTHING FAILED. There was no exit here at all, so the
# script always returned 0 and `./checks/check.sh && ...' proceeded through a red run.
# A skipped section counts as a failure for the exit status: it is the shape
# this file has been bitten by three times -- a section that quietly tests
# nothing and reports no failures. Found by review.
[ "$fail" = 0 ] && [ "$skipped" = 0 ]
