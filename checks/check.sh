#!/bin/sh
# The main check of the Qt 6 port (PORTING.md §7). It compiles every module
# and header, checks the forms, runs the guidrive scenarios below,
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
# guidrive is BUILT HERE. A failed build leaves the previous binary in place,
# so a section below that uses the build without building it again first
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
# What the GUI DOES, not only what it shows.
#
# This section drives the real SIG_MainWindow with Qt input events: QTest posts
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
