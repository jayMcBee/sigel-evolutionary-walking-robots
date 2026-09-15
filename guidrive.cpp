/*
  Drives the ported SIGEL interface with real Qt input events and prints a
  diffable dump of what the application does. PORTING.md section C10.

  WHAT THIS IS, precisely, because the distinction matters and the record
  should not overstate it: QTest posts QMouseEvent, QKeyEvent and
  QContextMenuEvent through QApplication::notify, so the widgets' own event
  handlers, hit-testing, menu popup logic, item-view selection and the slots
  behind them all run. It is NOT the same as a mouse, and the difference is
  more than "one missing hop": bypassing QWindowSystemInterface changes window
  activation, mouse grabs and double-click synthesis. (Enter/leave was on this
  list and should not have been -- see the xtest scenario, section 6.)
  Menu navigation is the plain example -- a real mouse presses, drags under a
  popup grab and releases, where this posts two independent clicks that happen
  to reach the same actions. So this proves the application's own logic is
  right; it does not prove the platform layer is. On the machine this was
  written for, nothing could drive that layer through the LIVE Wayland session --
  XTEST returns success there and has no effect. A nested plain X server was
  never tried and is not covered by that measurement; Xvfb and xdotool were
  installed 2026-09-07 and are unused so far. See C10.

  Scenarios (argv[1]):
    gate       the committed check.sh pass; deterministic, diffed against
               guibehaviour-baseline.txt
    open       File > Open and the state it produces
    sort       clicking the Fitness header, both directions
    add        Individuals > Add and where new individuals land
    delete     Individuals > Delete, answered No then Yes
    reset      Individuals > Reset
    rename     File > Rename, cancelled then applied
    delexp     File > Delete Experiment, answered No
    quit       File > Quit, answered No
    metagp     the MetaGP disable warning
    ctxmenu    context menus on the tree
    ctxempty   context menu on genuinely empty individuals-list space
    saveload   save, reload, save again, for a round-trip diff
    export     File > Export > Program for one named individual
    visualize  Individuals > Visualize, to capture the PVM payload
    evolution  Start and Stop  (needs PVM and a real sigel_slave)
    hold       open, start PVM, then leave the window to a person; no watchdog
  ... and pages, pagesave, exportall, roundtrip, overwrite, dialogs, metagui,
  clipcheck, formsize, slavegui, metadrive, runlock, rngseed, pvmcrash.
  The list above is not maintained in step with the code. The count that
  cannot go stale is
    command grep -o 'scenario == "[a-z]*"' guidrive.cpp | sed 's/.*"\(.*\)"/\1/' | sort -u | wc -l
  which reads 32 today. A plain -c over the same pattern gives 36, which is a
  count of LINES rather than of matches (`grep -o | wc -l' gives 40): four
  names -- evolution, visualize, pvmcrash and hold -- are each tested in more
  than one condition. The old wording said "two scenarios are tested twice", which stopped
  describing the tree when pvmcrash was added.

  Environment:
    SIGEL_ROOT      as the application needs it; must hold sigel_slave for
                    the visualize and evolution scenarios
    SIGEL_EXP       experiment file to open (default the twoBases reference)
    SIGEL_SCRATCH   where save/export scenarios write (default /tmp)
    SIGEL_GENERATIONS=N evolution only, and the PRIMARY lever: terminate by
                    GENERATION after N, set through the Evolution control tab.
                    Refuses a non-positive or unparseable value rather than
                    silently falling through. Saves the evolved experiment to
                    $SIGEL_SCRATCH/evolved.exp and asserts the run is in it.
    SIGEL_RUN_LONGER=1  evolution only: change the termination through the GUI
                    first, because every shipped experiment terminates on a
                    date in 2001 and would otherwise finish instantly

  Build: see check.sh, or the guidrive target in the Makefile. It MUST be
  linked with the master SIG_GPExperiment, exactly as sigel is -- linking the
  Clean variant leaves mtController uninitialised and the first tree selection
  dereferences garbage. That cost one false defect before it was understood.
*/
// Drives the REAL SIG_MainWindow with real Qt input events (QTest posts
// QMouseEvent/QKeyEvent through QApplication::notify, so the widgets' own
// event handlers, hit-testing, menu popup and item-view selection logic all
// run). It is NOT an X-level click: see PORTING.md for what that distinction
// costs. Every state dump below is meant to be diffed, against a baseline here
// and against what the 1.3 oracle reads off the running binary.
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QStackedWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QSpinBox>
#include <QLabel>
#include <QLCDNumber>
#include <QGroupBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QWidgetAction>
#include <QSlider>
#include <QProgressBar>
#include <QComboBox>
#include <QHeaderView>
#include <QTabWidget>
#include <QStyleOptionSlider>
#include <QStyle>
#include <QStyle>
#include <QCompleter>
#include <QStyleOptionSlider>
#include <QTabBar>
#include <QListWidget>
#include <QTextBrowser>
#include <QValidator>
#include <QAbstractSpinBox>
#include <QContextMenuEvent>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QProcess>
#include <QCursor>
#include <QAbstractNativeEventFilter>
extern "C" {
#include "pvm3.h"
}
#include <QTimer>
#include <QElapsedTimer>
#include <QtTest/QtTest>
#include <climits>
#include <clocale>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <algorithm>
#include <functional>
#include <map>

#include "SIGEL_MasterGUI/SIG_MainWindow.h"
#include "SIGEL_SlaveGUI/SIG_SimulationWindow.h"
#include "SIGEL_MasterGUI/SIG_ExperimentListView.h"
#include "SIGEL_MasterGUI/SIG_GUIGPExperiment.h"
#include "MT_GUI/MT_Editor.h"
#include "SIGEL_MasterGUI/SIG_AllIndividualsView.h"
#include "SIGEL_MasterGUI/SIG_IndividualListItem.h"
#include "SIGEL_MasterGUI/SIG_IndividualView.h"
#include "SIGEL_Tools/SIG_IO.h"

// The twenty committed form base classes, for the `formsize' scenario. Nothing
// else here instantiates a form on its own -- every other scenario reaches them
// through the windows that own them.
#include "MT_GUI/MT_AddConstantsWidgetBase.h"
#include "MT_GUI/MT_AddIndividualsWidget.h"
#include "MT_GUI/MT_EstimationWidgetBase.h"
#include "MT_GUI/MT_ExperimentWidgetBase.h"
#include "MT_GUI/MT_IndividualWidgetBase.h"
#include "MT_GUI/MT_PopulationWidgetBase.h"
#include "MT_GUI/MT_SearchWidgetBase.h"
#include "MT_GUI/MT_SelectionWidgetBase.h"
#include "MT_GUI/MT_StatisticsWidgetBase.h"
#include "SIGEL_MasterGUI/SIG_EditHostDialogBase.h"
#include "SIGEL_MasterGUI/SIG_EnvironmentBase.h"
#include "SIGEL_MasterGUI/SIG_ExperimentViewBase.h"
#include "SIGEL_MasterGUI/SIG_GPParameterBase.h"
#include "SIGEL_MasterGUI/SIG_IndividualListBase.h"
#include "SIGEL_MasterGUI/SIG_IndividualViewBase.h"
#include "SIGEL_MasterGUI/SIG_LanguageParametersBase.h"
#include "SIGEL_MasterGUI/SIG_RobotBase.h"
#include "SIGEL_MasterGUI/SIG_SimulationParameterBase.h"
#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialogBase.h"
#include "SIGEL_SlaveGUI/SIG_SimulationWidgetBase.h"

// sigel.cpp:95 owns this global; anything that pulls MT_Controller needs it.
bool guiEnabled = true;

// Where the save/export scenarios put their files. $SIGEL_SCRATCH or /tmp.
static QString scratch()
{
    return qEnvironmentVariable("SIGEL_SCRATCH", QStringLiteral("/tmp"));
}

using namespace SIGEL_MasterGUI;

static SIG_MainWindow *W = nullptr;
static std::map<QString, bool> prevEnabled;
static int stepNo = 0;

// ---------------------------------------------------------------- utilities
static QString actKey(QAction *a, const QString &path)
{
    return path + "/" + (a->objectName().isEmpty() ? a->text() : a->objectName());
}

static void walkMenu(QMenu *m, const QString &path, std::map<QString, bool> &out)
{
    for (QAction *a : m->actions()) {
        if (a->isSeparator()) continue;
        out[actKey(a, path)] = a->isEnabled();
        if (a->menu()) walkMenu(a->menu(), path + ">" + a->text(), out);
    }
}

static std::map<QString, bool> collectEnabled()
{
    std::map<QString, bool> m;
    for (QAction *top : W->menuBar()->actions())
        if (top->menu()) walkMenu(top->menu(), top->text(), m);
    for (QToolBar *tb : W->findChildren<QToolBar *>())
        for (QAction *a : tb->actions())
            if (!a->isSeparator()) m[actKey(a, "TB:" + tb->objectName())] = a->isEnabled();
    return m;
}

static void dumpEnabledDelta()
{
    std::map<QString, bool> now = collectEnabled();
    bool any = false;
    for (auto &kv : now) {
        auto it = prevEnabled.find(kv.first);
        if (it == prevEnabled.end() || it->second != kv.second) {
            if (!any) { printf("  [action changes]\n"); any = true; }
            printf("    %-58s %s\n", qPrintable(kv.first),
                   kv.second ? "ENABLED" : "greyed");
        }
    }
    for (auto &kv : prevEnabled)
        if (now.find(kv.first) == now.end()) {
            if (!any) { printf("  [action changes]\n"); any = true; }
            printf("    %-58s GONE\n", qPrintable(kv.first));
        }
    if (!any) printf("  [action changes] none\n");
    prevEnabled = now;
}

// EVERY action's ICON, which nothing in this project looked at until a review
// replaced all 30 menu and toolbar .xpm files with the text "NOT AN XPM AT ALL"
// and watched the whole gate pass. `gui vs 1.3' prints iconText (a string) and
// the toolbar's iconSize (a property); the forms section covers only .qrc-backed
// Designer resources. A pixmap that fails to load compiles, runs, and renders
// as nothing.
//
// isNull() on the QIcon is NOT enough: QIcon::addPixmap of a null pixmap leaves
// a non-null QIcon that draws nothing. availableSizes() is what empties out, so
// that is what is counted.
static void dumpIcons()
{
    struct Row { QString key; int sizes; bool nullIcon; };
    QList<Row> rows;
    std::function<void(QMenu *, const QString &)> walk =
        [&](QMenu *m, const QString &path) {
        for (QAction *a : m->actions()) {
            if (a->isSeparator()) continue;
            rows.append({ path + "/" + a->text(),
                          (int)a->icon().availableSizes().count(),
                          a->icon().isNull() });
            if (a->menu()) walk(a->menu(), path + ">" + a->text());
        }
    };
    for (QAction *top : W->menuBar()->actions())
        if (top->menu()) walk(top->menu(), top->text());
    for (QToolBar *tb : W->findChildren<QToolBar *>())
        for (QAction *a : tb->actions())
            if (!a->isSeparator())
                rows.append({ "TB:" + tb->objectName() + "/" + a->text(),
                              (int)a->icon().availableSizes().count(),
                              a->icon().isNull() });
    std::sort(rows.begin(), rows.end(),
              [](const Row &x, const Row &y) { return x.key < y.key; });
    int withIcon = 0, empty = 0;
    for (const Row &r : rows) { if (!r.nullIcon) ++withIcon; if (!r.nullIcon && !r.sizes) ++empty; }
    printf("\n== ICONS ==\nactions=%d withIcon=%d loadedNothing=%d\n",
           (int)rows.count(), withIcon, empty);
    for (const Row &r : rows)
        printf("  %-56s icon=%d sizes=%d\n", qPrintable(r.key),
               r.nullIcon ? 0 : 1, r.sizes);
    fflush(stdout);
}

static SIG_ExperimentListView *listView()
{
    return W->findChild<SIG_ExperimentListView *>();
}

static void dumpTreeItem(QTreeWidgetItem *it, int depth)
{
    printf("    %*s%-24s sel=%d exp=%d cur=%d selectable=%d\n", depth * 2, "",
           qPrintable(it->text(0)), it->isSelected() ? 1 : 0,
           it->isExpanded() ? 1 : 0,
           it->treeWidget()->currentItem() == it ? 1 : 0,
           (it->flags() & Qt::ItemIsSelectable) ? 1 : 0);
    for (int i = 0; i < it->childCount(); ++i) dumpTreeItem(it->child(i), depth + 1);
}

static void dumpTree()
{
    SIG_ExperimentListView *lv = listView();
    printf("  [tree] header=[%s] topLevel=%d\n",
           qPrintable(lv->headerItem() ? lv->headerItem()->text(0) : QString("?")),
           lv->topLevelItemCount());
    for (int i = 0; i < lv->topLevelItemCount(); ++i) dumpTreeItem(lv->topLevelItem(i), 1);
}

static void dumpPage()
{
    QStackedWidget *st = W->findChild<QStackedWidget *>();
    QWidget *cur = st ? st->currentWidget() : nullptr;
    printf("  [page] count=%d current=%s objectName=[%s]\n",
           st ? st->count() : -1,
           cur ? cur->metaObject()->className() : "(none)",
           cur ? qPrintable(cur->objectName()) : "");
}

static void dumpIndividuals()
{
    SIG_AllIndividualsView *v = W->findChild<SIG_AllIndividualsView *>();
    if (!v) { printf("  [individuals] no view\n"); return; }
    QTreeWidget *t = v->findChild<QTreeWidget *>();
    if (!t) { printf("  [individuals] no list widget\n"); return; }
    printf("  [individuals] rows=%d cols=%d headers=", t->topLevelItemCount(), t->columnCount());
    for (int c = 0; c < t->columnCount(); ++c)
        printf("[%s]", qPrintable(t->headerItem()->text(c)));
    printf(" selected=%d\n", (int)t->selectedItems().count());
    for (int i = 0; i < t->topLevelItemCount() && i < 6; ++i) {
        QTreeWidgetItem *it = t->topLevelItem(i);
        printf("    row%-2d ", i);
        for (int c = 0; c < t->columnCount(); ++c) printf("| %-14s", qPrintable(it->text(c)));
        printf("| sel=%d\n", it->isSelected() ? 1 : 0);
    }
    if (t->topLevelItemCount() > 6) printf("    ... %d more\n", t->topLevelItemCount() - 6);
    // The six rows above leave 95% of the list outside the diff: a mis-sort or
    // a corrupted fitness anywhere in rows 6..n-1 would change nothing here.
    // One checksum over every cell closes that, and it is order-sensitive, so
    // it catches a re-ordering as well as a changed value. Found by review.
    {
        QByteArray all;
        for (int i = 0; i < t->topLevelItemCount(); ++i)
            for (int c = 0; c < t->columnCount(); ++c)
                all += t->topLevelItem(i)->text(c).toUtf8() + '\x1f';
        printf("    allrows sha256=%s\n",
               QCryptographicHash::hash(all, QCryptographicHash::Sha256)
                   .toHex().constData());
    }
}

// Everything on the current page that carries observable state. Generic on
// purpose: the same dump has to serve the experiment page, the GP page and the
// individuals page, and the oracle reads the same facts off 1.3 by eye.
// Any widget whose rect leaves its parent is a control the user cannot fully
// see or hit. This is the mechanical form of the defect that hid seven
// navigation buttons behind a collapsed group box, and then a second one that
// hid seven controls in the movie dialog.
static int clippedWidgets(QWidget *root, const char *where)
{
    int checked = 0, bad = 0;
    for (QWidget *w : root->findChildren<QWidget *>()) {
        QWidget *par = w->parentWidget();
        if (!par || !w->isVisible() || w->size().isEmpty()) continue;
        ++checked;
        const QRect r = w->geometry(), p = par->rect();
        if (r.left() < p.left() || r.top() < p.top()
            || r.right() > p.right() || r.bottom() > p.bottom()) {
            ++bad;
            printf("    !! CLIPPED %s [%s] %dx%d at +%d+%d inside [%s] %dx%d\n", where,
                   qPrintable(w->objectName().isEmpty()
                       ? QString(w->metaObject()->className()) : w->objectName()),
                   r.width(), r.height(), r.x(), r.y(),
                   qPrintable(par->objectName().isEmpty()
                       ? QString(par->metaObject()->className()) : par->objectName()),
                   p.width(), p.height());
        }
    }
    printf("  %-26s %3d visible widgets, %d clipped\n", where, checked, bad);
    fflush(stdout);
    return bad;
}

static void dumpWidgets()
{
    QStackedWidget *st = W->findChild<QStackedWidget *>();
    QWidget *pg = st ? st->currentWidget() : nullptr;
    if (!pg) { printf("  [widgets] no page\n"); return; }
    printf("  [widgets]\n");
    for (QGroupBox *g : pg->findChildren<QGroupBox *>())
        printf("    group    [%s] enabled=%d\n", qPrintable(g->title()), g->isEnabled());
    for (QAbstractButton *b : pg->findChildren<QAbstractButton *>()) {
        const char *kind = qobject_cast<QCheckBox *>(b) ? "check"
                         : qobject_cast<QRadioButton *>(b) ? "radio" : "button";
        printf("    %-8s [%s] enabled=%d checkable=%d checked=%d\n", kind,
               qPrintable(b->text()), b->isEnabled(), b->isCheckable(), b->isChecked());
    }
    for (QLCDNumber *l : pg->findChildren<QLCDNumber *>())
        printf("    lcd      [%s] value=%g digits=%d\n",
               qPrintable(l->objectName()), l->value(), l->digitCount());
    for (QSlider *sl : pg->findChildren<QSlider *>())
        printf("    slider   [%s] value=%d min=%d max=%d enabled=%d\n",
               qPrintable(sl->objectName()), sl->value(), sl->minimum(), sl->maximum(),
               sl->isEnabled());
    for (QSpinBox *sp : pg->findChildren<QSpinBox *>())
        printf("    spin     [%s] value=%d min=%d max=%d enabled=%d\n",
               qPrintable(sp->objectName()), sp->value(), sp->minimum(), sp->maximum(),
               sp->isEnabled());
    for (QProgressBar *pb : pg->findChildren<QProgressBar *>())
        printf("    progress [%s] value=%d min=%d max=%d\n",
               qPrintable(pb->objectName()), pb->value(), pb->minimum(), pb->maximum());
    for (QLineEdit *le : pg->findChildren<QLineEdit *>())
        printf("    edit     [%s] text=[%s] enabled=%d\n",
               qPrintable(le->objectName()), qPrintable(le->text()), le->isEnabled());
    for (QComboBox *cb : pg->findChildren<QComboBox *>()) {
        printf("    combo    [%s] current=[%s] count=%d\n", qPrintable(cb->objectName()),
               qPrintable(cb->currentText()), cb->count());
    }
}

static void step(const char *what, bool tree = true, bool page = true,
                 bool inds = false, bool wid = false)
{
    printf("\n== STEP %d: %s ==\n", ++stepNo, what);
    printf("  [title] %s\n", qPrintable(W->windowTitle()));
    if (tree) dumpTree();
    if (page) dumpPage();
    if (inds) dumpIndividuals();
    if (wid)  dumpWidgets();
    dumpEnabledDelta();
    fflush(stdout);
}

// ------------------------------------------------------- modal interception
// PVM teardown. pvm_start_pvmd() returns 0 when THIS process started the
// daemon and PvmDupHost when one was already up; only the former may halt it,
// or a scenario would tear down someone else's running evolution. Without this
// the visualize and evolution scenarios left a pvmd3 and its slaves behind.
static bool g_pvmOurDaemon = false;
static bool g_pvmEnrolled  = false;

static void tearDownPvm()
{
    if (g_pvmOurDaemon) { g_pvmOurDaemon = false; pvm_halt(); }
    if (g_pvmEnrolled)  { g_pvmEnrolled  = false; pvm_exit(); }
}

// PVM KILLS US DURING ITS OWN SHUTDOWN, so a scenario's exit status never
// survives tearDownPvm() unaided. pvm_halt() sends TM_HALT and waits for a
// reply the daemon never sends (tdpro.c:1507-1516); the daemon's pvmbailout()
// SIGTERMs every local task on the way out (pvmd.c:1485-1517); this process is
// one, having enrolled with pvm_mytid(). Measured: 143, not the scenario's
// return value. That silently erased `return 1' from EVERY assertion in the
// scenarios that start PVM -- evolution, visualize, pvmcrash, hold -- so a
// failed assertion and a clean pass left the same status behind. Found by
// review. The handler re-exits with the code we already decided on.
static int g_exitCode = 0;
extern "C" void guidriveExitOnTerm(int) { _exit(g_exitCode); }
static void keepExitCodeThroughPvmShutdown(int rc)
{
    g_exitCode = rc;
    std::signal(SIGTERM, guidriveExitOnTerm);
}

// A scenario that blocks in a modal exec() that never closes would otherwise
// hang forever; check.sh's timeout would kill it and report the wrong reason.
// A modal exec() still runs an event loop, so this timer fires inside exactly
// the case it exists for.

// SIGEL's own diagnostics do NOT reach the terminal on their own.
// SIG_IO::cerr and ::cout are QTextStreams over stderr/stdout (SIG_IO.cpp:27-29)
// and the 2003 code ends its messages with "\n", never endl -- so the text sits
// in the QTextStream's buffer until the stream is destroyed at normal exit.
// MEASURED: a QTextStream on stderr written this way survives a clean return and
// is lost entirely on a kill, while a plain fprintf on the same descriptor
// survives both. Everything SIGEL prints about a failure -- `pvm_spawn() failed
// on "..."' and the SIGSEGV handler's `Invalid storage access' -- goes through
// those two streams.
// So any exit that skips destructors discards exactly the diagnostics a stuck or
// crashing scenario exists to capture.
static void flushSigelStreams()
{
    SIGEL_Tools::SIG_IO::cerr.flush();
    SIGEL_Tools::SIG_IO::cout.flush();
    fflush(stdout);
    fflush(stderr);
}

static void armWatchdog(int ms)
{
    QTimer *wd = new QTimer;
    wd->setSingleShot(true);
    QObject::connect(wd, &QTimer::timeout, [ms]() {
        QWidget *m = QApplication::activeModalWidget();
        printf("\n!! WATCHDOG: this scenario exceeded %d ms and is stuck.\n"
               "!! active modal: %s\n"
               "!! Aborting rather than hanging whatever is running this.\n",
               ms, m ? m->metaObject()->className() : "(none)");
        // _exit() runs no destructors, so without this the QTextStream buffers
        // go with the process -- and a watchdog fires precisely when SIGEL has
        // something to say about why it is stuck.
        flushSigelStreams();
        keepExitCodeThroughPvmShutdown(3);
        tearDownPvm();
        flushSigelStreams();
        _exit(3);
    });
    wd->start(ms);
}

// Only ONE of these may be armed at a time. An earlier version left each
// poller running until its own budget expired, so a handler armed for step N
// could still be alive at step N+1 and latch onto that step's dialog -- the
// MetaGP scenario demonstrably printed its "NONE appeared" line against the
// wrong step, and roughly a second of drift would have had it consume the
// disable warning before the intended handler saw it. That warning is what the
// ampersand assertion reads, so this is not a cosmetic problem.
static QTimer *g_modalPoller = nullptr;

static void cancelModalHandler()
{
    if (g_modalPoller) {
        g_modalPoller->stop();
        g_modalPoller->deleteLater();
        g_modalPoller = nullptr;
    }
}

static void whenModal(std::function<void(QWidget *)> fn, int budgetMs = 8000)
{
    if (g_modalPoller) {                       // cancel the previous one
        g_modalPoller->stop();
        g_modalPoller->deleteLater();
        g_modalPoller = nullptr;
    }
    QTimer *t = new QTimer;
    g_modalPoller = t;
    QElapsedTimer *clock = new QElapsedTimer;  // real time, not tick counting
    clock->start();
    QObject::connect(t, &QTimer::timeout, [t, fn, clock, budgetMs]() {
        QWidget *m = QApplication::activeModalWidget();
        if (m) {
            t->stop(); t->deleteLater();
            if (g_modalPoller == t) g_modalPoller = nullptr;
            printf("  [modal] class=%s title=[%s]\n",
                   m->metaObject()->className(), qPrintable(m->windowTitle()));
            fflush(stdout);
            delete clock;
            fn(m);
        } else if (clock->elapsed() > budgetMs) {
            t->stop(); t->deleteLater();
            if (g_modalPoller == t) g_modalPoller = nullptr;
            printf("  [modal] NONE appeared within %d ms\n", budgetMs);
            fflush(stdout);
            delete clock;
        }
    });
    t->start(50);
}

static void describeMessageBox(QWidget *m)
{
    QMessageBox *mb = qobject_cast<QMessageBox *>(m);
    if (!mb) return;
    printf("  [msgbox] text=[%s] icon=%d\n", qPrintable(mb->text()), (int)mb->icon());
    for (QAbstractButton *b : mb->buttons())
        printf("    button [%-18s] role=%d default=%d\n", qPrintable(b->text()),
               (int)mb->buttonRole(b), b == mb->defaultButton() ? 1 : 0);
    fflush(stdout);
}


// Generic dialog description: everything a person reading 1.3's dialog can see.
static void describeDialog(QWidget *m)
{
    printf("  [dialog] class=%s title=[%s] modal=%d\n", m->metaObject()->className(),
           qPrintable(m->windowTitle()), m->isModal());
    for (QLabel *l : m->findChildren<QLabel *>())
        if (!l->text().isEmpty()) printf("    label  [%s]\n", qPrintable(l->text()));
    for (QSpinBox *sp : m->findChildren<QSpinBox *>())
        printf("    spin   [%s] value=%d min=%d max=%d\n", qPrintable(sp->objectName()),
               sp->value(), sp->minimum(), sp->maximum());
    for (QLineEdit *le : m->findChildren<QLineEdit *>())
        printf("    edit   [%s] text=[%s] selected=[%s] focus=%d\n",
               qPrintable(le->objectName()), qPrintable(le->text()),
               qPrintable(le->selectedText()), le->hasFocus());
    for (QPushButton *b : m->findChildren<QPushButton *>())
        printf("    button [%-12s] default=%d enabled=%d focus=%d\n", qPrintable(b->text()),
               b->isDefault(), b->isEnabled(), b->hasFocus());
    fflush(stdout);
}

static QPushButton *dlgButton(QWidget *m, const QString &text)
{
    for (QPushButton *b : m->findChildren<QPushButton *>())
        if (b->text() == text || b->text() == "&" + text) return b;
    return nullptr;
}

static void clickDlgButton(QWidget *m, const QString &text)
{
    QPushButton *b = dlgButton(m, text);
    if (!b) { printf("  !! no button [%s]\n", qPrintable(text)); m->close(); return; }
    QTest::mouseClick(b, Qt::LeftButton, Qt::NoModifier, b->rect().center());
}

static void clickMsgButton(QWidget *m, QMessageBox::StandardButton which)
{
    QMessageBox *mb = qobject_cast<QMessageBox *>(m);
    if (!mb) { m->close(); return; }
    QAbstractButton *b = mb->button(which);
    if (!b) { printf("  !! msgbox has no requested button\n"); mb->reject(); return; }
    printf("  [answering] %s\n", qPrintable(b->text())); fflush(stdout);
    QTest::mouseClick(b, Qt::LeftButton, Qt::NoModifier, b->rect().center());
}

static QTreeWidget *indList()
{
    SIG_AllIndividualsView *v = W->findChild<SIG_AllIndividualsView *>();
    return v ? v->findChild<QTreeWidget *>() : nullptr;
}

// A real click on a row in the individuals list, at its on-screen rectangle.
static void clickRow(QTreeWidget *t, int row, Qt::KeyboardModifiers mods = Qt::NoModifier)
{
    QTreeWidgetItem *it = t->topLevelItem(row);
    if (!it) { printf("  !! no row %d\n", row); return; }
    t->scrollToItem(it);
    QRect r = t->visualItemRect(it);
    QTest::mouseClick(t->viewport(), Qt::LeftButton, mods, r.center());
    QTest::qWait(80);
}

// Context menus hang off customContextMenuRequested, which Qt raises from a
// QContextMenuEvent -- NOT from a raw right-button QMouseEvent. QTest::mouseClick
// posts only the latter, so a right "click" through it reaches nothing and every
// menu reads as absent. Sending the QContextMenuEvent is what a real right-click
// produces once the platform plugin has done its part; QApplication::notify
// propagates it up from the viewport to the view that owns the policy.
// The app shows these with popup(), which does not block, so the menu is simply
// the active popup afterwards.
static void dumpContextMenu(QWidget *target, const QPoint &pos, const char *what)
{
    QStackedWidget *st = W->findChild<QStackedWidget *>();
    QWidget *before = st ? st->currentWidget() : nullptr;

    QContextMenuEvent ev(QContextMenuEvent::Mouse, pos, target->mapToGlobal(pos));
    QApplication::sendEvent(target, &ev);
    QTest::qWait(250);

    QMenu *m = qobject_cast<QMenu *>(QApplication::activePopupWidget());
    if (!m) { printf("  [ctxmenu %s] NONE appeared\n", what); fflush(stdout); return; }
    printf("  [ctxmenu %s] entries=%d\n", what, (int)m->actions().count());
    for (QAction *a : m->actions()) {
        if (a->isSeparator()) { printf("    ---\n"); continue; }
        printf("    %-28s enabled=%d\n", qPrintable(a->text()), a->isEnabled());
    }
    QWidget *after = st ? st->currentWidget() : nullptr;
    if (before != after)
        printf("    (page switched to %s)\n", after ? after->metaObject()->className() : "none");
    fflush(stdout);
    m->close();
    QTest::qWait(120);
}

// ------------------------------------------------------------ menu clicking
static QAction *topAction(const QString &text)
{
    for (QAction *a : W->menuBar()->actions()) if (a->text() == text) return a;
    return nullptr;
}

static bool clickMenu(const QString &top, const QString &item, const QString &sub = QString())
{
    QMenuBar *mb = W->menuBar();
    QAction *ta = topAction(top);
    if (!ta || !ta->menu()) { printf("  !! no top menu [%s]\n", qPrintable(top));
                              cancelModalHandler(); return false; }
    QTest::mouseClick(mb, Qt::LeftButton, Qt::NoModifier, mb->actionGeometry(ta).center());
    QTest::qWait(120);
    QMenu *m = ta->menu();
    if (!m->isVisible()) { printf("  !! menu [%s] did not open\n", qPrintable(top));
                           cancelModalHandler(); return false; }
    for (QAction *a : m->actions()) {
        if (a->text() != item) continue;
        if (!sub.isEmpty() && a->menu()) {
            if (!a->isEnabled()) {
                printf("  !! submenu parent [%s>%s] is GREYED\n",
                       qPrintable(top), qPrintable(item));
                m->close(); cancelModalHandler(); return false;
            }
            QTest::mouseClick(m, Qt::LeftButton, Qt::NoModifier, m->actionGeometry(a).center());
            QTest::qWait(150);
            QMenu *s = a->menu();
            for (QAction *sa : s->actions())
                if (sa->text() == sub) {
                    if (!sa->isEnabled()) {
                        printf("  !! item [%s>%s>%s] is GREYED\n", qPrintable(top),
                               qPrintable(item), qPrintable(sub));
                        s->close(); m->close(); cancelModalHandler(); return false;
                    }
                    QTest::mouseClick(s, Qt::LeftButton, Qt::NoModifier, s->actionGeometry(sa).center());
                    QTest::qWait(150); return true;
                }
            s->close(); m->close(); cancelModalHandler(); return false;
        }
        if (!a->isEnabled()) {
            printf("  !! item [%s>%s] is GREYED -- click would do nothing\n",
                   qPrintable(top), qPrintable(item));
            m->close(); cancelModalHandler(); return false;
        }
        QTest::mouseClick(m, Qt::LeftButton, Qt::NoModifier, m->actionGeometry(a).center());
        QTest::qWait(200);
        return true;
    }
    printf("  !! no item [%s] in menu [%s]\n", qPrintable(item), qPrintable(top));
    m->close();
    cancelModalHandler();
    return false;
}

// ------------------------------------------------- the five parameter pages
// C11. Only ONE of the six View pages -- Population -- was ever driven; the
// other five hold every spin box, slider, combo, checkbox and validator in the
// application, and C7 converted 21 validators that nothing has ever typed into.
//
// dumpWidgets() above is deliberately NOT extended. Its output sits in the
// committed gate baseline, every line of which was diffed against the running
// 1.3 binary in C10, so adding a field there would rewrite evidence rather
// than add to it. Everything below is additional output in new scenarios.

// A validator's CONFIGURATION. The oracle cannot read this off 1.3 at all --
// Qt 2 has no accessibility API, so the 1.3 side reports only BEHAVIOUR, what
// a typed character does. This half is therefore the port's own record: it is
// what says whether C7's findChildren<QValidator*>() loop actually reached
// every validator, or only the ones somebody happened to look at.
static QString validatorDesc(const QValidator *v)
{
    if (!v) return QStringLiteral("none");
    if (const QDoubleValidator *d = qobject_cast<const QDoubleValidator *>(v))
        return QString("QDoubleValidator bottom=%1 top=%2 decimals=%3 notation=%4 "
                       "locale=[%5] numberOptions=%6")
            .arg(d->bottom()).arg(d->top()).arg(d->decimals())
            .arg((int)d->notation()).arg(d->locale().name())
            .arg((int)d->locale().numberOptions().toInt());
    if (const QIntValidator *i = qobject_cast<const QIntValidator *>(v))
        return QString("QIntValidator bottom=%1 top=%2 locale=[%3] numberOptions=%4")
            .arg(i->bottom()).arg(i->top()).arg(i->locale().name())
            .arg((int)i->locale().numberOptions().toInt());
    return QString::fromLatin1(v->metaObject()->className());
}

// Type a string into a line edit ONE KEY AT A TIME and report what survived.
// A validator that answers Invalid makes QLineEdit DROP that keystroke, so the
// surviving text carries a gap exactly where the rejection happened: typing
// "9,81" into a C-locale double field leaves "981", not "9,81" and not "9".
// That gap is the whole measurement, and it is the one thing the 1.3 oracle
// CAN read off the running binary, because it is displayed text rather than
// validator state. The field is restored afterwards, through setText(), which
// is programmatic and does not run the validator -- measured, not assumed:
// setText("9,81") on one of these fields stores "9,81" and reports
// hasAcceptableInput()==false. So probe order cannot carry state from one
// probe into the next.
// Empty a line edit and PROVE it is empty. select-all + Delete is not enough
// on every field -- the oracle found the same thing on 1.3, where its standard
// clear left a "0" behind and silently contaminated a whole battery; the tell
// was a restore reading "00" instead of "0". So: try the fast way, then fall
// back to End plus BackSpaces, then report if it still is not empty.
static bool clearEdit(QLineEdit *le)
{
    le->setFocus();
    le->selectAll();
    QTest::keyClick(le, Qt::Key_Delete);
    if (le->text().isEmpty()) return true;
    QTest::keyClick(le, Qt::Key_End);
    for (int i = le->text().size() + 2; i > 0; --i)
        QTest::keyClick(le, Qt::Key_Backspace);
    if (le->text().isEmpty()) return true;
    printf("    !! could not clear %s -- it still holds [%s]\n",
           qPrintable(le->objectName()), qPrintable(le->text()));
    return false;
}

static void probeEdit(QLineEdit *le, const char *probe)
{
    const QString before = le->text();
    le->setFocus();
    le->selectAll();
    QTest::keyClick(le, Qt::Key_Delete);
    QTest::keyClicks(le, QString::fromUtf8(probe));
    QTest::qWait(10);
    printf("      type [%-6s] -> text=[%s] acceptable=%d\n", probe,
           qPrintable(le->text()), le->hasAcceptableInput() ? 1 : 0);
    le->setText(before);
}

// The full battery, for the two fields the oracle types the same strings into.
static void batteryDouble(QLineEdit *le)
{
    printf("    battery %s  (was [%s])\n", qPrintable(le->objectName()),
           qPrintable(le->text()));
    for (const char *p : {"9.81", "9,81", "0,375", "-2.5", "1e3", "abc", "1.2.3"})
        probeEdit(le, p);
}

static void batteryInt(QLineEdit *le)
{
    printf("    battery %s  (was [%s])\n", qPrintable(le->objectName()),
           qPrintable(le->text()));
    for (const char *p : {"42", "-7", "4.2", "4,2", "abc"})
        probeEdit(le, p);
}

// One line per spin box: resting state, one Up, two Downs, a typed value one
// past the maximum, and THE VALUE THAT TYPING COMMITS TO.
//
// The commit is the load-bearing column and it took an oracle reading to know
// it. Qt 2's QIntValidator returned INTERMEDIATE for an out-of-range number
// (qvalidator.cpp:236, `else if ( tmp < b || tmp > t ) return Intermediate'),
// so QLineEdit accepted every digit; QSpinBox::interpretText then mapped the
// whole text and called setValue(), and QRangeControl::directSetValue CLAMPED
// it into the range. Type 100 into a [1..99] box in 1.3 and you get 99.
// Qt 6's QIntValidator returns INVALID once the prefix passes the top, so the
// keystroke is refused and the box keeps the truncated prefix -- type 100 and
// you get 10. Same range, same typing, a different number committed.
// `commit' presses Return to make the spin box interpret what was typed. That
// is safe on a page and NOT safe in a dialog: QAbstractSpinBox ignores Return,
// QApplication::notify walks it up the parent chain, and QDialog clicks its
// DEFAULT button -- which is OK on every dialog here. The `dialogs' scenario's
// "Add individuals (Cancel)" step was accepting the dialog inside this probe
// and only reaching Cancel afterwards, on a hidden widget; it printed the right
// answer for the wrong reason and would have printed it just the same with a
// dead Cancel connect. Found by review, and reproduced: the dialog reported
// visible=0 on return from here.
static void probeSpin(QSpinBox *sp, bool commit = true)
{
    const int start = sp->value();
    sp->setFocus();
    QTest::keyClick(sp, Qt::Key_Up);
    const int up = sp->value();
    QTest::keyClick(sp, Qt::Key_Down);
    QTest::keyClick(sp, Qt::Key_Down);
    const int down = sp->value();
    sp->setValue(start);

    sp->selectAll();
    QTest::keyClick(sp, Qt::Key_Delete);
    QTest::keyClicks(sp, QString::number((long long)sp->maximum() + 1));
    QTest::qWait(10);
    // QAbstractSpinBox::lineEdit() is protected; the editor is the spin
    // box's only QLineEdit child, so ask for it that way.
    QLineEdit *ed = sp->findChild<QLineEdit *>();
    const QString typed = ed ? ed->text() : QString();
    const int typedVal = sp->value();
    // Return is what commits. 1.3 clamps here; the port has nothing left to
    // clamp, having refused the digits that would have gone out of range.
    int committed = -1;
    if (commit) {
        QTest::keyClick(sp, Qt::Key_Return);
        QTest::qWait(20);
        committed = sp->value();
    }
    sp->setValue(start);

    printf("    spin   %-36s v=%-9d [%d..%d] step=%d pre=[%s] suf=[%s] "
           "up=%-9d dn2=%-9d typed(max+1)=[%s]->%d commits=%s enabled=%d\n",
           qPrintable(sp->objectName()), start, sp->minimum(), sp->maximum(),
           sp->singleStep(), qPrintable(sp->prefix()), qPrintable(sp->suffix()),
           up, down, qPrintable(typed), typedVal,
           commit ? qPrintable(QString::number(committed))
                  : "not-pressed(in a dialog)",
           sp->isEnabled() ? 1 : 0);
}

// The form pairs sliderXXX with lcdnumberXXX by name, through connections uic
// emitted. Reading the LCD back after a keyboard move is what proves the pair
// is still connected -- a dead connect() compiles, links and shows nothing.
static QLCDNumber *pairedLcd(QWidget *page, QSlider *sl)
{
    QString n = sl->objectName();
    if (!n.startsWith("slider")) return nullptr;
    return page->findChild<QLCDNumber *>("lcdnumber" + n.mid(6));
}

// A GROOVE CLICK, which is the operation the 1.3 oracle can actually perform.
// It reported that in 1.3 clicking the groove moves the handle by exactly +1
// and does NOT give the slider keyboard focus, so 200 arrow presses on its
// side went to the tree and moved the page instead. That is worth having in
// the record twice over: the keyboard probe below drives the slider's key
// handler DIRECTLY (QTest posts to the widget, focus or no focus), so it does
// not measure focus at all -- and a comparison built only on it would be
// pitting a keyboard-driven Qt 6 slider against something 1.3 will not do.
// The click path is the one both sides share.
static void probeSliderClick(QSlider *sl)
{
    const int start = sl->value();
    // Just inside the groove, past the handle, so the click is a page step
    // rather than a grab. Vertical centre; these are all horizontal.
    const QPoint p(sl->width() - 6, sl->height() / 2);
    QTest::mouseClick(sl, Qt::LeftButton, Qt::NoModifier, p);
    QTest::qWait(40);
    const int after = sl->value();
    printf("    click  %-36s %d -> %d (delta %+d) focusPolicy=%d hasFocus=%d\n",
           qPrintable(sl->objectName()), start, after, after - start,
           (int)sl->focusPolicy(), sl->hasFocus() ? 1 : 0);
    sl->setValue(start);
}

static void probeSlider(QWidget *page, QSlider *sl)
{
    QLCDNumber *lcd = pairedLcd(page, sl);
    const int start = sl->value();
    const double lcdStart = lcd ? lcd->value() : 0.0;
    sl->setFocus();
    QTest::keyClick(sl, Qt::Key_Up);
    const int up = sl->value();
    const double lcdUp = lcd ? lcd->value() : 0.0;
    QTest::keyClick(sl, Qt::Key_PageUp);
    const int pg = sl->value();
    const double lcdPg = lcd ? lcd->value() : 0.0;
    // A one-step nudge is too small to move some paired displays, and an LCD
    // that does not move reads exactly like a dead connect. sliderTournaments-
    // PerGeneration is the case: its slot shows int(slider/1000 * poolSize),
    // which with 120 individuals only changes every ~8.34 units, so up/pgup
    // both printed 60 and would have printed 60 with nothing connected at all.
    // End is one keystroke and moves every slider to its maximum.
    QTest::keyClick(sl, Qt::Key_End);
    const int endV = sl->value();
    const double lcdEnd = lcd ? lcd->value() : 0.0;
    sl->setValue(start);
    printf("    slider %-36s v=%-6d [%d..%d] page=%d up=%-6d pgup=%-6d end=%-6d "
           "lcd=%s %g/%g/%g/%g enabled=%d\n",
           qPrintable(sl->objectName()), start, sl->minimum(), sl->maximum(),
           sl->pageStep(), up, pg, endV,
           lcd ? qPrintable(lcd->objectName()) : "(none)",
           lcdStart, lcdUp, lcdPg, lcdEnd, sl->isEnabled() ? 1 : 0);
}

static void probeCombo(QComboBox *cb)
{
    printf("    combo  %-36s count=%d current=%d [%s] editable=%d enabled=%d\n",
           qPrintable(cb->objectName()), cb->count(), cb->currentIndex(),
           qPrintable(cb->currentText()), cb->isEditable() ? 1 : 0,
           cb->isEnabled() ? 1 : 0);
    for (int i = 0; i < cb->count(); ++i) printf("      item%-2d [%s]\n", i,
                                                 qPrintable(cb->itemText(i)));
    const int start = cb->currentIndex();
    // Walk the list with the keyboard, which is what a user does with it
    // closed. Selecting by index through the API would not exercise the
    // widget's own key handling or the activated()/currentIndexChanged()
    // signals the forms hang behaviour off. count Downs is one more than the
    // list is long from ANY starting index, so it must end CLAMPED at the last
    // item; count Ups must then clamp at the first. That pair is the
    // measurement -- not a round trip back to where it started, which it is
    // not and an earlier version of this wrongly implied.
    cb->setFocus();
    for (int i = 0; i < cb->count(); ++i) QTest::keyClick(cb, Qt::Key_Down);
    printf("      %d Down clamps at current=%d [%s]\n", cb->count(),
           cb->currentIndex(), qPrintable(cb->currentText()));
    for (int i = 0; i < cb->count(); ++i) QTest::keyClick(cb, Qt::Key_Up);
    printf("      %d Up   clamps at current=%d [%s]\n", cb->count(),
           cb->currentIndex(), qPrintable(cb->currentText()));
    cb->setCurrentIndex(start);
}

// Clicking a checkbox or radio, and reporting what ELSE moved. The forms wire
// real behaviour off these -- checkboxIgnoreMaxIndLength disables a spin box,
// the Environment radios switch which of two fields feeds the floor -- and a
// click is the only way to see it.
static void probeToggle(QWidget *page, QAbstractButton *b)
{
    const bool start = b->isChecked();
    const char *kind = qobject_cast<QCheckBox *>(b) ? "check" : "radio";
    // Record what is enabled AND what is checked before, so a side effect
    // shows up as a delta. The checked half is what catches a radio group:
    // clicking one member silently unchecks another, and the interlocks on
    // these pages -- Function vs Picture, the two constraint-manager modes --
    // are exactly that. Qt's own internal children (qt_*) are skipped: they
    // are implementation detail, not the form's.
    std::map<QString, bool> before, beforeChecked;
    for (QWidget *w : page->findChildren<QWidget *>()) {
        if (w->objectName().isEmpty() || w->objectName().startsWith("qt_")) continue;
        before[w->objectName()] = w->isEnabled();
        if (QAbstractButton *ab = qobject_cast<QAbstractButton *>(w))
            beforeChecked[w->objectName()] = ab->isChecked();
    }

    QTest::mouseClick(b, Qt::LeftButton, Qt::NoModifier, QPoint(8, b->height() / 2));
    QTest::qWait(30);
    const bool after = b->isChecked();

    printf("    %-6s %-36s text=[%s] checkable=%d %d->%d enabled=%d\n", kind,
           qPrintable(b->objectName()), qPrintable(b->text()),
           b->isCheckable() ? 1 : 0, start ? 1 : 0, after ? 1 : 0,
           b->isEnabled() ? 1 : 0);
    for (QWidget *w : page->findChildren<QWidget *>()) {
        if (w->objectName().isEmpty() || w->objectName().startsWith("qt_")) continue;
        auto it = before.find(w->objectName());
        if (it != before.end() && it->second != w->isEnabled())
            printf("      side effect: %s now %s\n", qPrintable(w->objectName()),
                   w->isEnabled() ? "ENABLED" : "greyed");
        QAbstractButton *ab = qobject_cast<QAbstractButton *>(w);
        auto ic = beforeChecked.find(w->objectName());
        if (ab && ab != b && ic != beforeChecked.end() && ic->second != ab->isChecked())
            printf("      side effect: %s now %s\n", qPrintable(w->objectName()),
                   ab->isChecked() ? "CHECKED" : "unchecked");
    }
    // Put it back, by clicking again -- a radio cannot be un-clicked, so only
    // a checkbox returns, and saying which is part of the record.
    if (qobject_cast<QCheckBox *>(b) && b->isChecked() != start) {
        QTest::mouseClick(b, Qt::LeftButton, Qt::NoModifier, QPoint(8, b->height() / 2));
        QTest::qWait(30);
    }
}

// The read-only half: everything on a container that carries state but is not
// driven here -- list boxes, trees, the line edits' text and validators.
static void dumpContainer(QWidget *c)
{
    for (QGroupBox *g : c->findChildren<QGroupBox *>())
        printf("    group  [%s] enabled=%d checkable=%d\n", qPrintable(g->title()),
               g->isEnabled() ? 1 : 0, g->isCheckable() ? 1 : 0);
    for (QLineEdit *le : c->findChildren<QLineEdit *>()) {
        if (qobject_cast<QAbstractSpinBox *>(le->parentWidget())) continue;
        printf("    edit   %-36s text=[%s] enabled=%d validator=%s\n",
               qPrintable(le->objectName()), qPrintable(le->text()),
               le->isEnabled() ? 1 : 0, qPrintable(validatorDesc(le->validator())));
    }
    for (QListWidget *lw : c->findChildren<QListWidget *>()) {
        printf("    list   %-36s count=%d current=%d enabled=%d\n",
               qPrintable(lw->objectName()), lw->count(), lw->currentRow(),
               lw->isEnabled() ? 1 : 0);
        // MT_IndividualsWidget's constants are GENERATED BY A RANDOMIZER at
        // construction, so their values differ every run and pinning them
        // would pin noise -- two runs of `metagui' differed by exactly these
        // 60 lines and nothing else. What IS invariant is the count and that
        // every one parses as an integer inside the generator's range, so
        // that is what is checked instead of the values.
        if (lw->objectName() == "constantsListBox") {
            int inRange = 0, parsed = 0, lo = INT_MAX, hi = INT_MIN;
            for (int i = 0; i < lw->count(); ++i) {
                bool ok = false;
                const int v = lw->item(i)->text().toInt(&ok);
                if (!ok) continue;
                ++parsed;
                lo = qMin(lo, v); hi = qMax(hi, v);
                if (v >= 0 && v <= 1000) ++inRange;
            }
            printf("      (randomly generated at construction -- values differ every\n"
                   "       run and are deliberately not pinned) parsedAsInt=%d/%d "
                   "inRange[0..1000]=%d\n", parsed, lw->count(), inRange);
            continue;
        }
        for (int i = 0; i < lw->count(); ++i)
            printf("      item%-2d [%s] hasIcon=%d\n", i,
                   qPrintable(lw->item(i)->text()), lw->item(i)->icon().isNull() ? 0 : 1);
    }
    for (QTreeWidget *tw : c->findChildren<QTreeWidget *>()) {
        printf("    tree   %-36s cols=%d rows=%d\n", qPrintable(tw->objectName()),
               tw->columnCount(), tw->topLevelItemCount());
        for (int i = 0; i < tw->topLevelItemCount(); ++i) {
            QTreeWidgetItem *ti = tw->topLevelItem(i);
            printf("      row%-2d ", i);
            for (int c2 = 0; c2 < tw->columnCount(); ++c2)
                printf("| %-22s", qPrintable(ti->text(c2)));
            // Column 0 of these tables carries no text; saying WHAT it carries
            // is the difference between "empty" and "the icon went missing".
            printf("| col0icon=%d col0check=%d\n", ti->icon(0).isNull() ? 0 : 1,
                   (int)ti->checkState(0));
        }
    }
}

// Drive everything in scope on one container: every spin box, slider, combo,
// checkbox and radio it owns, plus a comma probe into every validated field.
// The comma probe is the point of the whole exercise: C7 pinned 21 validators
// to QLocale::c() with RejectGroupSeparator so that a typed comma behaves as
// Qt 2's strtod-based one did, and nothing has ever typed a comma into any of
// them. Driving all of them, not a sample, is what says the loop reached them.
static void driveContainer(QWidget *page, QWidget *c)
{
    for (QLineEdit *le : c->findChildren<QLineEdit *>()) {
        if (qobject_cast<QAbstractSpinBox *>(le->parentWidget())) continue;
        if (!le->validator()) continue;
        printf("    comma  %-36s", qPrintable(le->objectName()));
        const QString before = le->text();
        le->setFocus();
        le->selectAll();
        QTest::keyClick(le, Qt::Key_Delete);
        QTest::keyClicks(le, QStringLiteral("9,81"));
        QTest::qWait(10);
        printf(" type[9,81] -> [%s] acceptable=%d\n", qPrintable(le->text()),
               le->hasAcceptableInput() ? 1 : 0);
        le->setText(before);
    }
    for (QSpinBox *sp : c->findChildren<QSpinBox *>())     probeSpin(sp);
    for (QSlider *sl : c->findChildren<QSlider *>()) {
        probeSlider(page, sl);
        probeSliderClick(sl);
    }
    for (QComboBox *cb : c->findChildren<QComboBox *>())    probeCombo(cb);
    for (QCheckBox *cb : c->findChildren<QCheckBox *>())    probeToggle(page, cb);
    for (QRadioButton *rb : c->findChildren<QRadioButton *>()) probeToggle(page, rb);
    // An empty [drive] block reads as a harness that failed to find anything.
    // Say which it is: the Robot page genuinely carries none of these five
    // widget kinds, only list boxes and four buttons that open dialogs.
    if (c->findChildren<QSpinBox *>().isEmpty() && c->findChildren<QSlider *>().isEmpty()
        && c->findChildren<QComboBox *>().isEmpty() && c->findChildren<QCheckBox *>().isEmpty()
        && c->findChildren<QRadioButton *>().isEmpty()) {
        int validated = 0;
        for (QLineEdit *le : c->findChildren<QLineEdit *>())
            if (le->validator() && !qobject_cast<QAbstractSpinBox *>(le->parentWidget()))
                ++validated;
        if (!validated) printf("    (nothing on this container is in C11's scope:"
                               " no spin box, slider, combo, checkbox, radio or"
                               " validated field)\n");
    }
}

// Visit one View page, survey it, then drive it tab by tab. Switching tabs by
// clicking the tab bar rather than setCurrentIndex() keeps every widget that
// gets driven actually VISIBLE when it is driven, which is the state a user
// would have it in and the state the oracle can photograph.
static void visitPage(const QString &menuItem)
{
    clickMenu("&View", menuItem);
    QTest::qWait(300);
    QStackedWidget *st = W->findChild<QStackedWidget *>();
    QWidget *pg = st ? st->currentWidget() : nullptr;
    printf("\n== PAGE %s ==\n", qPrintable(menuItem));
    if (!pg) { printf("  !! no current page\n"); fflush(stdout); return; }
    printf("  [class] %s objectName=[%s] enabled=%d\n", pg->metaObject()->className(),
           qPrintable(pg->objectName()), pg->isEnabled() ? 1 : 0);

    QTabWidget *tabs = pg->findChild<QTabWidget *>();
    if (!tabs) {
        printf("  [tabs] none\n");
        printf("  [survey]\n");   dumpContainer(pg);
        printf("  [drive]\n");    driveContainer(pg, pg);
        fflush(stdout);
        return;
    }
    printf("  [tabs] %s count=%d current=%d\n", qPrintable(tabs->objectName()),
           tabs->count(), tabs->currentIndex());
    for (int i = 0; i < tabs->count(); ++i)
        printf("    tab%-2d [%s] enabled=%d\n", i, qPrintable(tabs->tabText(i)),
               tabs->isTabEnabled(i) ? 1 : 0);
    for (int i = 0; i < tabs->count(); ++i) {
        QTabBar *bar = tabs->tabBar();
        QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier, bar->tabRect(i).center());
        QTest::qWait(150);
        QWidget *t = tabs->widget(i);
        printf("  -- tab %d [%s] shown=%d visible=%d\n", i, qPrintable(tabs->tabText(i)),
               tabs->currentIndex() == i ? 1 : 0, t->isVisible() ? 1 : 0);
        printf("  [survey]\n");   dumpContainer(t);
        printf("  [drive]\n");    driveContainer(pg, t);
        fflush(stdout);
    }
    // Anything the tab widget does not own -- the pages put buttons outside it.
    printf("  -- outside the tab widget\n");
    printf("  [survey]\n");
    for (QAbstractButton *b : pg->findChildren<QAbstractButton *>()) {
        if (tabs->isAncestorOf(b)) continue;
        printf("    button %-36s text=[%s] enabled=%d default=%d\n",
               qPrintable(b->objectName()), qPrintable(b->text()),
               b->isEnabled() ? 1 : 0,
               qobject_cast<QPushButton *>(b) ?
                   (qobject_cast<QPushButton *>(b)->isDefault() ? 1 : 0) : 0);
    }
    fflush(stdout);
}

// The GP page's three genetic-operator sliders are not independent: Mutation
// and Crossover are clamped so their sum cannot pass 1000, and Reproduction is
// the disabled remainder, recomputed by the slot each time either moves. The
// generic slider probe above cannot see any of this, because a one-step nudge
// leaves the sum far from the ceiling. This drives it into the clamp.
// Press the HANDLE and drag it past the right-hand end. QSlider has no public
// way to ask where its handle is, so the geometry comes from the style: the
// groove span is the width less the handle length, which is exactly what
// QStyle::sliderPositionFromValue is defined against.
static void dragSliderHardRight(QSlider *sl)
{
    const int handleLen = sl->style()->pixelMetric(QStyle::PM_SliderLength, nullptr, sl);
    const int span = sl->width() - handleLen;
    const int x = QStyle::sliderPositionFromValue(sl->minimum(), sl->maximum(),
                                                  sl->value(), span) + handleLen / 2;
    const QPoint from(x, sl->height() / 2);
    const QPoint to(sl->width() + 400, sl->height() / 2);
    QTest::mousePress(sl, Qt::LeftButton, Qt::NoModifier, from);
    QTest::qWait(30);
    QTest::mouseMove(sl, to);
    QTest::qWait(60);
    QTest::mouseRelease(sl, Qt::LeftButton, Qt::NoModifier, to);
    QTest::qWait(60);
}

static void probeGeneticSliders(QWidget *page)
{
    QSlider *mut = page->findChild<QSlider *>("sliderMutation");
    QSlider *cro = page->findChild<QSlider *>("sliderCrossover");
    QSlider *rep = page->findChild<QSlider *>("sliderReproduction");
    QLCDNumber *lm = page->findChild<QLCDNumber *>("lcdnumberMutation");
    QLCDNumber *lc = page->findChild<QLCDNumber *>("lcdnumberCrossover");
    QLCDNumber *lr = page->findChild<QLCDNumber *>("lcdnumberReproduction");
    if (!mut || !cro || !rep || !lm || !lc || !lr) {
        printf("  !! genetic sliders not all present\n"); return; }

    auto line = [&](const char *what) {
        printf("    %-28s mut=%-5d cro=%-5d rep=%-5d  lcd %g / %g / %g  sum=%d\n",
               what, mut->value(), cro->value(), rep->value(),
               lm->value(), lc->value(), lr->value(),
               mut->value() + cro->value() + rep->value());
    };
    printf("\n  [genetic slider interlock]\n");
    line("as loaded");

    // Drive with the keyboard, one page step at a time, far enough for the
    // clamp to engage. PageUp on these is 1, so this is deliberately a slow
    // walk rather than setValue() -- setValue() would emit the same signal but
    // would not prove the widget's own key handling reaches the slot.
    mut->setFocus();
    for (int i = 0; i < 200; ++i) QTest::keyClick(mut, Qt::Key_Up);
    line("after 200 Up on Mutation");
    cro->setFocus();
    for (int i = 0; i < 200; ++i) QTest::keyClick(cro, Qt::Key_Up);
    line("after 200 Up on Crossover");
    mut->setFocus();
    for (int i = 0; i < 200; ++i) QTest::keyClick(mut, Qt::Key_Down);
    line("after 200 Down on Mutation");

    // The same clamp reached by DRAGGING the handle -- ONE operation, and the
    // one the oracle can perform on 1.3, where arrow keys demonstrably do not
    // reach the slider and 280 groove clicks would not be worth anyone's time.
    // The press must land ON THE HANDLE: an earlier version pressed at x=6,
    // which is groove, so it page-stepped DOWN by one and the drag never
    // started -- Mutation went 50 -> 49 and read as a clamp that was really a
    // missed grab. The handle position comes from the style rather than a
    // guess, for the same reason.
    dragSliderHardRight(mut);
    line("after dragging Mutation hard right");
    fflush(stdout);
}

// sliderAlpha is the Environment page's only slider and it starts DISABLED --
// the texture checkbox owns it. The generic probe therefore measures a slider
// that cannot move, which is correct but says nothing. Turn the checkbox on
// first and drive it for real.
static void probeAlphaSlider(QWidget *page)
{
    QCheckBox *tex = page->findChild<QCheckBox *>("checkboxTextureFile");
    QSlider *al = page->findChild<QSlider *>("sliderAlpha");
    if (!tex || !al) { printf("  !! texture checkbox or alpha slider missing\n"); return; }
    printf("\n  [alpha slider, once the texture checkbox enables it]\n");
    printf("    before: checkbox=%d slider enabled=%d value=%d\n",
           tex->isChecked() ? 1 : 0, al->isEnabled() ? 1 : 0, al->value());
    if (!tex->isChecked())
        QTest::mouseClick(tex, Qt::LeftButton, Qt::NoModifier, QPoint(8, tex->height() / 2));
    QTest::qWait(50);
    printf("    after checkbox click: checkbox=%d slider enabled=%d value=%d\n",
           tex->isChecked() ? 1 : 0, al->isEnabled() ? 1 : 0, al->value());
    al->setFocus();
    for (int i = 0; i < 10; ++i) QTest::keyClick(al, Qt::Key_Down);
    printf("    after 10 Down: value=%d\n", al->value());
    QTest::keyClick(al, Qt::Key_PageUp);
    printf("    after 1 PageUp (page=%d): value=%d\n", al->pageStep(), al->value());
    QTest::keyClick(al, Qt::Key_End);
    printf("    after End: value=%d  (max=%d)\n", al->value(), al->maximum());
    QTest::keyClick(al, Qt::Key_Home);
    printf("    after Home: value=%d  (min=%d)\n", al->value(), al->minimum());
    fflush(stdout);
}

// What an exported file IS, in a form that compares across architectures: the
// size, a checksum, the line count and the ends. The 1.3 oracle can produce
// the same four things on the 2003 box with sha256sum and sed.
static void describeFile(const QString &out, const QString &stem)
{
    if (!QFile::exists(out)) {
        // checkEnding() is supposed to append the extension. If the file
        // landed WITHOUT it, say so rather than reporting a missing file.
        // Basenames only: the scratch directory differs per machine and this
        // output is diffed against a committed baseline.
        if (QFile::exists(stem))
            printf("  !! written WITHOUT the extension: [%s]\n",
                   qPrintable(QFileInfo(stem).fileName()));
        else
            printf("  !! no file at [%s] and none at [%s]\n",
                   qPrintable(QFileInfo(out).fileName()),
                   qPrintable(QFileInfo(stem).fileName()));
        fflush(stdout);
        return;
    }
    QFile f(out);
    if (!f.open(QIODevice::ReadOnly)) { printf("  !! cannot read it\n"); return; }
    const QByteArray all = f.readAll();
    f.close();
    const QByteArray sum = QCryptographicHash::hash(all, QCryptographicHash::Sha256).toHex();
    const QList<QByteArray> lines = all.split('\n');
    // A trailing newline leaves an empty last element; report both so the
    // oracle's `wc -l' and this agree on what is being counted.
    printf("  bytes=%lld sha256=%s newlines=%d endsWithNewline=%d\n",
           (long long)all.size(), sum.constData(), (int)all.count('\n'),
           all.endsWith('\n') ? 1 : 0);
    printf("  crCount=%d\n", (int)all.count('\r'));
    for (int i = 0; i < 3 && i < lines.size(); ++i)
        printf("  head%-2d [%s]\n", i, lines.at(i).constData());
    int last = lines.size() - 1;
    while (last > 0 && lines.at(last).isEmpty()) --last;
    for (int i = qMax(0, last - 2); i <= last; ++i)
        printf("  tail%-2d [%s]\n", i - last, lines.at(i).constData());
    fflush(stdout);
}

// -------------------------------------------------------------------- steps
// Hand a file dialog a path and accept it.
//
// THIS WAS AN INTERMITTENT FAILURE FOR MOST OF C11a AND C11b, and three
// diagnoses were wrong before the right one. Recording all three, because each
// looked right and the last is not guessable from the first two:
//
//  1. "The completer popup eats the Return." The popup is real, so the first
//     fix escaped it and pressed Return again. WORSE than the hang it fixed:
//     it accepted with the completion's filename, so the experiment silently
//     did not load and the run carried on against an empty tree.
//  2. "The wait after Return is too short." It was not; polling until the
//     dialog hid did not help.
//  3. THE ACTUAL CAUSE. QFileDialog::accept() treats a filename that carries a
//     DIRECTORY as a navigation request: it calls setDirectory() and RETURNS
//     WITHOUT ACCEPTING, expecting a second accept once the listing is there.
//     Whether one accept sufficed depended on whether the model had finished
//     populating -- so it worked most of the time, which is the worst way for
//     a race to behave: two of four recorded exportall runs lost one file
//     each -- 2 exports out of 32 -- and which one moved between runs.
//
// So the directory is set on the dialog FIRST and only the BASENAME is typed.
// Then accept has nothing to navigate to and closes the dialog every time.
// The typing is still real key events, which is the part under test; which
// directory Qt's own dialog is looking at is not SIGEL behaviour, and section
// 7 already accepts file-dialog differences.
static void acceptFileDialog(QFileDialog *fd, const QString &path)
{
    QLineEdit *le = fd->findChild<QLineEdit *>("fileNameEdit");
    if (!le) { printf("  !! no fileNameEdit\n"); fd->reject(); return; }
    const QFileInfo fi(QFileInfo(path).absoluteFilePath());
    fd->setDirectory(fi.absolutePath());
    QTest::qWait(60);
    le->setCompleter(nullptr);            // one less thing to race with
    le->setFocus();
    le->selectAll();
    QTest::keyClick(le, Qt::Key_Delete);
    QTest::keyClicks(le, fi.fileName());  // real key events, one per character
    QTest::qWait(60);
    if (le->text() != fi.fileName()) {
        printf("  [filedialog] typed [%s] but the field holds [%s]\n",
               qPrintable(fi.fileName()), qPrintable(le->text()));
        le->setText(fi.fileName());
    }
    QPushButton *accept = nullptr;
    for (QPushButton *b : fd->findChildren<QPushButton *>())
        if (b->isDefault() && b->isEnabled()) { accept = b; break; }
    if (accept)
        QTest::mouseClick(accept, Qt::LeftButton, Qt::NoModifier, accept->rect().center());
    else
        QTest::keyClick(le, Qt::Key_Return);
    for (int i = 0; i < 40 && fd->isVisible(); ++i) QTest::qWait(50);
    if (!fd->isVisible()) {
        // Say what was actually accepted. An earlier version returned here
        // without looking, and a run in which setDirectory() had not taken
        // would have written the right basename in the wrong place with no
        // symptom but a missing file three lines later.
        const QString got = fd->selectedFiles().isEmpty()
                                ? QString() : fd->selectedFiles().first();
        if (got != fi.absoluteFilePath())
            printf("  !! filedialog accepted [%s], not [%s]\n",
                   qPrintable(got), qPrintable(fi.absoluteFilePath()));
        return;
    }
    // Still open because something MODAL is on top of it -- Qt's own overwrite
    // confirmation is the case that happens -- is not the dialog failing to
    // accept. Leave it to whoever armed the next handler. SAY SO: returning
    // silently here is how a population export once produced no file at all,
    // with nothing in the output between the dialog opening and the missing
    // file, and that run was very nearly committed as a baseline.
    if (QApplication::activeModalWidget() != fd) {
        printf("  !! filedialog left open under a %s -- not accepted here\n",
               QApplication::activeModalWidget()
                   ? QApplication::activeModalWidget()->metaObject()->className()
                   : "(none)");
        fflush(stdout);
        return;
    }
    printf("  [filedialog] STILL open after clicking accept -- rejecting it\n");
    fd->reject();
    fflush(stdout);
}

// File > Export > <item>, answering the save dialog with `stem' -- no
// extension, because checkEnding() appends one and whether it does is part of
// what C11b measures. Returns the path that should have appeared.
static QString exportTo(const char *item, const QString &stem, const char *ext)
{
    const QString out = stem + "." + QString::fromLatin1(ext);
    QFile::remove(out);
    QFile::remove(stem);
    whenModal([stem](QWidget *m) {
        QFileDialog *fd = qobject_cast<QFileDialog *>(m);
        if (!fd) { printf("  !! export modal is not a QFileDialog: %s\n",
                          m->metaObject()->className()); m->close(); return; }
        acceptFileDialog(fd, stem);
    });
    if (!clickMenu("&File", "Export", QString::fromLatin1(item))) return QString();
    QTest::qWait(2500);
    return out;
}

static bool importFrom(const char *item, const QString &path)
{
    whenModal([path](QWidget *m) {
        QFileDialog *fd = qobject_cast<QFileDialog *>(m);
        if (!fd) { printf("  !! import modal is not a QFileDialog: %s [%s]\n",
                          m->metaObject()->className(), qPrintable(m->windowTitle()));
                   m->close(); return; }
        acceptFileDialog(fd, path);
    });
    if (!clickMenu("&File", "Import", QString::fromLatin1(item))) return false;
    QTest::qWait(2500);
    return true;
}

// `bytes' is optional and, when given, is the length of the EXACT content this
// hash was computed over -- not a separate stat of the file, which can disagree
// with it. See the roundtrip scenario for the run that made that necessary.
static QString sha256Of(const QString &path, qint64 *bytes = nullptr)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (bytes) *bytes = -1;
        return QStringLiteral("(unreadable)");
    }
    const QByteArray data = f.readAll();
    if (bytes) *bytes = data.size();
    const QByteArray sum =
        QCryptographicHash::hash(data, QCryptographicHash::Sha256).toHex();
    return QString::fromLatin1(sum);
}

static void openExperiment(const QString &path)
{
    whenModal([path](QWidget *m) {
        QFileDialog *fd = qobject_cast<QFileDialog *>(m);
        if (!fd) { printf("  !! modal is not a QFileDialog\n"); m->close(); return; }
        printf("  [filedialog] title=[%s] mode=%d nameFilters=[%s] labelAccept=[%s]\n",
               qPrintable(fd->windowTitle()), (int)fd->fileMode(),
               qPrintable(fd->nameFilters().join(" ;; ")),
               qPrintable(fd->labelText(QFileDialog::Accept)));
        acceptFileDialog(fd, path);          // real key events into the dialog
        fflush(stdout);
    });
    clickMenu("&File", "&Open Experiment");
    QTest::qWait(800);
}

// ------------------------------------------------------ REAL X INPUT (xtest)
// Every other scenario in this file drives Qt through QTest, which builds a
// QMouseEvent and hands it to QApplication::notify. That reaches every slot
// SIGEL has, which is why those scenarios are worth something -- but it never
// goes through QWindowSystemInterface, so four things a real mouse does are
// missing: window activation, the pointer grab a popup takes, Qt's synthesis
// of a double click out of two presses. PORTING.md's
// C10 section names exactly those four.
//
// WHAT DOES NOT DISTINGUISH THE TWO, because the obvious test is wrong:
// QEvent::spontaneous(). /usr/include/aarch64-linux-gnu/qt6/QtTest/qtestmouse.h
// :215-216 calls QSpontaneKeyEvent::setSpontaneous(&me) and only then
// qApp->notify(), so a QTest click reports spontaneous()==true exactly like a
// real one. Read out of the installed header, not assumed. A probe built on
// spontaneous() would have passed with no real click anywhere in it.
//
// The discriminator below is a NATIVE event filter. A real click arrives from
// the X server before Qt has any QMouseEvent to show; QTest's widget path
// produces no native event at all. This wants no X headers -- which are not
// installed here -- because only the leading bytes of the event are read.
//
// AND IT HAS TO UNDERSTAND XINPUT 2, which the first version did not. Qt's xcb
// plugin selects XI2 for pointer input, so a button press does NOT arrive as a
// core ButtonPress(4): it arrives as a GenericEvent(35) whose XI2 evtype is a
// uint16 at byte 8. The first version counted core events only and reported
// press=0 for a real click that had visibly opened a menu -- a false negative
// that would have read as "the port ignores real clicks". Core Enter, Leave and
// FocusIn still arrive as core events, which is why those counted and buttons
// did not. Both paths are counted now, and the run with QT_XCB_NO_XI2=1 in
// PORTING.md is the control: it moves the same clicks onto the core numbers.
static const int X_ButtonPress = 4, X_ButtonRelease = 5, X_MotionNotify = 6,
                 X_EnterNotify = 7, X_LeaveNotify = 8, X_FocusIn = 9,
                 X_GenericEvent = 35;
static const int XI_ButtonPress = 4, XI_ButtonRelease = 5, XI_Motion = 6,
                 XI_Enter = 7, XI_Leave = 8;

struct NativeSpy : public QAbstractNativeEventFilter
{
    int press = 0, release = 0, motion = 0, enter = 0, leave = 0, focusIn = 0;
    int viaXi2 = 0, viaCore = 0;
    quint8 xiOpcode = 0;                  // latched, deliberately NOT reset
    void reset() { press = release = motion = enter = leave = focusIn = 0;
                   viaXi2 = viaCore = 0; }
    bool nativeEventFilter(const QByteArray &, void *message, qintptr *) override
    {
        const quint8 *b = reinterpret_cast<const quint8 *>(message);
        // Bit 7 marks an event sent with SendEvent rather than generated by
        // the server; XTEST events are server-generated and do not carry it.
        const int t = int(b[0]) & 0x7f;
        if (t == X_GenericEvent) {
            // Byte 1 is the extension's major opcode. Only XInput's generic
            // events carry the evtype this decodes, so latch the opcode from
            // the first generic event that decodes as a pointer event and
            // ignore any other extension afterwards. Without this the decode
            // assumed every GenericEvent was XI2 -- harmless here, since a
            // wrong count fails the gate rather than faking a pass, but it was
            // an unchecked assumption. Found by review.
            if (xiOpcode && b[1] != xiOpcode) return false;
            quint16 ev = 0;
            memcpy(&ev, b + 8, sizeof ev);        // xcb_ge_generic_event_t.event_type
            switch (ev) {
            case XI_ButtonPress:   ++press;   ++viaXi2; xiOpcode = b[1]; break;
            case XI_ButtonRelease: ++release; ++viaXi2; break;
            case XI_Motion:        ++motion;  ++viaXi2; break;
            case XI_Enter:         ++enter;   ++viaXi2; break;
            case XI_Leave:         ++leave;   ++viaXi2; break;
            default: break;
            }
            return false;
        }
        switch (t) {
        case X_ButtonPress:   ++press;   ++viaCore; break;
        case X_ButtonRelease: ++release; ++viaCore; break;
        case X_MotionNotify:  ++motion;  ++viaCore; break;
        case X_EnterNotify:   ++enter;   ++viaCore; break;
        case X_LeaveNotify:   ++leave;   ++viaCore; break;
        case X_FocusIn:       ++focusIn; ++viaCore; break;
        default: break;
        }
        return false;
    }
};

// Qt sends WindowActivate and WindowDeactivate to EVERY widget in the window,
// not to the window: an app-wide filter counting them reported 63 for a single
// activation, which reads like a storm and is one event per widget. So the
// activation counters are restricted to the main window itself. Enter and
// Leave are genuinely per-widget -- one Enter per widget in the nested stack
// under the pointer -- and are left as counts, which is what they mean.
struct QtSpy : public QObject
{
    int press = 0, dbl = 0, enter = 0, leave = 0, activate = 0, deactivate = 0;
    QObject *main = nullptr;
    QStringList pressOn;   // WHO got the press -- a count alone cannot say
                           // whether a popup ate a click or a widget under it
                           // also received one, which is probe 4's question.
    void reset() { press = dbl = enter = leave = activate = deactivate = 0;
                   pressOn.clear(); }
    bool eventFilter(QObject *o, QEvent *e) override
    {
        switch (e->type()) {
        case QEvent::MouseButtonPress:
            ++press;
            if (pressOn.size() < 6)
                pressOn << QString::fromLatin1(o->metaObject()->className());
            break;
        case QEvent::MouseButtonDblClick: ++dbl;        break;
        case QEvent::Enter:               ++enter;      break;
        case QEvent::Leave:               ++leave;      break;
        case QEvent::WindowActivate:      if (o == main) ++activate;   break;
        case QEvent::WindowDeactivate:    if (o == main) ++deactivate; break;
        default: break;
        }
        return false;
    }
};

static NativeSpy g_nspy;
static QtSpy     g_qspy;

// xdotool rather than libXtst: XTest.h is not installed on this machine and
// xdotool is (PORTING.md records it being installed 2026-09-07 and unused).
// It talks XTEST to the same server, which is the only property that matters.
//
// The Qt event loop is PUMPED while xdotool runs rather than blocked on it.
// Blocking would let the whole burst queue up in the server and arrive in one
// batch afterwards, which is not what a user's clicks look like and would put
// Qt's double-click timing at the mercy of when the loop happened to resume.
static bool xdo(const QStringList &args)
{
    QProcess p;
    p.start(QStringLiteral("xdotool"), args);
    if (!p.waitForStarted(5000)) {
        printf("  !! xdotool would not start (%s)\n", qPrintable(p.errorString()));
        return false;
    }
    QElapsedTimer t; t.start();
    while (p.state() != QProcess::NotRunning && t.elapsed() < 20000)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    if (p.state() != QProcess::NotRunning) { p.kill(); p.waitForFinished(2000);
        printf("  !! xdotool did not finish\n"); return false; }
    const bool ok = p.exitStatus() == QProcess::NormalExit && p.exitCode() == 0;
    if (!ok) printf("  !! xdotool exit %d\n", p.exitCode());
    return ok;
}

// Move the pointer, then act, in ONE xdotool run -- `click' acts wherever the
// pointer already is, so a separate mousemove is a race with nothing to gain.
static bool xClickAt(const QPoint &g, int repeat = 1, int delayMs = 0)
{
    QStringList a{ QStringLiteral("mousemove"), QString::number(g.x()),
                   QString::number(g.y()), QStringLiteral("click") };
    if (repeat > 1) a << QStringLiteral("--repeat") << QString::number(repeat)
                      << QStringLiteral("--delay") << QString::number(delayMs);
    a << QStringLiteral("1");
    return xdo(a);
}

static bool xMoveTo(const QPoint &g)
{
    return xdo({ QStringLiteral("mousemove"), QString::number(g.x()),
                 QString::number(g.y()) });
}

// XGetInputFocus, as a name this file can print.
//
// xdpyinfo, NOT `xdotool getwindowfocus'. Review measured what xdotool does on
// an idle nested server: it exits 1, prints nothing on stdout, and says
// "XGetInputFocus returned the focused window of 1" on stderr. The true state
// there is PointerRoot. The first version of this function read that empty
// stdout as "None" -- so PointerRoot(1), a genuine None(0) and any xdotool
// failure all printed the same word. That is an absence read as a measurement,
// and it collides with the exact label the oracle uses for 1.3's state after a
// menu is dismissed: a port that reached PointerRoot would have printed None
// and looked like it matched 1.3.
//
// xdpyinfo prints the three apart: "focus:  PointerRoot", "focus:  None", or
// "focus:  window 0x2cf, revert to ...".
static QString runOut(const QString &prog, const QStringList &args)
{
    QProcess p;
    p.start(prog, args);
    if (!p.waitForStarted(5000)) return QString();
    QElapsedTimer t; t.start();
    while (p.state() != QProcess::NotRunning && t.elapsed() < 10000)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 10);
    if (p.state() != QProcess::NotRunning) { p.kill(); p.waitForFinished(2000); }
    return QString::fromLatin1(p.readAllStandardOutput());
}

// NAMED, never as a raw id. X window ids are assigned per run, so printing the
// number would make this file's baseline differ from itself on every run --
// caught by re-running it, not by reading it.
static QString xFocusName()
{
    QString line;
    for (const QString &l : runOut(QStringLiteral("xdpyinfo"), {}).split(QLatin1Char('\n')))
        if (l.startsWith(QLatin1String("focus:"))) { line = l.trimmed(); break; }
    if (line.isEmpty()) return QStringLiteral("(xdpyinfo said nothing)");
    if (line.contains(QLatin1String("PointerRoot"))) return QStringLiteral("PointerRoot");
    if (line.contains(QLatin1String("None")) && !line.contains(QLatin1String("window")))
        return QStringLiteral("None");
    const qsizetype i = line.indexOf(QLatin1String("window 0x"));
    if (i < 0) return QStringLiteral("(unreadable: ") + line + QLatin1Char(')');
    QString hex = line.mid(i + 9);
    hex = hex.left(hex.indexOf(QLatin1Char(',')));
    bool ok = false; const qulonglong v = hex.toULongLong(&ok, 16);
    if (!ok) return QStringLiteral("(unreadable: ") + line + QLatin1Char(')');
    if (W && v == qulonglong(W->winId())) return QStringLiteral("main window");
    if (QWidget *p = QApplication::activePopupWidget())
        if (v == qulonglong(p->winId())) return QStringLiteral("the popup");
    for (QWidget *w : QApplication::topLevelWidgets())
        if (w->windowHandle() && v == qulonglong(w->winId()))
            return QStringLiteral("top level ") + w->metaObject()->className();
    return QStringLiteral("a window that is not ours (the root, most likely)");
}

// Let whatever the server just sent actually reach the widgets.
static void settle(int ms = 300) { QTest::qWait(ms); }

static void spies(const char *what)
{
    printf("    %-26s native[press=%d rel=%d motion=%d enter=%d leave=%d focusIn=%d "
           "xi2=%d core=%d] qt[press=%d dbl=%d enter=%d leave=%d act=%d deact=%d]\n",
           what, g_nspy.press, g_nspy.release, g_nspy.motion, g_nspy.enter,
           g_nspy.leave, g_nspy.focusIn, g_nspy.viaXi2, g_nspy.viaCore,
           g_qspy.press, g_qspy.dbl, g_qspy.enter,
           g_qspy.leave, g_qspy.activate, g_qspy.deactivate);
    if (!g_qspy.pressOn.isEmpty())
        printf("    %-26s pressOn=[%s]\n", "", qPrintable(g_qspy.pressOn.join(", ")));
    fflush(stdout);
}

static void resetSpies() { g_nspy.reset(); g_qspy.reset(); }

// ---------------------------------------------------------------------- main
static int guidriveMain(int argc, char **argv)
{
    QApplication app(argc, argv);
    // WIDGET FILE DIALOGS, ALWAYS. Under QT_QPA_PLATFORM=offscreen no platform
    // theme offers a native one, so every baseline here was captured against
    // Qt's own widget dialog. Under xcb this machine's theme DOES offer one:
    // measured 2026-09-07, the same `guidrive open' run gives
    // labelAccept=[&Open] offscreen and labelAccept=[] with no `fileNameEdit'
    // child on :77, which is QFileDialogPrivate::usingWidgets() answering
    // false -- the dialog is a foreign process and acceptFileDialog() cannot
    // type into it. Neither QT_NO_XDG_DESKTOP_PORTAL=1, nor clearing
    // DBUS_SESSION_BUS_ADDRESS and XDG_RUNTIME_DIR, nor XDG_CURRENT_DESKTOP=,
    // nor QT_QPA_PLATFORMTHEME=minimal|gtk3 changed it, so it is not switchable
    // from the environment and the attribute is the only lever.
    // It is also the FAITHFUL setting: Qt 2.3 had no native dialog path at all,
    // so 1.3 always showed its own widgets. Inert offscreen, where there was
    // never a native helper to decline.
    QApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
    // The C locale for THIS PROGRAM'S OWN output, and it has to come AFTER the
    // QApplication constructor, which calls setlocale(LC_ALL, "") itself --
    // setting it before is silently undone. Every %g and %f below would
    // otherwise follow LC_NUMERIC, so the locale re-run printed `5,1' where the
    // ambient run printed `5.1': eight lines of harness artefact with nothing
    // to do with SIGEL, which is what made that check a tautology on a box
    // where the chosen locale was not even installed.
    // Qt's own locale handling is untouched: QLocale reads the environment,
    // which is exactly what C7's validator pinning is tested against.
    setlocale(LC_NUMERIC, "C");
    // PVM teardown used to be a guard object HERE, which ran inside this
    // function and took the process down with it before any return value
    // reached the caller. It now happens in main() below, after the status is
    // known and after the handler that preserves it is installed.
    // POSITIVE CONTROL for check.sh's `runtime connect' check, and it has to
    // be a real dead connect rather than a plain qWarning. That check greps
    // this process's stderr for Qt's "No such signal"/"No such slot", which is
    // the only thing that catches a string-based connect naming something
    // Qt 6 does not have -- $DEAD_SIGNALS is a closed regex over nine Qt 2
    // spellings and cannot see a tenth.
    //
    // Qt emits that warning under the LOGGING CATEGORY qt.core.qobject.connect,
    // and categories are filterable: QT_LOGGING_RULES='*=false' in the ambient
    // environment, or a qtlogging.ini, silences it. An empty stderr then looks
    // exactly like a clean run, so the check would pass while seeing nothing --
    // demonstrated by review, which ran the full gate green with a genuinely
    // dead connect injected.
    //
    // A control in the `default' category would not close it:
    // qt.core.qobject.connect can be disabled on its own, leaving the control
    // visible and the check blind. So the control is a deliberately bogus
    // connect, which fires through the SAME category by the SAME mechanism.
    // check.sh requires this line to be present and ignores it when grepping
    // for real ones.
    {
        QObject control;
        QObject::connect( &control, SIGNAL( guidriveStderrControl() ),
                          &control, SLOT( deleteLater() ) );
    }

    QString scenario = argc > 1 ? argv[1] : "open";
    QString expFile  = argc > 2 ? argv[2]
        : qEnvironmentVariable("SIGEL_EXP",
              "data-reordered/Experiments/twoBasesSimpleFitness2.exp");

    // sigel.cpp, main brings PVM up before the window exists. Only the visualize
    // scenario needs it, and starting a daemon for the others would be noise.
    // Evolution genuinely takes minutes; everything else that runs longer than
    // this is stuck, not busy.
    const bool slow = (scenario == "evolution" || scenario == "visualize"
                       || scenario == "pvmcrash");
    // `hold' hands the window to a person, so it has no time limit.
    if (scenario != "hold")
        armWatchdog(qEnvironmentVariableIntValue("SIGEL_WATCHDOG_MS") > 0
                        ? qEnvironmentVariableIntValue("SIGEL_WATCHDOG_MS")
                        : (slow ? 900000 : 240000));

    if (scenario == "visualize" || scenario == "evolution"
        || scenario == "pvmcrash" || scenario == "hold") {
        int info = pvm_start_pvmd(0, 0, 0);
        int mytid = pvm_mytid();
        g_pvmOurDaemon = (info == 0);   // PvmDupHost means someone else's
        g_pvmEnrolled  = (mytid >= 0);
        // The tid is a per-run value, so it is deliberately NOT printed: these
        // two scenarios could otherwise never be baselined.
        printf("  [pvm] start_pvmd=%d ourDaemon=%d enrolled=%d\n", info,
               g_pvmOurDaemon ? 1 : 0, g_pvmEnrolled ? 1 : 0);
        fflush(stdout);
    }

    SIG_MainWindow w(nullptr, "MainWindow");
    W = &w;
    w.resize(900, 750);
    w.show();
    (void)QTest::qWaitForWindowExposed(&w);

    prevEnabled = collectEnabled();
    printf("== SCENARIO %s ==\n", qPrintable(scenario));
    step("fresh start, no experiment");

    // --- open through the application's own File > Open path ---------------
    openExperiment(expFile);
    step("after File > Open Experiment", true, true, false, true);

    // --- the individuals page, reached through the View menu --------------
    clickMenu("&View", "&Population");
    QTest::qWait(300);
    step("after View > Population", true, true, true);

    if (scenario == "open") return 0;

    // `hold': the experiment is open and PVM is up. Hand the window to a
    // person until they close it.
    if (scenario == "hold") return app.exec();

    // --- the gate: one deterministic pass over the behaviour that has an
    // --- oracle reading behind it. Everything here was compared against the
    // --- running 1.3 binary; see PORTING.md's C10 section.
    if (scenario == "gate") {
        dumpIcons();
        QTreeWidget *t = indList();
        if (!t) { printf("!! no individuals list\n"); return 1; }

        // 1. sort by Fitness -- pins SIG_IndividualListItem::key(), including
        //    the tie order of the two 4.19675e-05 rows.
        QHeaderView *h = t->header();
        // viewport coordinates: sectionPosition() is in header space and only
            // agrees while nothing scrolls horizontally.
            int x = h->sectionViewportPosition(1) + h->sectionSize(1) / 2;
        QTest::mouseClick(h->viewport(), Qt::LeftButton, Qt::NoModifier,
                          QPoint(x, h->height() / 2));
        QTest::qWait(400);
        printf("\n== SORT BY FITNESS (one click) ==\norder=%s\n",
               h->sortIndicatorOrder() == Qt::AscendingOrder ? "Ascending" : "Descending");
        for (int i = 0; i < 5; ++i)
            printf("  first%d %s | %s | %s\n", i, qPrintable(t->topLevelItem(i)->text(0)),
                   qPrintable(t->topLevelItem(i)->text(1)), qPrintable(t->topLevelItem(i)->text(2)));
        for (int i = t->topLevelItemCount() - 3; i < t->topLevelItemCount(); ++i)
            printf("  last   %s | %s | %s\n", qPrintable(t->topLevelItem(i)->text(0)),
                   qPrintable(t->topLevelItem(i)->text(1)), qPrintable(t->topLevelItem(i)->text(2)));

        // 2. add -- dialog shape and where the new individuals land
        whenModal([](QWidget *m) {
            printf("dialog [%s]\n", qPrintable(m->windowTitle()));
            QSpinBox *sp = m->findChild<QSpinBox *>();
            if (sp) { printf("  spin default=%d min=%d max=%d\n", sp->value(),
                             sp->minimum(), sp->maximum());
                      sp->setFocus();
                      QTest::keyClick(sp, Qt::Key_A, Qt::ControlModifier);
                      QTest::keyClicks(sp, "5"); QTest::qWait(120); }
            for (QPushButton *b : m->findChildren<QPushButton *>())
                printf("  button [%s] default=%d\n", qPrintable(b->text()), b->isDefault());
            fflush(stdout);
            clickDlgButton(m, "OK");
        });
        printf("\n== ADD 5 ==\n");
        clickMenu("&Individuals", "&Add");
        QTest::qWait(2500);
        // The list is still sorted by Fitness ascending, and new individuals
        // carry -1, so they sort to the TOP. Printing the head both catches the
        // five and pins that the numeric sort key orders negatives correctly.
        printf("rows=%d\n", t->topLevelItemCount());
        for (int i = 0; i < 5; ++i)
            printf("  new %s | %s | %s\n", qPrintable(t->topLevelItem(i)->text(0)),
                   qPrintable(t->topLevelItem(i)->text(1)), qPrintable(t->topLevelItem(i)->text(2)));

        // 3. The large delete. NOTE WHAT THIS DOES AND DOES NOT CATCH: the
        //    list is sorted by Fitness here, and the item that is current when
        //    clear() runs turns out to be poolPosition 4 against a surviving
        //    pool of 5 -- IN RANGE. So reverting the blockSignals guard does
        //    NOT crash this scenario; getIndividual() is never called out of
        //    range. What it does, every time, is let slotSelectionChanged()
        //    run during clear() and repoint the detail pane at a SURVIVING
        //    individual, which is why `nameIsASurvivor' below is the line that
        //    actually guards the fix. The crash itself reproduces under the
        //    `ctxempty' scenario, where the list is unsorted and the stale
        //    position was 103 against a pool of 5.
        printf("\n== DELETE A LARGE BLOCK ==\n");
        clickRow(t, 5);
        clickRow(t, t->topLevelItemCount() - 1, Qt::ShiftModifier);
        printf("selected=%d\n", (int)t->selectedItems().count());
        whenModal([](QWidget *m) {
            QMessageBox *mb = qobject_cast<QMessageBox *>(m);
            printf("dialog [%s] text=[%s]\n", qPrintable(m->windowTitle()),
                   mb ? qPrintable(mb->text()) : "");
            if (mb) for (QAbstractButton *b : mb->buttons())
                printf("  button [%s] default=%d\n", qPrintable(b->text()),
                       b == mb->defaultButton());
            fflush(stdout);
            clickMsgButton(m, QMessageBox::Yes);
        });
        clickMenu("&Individuals", "&Delete");
        QTest::qWait(6000);
        printf("SURVIVED, rows=%d selected=%d\n", t->topLevelItemCount(),
               (int)t->selectedItems().count());
        // 1.3 leaves the DELETED individual's values in the detail pane until
        // the next selection change -- it only calls individualView->clear()
        // when the list becomes empty. The oracle read exactly that off the
        // running binary (a name that was not among the survivors, with its
        // history still populated), so blanking the pane here would be a
        // divergence, not an improvement.
        if (SIG_IndividualView *iv = W->findChild<SIG_IndividualView *>()) {
            const bool survivor = [&]{
                for (int i = 0; i < t->topLevelItemCount(); ++i)
                    if (t->topLevelItem(i)->text(0) == iv->textlabelShowName->text())
                        return true;
                return false;
            }();
            printf("detail pane name=[%s] age=[%s] fitness=[%s] historyEmpty=%d "
                   "nameIsASurvivor=%d\n",
                   qPrintable(iv->textlabelShowName->text()),
                   qPrintable(iv->textlabelShowAge->text()),
                   qPrintable(iv->textlabelShowFitness->text()),
                   iv->multilineeditHistory->toPlainText().isEmpty(),
                   survivor);
        }

        // 4. the two context menus, which differ only when the click really
        //    misses every row -- itemAt() has to be null for that.
        QTreeWidgetItem *last = t->topLevelItem(t->topLevelItemCount() - 1);
        QPoint empty(t->viewport()->width() / 2, t->visualItemRect(last).bottom() + 24);
        printf("\n== CONTEXT MENUS ==\nitemAt(empty)=%s\n",
               t->itemAt(empty) ? "a row -- probe is wrong" : "null");
        clickRow(t, 0);
        printf("selected before=%d\n", (int)t->selectedItems().count());
        dumpContextMenu(t->viewport(), empty, "empty space");
        printf("selected after=%d\n", (int)t->selectedItems().count());
        dumpContextMenu(t->viewport(), t->visualItemRect(t->topLevelItem(0)).center(), "a row");

        // 5. the MetaGP disable warning -- three custom button labels, one of
        //    which carries an ampersand Qt 6 would otherwise eat.
        printf("\n== METAGP DISABLE WARNING ==\n");
        clickMenu("&MetaGP", "&Use MetaGP");
        QTest::qWait(1200);
        whenModal([](QWidget *m) {
            QMessageBox *mb = qobject_cast<QMessageBox *>(m);
            printf("dialog [%s]\n", qPrintable(m->windowTitle()));
            if (mb) {
                printf("  text=[%s]\n", qPrintable(mb->text()));
                for (QAbstractButton *b : mb->buttons())
                    printf("  button [%s] default=%d\n", qPrintable(b->text()),
                           b == mb->defaultButton());
            }
            fflush(stdout);
            m->close();
        }, 5000);
        clickMenu("&MetaGP", "&Use MetaGP");
        QTest::qWait(2000);
        return 0;
    }

    // --- the five View pages C10 never opened ------------------------------
    // Population was the only one of the six that was ever driven. These five
    // hold every spin box, slider, combo, checkbox and validator in the
    // application. The population is untouched here (120 individuals), which
    // matters: slotTourPerGenChanged reads the pool size to compute what its
    // LCD shows, so this scenario must not follow a delete.
    if (scenario == "pages") {
        QStackedWidget *st2 = W->findChild<QStackedWidget *>();

        // THE BATTERY RUNS FIRST, and the reason is a probe error worth
        // recording: run after the page drive, it reported that lineeditXDim
        // ignored every one of its five probes. The field had not stopped
        // working -- the radio drive a moment earlier had clicked
        // radiobuttonPictureFile, whose side effect GREYS lineeditXDim, and a
        // disabled QLineEdit correctly ignores key events. Five false
        // negatives from one click, in the probe rather than the port.
        printf("\n== VALIDATOR BATTERY ==\n");
        printf("  (on a pristine Environment page -- see the note in the source:\n"
               "   running this after the page drive greys the integer field and\n"
               "   silently turns all five of its probes into false negatives)\n");
        clickMenu("&View", "&Environment");
        QTest::qWait(300);
        QWidget *env = st2 ? st2->currentWidget() : nullptr;
        if (!env) { printf("  !! no Environment page\n"); return 1; }
        if (QLineEdit *g = env->findChild<QLineEdit *>("lineeditGravityX"))
            batteryDouble(g);
        else printf("  !! no lineeditGravityX\n");
        if (QLineEdit *x = env->findChild<QLineEdit *>("lineeditXDim")) {
            printf("    (enabled=%d -- a greyed field would eat every probe)\n",
                   x->isEnabled() ? 1 : 0);
            batteryInt(x);
        } else printf("  !! no lineeditXDim\n");
        fflush(stdout);

        // Then the survey and the drive, page by page. NOTE THE ORDER
        // DEPENDENCE: the drive deliberately leaves what it clicked clicked --
        // a radio cannot be un-clicked, and restoring state would hide exactly
        // the interlocks this is here to find. So within a page the validated
        // fields are probed BEFORE the toggles that can grey them, and the two
        // interlock probes below run on the page they belong to, last.
        for (const char *m : {"&GP Parameters", "&Simulation Parameters",
                              "&Language Parameters", "&Robot", "&Environment"})
            visitPage(QString::fromLatin1(m));

        clickMenu("&View", "&Environment");
        QTest::qWait(300);
        if (st2 && st2->currentWidget()) probeAlphaSlider(st2->currentWidget());
        clickMenu("&View", "&GP Parameters");
        QTest::qWait(300);
        if (st2 && st2->currentWidget()) probeGeneticSliders(st2->currentWidget());
        return 0;
    }

    // --- does a value typed on a page reach the FILE? --------------------
    // The strongest check this project has, because the answer is bytes and
    // not a number a machine can round differently: putAllIntoExperiment()
    // runs on save, so the parameter block of the written .exp is exactly what
    // the five pages hold. i386/Qt 2 and aarch64/Qt 6 must write the same
    // block from the same typing, and that comparison cannot be muddied by
    // x87 vs IEEE the way a fitness value can.
    //
    // Run it TWICE, once with SIGEL_PAGEEDIT=1 and once without: both are the
    // FIRST save of a freshly loaded experiment, so the history whitespace
    // that C10 measured growing by 840 bytes a save is identical in the two,
    // and diffing them shows the edits and nothing else.
    if (scenario == "pagesave") {
        const bool edit = qgetenv("SIGEL_PAGEEDIT") == "1";
        QString out = scratch() + (edit ? "/pagesave-edited.exp"
                                        : "/pagesave-base.exp");
        QFile::remove(out);
        QStackedWidget *st3 = W->findChild<QStackedWidget *>();
        printf("\n== PAGE SAVE ROUND TRIP (edit=%d) ==\n", edit ? 1 : 0);

        auto page = [&](const char *m) -> QWidget * {
            clickMenu("&View", QString::fromLatin1(m));
            QTest::qWait(300);
            return st3 ? st3->currentWidget() : nullptr;
        };
        auto tab = [&](QWidget *pg, int i) {
            QTabWidget *tw = pg ? pg->findChild<QTabWidget *>() : nullptr;
            if (!tw) return;
            QTabBar *bar = tw->tabBar();
            QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier, bar->tabRect(i).center());
            QTest::qWait(120);
        };
        // Type into a line edit the way a person does: select all, delete,
        // then the characters. NOT setText() -- setText bypasses the validator
        // entirely, which is the one thing under test here.
        auto typeInto = [&](QWidget *pg, const char *name, const char *val) {
            QLineEdit *le = pg ? pg->findChild<QLineEdit *>(name) : nullptr;
            if (!le) { printf("  !! no %s\n", name); return; }
            le->setFocus(); le->selectAll();
            QTest::keyClick(le, Qt::Key_Delete);
            QTest::keyClicks(le, QString::fromLatin1(val));
            QTest::qWait(20);
            printf("  typed [%s] into %-34s -> [%s] acceptable=%d\n", val, name,
                   qPrintable(le->text()), le->hasAcceptableInput() ? 1 : 0);
        };
        auto typeSpin = [&](QWidget *pg, const char *name, const char *val) {
            QSpinBox *sp = pg ? pg->findChild<QSpinBox *>(name) : nullptr;
            if (!sp) { printf("  !! no %s\n", name); return; }
            sp->setFocus(); sp->selectAll();
            QTest::keyClick(sp, Qt::Key_Delete);
            QTest::keyClicks(sp, QString::fromLatin1(val));
            QTest::qWait(20);
            printf("  typed [%s] into %-34s -> value=%d\n", val, name, sp->value());
        };

        if (edit) {
            QWidget *pg = page("&Environment");
            tab(pg, 0);
            typeInto(pg, "lineeditGravityX", "1.25");
            typeInto(pg, "lineeditXDim", "77");
            tab(pg, 2);
            typeInto(pg, "lineeditYPlaneLevel", "2.5");

            pg = page("&Simulation Parameters");
            tab(pg, 0);
            typeInto(pg, "lineeditStepSize", "0.02");
            tab(pg, 1);
            if (QComboBox *cb = pg->findChild<QComboBox *>("comboboxDynaMechsIntegrator")) {
                cb->setFocus(); QTest::keyClick(cb, Qt::Key_Down);
                printf("  combo comboboxDynaMechsIntegrator -> %d [%s]\n",
                       cb->currentIndex(), qPrintable(cb->currentText()));
            }
            tab(pg, 2);
            typeInto(pg, "lineeditMaximalError", "0.2");

            pg = page("&GP Parameters");
            tab(pg, 0);
            typeSpin(pg, "spinboxRandomSeed", "123");
            typeSpin(pg, "spinboxMaxAge", "42");
            if (QComboBox *cb = pg->findChild<QComboBox *>("comboboxFitnessName")) {
                cb->setFocus(); QTest::keyClick(cb, Qt::Key_Down);
                printf("  combo comboboxFitnessName -> %d [%s]\n",
                       cb->currentIndex(), qPrintable(cb->currentText()));
            }
            if (QSlider *sl = pg->findChild<QSlider *>("sliderMutation")) {
                sl->setFocus();
                for (int i = 0; i < 200; ++i) QTest::keyClick(sl, Qt::Key_Up);
                printf("  slider sliderMutation -> %d\n", sl->value());
            }

            pg = page("&Language Parameters");
            typeSpin(pg, "spinboxNumberOfRegisters", "9");
            fflush(stdout);
        }

        whenModal([out](QWidget *m) {
            QFileDialog *fd = qobject_cast<QFileDialog *>(m);
            if (!fd) { printf("  !! save modal is not a QFileDialog\n"); m->close(); return; }
            acceptFileDialog(fd, out);
        });
        clickMenu("&File", "&Save Experiment");
        QTest::qWait(4000);
        printf("  [saved] exists=%d size=%lld\n", QFile::exists(out),
               QFileInfo(out).size());

        // The parameter block is everything before POPULATION BEGIN{ -- about
        // 170 lines, and it is what the five pages own. Printing it here makes
        // it part of the diffable record rather than a file to go and look at.
        QFile f(out);
        if (!f.open(QIODevice::ReadOnly)) { printf("  !! cannot re-read it\n"); return 1; }
        printf("\n== WRITTEN PARAMETER BLOCK ==\n");
        int n = 0;
        while (!f.atEnd()) {
            QByteArray l = f.readLine();
            if (l.startsWith("POPULATION BEGIN{")) break;
            printf("%3d| %s", ++n, l.constData());
            if (!l.endsWith("\n")) printf("\n");
        }
        printf("== %d lines ==\n", n);
        fflush(stdout);
        return 0;
    }

    // --- every File > Export child --------------------------------------
    // C11b. C10 drove ONE of the sixteen Import/Export children. These are
    // FILE FORMATS, so the comparison is bytes rather than a rendered widget
    // -- the one kind of check i386-x87 versus aarch64-IEEE cannot muddy, and
    // the reason this item was ordered ahead of the dialogs.
    if (scenario == "exportall") {
        struct Item { const char *menu; const char *ext; bool needsIndividual; };
        static const Item items[] = {
            { "GP-Parameters",         "gpp", false },
            { "Simulation-Parameters", "sip", false },
            { "Language-Parameters",   "lap", false },
            { "Environment",           "env", false },
            { "Population",            "pop", false },
            { "Program",               "prg", true  },
            { "Individual",            "ind", true  },
            { "...to GNU plot",        "dat", false },
        };

        // Program and Individual export whatever is SELECTED, so pick a named
        // individual -- the same 55658 C10 used, so the two steps compare.
        QTreeWidget *t = indList();
        if (!t) { printf("!! no individuals list\n"); return 1; }
        int want = -1;
        for (int i = 0; i < t->topLevelItemCount(); ++i)
            if (t->topLevelItem(i)->text(0) == "55658") { want = i; break; }
        if (want < 0) { printf("!! individual 55658 not found\n"); return 1; }
        clickRow(t, want);
        printf("\n== EXPORT ALL ==\nselected=%s\n",
               qPrintable(t->topLevelItem(want)->text(0)));

        for (const Item &it : items) {
            // Deliberately give a name with NO extension: checkEnding() is
            // supposed to append one, and that is behaviour worth diffing.
            const QString stem = scratch() + "/x11b-" + QString::fromLatin1(it.ext);
            const QString out  = stem + "." + QString::fromLatin1(it.ext);
            QFile::remove(out);
            QFile::remove(stem);
            printf("\n-- Export > %s\n", it.menu);
            whenModal([stem](QWidget *m) {
                QFileDialog *fd = qobject_cast<QFileDialog *>(m);
                if (!fd) { printf("  !! modal is not a QFileDialog: %s\n",
                                  m->metaObject()->className()); m->close(); return; }
                printf("  [dialog] title=[%s] filters=[%s] accept=[%s]\n",
                       qPrintable(fd->windowTitle()),
                       qPrintable(fd->nameFilters().join(" ;; ")),
                       qPrintable(fd->labelText(QFileDialog::Accept)));
                printf("  [dialog] parentIsTheMainWindow=%d\n", (fd->parentWidget() && fd->parentWidget()->window() == W) ? 1 : 0);
                acceptFileDialog(fd, stem);
            });
            if (!clickMenu("&File", "Export", QString::fromLatin1(it.menu))) continue;
            QTest::qWait(2500);
            describeFile(out, stem);
        }
        return 0;
    }

    // --- the round trip: export, MUTATE, import it back, export again -----
    // The mutation is the whole point and an earlier version did not have it.
    // Without it both exports serialise the SAME in-memory object and the file
    // imported in between came from that object, so an importer that opens
    // nothing, parses nothing or is never reached leaves the object untouched
    // and the probe prints STABLE. It could see a LOSSY reader and was
    // structurally blind to a NO-OP one -- which is the more likely failure,
    // since a dead connect or a greyed menu item produces exactly that. Found
    // by review.
    //
    // So: export A, change one field on the owning page, import A back, export
    // B. A working importer overwrites the change and B == A. A no-op importer
    // leaves the change in place and B != A. importFrom's return value is
    // checked now too, so a greyed Import item fails rather than passing.
    if (scenario == "roundtrip") {
        QStackedWidget *st4b = W->findChild<QStackedWidget *>();
        struct Item { const char *menu; const char *ext; };
        static const Item items[] = {
            { "GP-Parameters",         "gpp" },
            { "Simulation-Parameters", "sip" },
            { "Language-Parameters",   "lap" },
            { "Environment",           "env" },
            { "Population",            "pop" },
        };
        printf("\n== ROUND TRIP ==\n");
        // One field per format, on the page that owns it, changed between the
        // two exports so that a no-op importer cannot pass.
        struct Mut { const char *page; int tab; const char *widget; const char *value; };
        auto mutate = [&](const Mut &mu) -> bool {
            clickMenu("&View", QString::fromLatin1(mu.page));
            QTest::qWait(300);
            QWidget *pg = st4b ? st4b->currentWidget() : nullptr;
            if (!pg) { printf("  !! no page %s\n", mu.page); return false; }
            if (QTabWidget *tw = pg->findChild<QTabWidget *>()) {
                QTabBar *bar = tw->tabBar();
                QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier,
                                  bar->tabRect(mu.tab).center());
                QTest::qWait(150);
            }
            if (QSpinBox *sp = pg->findChild<QSpinBox *>(mu.widget)) {
                sp->setFocus(); sp->selectAll();
                QTest::keyClick(sp, Qt::Key_Delete);
                QTest::keyClicks(sp, QString::fromLatin1(mu.value));
                QTest::qWait(30);
                printf("  mutated %s -> %d\n", mu.widget, sp->value());
                return true;
            }
            if (QLineEdit *le = pg->findChild<QLineEdit *>(mu.widget)) {
                le->setFocus(); le->selectAll();
                QTest::keyClick(le, Qt::Key_Delete);
                QTest::keyClicks(le, QString::fromLatin1(mu.value));
                QTest::qWait(30);
                printf("  mutated %s -> [%s]\n", mu.widget, qPrintable(le->text()));
                return true;
            }
            printf("  !! no widget %s on %s\n", mu.widget, mu.page);
            return false;
        };

        // TYPING INTO THE WIDGET IS NOT ENOUGH, AND WITHOUT THIS THE WHOLE
        // SCENARIO CANNOT FAIL.
        //
        // The round trip is: export1, change something, import export1,
        // export2, require export2 == export1 -- so that an importer which
        // opens nothing, parses nothing or is never reached cannot pass. C11c
        // fixed one version of that hole (both exports used to serialise the
        // same in-memory object). This is a second version and it survived:
        // `mutate' types into a WIDGET, but importFrom ends with
        // getOutOfExperiment(), which refreshes every widget FROM the
        // experiment -- and the experiment never saw the typed value, because
        // only putIntoExperiment moves it there. So the change was undone
        // before export2 whether the reader ran or not.
        //
        // Demonstrated: with `gpExperiment.gpParameter.readFromFile()' commented
        // out entirely, the scenario still printed
        //   export1 1299 bytes 88e851e2...
        //   export2 1299 bytes 88e851e2...
        //   ROUND TRIP STABLE (and the import undid the change)
        // -- the exact words, on a gutted importer.
        //
        // putAllIntoExperiment() is the aggregator the Save path uses. It
        // pushes experimentView, gpParameter, simulationParameter,
        // languageParameters and environmentView -- NOT robotView and NOT the
        // population, so it is not "all five View pages" and it does not cover
        // the Population entry. That entry needs no push: `Individuals >
        // Delete' changes the experiment directly, so it was falsifiable
        // already. The four widget-typed entries are the ones that needed it,
        // and they needed it for an ORDERING reason rather than a missing
        // call -- three of their four export slots DO call putIntoExperiment,
        // but importFrom's closing getOutOfExperiment() has already wiped the
        // widget by then.
        // THROUGH THE GUI, not by calling putAllIntoExperiment() directly.
        // Selecting a row in the experiment tree runs it
        // (SIG_ExperimentListView::slotSelectionChanged), and every
        // View page switch calls selectItem() -> setCurrentItem(), so one
        // click on a page the mutation did not touch pushes the typed value
        // along the path a user actually takes. A direct call would work and
        // would not contaminate the exports -- measured -- but this file has
        // already recorded one reading that was wrong BECAUSE a direct invoke
        // ran putAllIntoExperiment() at a moment no GUI action would (§9, the
        // generation-counter LCD). Using the real path removes the question
        // rather than answering it.
        auto pushToExperiment = [&]() -> bool {
            if (!clickMenu("&View", "&Robot")) {
                printf("  !! could not switch page to push the mutation\n");
                return false;
            }
            QTest::qWait(300);
            return true;
        };
        static const Mut muts[] = {
            { "&GP Parameters",         0, "spinboxRandomSeed",        "321"  },
            { "&Simulation Parameters", 0, "lineeditStepSize",         "0.09" },
            { "&Language Parameters",   0, "spinboxNumberOfRegisters", "7"    },
            { "&Environment",           0, "lineeditGravityX",         "3.5"  },
        };
        for (int i = 0; i < (int)(sizeof(items) / sizeof(items[0])); ++i) {
            const Item &it = items[i];
            const QString a = scratch() + "/rt-a-" + QString::fromLatin1(it.ext);
            const QString b = scratch() + "/rt-b-" + QString::fromLatin1(it.ext);
            printf("\n-- %s\n", it.menu);
            const QString fa = exportTo(it.menu, a, it.ext);
            if (fa.isEmpty() || !QFile::exists(fa)) { printf("  !! first export failed\n"); continue; }
            // SIZE AND HASH MUST COME FROM THE SAME READ. They used to be
            // taken at different moments -- the hash here, the size in the
            // printf ~30 lines and one import later -- and a run printed
            // `export1 0 bytes' beside the hash of a 299-byte file. Those two
            // cannot both be true of one stable file (sha256 of empty is
            // e3b0c442...), so the line was reporting two different instants
            // as if they were one. sha256Of already reads the whole file;
            // taking the length from the SAME QByteArray removes the race from
            // the probe rather than papering over it with a retry.
            qint64 na = -1;
            const QString sa = sha256Of(fa, &na);

            // Population has no single field to type into; shrink the pool
            // instead, which is a change the .pop must undo.
            bool mutated = false;
            if (i < (int)(sizeof(muts) / sizeof(muts[0]))) mutated = mutate(muts[i]);
            else {
                clickMenu("&View", "&Population");
                QTest::qWait(300);
                QTreeWidget *tl = indList();
                if (tl && tl->topLevelItemCount() > 3) {
                    clickRow(tl, 0);
                    clickRow(tl, 2, Qt::ShiftModifier);
                    whenModal([](QWidget *m) { clickMsgButton(m, QMessageBox::Yes); });
                    clickMenu("&Individuals", "&Delete");
                    QTest::qWait(4000);
                    printf("  mutated the pool -> %d rows\n", tl->topLevelItemCount());
                    mutated = true;
                }
            }
            if (!mutated) { printf("  !! could not mutate -- this run proves nothing\n"); continue; }
            // Without this the change never reaches the experiment and the
            // comparison below is an identity test -- see pushToExperiment.
            if (!pushToExperiment()) { printf("  !! could not push -- this run proves nothing\n"); continue; }

            if (!importFrom(it.menu, fa)) { printf("  !! IMPORT FAILED\n"); continue; }
            const QString fb = exportTo(it.menu, b, it.ext);
            if (fb.isEmpty() || !QFile::exists(fb)) { printf("  !! second export failed\n"); continue; }
            qint64 nb = -1;
            const QString sb = sha256Of(fb, &nb);
            printf("  export1 %lld bytes  %s\n", (long long)na, qPrintable(sa));
            printf("  export2 %lld bytes  %s\n", (long long)nb, qPrintable(sb));
            if (sa == sb) { printf("  ROUND TRIP STABLE (and the import undid the change)\n");
                            fflush(stdout); continue; }
            // Whitespace-only growth is C10's documented history defect, not a
            // reader/writer disagreement -- say which before calling it either.
            QFile x(fa), y(fb);
            QByteArray xa, yb;
            if (x.open(QIODevice::ReadOnly)) { xa = x.readAll(); x.close(); }
            if (y.open(QIODevice::ReadOnly)) { yb = y.readAll(); y.close(); }
            auto squeeze = [](const QByteArray &in) {
                QByteArrayList keep;
                for (const QByteArray &l : in.split('\n'))
                    if (!l.trimmed().isEmpty()) keep << l.trimmed();
                return keep.join('\n');
            };
            const bool onlyBlank = squeeze(xa) == squeeze(yb);
            printf("  ROUND TRIP %s\n", onlyBlank
                   ? "differs ONLY in blank lines -- C10's history growth, "
                     "not a reader/writer disagreement"
                   : "*** CHANGED IN CONTENT ***");
            printf("  delta=%lld bytes  blankLineDelta=%d\n",
                   (long long)(yb.size() - xa.size()),
                   (int)(yb.count('\n') - xa.count('\n')));
            fflush(stdout);
        }

        // Robot import is the odd one out: there is no Export > Robot, so it
        // cannot round-trip through itself. Import the experiment's OWN robot
        // and check nothing moves -- the robot the file already names.
        printf("\n-- Robot (import only; there is no Export > Robot)\n");
        const QString lapBefore = exportTo("Language-Parameters",
                                           scratch() + "/rt-rob-before", "lap");
        const QString before = lapBefore.isEmpty() ? QString() : sha256Of(lapBefore);
        const QString rrb = QStringLiteral("data-reordered/twoBases/twoBases.rrb");
        printf("  importing %s (exists=%d)\n", qPrintable(rrb), QFile::exists(rrb));
        importFrom("Robot", QFileInfo(rrb).absoluteFilePath());
        clickMenu("&View", "&Robot");
        QTest::qWait(300);
        if (QStackedWidget *st = W->findChild<QStackedWidget *>())
            if (QWidget *pg = st->currentWidget())
                for (QListWidget *lw : pg->findChildren<QListWidget *>())
                    printf("  list %-22s count=%d\n", qPrintable(lw->objectName()), lw->count());
        const QString lapAfter = exportTo("Language-Parameters",
                                          scratch() + "/rt-rob-after", "lap");
        const QString after = lapAfter.isEmpty() ? QString() : sha256Of(lapAfter);
        printf("  language-parameters before %s\n  language-parameters after  %s\n",
               qPrintable(before), qPrintable(after));
        printf("  ROBOT IMPORT %s\n",
               before == after ? "left the language parameters alone"
                               : "*** CHANGED the language parameters ***");

        // Program and Individual import differ, and the difference is the
        // point: slotImportProgram REPLACES the selected individual's program
        // (SIG_AllIndividualsView.cpp, slotEvolutionNotRunning, importProgram on the selected item),
        // while slotImportIndividual adds a new one. So +0 for Program is
        // correct and a +1 there would be the defect. Program therefore gets
        // the real round trip -- export, import into the same individual,
        // export again -- and Individual gets the row count.
        printf("\n-- Program import (replaces the selected individual's program)\n");
        {
            QTreeWidget *tl = indList();
            int row = -1;
            for (int i = 0; tl && i < tl->topLevelItemCount(); ++i)
                if (tl->topLevelItem(i)->text(0) == "55658") { row = i; break; }
            if (row < 0) printf("  !! 55658 not found\n");
            else {
                clickRow(tl, row);
                const QString a1 = exportTo("Program", scratch() + "/rt-prg-a", "prg");
                const int before = tl->topLevelItemCount();
                if (!a1.isEmpty() && QFile::exists(a1)) {
                    importFrom("Program", a1);
                    clickMenu("&View", "&Population");
                    QTest::qWait(300);
                    tl = indList();
                    for (int i = 0; tl && i < tl->topLevelItemCount(); ++i)
                        if (tl->topLevelItem(i)->text(0) == "55658") { clickRow(tl, i); break; }
                    const QString b1 = exportTo("Program", scratch() + "/rt-prg-b", "prg");
                    printf("  rows %d -> %d (delta %+d, 0 is correct -- it replaces)\n",
                           before, tl ? tl->topLevelItemCount() : -1,
                           (tl ? tl->topLevelItemCount() : 0) - before);
                    printf("  export1 %s\n  export2 %s\n", qPrintable(sha256Of(a1)),
                           qPrintable(sha256Of(b1)));
                    printf("  ROUND TRIP %s\n",
                           sha256Of(a1) == sha256Of(b1) ? "STABLE" : "*** CHANGED ***");
                }
            }
        }

        printf("\n-- Individual import (adds one)\n");
        {
            QTreeWidget *tl = indList();
            int row = -1;
            for (int i = 0; tl && i < tl->topLevelItemCount(); ++i)
                if (tl->topLevelItem(i)->text(0) == "55658") { row = i; break; }
            if (row >= 0) clickRow(tl, row);
            const QString a1 = exportTo("Individual", scratch() + "/rt-ind-a", "ind");
            const int before = tl ? tl->topLevelItemCount() : -1;
            if (!a1.isEmpty() && QFile::exists(a1)) {
                importFrom("Individual", a1);
                clickMenu("&View", "&Population");
                QTest::qWait(300);
                tl = indList();
                const int after = tl ? tl->topLevelItemCount() : -1;
                printf("  rows %d -> %d (delta %+d)\n", before, after, after - before);
                // Where does the newcomer land, and is it a copy of 55658?
                int copies = 0;
                for (int i = 0; tl && i < tl->topLevelItemCount(); ++i)
                    if (tl->topLevelItem(i)->text(0) == "55658") ++copies;
                printf("  individuals now named 55658: %d\n", copies);
                if (tl && after > 0) {
                    QTreeWidgetItem *last = tl->topLevelItem(after - 1);
                    printf("  last row: %s | %s | %s\n", qPrintable(last->text(0)),
                           qPrintable(last->text(1)), qPrintable(last->text(2)));
                }
            }
        }
        fflush(stdout);
        return 0;
    }

    // --- overwrite: an export over an existing file (D35) ------------------
    // SIGEL itself never asks. A name without the extension, over a taken
    // name, must give a date-stamped file and no prompt. The name with the
    // extension must raise the file dialog's own confirmation, as a child of
    // the dialog, and No must leave the file alone. The export's file dialog
    // must have the main window as its window.
    if (scenario == "overwrite") {
        const QString stem = scratch() + "/x11b-ow";
        const QString out  = stem + ".sip";
        QFile::remove(out);
        // The EXTENSIONLESS name too. checkEnding() appends .sip after the
        // dialog closes, so nothing normally creates `x11b-ow' -- but if
        // checkEnding ever regressed, the first export would write it, nothing
        // would clean it up, and every later run would spend 240 seconds in the
        // watchdog and blame the pool position. Found by review.
        QFile::remove(stem);
        // Date-stamped names from earlier runs, so the count below is this run's.
        const QRegularExpression stampedName(QStringLiteral(
            "^x11b-ow-\\d{4}-\\d{2}-\\d{2}-\\d{2}-\\d{2}-\\d{2}\\.sip$"));
        auto stampedFiles = [&]() {
            QStringList names;
            for (const QString &n : QDir(scratch()).entryList(
                     QStringList() << QStringLiteral("x11b-ow-*.sip"), QDir::Files))
                if (stampedName.match(n).hasMatch()) names << n;
            return names;
        };
        for (const QString &n : stampedFiles()) QFile::remove(scratch() + "/" + n);
        auto firstLine = [&]() -> QString {
            QFile g(out);
            if (!g.open(QIODevice::ReadOnly)) return QString();
            return QString::fromLatin1(g.readLine()).trimmed();
        };
        printf("\n== OVERWRITE ==\n");

        // 1. First export: the file does not exist, so nothing asks.
        whenModal([stem](QWidget *m) {
            QFileDialog *fd = qobject_cast<QFileDialog *>(m);
            if (!fd) { m->close(); return; }
            printf("  [export dialog] parentIsTheMainWindow=%d\n", (fd->parentWidget() && fd->parentWidget()->window() == W) ? 1 : 0);
            acceptFileDialog(fd, stem);
        });
        clickMenu("&File", "Export", "Simulation-Parameters");
        QTest::qWait(2500);
        printf("  [first export] exists=%d size=%lld\n", QFile::exists(out),
               QFileInfo(out).size());

        QFile f(out);
        if (f.open(QIODevice::WriteOnly)) { f.write("SENTINEL\n"); f.close(); }
        printf("  [overwritten with sentinel] size=%lld\n", QFileInfo(out).size());

        // 2. The name WITHOUT the extension while x11b-ow.sip exists. The file
        //    dialog never sees x11b-ow.sip, so it cannot ask. checkEnding()
        //    must write a date-stamped name instead, and nothing may ask.
        int modals = 0;
        std::function<void(QWidget *)> handler = [&](QWidget *m) {
            ++modals;
            if (QFileDialog *fd = qobject_cast<QFileDialog *>(m)) {
                acceptFileDialog(fd, stem);
                whenModal(handler, 4000);
                return;
            }
            printf("  !! a %s [%s] asked, and nothing should\n",
                   m->metaObject()->className(), qPrintable(m->windowTitle()));
            m->close();
        };
        whenModal(handler);
        clickMenu("&File", "Export", "Simulation-Parameters");
        QTest::qWait(4500);
        cancelModalHandler();
        const QStringList stamped = stampedFiles();
        printf("  [no extension] modals=%d stampedFiles=%d stampedSize=%lld"
               " firstLine=[%s] sentinelSurvived=%d\n",
               modals, int(stamped.size()),
               stamped.isEmpty() ? -1LL
                                 : (long long)QFileInfo(scratch() + "/" + stamped.first()).size(),
               qPrintable(firstLine()), firstLine() == "SENTINEL" ? 1 : 0);
        fflush(stdout);

        // 3. The name WITH the extension while it exists. Now the file dialog's
        //    own confirmation asks, as a child of the dialog. Qt raises it
        //    inside accept(), in an exec() nested under the click on Save, so
        //    its poller runs BEFORE acceptFileDialog clicks. After No the
        //    dialog is still open, and acceptFileDialog rejects it.
        auto withExtension = [&](const char *label, QMessageBox::StandardButton answer) {
            int confirmations = 0, childOfDialog = -1;
            QString text;
            QFileDialog *dialog = nullptr;
            QTimer poll;
            QObject::connect(&poll, &QTimer::timeout, [&]() {
                QMessageBox *mb = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                if (!mb) return;
                poll.stop();
                ++confirmations;
                childOfDialog = (dialog && mb->parentWidget() == dialog) ? 1 : 0;
                text = mb->text();
                text.replace(scratch(), QStringLiteral("<scratch>"));
                clickMsgButton(mb, answer);
            });
            whenModal([&](QWidget *m) {
                dialog = qobject_cast<QFileDialog *>(m);
                if (!dialog) { m->close(); return; }
                poll.start(50);
                acceptFileDialog(dialog, out);
                poll.stop();
            });
            clickMenu("&File", "Export", "Simulation-Parameters");
            QTest::qWait(4500);
            cancelModalHandler();
            printf("  [%s] confirmations=%d childOfTheFileDialog=%d\n",
                   label, confirmations, childOfDialog);
            printf("    text=[%s]\n", qPrintable(text));
            printf("  [%s] size=%lld firstLine=[%s] sentinelSurvived=%d stampedFiles=%d\n",
                   label, (long long)QFileInfo(out).size(), qPrintable(firstLine()),
                   firstLine() == "SENTINEL" ? 1 : 0, int(stampedFiles().size()));
            fflush(stdout);
        };
        withExtension("answered No", QMessageBox::No);
        withExtension("answered Yes", QMessageBox::Yes);
        return 0;
    }

    // --- the six dialogs C10 never opened ---------------------------------
    // C11c. Five of the six are modal exec()s and one -- SIG_IndividualView --
    // is a non-modal top-level window opened with show(), so it has to be
    // found among topLevelWidgets() rather than caught by whenModal().
    //
    // Both tree-driven dialogs are reached here through their PUSH BUTTON:
    // slotPushButtonEditClicked and slotEditHost both forward to the
    // double-click slot with currentItem(), so the button exercises the same
    // code by a route that needs no coordinates.
    //
    // NOT because double-click is unreliable. The oracle reported that in C11a
    // and then RETRACTED it after testing properly -- 15 of 15 -- so the
    // sequence sent to it uses double-click. Its C11a failures were a
    // collapsed tree shifting every row by about 22px under a fixed y, which
    // is a coordinate bug wearing a double-click costume. Recorded because the
    // retraction is the useful part: "nothing happened" is a symptom that
    // hides its own cause, where "the wrong thing happened" does not.
    if (scenario == "dialogs") {
        QStackedWidget *st4 = W->findChild<QStackedWidget *>();
        auto page = [&](const char *m) -> QWidget * {
            clickMenu("&View", QString::fromLatin1(m));
            QTest::qWait(300);
            return st4 ? st4->currentWidget() : nullptr;
        };

        // ---- 1. Edit Command, and C7's TWENTY-FIRST validator ------------
        // C11a drove the other twenty. This one is on a dialog, so it was out
        // of that step's scope; driving it here closes the set.
        printf("\n== EDIT COMMAND ==\n");
        QWidget *lang = page("&Language Parameters");
        QTreeWidget *cmds = lang ? lang->findChild<QTreeWidget *>("listviewCommands") : nullptr;
        QPushButton *editCmd = nullptr;
        if (lang) for (QPushButton *b : lang->findChildren<QPushButton *>())
            if (b->text().contains("Edit")) { editCmd = b; break; }
        if (!cmds || !editCmd) { printf("  !! commands list or Edit button missing\n"); return 1; }
        // MOVE is the one command the experiment gives a duration of its own.
        int moveRow = -1;
        for (int i = 0; i < cmds->topLevelItemCount(); ++i)
            if (cmds->topLevelItem(i)->text(1) == "MOVE") { moveRow = i; break; }
        printf("  rows=%d MOVE at row %d duration=[%s]\n", cmds->topLevelItemCount(),
               moveRow, moveRow < 0 ? "?" : qPrintable(cmds->topLevelItem(moveRow)->text(2)));
        if (moveRow >= 0) {
            cmds->setCurrentItem(cmds->topLevelItem(moveRow));
            cmds->topLevelItem(moveRow)->setSelected(true);
        }
        whenModal([](QWidget *m) {
            describeDialog(m);
            for (QRadioButton *rb : m->findChildren<QRadioButton *>())
                printf("    radio  [%s] checked=%d\n", qPrintable(rb->text()),
                       rb->isChecked() ? 1 : 0);
            for (QGroupBox *g : m->findChildren<QGroupBox *>())
                printf("    group  [%s]\n", qPrintable(g->title()));
            QLineEdit *dur = m->findChild<QLineEdit *>();
            if (dur) {
                // THE CONSEQUENCE OF THE SELECTION, which is the thing that
                // matters and not the highlight. 1.3 pre-fills without
                // selecting, so a typed digit APPENDS: 0.01 becomes 0.015.
                // Qt 6 selects on the dialog's initial focus, which would
                // REPLACE and give 5. Type one character, changing nothing
                // else, and read it back.
                const QString pre = dur->text();
                QTest::keyClicks(dur, QStringLiteral("5"));
                QTest::qWait(20);
                printf("    prefill=[%s] selected=[%s]; typing \"5\" gives [%s]"
                       "  (1.3 appends -> 0.015)\n",
                       qPrintable(pre), qPrintable(dur->selectedText()),
                       qPrintable(dur->text()));
                dur->setText(pre);
                printf("    validator %s\n", qPrintable(validatorDesc(dur->validator())));
                printf("    -- C7's 21st validator, the one C11a could not reach\n");
                batteryDouble(dur);
            }
            // Radio exclusivity: Qt 2 grouped these with a QButtonGroup widget,
            // Qt 6 with a QGroupBox parent. Clicking one must clear the other.
            QRadioButton *allow = nullptr, *disallow = nullptr;
            for (QRadioButton *rb : m->findChildren<QRadioButton *>()) {
                if (rb->text() == "Allow") allow = rb;
                if (rb->text() == "Disallow") disallow = rb;
            }
            if (allow && disallow) {
                QTest::mouseClick(disallow, Qt::LeftButton, Qt::NoModifier,
                                  QPoint(8, disallow->height() / 2));
                QTest::qWait(40);
                printf("    after clicking Disallow: Allow=%d Disallow=%d\n",
                       allow->isChecked() ? 1 : 0, disallow->isChecked() ? 1 : 0);
                QTest::mouseClick(allow, Qt::LeftButton, Qt::NoModifier,
                                  QPoint(8, allow->height() / 2));
                QTest::qWait(40);
                printf("    after clicking Allow:    Allow=%d Disallow=%d\n",
                       allow->isChecked() ? 1 : 0, disallow->isChecked() ? 1 : 0);
            }
            clickDlgButton(m, "Cancel");
        });
        QTest::mouseClick(editCmd, Qt::LeftButton, Qt::NoModifier, editCmd->rect().center());
        QTest::qWait(1500);
        printf("  after Cancel, MOVE duration=[%s]\n",
               moveRow < 0 ? "?" : qPrintable(cmds->topLevelItem(moveRow)->text(2)));

        // ---- 1b. ALLOWING A DISALLOWED COMMAND, and where it lands -------
        // The experiment allows 13 of the 15; JMP and NOP are the two it does
        // not. Allowing one through this dialog calls addCommand(), which
        // APPENDS to the ordered QList Phase D put in. Qt 2 inserted into a
        // QDict, so 1.3 puts it at its HASH position instead -- which for JMP
        // is straight after LOAD, not at the end.
        //
        // Same family as C11b's constructor order, but NOT the same decision:
        // there the port had a free choice of a static order and 1.3's was
        // reproducible for nothing, so it was fixed. Here the insertion POINT
        // depends on runtime hashing, so matching it would mean reimplementing
        // Q2Dict -- which is what Phase D deliberately removed. Measured and
        // recorded rather than fixed; see PORTING.md, C11c.
        printf("\n== ALLOW A DISALLOWED COMMAND (JMP) ==\n");
        int jmpRow = -1;
        for (int i = 0; i < cmds->topLevelItemCount(); ++i)
            if (cmds->topLevelItem(i)->text(1) == "JMP") { jmpRow = i; break; }
        if (jmpRow < 0) printf("  !! JMP not in the list\n");
        else {
            printf("  JMP at row %d duration=[%s] (0 means disallowed)\n", jmpRow,
                   qPrintable(cmds->topLevelItem(jmpRow)->text(2)));
            for (int i = 0; i < cmds->topLevelItemCount(); ++i)
                cmds->topLevelItem(i)->setSelected(i == jmpRow);
            cmds->setCurrentItem(cmds->topLevelItem(jmpRow));
            whenModal([](QWidget *m) {
                for (QRadioButton *rb : m->findChildren<QRadioButton *>())
                    printf("    as opened: [%s] checked=%d\n", qPrintable(rb->text()),
                           rb->isChecked() ? 1 : 0);
                QRadioButton *allow = nullptr;
                for (QRadioButton *rb : m->findChildren<QRadioButton *>())
                    if (rb->text() == "Allow") allow = rb;
                if (allow) QTest::mouseClick(allow, Qt::LeftButton, Qt::NoModifier,
                                             QPoint(8, allow->height() / 2));
                QLineEdit *dur = m->findChild<QLineEdit *>();
                if (dur) { dur->setFocus(); dur->selectAll();
                           QTest::keyClick(dur, Qt::Key_Delete);
                           QTest::keyClicks(dur, QStringLiteral("0.007")); }
                QTest::qWait(60);
                printf("    typed duration [%s]\n", dur ? qPrintable(dur->text()) : "?");
                clickDlgButton(m, "OK");
            });
            QTest::mouseClick(editCmd, Qt::LeftButton, Qt::NoModifier, editCmd->rect().center());
            QTest::qWait(1500);
            printf("  JMP duration now [%s]\n",
                   qPrintable(cmds->topLevelItem(jmpRow)->text(2)));
            // The written order is the measurement, not the list widget, which
            // is always alphabetical because the form builds it that way.
            const QString out = exportTo("Language-Parameters",
                                         scratch() + "/c11c-lap", "lap");
            if (!out.isEmpty() && QFile::exists(out)) {
                QFile f(out);
                if (f.open(QIODevice::ReadOnly)) {
                    QStringList names;
                    QString header = QString::fromLatin1(f.readLine()).trimmed();
                    while (!f.atEnd())
                        names << QString::fromLatin1(f.readLine()).trimmed().section(' ', 0, 0);
                    f.close();
                    printf("  header  [%s]\n", qPrintable(header));
                    printf("  written [%s]\n", qPrintable(names.join(' ')));
                }
            }
        }

        // ---- 2. Edit Host ------------------------------------------------
        printf("\n== EDIT HOST ==\n");
        QWidget *gp = page("&GP Parameters");
        QTabWidget *gptabs = gp ? gp->findChild<QTabWidget *>() : nullptr;
        if (gptabs) {   // the PVM tab, by name rather than by index
            for (int i = 0; i < gptabs->count(); ++i)
                if (gptabs->tabText(i) == "PVM") {
                    QTabBar *bar = gptabs->tabBar();
                    QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier,
                                      bar->tabRect(i).center());
                    QTest::qWait(200);
                    break;
                }
        }
        QTreeWidget *hosts = gp ? gp->findChild<QTreeWidget *>("listviewHosts") : nullptr;
        QPushButton *editHost = nullptr;
        if (gp) for (QPushButton *b : gp->findChildren<QPushButton *>())
            if (b->objectName() == "pushbuttonEdit") { editHost = b; break; }
        if (!hosts || !editHost) printf("  !! host list or Edit button missing\n");
        else {
            printf("  rows=%d row0=[%s|%s|%s]\n", hosts->topLevelItemCount(),
                   qPrintable(hosts->topLevelItem(0)->text(1)),
                   qPrintable(hosts->topLevelItem(0)->text(2)),
                   qPrintable(hosts->topLevelItem(0)->text(3)));
            hosts->setCurrentItem(hosts->topLevelItem(0));
            hosts->topLevelItem(0)->setSelected(true);
            whenModal([](QWidget *m) {
                describeDialog(m);
                for (QCheckBox *cb : m->findChildren<QCheckBox *>())
                    printf("    check  [%s] checked=%d\n", qPrintable(cb->text()),
                           cb->isChecked() ? 1 : 0);
                // The slave directory is the field the QTextStream >> char trap
                // emptied: every PVMHOST parsed from an .exp got an empty dir
                // until it was fixed. This is where a regression would show.
                for (QLineEdit *le : m->findChildren<QLineEdit *>())
                    printf("    edit   [%s] text=[%s] empty=%d\n",
                           qPrintable(le->objectName()), qPrintable(le->text()),
                           le->text().isEmpty() ? 1 : 0);
                clickDlgButton(m, "Cancel");
            });
            QTest::mouseClick(editHost, Qt::LeftButton, Qt::NoModifier,
                              editHost->rect().center());
            QTest::qWait(1500);
        }

        // ---- 3. Help > About, the InfoBox --------------------------------
        // SIG_TextView runs a 100 ms auto-scroll timer, so the scroll position
        // is NOT dumped -- it would make this scenario unbaselineable. The
        // text and the geometry are what is compared.
        printf("\n== ABOUT / INFOBOX ==\n");
        whenModal([](QWidget *m) {
            printf("  [dialog] class=%s title=[%s] modal=%d\n",
                   m->metaObject()->className(), qPrintable(m->windowTitle()),
                   m->isModal() ? 1 : 0);
            for (QLabel *l : m->findChildren<QLabel *>())
                printf("    label pixmap=%d size=%dx%d\n", l->pixmap().isNull() ? 0 : 1,
                       l->pixmap().width(), l->pixmap().height());
            for (QTextBrowser *tb : m->findChildren<QTextBrowser *>()) {
                const QStringList lines = tb->toPlainText().split('\n');
                printf("    text lines=%d vScrollPolicy=%d\n", (int)lines.count(),
                       (int)tb->verticalScrollBarPolicy());
                for (const QString &l : lines) printf("    | %s\n", qPrintable(l));
            }
            for (QPushButton *b : m->findChildren<QPushButton *>())
                printf("    button [%s] default=%d\n", qPrintable(b->text()), b->isDefault());
            m->close();
        }, 6000);
        clickMenu("&Help", "About");   // no accelerator on this one
        QTest::qWait(2000);

        // ---- 4. Robot Info -----------------------------------------------
        // Not a SIG_TextView: slotRobotInfo builds a text blob with
        // getRobotInformation() and shows it in a QMessageBox. The blob is
        // generated from the robot, so it is byte-comparable across machines.
        printf("\n== ROBOT INFO ==\n");
        QWidget *rob = page("&Robot");
        QPushButton *info = nullptr;
        if (rob) for (QPushButton *b : rob->findChildren<QPushButton *>())
            if (b->objectName() == "pushbuttonRobInfo") { info = b; break; }
        if (!info) printf("  !! Robot Info button missing\n");
        else {
            whenModal([](QWidget *m) {
                QMessageBox *mb = qobject_cast<QMessageBox *>(m);
                printf("  [dialog] class=%s title=[%s]\n", m->metaObject()->className(),
                       qPrintable(m->windowTitle()));
                if (mb) {
                    const QStringList lines = mb->text().split('\n');
                    printf("  text lines=%d\n", (int)lines.count());
                    for (const QString &l : lines) printf("  | %s\n", qPrintable(l));
                    for (QAbstractButton *b : mb->buttons())
                        printf("  button [%s] default=%d\n", qPrintable(b->text()),
                               b == mb->defaultButton());
                }
                m->close();
            }, 6000);
            QTest::mouseClick(info, Qt::LeftButton, Qt::NoModifier, info->rect().center());
            QTest::qWait(2000);
        }

        // ---- 5. IndividualView, which is NOT modal ------------------------
        printf("\n== INDIVIDUAL VIEW (double-click a population row) ==\n");
        clickMenu("&View", "&Population");
        QTest::qWait(300);
        QTreeWidget *t5 = indList();
        if (!t5) printf("  !! no individuals list\n");
        else {
            QTreeWidgetItem *row0 = t5->topLevelItem(0);
            t5->scrollToItem(row0);
            const QRect r = t5->visualItemRect(row0);
            printf("  double-clicking row0 = %s at (%d,%d) itemAt=%s\n",
                   qPrintable(row0->text(0)), r.center().x(), r.center().y(),
                   t5->itemAt(r.center()) ? "the row" : "NOTHING -- probe is wrong");
            // Distinguish "the double click never reached the view" from "it
            // reached it and no window appeared". Without this the two look
            // identical, and C10 lost time to exactly that with the context
            // menus -- a right click that reached nothing read as five absent
            // menus rather than as a broken probe.
            QSignalSpy dbl(t5, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)));
            QTest::mouseClick(t5->viewport(), Qt::LeftButton, Qt::NoModifier, r.center());
            QTest::qWait(60);
            QTest::mouseDClick(t5->viewport(), Qt::LeftButton, Qt::NoModifier, r.center());
            QTest::qWait(1200);
            printf("  itemDoubleClicked emitted %d time(s)\n", (int)dbl.count());
            // show(), not exec(), so it is a top-level window rather than a modal.
            SIG_IndividualView *view = nullptr;
            for (QWidget *w : QApplication::topLevelWidgets())
                if (SIG_IndividualView *v = qobject_cast<SIG_IndividualView *>(w))
                    if (w != W) { view = v; break; }
            if (!view) printf("  !! no SIG_IndividualView appeared\n");
            else {
                printf("  [window] class=%s title=[%s] modal=%d visible=%d "
                       "deleteOnClose=%d\n", view->metaObject()->className(),
                       qPrintable(view->windowTitle()), view->isModal() ? 1 : 0,
                       view->isVisible() ? 1 : 0,
                       view->testAttribute(Qt::WA_DeleteOnClose) ? 1 : 0);
                printf("  name=[%s] age=[%s] fitness=[%s] historyLines=%d\n",
                       qPrintable(view->textlabelShowName->text()),
                       qPrintable(view->textlabelShowAge->text()),
                       qPrintable(view->textlabelShowFitness->text()),
                       (int)view->multilineeditHistory->toPlainText().split('\n').count());
                for (QPushButton *b : view->findChildren<QPushButton *>())
                    printf("  button [%s] default=%d\n", qPrintable(b->text()), b->isDefault());
                view->close();
                QTest::qWait(300);
                printf("  after close, main window still alive=%d\n", W->isVisible() ? 1 : 0);
            }
        }

        // ---- 6. Add Individuals, the Cancel path C10 never took ----------
        printf("\n== ADD INDIVIDUALS (Cancel) ==\n");
        const int before6 = t5 ? t5->topLevelItemCount() : -1;
        whenModal([](QWidget *m) {
            describeDialog(m);
            // NOT probeSpin(sp) -- its Return would click this dialog's
            // default button and accept it, so the Cancel below would land on a
            // hidden widget and test nothing.
            QSpinBox *sp = m->findChild<QSpinBox *>();
            // The same select-on-focus consequence as Edit Command, and the
            // costly one: 1.3 leaves the "1" unselected, so a user who types 2
            // adds TWELVE individuals. Measured on 1.3 as 12.
            if (QLineEdit *ed = sp ? sp->findChild<QLineEdit *>() : nullptr) {
                const QString pre = ed->text();
                QTest::keyClicks(sp, QStringLiteral("2"));
                QTest::qWait(20);
                printf("    prefill=[%s] selected=[%s]; typing \"2\" gives [%s] "
                       "value=%d  (1.3 appends -> 12)\n", qPrintable(pre),
                       qPrintable(ed->selectedText()), qPrintable(ed->text()),
                       sp->value());
                sp->setValue(1);
            }
            if (sp) probeSpin(sp, false);
            printf("    [still open before Cancel] visible=%d\n", m->isVisible());
            clickDlgButton(m, "Cancel");
        });
        clickMenu("&Individuals", "&Add");
        QTest::qWait(1500);
        printf("  rows %d -> %d (Cancel must add nothing)\n", before6,
               t5 ? t5->topLevelItemCount() : -1);
        fflush(stdout);
        return 0;
    }

    // --- the MetaGP window, MT_GUI ---------------------------------------
    // 23 sources and seven widgets that nothing has ever opened. Reached by
    // MetaGP > Use MetaGP, then MetaGP > Configure System, which builds an
    // MT_MainWindow ONCE and keeps it -- so a first open and a second open are
    // not the same code path. It is a QMainWindow with a QSplitter: an
    // MT_ExperimentWidget on the left and a QStackedWidget of six on the right.
    //
    // configureSystem() has two failure paths that only reach stderr,
    // "couldn't create the GP system" and "couldn't open the configuration
    // window". Either looks like nothing happening, so this reports what it
    // finds rather than assuming a window appeared.
    if (scenario == "metagui") {
        SIG_ExperimentListView *lv = listView();
        lv->setCurrentItem(lv->topLevelItem(0));
        QTest::qWait(300);
        printf("\n== METAGP: enabling ==\n");
        whenModal([](QWidget *m) {
            QMessageBox *mb = qobject_cast<QMessageBox *>(m);
            printf("  [modal on enable] %s [%s]\n", m->metaObject()->className(),
                   qPrintable(m->windowTitle()));
            if (mb) printf("    text=[%s]\n", qPrintable(mb->text()));
            m->close();
        }, 4000);
        clickMenu("&MetaGP", "&Use MetaGP");
        QTest::qWait(1500);
        for (QAction *a : W->menuBar()->actions())
            if (a->text() == "&MetaGP" && a->menu())
                for (QAction *b : a->menu()->actions())
                    if (!b->isSeparator())
                        printf("  [menu] %-24s enabled=%d checkable=%d checked=%d\n",
                               qPrintable(b->text()), b->isEnabled() ? 1 : 0,
                               b->isCheckable() ? 1 : 0, b->isChecked() ? 1 : 0);
        fflush(stdout);

        printf("\n== METAGP: Configure System ==\n");
        // MT_MainWindow itself comes back from activeModalWidget(), so a
        // handler that closes whatever it finds closes the window under test.
        // It did, and the run then reported "no MetaGP window appeared".
        // Only a QMessageBox is a failure here; anything else is left alone.
        whenModal([](QWidget *m) {
            printf("  [modal on configure] %s [%s]\n", m->metaObject()->className(),
                   qPrintable(m->windowTitle()));
            if (qobject_cast<QMessageBox *>(m)) { describeMessageBox(m); m->close(); }
            else printf("    (left open -- this is the window under test)\n");
            fflush(stdout);
        }, 4000);
        clickMenu("&MetaGP", "&Configure System");
        QTest::qWait(3000);
        cancelModalHandler();

        QWidget *mt = nullptr;
        for (QWidget *w : QApplication::topLevelWidgets())
            if (w != W && w->windowTitle() == "SIGEL MetaGP") mt = w;
        if (!mt) {
            printf("  !! no MetaGP window appeared. Top-level widgets now:\n");
            for (QWidget *w : QApplication::topLevelWidgets())
                if (w->isVisible())
                    printf("     %-34s [%s]\n", w->metaObject()->className(),
                           qPrintable(w->windowTitle()));
            fflush(stdout);
            return 0;
        }
        printf("  [window] class=%s title=[%s] %dx%d modal=%d\n",
               mt->metaObject()->className(), qPrintable(mt->windowTitle()),
               mt->width(), mt->height(), mt->isModal() ? 1 : 0);
        for (QToolBar *tb : mt->findChildren<QToolBar *>()) {
            printf("  [toolbar] %-22s title=[%s]\n", qPrintable(tb->objectName()),
                   qPrintable(tb->windowTitle()));
            for (QAction *a : tb->actions()) {
                if (a->isSeparator()) { printf("    ---\n"); continue; }
                printf("    action %-22s enabled=%d checkable=%d checked=%d "
                       "icon=%d sizes=%d\n", qPrintable(a->text()),
                       a->isEnabled() ? 1 : 0, a->isCheckable() ? 1 : 0,
                       a->isChecked() ? 1 : 0, a->icon().isNull() ? 0 : 1,
                       (int)a->icon().availableSizes().count());
            }
        }
        QStackedWidget *ws = mt->findChild<QStackedWidget *>("WidgetStack");
        if (!ws) { printf("  !! no WidgetStack\n"); return 1; }
        printf("  [stack] count=%d current=%d\n", ws->count(), ws->currentIndex());

        // The left-hand MT_ExperimentWidget's tree is what raises each page:
        // slotRaiseWidget(QTreeWidgetItem*) hangs off its currentItemChanged.
        // Clicking a row is therefore the real route, and it is the one both
        // sides can drive.
        // By name first, but the objectName uic gives it is not guaranteed to
        // be the member name; fall back to the one QTreeWidget that is NOT
        // inside the page stack, and SAY which route was taken.
        QTreeWidget *nav = mt->findChild<QTreeWidget *>("MTExperimentListView");
        if (!nav) {
            for (QTreeWidget *tw : mt->findChildren<QTreeWidget *>())
                if (!ws->isAncestorOf(tw)) { nav = tw; break; }
            printf("  [nav] not found by name; using the tree outside the stack: [%s]\n",
                   nav ? qPrintable(nav->objectName()) : "(none)");
        }
        if (!nav) {
            printf("  !! no navigation tree. Trees under the window:\n");
            for (QTreeWidget *tw : mt->findChildren<QTreeWidget *>())
                printf("     [%s] rows=%d inStack=%d\n", qPrintable(tw->objectName()),
                       tw->topLevelItemCount(), ws->isAncestorOf(tw) ? 1 : 0);
            fflush(stdout);
            return 1;
        }
        printf("  [nav] rows=%d\n", nav->topLevelItemCount());
        for (int i = 0; i < nav->topLevelItemCount(); ++i)
            printf("    row%-2d [%s]\n", i, qPrintable(nav->topLevelItem(i)->text(0)));
        fflush(stdout);

        for (int i = 0; i < nav->topLevelItemCount(); ++i) {
            QTreeWidgetItem *it = nav->topLevelItem(i);
            nav->scrollToItem(it);
            QTest::mouseClick(nav->viewport(), Qt::LeftButton, Qt::NoModifier,
                              nav->visualItemRect(it).center());
            QTest::qWait(400);
            QWidget *pg = ws->currentWidget();
            printf("\n== METAGP PAGE %d: clicked [%s] ==\n", i, qPrintable(it->text(0)));
            printf("  [page] index=%d class=%s objectName=[%s]\n", ws->currentIndex(),
                   pg ? pg->metaObject()->className() : "(none)",
                   pg ? qPrintable(pg->objectName()) : "");
            if (!pg) continue;
            printf("  [survey]\n");  dumpContainer(pg);
            printf("  [drive]\n");   driveContainer(pg, pg);
            // THE GROUP-SEPARATOR PROBE, which needs no foreign locale to show
            // the defect. MT_GUI's validators were never given C7's treatment,
            // so they carry QLocale::system(); en_US's GROUP separator is ','
            // and the read-back is toInt(), which is locale-independent. So a
            // typed "1,000" is ACCEPTED as one thousand by the validator and
            // read back as ONE. C7 pinned the master GUI's 21 validators to
            // QLocale::c() with RejectGroupSeparator for exactly this.
            for (QLineEdit *le : pg->findChildren<QLineEdit *>()) {
                if (!le->validator()) continue;
                if (qobject_cast<QAbstractSpinBox *>(le->parentWidget())) continue;
                const QString before = le->text();
                clearEdit(le);
                QTest::keyClicks(le, QStringLiteral("1,000"));
                QTest::qWait(10);
                bool ok = false;
                const int asInt = le->text().toInt(&ok);
                printf("    group  %-30s type[1,000] -> [%s] acceptable=%d "
                       "toInt=%d(ok=%d)\n", qPrintable(le->objectName()),
                       qPrintable(le->text()), le->hasAcceptableInput() ? 1 : 0,
                       asInt, ok ? 1 : 0);
                le->setText(before);
                break;   // one per page is enough; they share a validator
            }
            fflush(stdout);
        }

        // Close it the way a user does. closeEvent() asks the top widget's
        // onHide() whether it may close and IGNORES the event if not, so a
        // refusal is a real outcome and is reported rather than assumed.
        printf("\n== METAGP: closing ==\n");
        mt->close();
        QTest::qWait(600);
        bool still = false;
        for (QWidget *w : QApplication::topLevelWidgets())
            if (w->windowTitle() == "SIGEL MetaGP" && w->isVisible()) still = true;
        printf("  closed=%d (onHide() may refuse) mainWindowAlive=%d\n",
               still ? 0 : 1, W->isVisible() ? 1 : 0);
        fflush(stdout);
        return 0;
    }

    // --- MetaGP: the things metagui SURVEYED but never CLICKED -------------
    // §9's undriven list: MT_Editor, MT_AddConstantsWidget, `update
    // statistics' and MT_GUI's toolbar actions. metagui prints their labels
    // and enabled state; nothing has ever pressed one, on either version.
    //
    // This is about CORE FUNCTIONALITY, not appearance: can a user still edit
    // a constant, still create constants, still reach the toolbars -- and does
    // each still do what 1.3 did. So every assertion below is a BOOLEAN or a
    // COUNT, never a value: the ~30 constants are generated by a randomizer at
    // construction and differ every run, and pinning them would pin noise (two
    // metagui runs differ by exactly those lines). "changed=1" is stable;
    // "changed 512 -> 77" is not.
    if (scenario == "metadrive") {
        SIG_ExperimentListView *lv = listView();
        lv->setCurrentItem(lv->topLevelItem(0));
        QTest::qWait(300);

        // NOTE: enabling MetaGP raises NOTHING. The information box with three
        // custom buttons is on DISABLING (MT_Controller.cpp:255-270, the
        // state == false branch), which is why the baseline shows no
        // QMessageBox here and this handler never fires. The "Save && Remove"
        // ampersand fix documented at that site is therefore UNDRIVEN. The
        // handler is kept because it costs nothing and a box appearing here
        // would otherwise block the scenario.
        whenModal([](QWidget *m) { if (qobject_cast<QMessageBox *>(m)) m->close(); },
                  4000);
        clickMenu("&MetaGP", "&Use MetaGP");
        QTest::qWait(1500);
        // ONLY A QMessageBox IS A FAILURE HERE. MT_MainWindow is itself modal
        // and comes back from activeModalWidget(), so a handler that closes
        // whatever it finds CLOSES THE WINDOW UNDER TEST -- metagui documents
        // this trap 150 lines above and the first version of this scenario
        // walked straight into it anyway. It cost more than a wasted run:
        // with the window closed, `closed=1' could not fail, the statistics
        // section's "no modal appeared" was an artefact, and the focus-out
        // probe's flakiness was blamed on the offscreen platform not mapping
        // the window when the real cause was this handler closing it.
        whenModal([](QWidget *m) {
            if (qobject_cast<QMessageBox *>(m)) { describeMessageBox(m); m->close(); }
        }, 4000);
        clickMenu("&MetaGP", "&Configure System");
        QTest::qWait(2500);

        QWidget *mt = nullptr;
        for (QWidget *w : QApplication::topLevelWidgets())
            if (w->windowTitle() == "SIGEL MetaGP") { mt = w; break; }
        if (!mt) { printf("\n!! no MetaGP window appeared\n"); return 1; }
        // The control for the fix above: if this reads 0 the handler has
        // closed the window again and everything below is measuring a corpse.
        printf("  [window] visible=%d hidden=%d\n",
               mt->isVisible() ? 1 : 0, mt->isHidden() ? 1 : 0);

        QStackedWidget *ws = mt->findChild<QStackedWidget *>("WidgetStack");
        QTreeWidget *nav = nullptr;
        for (QTreeWidget *tw : mt->findChildren<QTreeWidget *>())
            if (ws && !ws->isAncestorOf(tw)) { nav = tw; break; }
        if (!ws || !nav) { printf("\n!! no stack or nav tree\n"); return 1; }

        auto raisePage = [&](const char *label) -> QWidget * {
            for (int i = 0; i < nav->topLevelItemCount(); ++i) {
                QTreeWidgetItem *it = nav->topLevelItem(i);
                if (it->text(0) != QString::fromLatin1(label)) continue;
                nav->setCurrentItem(it);
                QTest::qWait(500);
                return ws->currentWidget();
            }
            printf("  !! no page labelled %s\n", label);
            return nullptr;
        };

        // ---- 1. MT_Editor, the inline constant editor -------------------
        // The whole contract is three outcomes: Return COMMITS, Escape
        // DISCARDS, focus-out DISCARDS. MT_Editor::hideEvent emits newText
        // only when `oldText != text() && acceptChange', and acceptChange is
        // set by Return alone -- so all three ride on one flag.
        printf("\n== MT_EDITOR (inline constant editor) ==\n");
        QWidget *indPage = raisePage("Individual");
        QListWidget *consts = indPage
            ? indPage->findChild<QListWidget *>("constantsListBox") : nullptr;
        if (!consts || consts->count() == 0) {
            printf("  !! no constantsListBox, or it is empty\n");
        } else {
            printf("  constants=%d (values are randomiser output and are NOT pinned)\n",
                   consts->count());
            MT_Editor *ed = indPage->findChild<MT_Editor *>();
            printf("  editor found=%d hiddenBefore=%d\n", ed ? 1 : 0,
                   ed && ed->isHidden() ? 1 : 0);

            // itemActivated is what the list wires to slotEditConst, and it is
            // what a double-click emits. Drive the row, not the slot.
            consts->setCurrentRow(0);
            QTest::qWait(80);
            const QString orig = consts->item(0)->text();

            // Diagnostics before the verdict: "the editor did not open" and
            // "the editor opened and something hid it again" and "the list is
            // not visible so nothing can be" all look identical from
            // isVisible() alone.
            // THE PREDICATE IS isHidden(), NOT isVisible(). The reason first
            // recorded here was WRONG and is corrected: it said the offscreen
            // platform never maps the MetaGP window, so isVisible() is false
            // for everything inside it. The window IS mapped -- instrumenting
            // it now finds 43 of 377 descendants visible -- and the readings
            // that produced that claim came from a run in which this
            // scenario's own modal handler had CLOSED the window.
            //
            // isHidden() is still the right predicate, on its own merits: it
            // is exactly what MT_Editor's show()/hide() pair sets, so it
            // answers what this probe asks -- did the code show the editor,
            // and did Return/Escape hide it again -- without depending on
            // whether an ancestor happens to be mapped.
            auto editorOpen = [&]() { return ed && !ed->isHidden(); };
            auto openEditor = [&]() -> bool {
                emit consts->itemActivated(consts->item(0));
                QTest::qWait(250);
                return editorOpen();
            };

            // (a) Return must COMMIT.
            bool opened = openEditor();
            printf("  [open] editorVisible=%d\n", opened ? 1 : 0);
            if (opened) {
                ed->selectAll();
                QTest::keyClick(ed, Qt::Key_Delete);
                QTest::keyClicks(ed, QStringLiteral("4242"));
                QTest::keyClick(ed, Qt::Key_Return);
                QTest::qWait(300);
                const QString after = consts->item(0)->text();
                printf("  [Return]  committed=%d  editorHidden=%d\n",
                       after == QStringLiteral("4242") ? 1 : 0,
                       ed->isHidden() ? 1 : 0);

                // (b) Escape must DISCARD. The positive control for this is
                // (a) above: the same open-and-type sequence demonstrably
                // changes the entry, so an unchanged entry here is Escape
                // working and not the keystrokes going nowhere.
                const QString before2 = consts->item(0)->text();
                if (openEditor()) {
                    ed->selectAll();
                    QTest::keyClick(ed, Qt::Key_Delete);
                    QTest::keyClicks(ed, QStringLiteral("9999"));
                    QTest::keyClick(ed, Qt::Key_Escape);
                    QTest::qWait(300);
                    printf("  [Escape]  discarded=%d  editorHidden=%d\n",
                           consts->item(0)->text() == before2 ? 1 : 0,
                           ed->isHidden() ? 1 : 0);
                } else printf("  !! editor did not reopen for the Escape probe\n");

                // (c) focus-out must DISCARD too -- MT_Editor::focusOutEvent
                // hides without setting acceptChange.
                //
                // This WAS reported as undrivable, blamed on the offscreen
                // platform not mapping the MetaGP window. That diagnosis was
                // wrong: the window was mapped, and this scenario's own modal
                // handler had CLOSED it. With the handler fixed the probe is
                // stable. hadFocus is its control -- with no focus there is no
                // focus-out to lose, and the probe says so rather than
                // reporting a pass.
                // The style hint that decides whether a SINGLE click opens the
                // editor. 1.3 reaches slotEditConst through Qt 2's
                // QListBox::selected, emitted from mouseDoubleClickEvent and
                // from Return/Enter; the port reaches it through
                // itemActivated, which is that same pair ONLY while this reads
                // 0. It is pinned because a style saying 1 would silently make
                // one click open the editor where 1.3 needs two. *This line
                // was deleted by an earlier commit while PORTING.md went on
                // claiming it was pinned -- a coverage removal, not a wording
                // slip.*
                printf("  [opener] activateOnSingleClick=%d\n",
                       QApplication::style()->styleHint(
                           QStyle::SH_ItemView_ActivateItemOnSingleClick) ? 1 : 0);
                const QString before3 = consts->item(0)->text();
                if (openEditor()) {
                    ed->selectAll();
                    QTest::keyClick(ed, Qt::Key_Delete);
                    QTest::keyClicks(ed, QStringLiteral("7777"));
                    const bool hadFocus = ed->hasFocus();
                    consts->setFocus();
                    QTest::qWait(300);
                    if (!hadFocus)
                        printf("  [focusOut] INCONCLUSIVE: the editor never held focus\n");
                    else
                        printf("  [focusOut] discarded=%d  editorHidden=%d\n",
                               consts->item(0)->text() == before3 ? 1 : 0,
                               ed->isHidden() ? 1 : 0);
                    if (!ed->isHidden()) { QTest::keyClick(ed, Qt::Key_Escape);
                                           QTest::qWait(150); }
                } else printf("  !! editor did not reopen for the focus probe\n");
            }
            printf("  (row 0 started as a randomiser value; only the BOOLEANS above\n"
                   "   are pinned, because the value differs every run)\n");
            (void)orig;
        }
        fflush(stdout);

        // ---- 2. MT_AddConstantsWidget, and the swapped float bounds ------
        // The 2003 source builds the float validators with bottom ABOVE top --
        // QDoubleValidator(100000.0, -100000.0, 4, ...) -- and REBUILDS them at
        // +/-10000.0 the first time the type radio is clicked. Both are 1.3's
        // own code, confirmed against the pristine tarball, so both are
        // behaviour to preserve rather than bugs to fix. The consequence is
        // that switching float -> int -> float NARROWS what the field accepts
        // tenfold, and that is worth measuring on both versions.
        //
        // Typed one character at a time, as C11a's battery is: a validator
        // that answers Invalid makes QLineEdit drop that keystroke, so the
        // surviving text carries a gap exactly where the rejection happened.
        printf("\n== MT_ADDCONSTANTSWIDGET (Create) ==\n");
        {
            const int before = consts ? consts->count() : -1;
            auto probeField = [](QWidget *dlg, const char *name, const char *val) {
                QLineEdit *le = dlg->findChild<QLineEdit *>(name);
                if (!le) { printf("    !! no %s\n", name); return; }
                le->setFocus(); le->selectAll();
                QTest::keyClick(le, Qt::Key_Delete);
                QTest::keyClicks(le, QString::fromLatin1(val));
                QTest::qWait(20);
                const QValidator *v = le->validator();
                printf("    %-14s typed[%-7s] -> [%s] acceptable=%d validator=%s\n",
                       name, val, qPrintable(le->text()),
                       le->hasAcceptableInput() ? 1 : 0,
                       v ? v->metaObject()->className() : "(none)");
            };
            whenModal([&](QWidget *m) {
                printf("  [modal] %s [%s]\n", m->metaObject()->className(),
                       qPrintable(m->windowTitle()));
                QRadioButton *ri = m->findChild<QRadioButton *>("intRadioButton");
                QRadioButton *rf = m->findChild<QRadioButton *>("floatRadioButton");
                printf("  on open: intChecked=%d floatChecked=%d intDown=%d floatDown=%d\n",
                       ri && ri->isChecked() ? 1 : 0, rf && rf->isChecked() ? 1 : 0,
                       ri && ri->isDown() ? 1 : 0, rf && rf->isDown() ? 1 : 0);
                // AS OPENED is the case that matters: the dialog opens in float
                // mode, and 2003 built THIS validator with the bounds swapped,
                // QDoubleValidator(100000.0, -100000.0, 4). A first version of
                // this probe typed only "50000" here and typed "-50000" only
                // after the radio had rebuilt the validator -- so it missed
                // that Qt 6 drops a leading minus outright when bottom >= 0,
                // turning -50000 into 50000. A SIGN FLIP, in the default mode,
                // on the value that becomes the generated constants.
                printf("  as opened (float, 2003's swapped bounds):\n");
                probeField(m, "minValueEdit", "50000");
                probeField(m, "minValueEdit", "-50000");
                probeField(m, "minValueEdit", "1.23456");
                probeField(m, "minValueEdit", "9.87654321");
                probeField(m, "minValueEdit", "-0.0001");
                // Qt 2 enforces WELL-FORMEDNESS ONLY here -- one leading minus,
                // one decimal point -- and no constraint on magnitude or on the
                // number of decimals. These three are the oracle's control that
                // the validator is doing something in this state, measured on
                // 1.3: abc -> empty, 1.2.3 -> 1.23, --5 -> -5.
                probeField(m, "minValueEdit", "abc");
                probeField(m, "minValueEdit", "1.2.3");
                probeField(m, "minValueEdit", "--5");
                probeField(m, "maxValueEdit", "50000");
                probeField(m, "maxValueEdit", "-50000");
                // Switch to integer and back to float; the second click is what
                // deletes and rebuilds the validators at the tenfold-tighter
                // range. Clicking the radio, not calling the slot.
                if (ri) { QTest::mouseClick(ri, Qt::LeftButton); QTest::qWait(150); }
                printf("  after clicking integer:\n");
                probeField(m, "minValueEdit", "50000");
                if (rf) { QTest::mouseClick(rf, Qt::LeftButton); QTest::qWait(150); }
                printf("  after clicking float again:\n");
                probeField(m, "minValueEdit", "50000");
                probeField(m, "maxValueEdit", "50000");
                // WHAT THE ORACLE MEASURED ON 1.3, so it can be compared
                // rather than reasoned about. Its results, from the running
                // binary: 50000 and -50000 survive in BOTH modes and at every
                // point (no range narrowing at all -- the prediction that
                // switching float->int->float tightens the range tenfold is
                // FALSIFIED); letters are rejected in both modes, leaving the
                // field empty; and "12.5" stays 12.5 in float but becomes
                // "125" in integer, the point stripped. Those last two are the
                // control that Qt 2's validator is live and mode-sensitive
                // rather than absent.
                printf("  the three probes the oracle ran on 1.3:\n");
                probeField(m, "minValueEdit", "-50000");
                probeField(m, "minValueEdit", "abc");
                probeField(m, "minValueEdit", "12.5");
                if (ri) { QTest::mouseClick(ri, Qt::LeftButton); QTest::qWait(150); }
                printf("  the same three in INTEGER mode:\n");
                probeField(m, "minValueEdit", "-50000");
                probeField(m, "minValueEdit", "abc");
                probeField(m, "minValueEdit", "12.5");
                if (rf) { QTest::mouseClick(rf, Qt::LeftButton); QTest::qWait(150); }
                // MIN = MAX = -50000 IN INTEGER MODE, which is what the oracle
                // ran on 1.3. Setting them equal makes the generator
                // DEGENERATE: every constant it produces must be exactly that
                // value, so the constants themselves become checkable instead
                // of being randomiser noise. Without this the scenario pinned
                // only the COUNT, and replacing boss->minValue/maxValue with 0
                // in accept() -- the typed bounds never reaching the generator
                // at all -- left the output byte-identical. The bounds are the
                // whole point of this dialog and they were ungated.
                if (ri) { QTest::mouseClick(ri, Qt::LeftButton); QTest::qWait(150); }
                for (const char *n : { "minValueEdit", "maxValueEdit" })
                    if (QLineEdit *le = m->findChild<QLineEdit *>(n)) {
                        le->setFocus(); le->selectAll();
                        QTest::keyClick(le, Qt::Key_Delete);
                        QTest::keyClicks(le, QStringLiteral("-50000"));
                        printf("  %s committed as [%s]\n", n, qPrintable(le->text()));
                    }
                if (QSpinBox *sp = m->findChild<QSpinBox *>("numConstantsSpinBox")) {
                    sp->setFocus(); sp->selectAll();
                    QTest::keyClick(sp, Qt::Key_Delete); QTest::keyClicks(sp, "3");
                    printf("  numConstants set to %d\n", sp->value());
                }
                if (QDialog *d = qobject_cast<QDialog *>(m)) d->accept();
            }, 8000);
            if (QPushButton *create = indPage
                    ? indPage->findChild<QPushButton *>("createButton") : nullptr) {
                QTest::mouseClick(create, Qt::LeftButton);
            } else {
                // The button's objectName is not guaranteed; fall back to text
                // and SAY which route was taken rather than failing silently.
                QPushButton *hit = nullptr;
                if (indPage)
                    for (QPushButton *b : indPage->findChildren<QPushButton *>())
                        if (b->text().contains("reate")) { hit = b; break; }
                printf("  [create] not found by objectName; by text=%d\n", hit ? 1 : 0);
                if (hit) QTest::mouseClick(hit, Qt::LeftButton);
                else printf("  !! no Create button on the Individual page\n");
            }
            QTest::qWait(3000);
            const int after = consts ? consts->count() : -1;
            printf("  constants %d -> %d  delta=%d (3 asked for)\n",
                   before, after, after - before);
            // The generated values, which the degenerate bounds make
            // deterministic. 1.3, measured: three constants of -50000.
            int matched = 0;
            QStringList tail;
            for (int i = before; i >= 0 && i < after; ++i) {
                tail << consts->item(i)->text();
                if (consts->item(i)->text() == QStringLiteral("-50000")) ++matched;
            }
            printf("  new constants=[%s]  allAreMinusFiftyThousand=%d\n",
                   qPrintable(tail.join(",")), matched == after - before ? 1 : 0);
        }
        fflush(stdout);

        // ---- 3. `update statistics' --------------------------------------
        // One toolbar action on the Statistics page, unclicked on either
        // version. With no evolution run, 1.3 does nothing visible either --
        // measured by the oracle at 0 changed pixels, against a control (a real
        // tab switch on that page moves 34,188) proving its diff could see a
        // change if there were one.
        //
        // WHAT THIS SECTION CAN AND CANNOT SEE, stated because the difference
        // is easy to miss. It pins that the action EXISTS, is ENABLED, can be
        // TRIGGERED without raising a message box or changing the page, and
        // that the three Fitness fields read ERR exactly as 1.3's do. It
        // CANNOT tell whether the slot behind the action ran: gutting
        // MT_StatisticsWidget::slotUpdateGUI() to `return;' leaves this output
        // byte-identical. The ERR values are the .ui's own static text and the
        // two fields that are not ERR are written by onShow() at page-raise,
        // not by this action. So this is an agreement about STATE, matching
        // 1.3's own "nothing happens", and not a test of the slot.
        printf("\n== UPDATE STATISTICS ==\n");
        if (QWidget *stat = raisePage("Statistics")) {
            QAction *upd = nullptr;
            for (QAction *a : mt->findChildren<QAction *>())
                if (a->text().contains("update statistics")) { upd = a; break; }
            printf("  action found=%d enabled=%d\n", upd ? 1 : 0,
                   upd && upd->isEnabled() ? 1 : 0);
            const int widgetsBefore = stat->findChildren<QWidget *>().count();
            bool modalAppeared = false;
            // Only a QMessageBox counts. MT_MainWindow is modal and comes back
            // from activeModalWidget(), so counting "a modal appeared" without
            // this reported 1 on every run and measured nothing.
            whenModal([&](QWidget *m) {
                if (!qobject_cast<QMessageBox *>(m)) return;
                modalAppeared = true;
                printf("  [modal] %s [%s]\n", m->metaObject()->className(),
                       qPrintable(m->windowTitle()));
                m->close();
            }, 3000);
            if (upd && upd->isEnabled()) upd->trigger();
            QTest::qWait(1500);
            printf("  triggered=%d modalAppeared=%d widgets %d -> %d\n",
                   upd && upd->isEnabled() ? 1 : 0, modalAppeared ? 1 : 0,
                   widgetsBefore, stat->findChildren<QWidget *>().count());
            // The oracle measured 1.3 showing `ERR' in the Fitness fields with
            // no evolution run, and four
            //   QRangeControl::setRange: minValue 1 > maxValue 0
            // lines on stderr per click -- the same inverted-bounds flavour as
            // the AddConstants validator. Qt 6 has no QRangeControl so that
            // exact message cannot appear, but whatever the fields DISPLAY is
            // comparable, so it is printed.
            // Only the VALUE labels: Designer names its static captions
            // TextLabel<n>, and dumping all 52 buried the three that matter in
            // thirty captions reading "Mutations", "Crossovers" and so on.
            QStringList shown;
            int err = 0;
            for (QLabel *l : stat->findChildren<QLabel *>()) {
                if (l->objectName().startsWith("TextLabel")) continue;
                if (l->objectName().isEmpty() || l->text().isEmpty()) continue;
                if (l->text() == "ERR") ++err;
                shown << l->objectName() + "=" + l->text();
            }
            shown.sort();
            printf("  value labels=%d readingERR=%d"
                   "  (1.3 shows ERR here too; these are .ui defaults plus\n"
                   "   two fields onShow() writes -- NOT evidence the action ran)\n",
                   (int)shown.count(), err);
            for (const QString &t : shown) printf("    %s\n", qPrintable(t));
        }
        fflush(stdout);

        // ---- 4. The MetaGP toolbar actions ------------------------------
        // metagui prints these; nothing has pressed one. Start and Stop are
        // disabled on arrival and a running MetaGP evolution is a separate
        // item, so what is drivable here is the enabled ones. `Default' is the
        // interesting one -- it resets the parameter pages -- and Open/Save
        // open file dialogs, which are driven through acceptFileDialog like
        // every other file path in this driver.
        printf("\n== METAGP TOOLBAR ACTIONS ==\n");
        {
            // Keyed by text AND objectName: keying by text alone collapsed
            // every same-named action into one entry and printed a blank row
            // for an action whose text is empty, with nothing to identify it.
            // A LIST, not a QMap: two of these widgets are unnamed QSpinBoxes
            // (mtHour and mtMin) and a map keyed by their description silently
            // dropped one of the pair. Sorted so the order is stable.
            QStringList actLines;
            QMap<QString, QAction *> acts;
            for (QToolBar *tb : mt->findChildren<QToolBar *>())
                for (QAction *a : tb->actions()) {
                    if (a->isSeparator()) continue;
                    // MT_MainWindow puts real widgets on the evolution-control
                    // toolbar with addWidget() -- the status label and the
                    // hour/minute boxes -- and each of those is a QWidgetAction
                    // with NO text and NO objectName. Keyed by text they all
                    // collapsed into one blank row, hiding four of the five.
                    // They are reported by their widget instead, because they
                    // are visible UI a user reads during a run.
                    if (QWidgetAction *wa = qobject_cast<QWidgetAction *>(a)) {
                        QWidget *dw = wa->defaultWidget();
                        actLines << QString::asprintf(
                            "  (widget) %-13s obj=%-20s enabled=%d",
                            dw ? dw->metaObject()->className() : "(null)",
                            dw && !dw->objectName().isEmpty()
                                ? qPrintable(dw->objectName()) : "(none)",
                            a->isEnabled() ? 1 : 0);
                        continue;
                    }
                    acts.insert(a->text() + "\t" + a->objectName(), a);
                }
            for (auto it = acts.constBegin(); it != acts.constEnd(); ++it) {
                const QString txt = it.key().section('\t', 0, 0);
                printf("  %-22s enabled=%d checkable=%d checked=%d\n",
                       txt.isEmpty() ? "(no text)" : qPrintable(txt),
                       it.value()->isEnabled() ? 1 : 0,
                       it.value()->isCheckable() ? 1 : 0,
                       it.value()->isChecked() ? 1 : 0);
            }
            actLines.sort();
            for (const QString &l : actLines) printf("%s\n", qPrintable(l));
            printf("  widgetsOnToolbars=%d\n", (int)actLines.count());

            // `manual/timed stop' is a toggle and is the one action that is
            // safe to press twice and leave as it was.
            // The keys are "text\tobjectName" and every objectName here is
            // empty, so a lookup of the bare text found NOTHING and both press
            // blocks below were dead code -- the baseline carried neither
            // result line, which is how it went unnoticed.
            if (QAction *a = acts.value(QStringLiteral("manual/timed stop\t"))) {
                const bool was = a->isChecked();
                a->trigger(); QTest::qWait(200);
                const bool mid = a->isChecked();
                a->trigger(); QTest::qWait(200);
                printf("  [manual/timed stop] %d -> %d -> %d  toggles=%d restored=%d\n",
                       was ? 1 : 0, mid ? 1 : 0, a->isChecked() ? 1 : 0,
                       mid != was ? 1 : 0, a->isChecked() == was ? 1 : 0);
                // isChecked() alone proves NOTHING: Qt toggles a checkable
                // action whether or not anything is connected to it, so
                // deleting the connect left this line byte-identical. The
                // slot's real effect is that slotAutoStop(true) disables the
                // hour and minute boxes (MT_MainWindow.cpp:217-232), so their
                // enabled state is read across the toggle and that is what
                // gives the probe teeth.
                QList<QSpinBox *> tb;
                for (QToolBar *t2 : mt->findChildren<QToolBar *>())
                    for (QSpinBox *sp : t2->findChildren<QSpinBox *>()) tb << sp;
                QString st;
                a->trigger(); QTest::qWait(200);
                for (QSpinBox *sp : tb) st += sp->isEnabled() ? "1" : "0";
                a->trigger(); QTest::qWait(200);
                QString st2;
                for (QSpinBox *sp : tb) st2 += sp->isEnabled() ? "1" : "0";
                printf("    toolbar spin boxes=%d enabledWhileTimed=[%s] "
                       "enabledAfterRestore=[%s] slotHadEffect=%d\n",
                       (int)tb.count(), qPrintable(st), qPrintable(st2),
                       st != st2 ? 1 : 0);
            }

            // Default resets the MetaGP parameters. Its effect is read off the
            // Strategy page's own fields rather than asserted, because what it
            // resets them TO is 1.3's business and this side must not invent it.
            if (QAction *a = acts.value(QStringLiteral("&Default\t"))) {
                QWidget *strat = raisePage("Strategy");
                // PERTURB FIRST. Without this, "Default reset the values" and
                // "Default did nothing" are the same observation -- gutting
                // slotLoadDefault() to `return;' left the output
                // byte-identical. Moving the boxes off their defaults means a
                // working Default has to move them back.
                QStringList before;
                if (strat)
                    for (QSpinBox *sp : strat->findChildren<QSpinBox *>()) {
                        sp->setValue(sp->value() == sp->maximum() ? sp->minimum()
                                                                  : sp->maximum());
                        before << QString("%1=%2").arg(sp->objectName()).arg(sp->value());
                    }
                printf("  [&Default] perturbed to: %s\n",
                       qPrintable(before.join(" ")));
                bool modal = false;
                whenModal([&](QWidget *m) {
                    if (!qobject_cast<QMessageBox *>(m)) return;
                    modal = true;
                    printf("  [modal on Default] %s [%s]\n",
                           m->metaObject()->className(), qPrintable(m->windowTitle()));
                    m->close(); }, 3000);
                a->trigger();
                QTest::qWait(1200);
                QStringList after;
                if (strat)
                    for (QSpinBox *sp : strat->findChildren<QSpinBox *>())
                        after << QString("%1=%2").arg(sp->objectName()).arg(sp->value());
                printf("  [&Default] enabled=1 modal=%d strategySpinBoxes=%d changed=%d\n",
                       modal ? 1 : 0, after.count(), before != after ? 1 : 0);
                for (const QString &s : after) printf("    %s\n", qPrintable(s));
            }
        }
        fflush(stdout);

        // ---- 5. The Population page's six actions ------------------------
        // Enabled-state map plus the two that need no file dialog. Add and
        // Delete change the MetaGP population, which is what a user would do
        // first, and the count is the invariant.
        printf("\n== METAGP POPULATION ACTIONS ==\n");
        if (QWidget *pop = raisePage("Population")) {
            QListWidget *pl = pop->findChild<QListWidget *>();
            QTreeWidget *pt = pop->findChild<QTreeWidget *>();
            const int rowsBefore = pl ? pl->count()
                                      : (pt ? pt->topLevelItemCount() : -1);
            printf("  population rows=%d (list=%d tree=%d)\n", rowsBefore,
                   pl ? 1 : 0, pt ? 1 : 0);
            // The six population actions are NOT children of the page --
            // MT_PopulationWidget puts them on a toolbar owned by the MetaGP
            // window, so a findChildren on the page returns almost nothing.
            // That is why they appear in the toolbar map above; this section
            // reports only what the page itself owns, and says so.
            int own = 0;
            for (QAction *a : pop->findChildren<QAction *>())
                if (!a->isSeparator()) ++own;
            printf("  actions owned by the page itself=%d"
                   " (the six Add/Delete/Import/Export/Load/Save live on the\n"
                   "   window's toolbar and are listed in the toolbar map above)\n",
                   own);
        }
        fflush(stdout);

        printf("\n== METAGP: closing ==\n");
        mt->close();
        QTest::qWait(600);
        bool alive = false;
        for (QWidget *w : QApplication::topLevelWidgets())
            if (w->windowTitle() == "SIGEL MetaGP" && w->isVisible()) alive = true;
        printf("  closed=%d mainWindowAlive=%d\n", alive ? 0 : 1,
               W->isVisible() ? 1 : 0);
        fflush(stdout);
        return 0;
    }

    // --- D29: the run lock, and it is gated by THIS and nothing else -------
    // A review measured that neither mechanism of D29 was gated: deleting the
    // line that arms the lock, and reverting the tree-click emit wholesale,
    // both left all 846 checks green. Its positive control showed the emit
    // line IS observable by the gate, so those passes were a real absence of
    // teeth. Nothing in the repo had ever executed the guard's true branch --
    // `evolution' is not one of the scenarios the gate runs, and inside it
    // slotStartEvolution blocks, so every observation there is post-run.
    //
    // A real evolution is not needed to test the contract. slotStartEvolution
    // sets the protected SIG_GUIGPExperiment::evolutionRunning just before
    // guiGPManager->start(); setting it here through RunState puts the
    // application in the state the lock exists for, deterministically and in
    // about a second.
    //
    // The observable is the page round trip: type into a page, switch away --
    // which calls putAllIntoExperiment -- then switch back, which refreshes
    // the widgets FROM the experiment. If the write happened the new value
    // survives; if the lock held, the old one comes back. No getter, no test
    // hook, only what a user can see.
    if (scenario == "runlock") {
        SIG_ExperimentListView *lv = listView();
        lv->setCurrentItem(lv->topLevelItem(0));
        QTest::qWait(300);
        QStackedWidget *st = W->findChild<QStackedWidget *>();
        printf("\n== D29 RUN LOCK ==\n");
        printf("  atRest isRunning=%d\n", lv->isRunning() ? 1 : 0);

        auto page = [&](const char *m) -> QWidget * {
            clickMenu("&View", QString::fromLatin1(m));
            QTest::qWait(300);
            return st ? st->currentWidget() : nullptr;
        };
        // spinboxMaxAge lives on the GP page and is a plain int, so the round
        // trip is unambiguous.
        auto readAge = [&]() -> int {
            QWidget *pg = page("&GP Parameters");
            QSpinBox *sp = pg ? pg->findChild<QSpinBox *>("spinboxMaxAge") : nullptr;
            return sp ? sp->value() : -1;
        };
        auto typeAge = [&](int v) {
            QWidget *pg = page("&GP Parameters");
            QSpinBox *sp = pg ? pg->findChild<QSpinBox *>("spinboxMaxAge") : nullptr;
            if (!sp) { printf("  !! no spinboxMaxAge\n"); return; }
            sp->setFocus(); sp->selectAll();
            QTest::keyClick(sp, Qt::Key_Delete);
            QTest::keyClicks(sp, QString::number(v));
            QTest::qWait(30);
        };
        // THE OBSERVABLE IS THE EXPERIMENT ITSELF. Two earlier attempts were
        // wrong, and both failure modes are worth keeping:
        //   - reading the spin box back after a page switch. A page switch
        //     calls putAllIntoExperiment (widget -> experiment) and nothing
        //     calls getOutOfExperiment on the way back, so the widget keeps
        //     whatever was typed whether the write happened or not. It
        //     reported `refused=0' against a guard that was working.
        //   - saving the experiment and reading MAXAGE out of the file. File >
        //     Save Experiment is itself one of the 23 actions the lock
        //     disables, so under the lock there is no file to read.
        // What the guard actually protects is the model, so the probe reads
        // the model: gpParameter.getMaxAge(), which putIntoExperiment writes
        // from spinboxMaxAge (SIG_GPParameter.cpp:95).
        SIG_GUIGPExperiment *theExp = lv->currentlySelectedExperiment();
        if (!theExp) { printf("  !! no selected experiment\n"); return 1; }
        auto modelAge = [&]() -> long { return theExp->gpExperiment.gpParameter.getMaxAge(); };
        auto writtenAge = [&](int v) -> long {
            typeAge(v);
            page("&Environment");          // a page switch fires putAllIntoExperiment
            return modelAge();
        };

        const int before = readAge();
        printf("  maxAge as loaded=%d\n", before);

        const long unlocked = writtenAge(before + 7);
        printf("  [unlocked] typed=%d, model reads %ld  written=%d\n",
               before + 7, unlocked, unlocked == before + 7 ? 1 : 0);

        // D30's POSITIVE CONTROL. Every check below asks whether a MetaGP
        // action is OFF during a run. All four are off at rest too, so without
        // this the whole block passes on a window where MetaGP was never
        // enabled and proves nothing. Turn it on first and prove it CAN be on.
        auto actionByText = [&](const char *t) -> QAction * {
            for (QAction *a : W->findChildren<QAction *>())
                if (a->text() == QString::fromLatin1(t)) return a;
            return nullptr;
        };
        auto en = [&](const char *t) -> int {
            QAction *a = actionByText(t);
            return a ? (a->isEnabled() ? 1 : 0) : -1;
        };
        clickMenu("&MetaGP", "&Use MetaGP");
        QTest::qWait(400);
        // Read isChecked(), not isEnabled(). Configure System is ENABLED by any
        // tree click at rest whether MetaGP is on or not, so enabled=1 does not
        // prove the control worked -- it proves a tree click happened. The
        // checked state of Use MetaGP is the thing that says MetaGP is on.
        // Found by review.
        QAction *useMt = actionByText("&Use MetaGP");
        printf("  [control] MetaGP on at rest: UseMetaGP checked=%d"
               "  ConfigureSystem enabled=%d  (both 1 = the checks below can fail)\n",
               useMt && useMt->isChecked() ? 1 : 0, en("&Configure System"));
        if (!useMt || !useMt->isChecked() || en("&Configure System") != 1) {
            printf("!! MetaGP did not come on at rest, so the run-lock checks below"
                   " would pass vacuously\n");
            fflush(stdout); return 1;
        }
        fflush(stdout);

        // evolutionRunning is protected. A pointer to it, formed inside a
        // derived class, sets it on the real experiment.
        struct RunState : SIG_GUIGPExperiment {
            static bool SIG_GUIGPExperiment::*flag() { return &RunState::evolutionRunning; }
        };
        {
            theExp->*RunState::flag() = true;
            printf("  [locked] isRunning=%d\n", lv->isRunning() ? 1 : 0);
            const long locked = writtenAge(before + 21);
            printf("  [locked] typed=%d, model reads %ld  refused=%d\n",
                   before + 21, locked, locked == before + 7 ? 1 : 0);

            // The tree-click emit, which drives the 23 locked actions. While
            // the flag is set a selection change must NOT re-enable them, and that is
            // the half a review found revertible with the gate still green.
            QAction *imp = nullptr, *add = nullptr;
            for (QAction *a : W->findChildren<QAction *>()) {
                if (a->text() == "GP-Parameters" && !imp) imp = a;
                if (a->text() == "&Add") add = a;
            }
            // A REAL selection change. setCurrentItem on the item that is
            // already current emits nothing, and this check used to re-select
            // topLevelItem(0) -- which the View page switches above may or may
            // not have moved away from. Pick an item that is definitely not
            // the current one, or say the check did not run.
            QTreeWidgetItem *cur = lv->currentItem(), *other = nullptr;
            for (int i = 0; i < lv->topLevelItemCount() && !other; ++i) {
                QTreeWidgetItem *t = lv->topLevelItem(i);
                if (t != cur) other = t;
                for (int j = 0; j < t->childCount() && !other; ++j)
                    if (t->child(j) != cur) other = t->child(j);
            }
            if (!other) {
                printf("!! no second tree item -- the tree-click check did NOT run\n");
                fflush(stdout); return 1;
            }
            lv->setCurrentItem(other);
            QTest::qWait(300);
            printf("  [locked] after a tree click ([%s]): Add=%d ConfigureSystem=%d"
                   "  UseMetaGP=%d  (0 = still locked)\n",
                   qPrintable(other->text(0)), add && add->isEnabled() ? 1 : 0,
                   en("&Configure System"), en("&Use MetaGP"));
            // Assert HERE. The slot call below re-applies the lock, so a
            // regression in the tree-click guard alone was silently repaired
            // before the end-of-block check could see it -- the gate stayed
            // green and only the baseline text moved. Found by review.
            if ((add && add->isEnabled()) || en("&Configure System") == 1
                || en("&Use MetaGP") == 1) {
                printf("!! D30: a tree click during a run re-enabled a locked action\n");
                fflush(stdout); return 1;
            }

            // D30's SECOND ROUTE. File > New Experiment and File > Open
            // Experiment emit isNotEmpty(true) into slotEnableNoExperimentActions,
            // which enables 30 actions -- 24 of them also run-locked. Invoke the
            // slot directly: the menu route needs a file dialog, and what is
            // under test is the slot, not the dialog.
            QMetaObject::invokeMethod(W, "slotEnableNoExperimentActions",
                                      Qt::DirectConnection, Q_ARG(bool, true));
            QTest::qWait(200);
            printf("  [locked] after slotEnableNoExperimentActions(true):"
                   " Add=%d ConfigureSystem=%d UseMetaGP=%d  (0 = still locked)\n",
                   add && add->isEnabled() ? 1 : 0,
                   en("&Configure System"), en("&Use MetaGP"));
            // The two doors themselves. Adding them to evolutionRunningActions
            // is a third of this change and NOTHING read them -- revert that
            // append and every check stayed green. Found by review.
            printf("  [locked] the two doors: NewExperiment=%d OpenExperiment=%d"
                   "  (0 = locked during a run)\n",
                   en("&New Experiment"), en("&Open Experiment"));
            if (en("&New Experiment") != 0 || en("&Open Experiment") != 0) {
                printf("!! D30: New or Open Experiment is live during a run --"
                       " either is a route that re-enables everything else\n");
                fflush(stdout); return 1;
            }
            if ((add && add->isEnabled()) || en("&Configure System") == 1
                || en("&Use MetaGP") == 1) {
                printf("!! D30: a locked action came back during a run\n");
                fflush(stdout); return 1;
            }
            (void)imp;
        }

        // This checks that slotEvolutionStopped clears the flag and shows the
        // pool generation. Neither call to it in slotStartEvolution runs here.
        // The `[locked] isRunning=1' line above is the flag's positive control.
        //
        // slotEvolutionStopped also shows the generation the run reached. A
        // run raises poolGeneration and leaves the display alone, so this does
        // the same: the `before' value must differ from the model, which is
        // the positive control for the `after' value.
        QLCDNumber *gens = theExp->experimentView->lcdnumberGenerations;
        const int genAtRest = theExp->gpExperiment.population.getPoolGeneration();
        theExp->gpExperiment.population.poolGeneration = genAtRest + 3;
        printf("  [counter] model=%d display before slotEvolutionStopped=%d\n",
               theExp->gpExperiment.population.getPoolGeneration(), gens->intValue());
        if (gens->intValue() == theExp->gpExperiment.population.getPoolGeneration()) {
            printf("!! the counter already showed the new generation, so the check"
                   " below cannot fail\n");
            fflush(stdout); return 1;
        }
        theExp->slotEvolutionStopped();
        printf("  [released] after slotEvolutionStopped: isRunning=%d (0 = released)\n",
               lv->isRunning() ? 1 : 0);
        const int refreshed =
            gens->intValue() == theExp->gpExperiment.population.getPoolGeneration() ? 1 : 0;
        printf("  [counter] display after slotEvolutionStopped=%d refreshed=%d\n",
               gens->intValue(), refreshed);
        theExp->gpExperiment.population.poolGeneration = genAtRest;
        if (!refreshed) {
            printf("!! slotEvolutionStopped left the generation counter stale\n");
            fflush(stdout); return 1;
        }
        lv->setCurrentItem(lv->topLevelItem(0));
        QTest::qWait(300);
        const long after = writtenAge(before + 33);
        printf("  [released] typed=%d, model reads %ld  writtenAgain=%d\n",
               before + 33, after, after == before + 33 ? 1 : 0);
        fflush(stdout);
        return 0;
    }

    // --- The randomiser stream, with no physics in it ----------------------
    // A cross-machine lockstep check for SIG_Randomizer, designed by the 1.3
    // oracle, and the point is that BOTH SIDES CAN PRODUCE IT. A draw counter
    // in SIG_Randomizer would have been half a measurement -- the oracle has
    // 2003 binaries and no source, so it can never instrument anything, and
    // this project's own rule is to design no check that assumes both halves
    // can be instrumented.
    //
    // The recipe needs nothing but the GUI:
    //   File > New Experiment          -- an EMPTY population, 0 individuals
    //   GP-Parameters > Random seed    -- the GP seed
    //   Individuals > Add, N           -- N programs straight from the
    //                                     randomiser: FITNESS -1, AGE 0
    //   File > Save Experiment
    // No PVM, no DynaMechs, no fitness, no machine-dependent arithmetic
    // anywhere in it. Every opcode, both operands and the program LENGTH are
    // draws, so one extra or missing draw shifts the whole remaining stream --
    // strictly more sensitive than counting draws.
    //
    // TRAP the oracle hit and passed on: a .exp has TWO RANDOMSEED keys. The
    // first is the SIMULATION seed, the second the GP seed. Both ship as 0 and
    // setting the GP one leaves the first at 0, so reading the first and
    // concluding the seed did not take is the obvious mistake.
    if (scenario == "rngseed") {
        const int seed = qEnvironmentVariableIntValue("SIGEL_RNGSEED") > 0
                             ? qEnvironmentVariableIntValue("SIGEL_RNGSEED") : 12345;
        const int want = qEnvironmentVariableIntValue("SIGEL_RNGCOUNT") > 0
                             ? qEnvironmentVariableIntValue("SIGEL_RNGCOUNT") : 5;
        printf("\n== RANDOMISER STREAM (seed=%d n=%d) ==\n", seed, want);

        // The driver loads an experiment at startup, so New Experiment leaves
        // TWO in the tree -- and a first version of this scenario then added
        // to, and saved, the WRONG one: 125 programs and 40,346 instruction
        // lines, which is the loaded 120-individual population plus the 5
        // added. The count 120 + 5 is what gave it away. Delete the loaded one
        // first so that exactly one experiment exists and nothing is ambiguous.
        SIG_ExperimentListView *lv = listView();
        clickMenu("&File", "&New Experiment");
        QTest::qWait(1200);
        printf("  experiments after New=%d\n", lv->topLevelItemCount());
        for (int i = 0; i < lv->topLevelItemCount(); ++i)
            printf("    row%d [%s]\n", i, qPrintable(lv->topLevelItem(i)->text(0)));
        // BY NAME, NOT BY INDEX. slotNewExperiment names it Experiment-<n>.exp,
        // and C7 restored Qt 2's PREPENDING item insertion -- so the new
        // experiment lands at index 0 and the loaded one moves to 1. A first
        // version deleted topLevelItem(0) as "the loaded one" and removed the
        // new empty experiment instead, leaving the 120-individual population
        // to be added to and saved.
        QTreeWidgetItem *loaded = nullptr;
        for (int i = 0; i < lv->topLevelItemCount(); ++i)
            if (!lv->topLevelItem(i)->text(0).startsWith("Experiment-"))
                loaded = lv->topLevelItem(i);
        if (loaded) {
            lv->setCurrentItem(loaded);
            QTest::qWait(300);
            whenModal([](QWidget *m) { clickMsgButton(m, QMessageBox::Yes); }, 4000);
            clickMenu("&File", "&Delete Experiment");
            QTest::qWait(1500);
        }
        printf("  experiments after deleting the loaded one=%d\n",
               lv->topLevelItemCount());
        if (lv->topLevelItemCount() != 1) {
            printf("  !! expected exactly one experiment to remain\n");
            return 1;
        }
        lv->setCurrentItem(lv->topLevelItem(0));
        QTest::qWait(400);
        printf("  remaining experiment=[%s]\n",
               qPrintable(lv->topLevelItem(0)->text(0)));

        QStackedWidget *st = W->findChild<QStackedWidget *>();
        clickMenu("&View", "&GP Parameters");
        QTest::qWait(400);
        QWidget *pg = st ? st->currentWidget() : nullptr;
        QSpinBox *sd = pg ? pg->findChild<QSpinBox *>("spinboxRandomSeed") : nullptr;
        if (!sd) { printf("  !! no spinboxRandomSeed\n"); return 1; }
        sd->setFocus(); sd->selectAll();
        QTest::keyClick(sd, Qt::Key_Delete);
        QTest::keyClicks(sd, QString::number(seed));
        QTest::qWait(60);
        printf("  GP random seed set to %d\n", sd->value());

        // The pool must be EMPTY before Add, or the programs are not a clean
        // stream from the seed. Reported, not assumed.
        clickMenu("&View", "&Population");
        QTest::qWait(400);
        QTreeWidget *il = indList();
        printf("  individuals before Add=%d (0 expected from a new experiment)\n",
               il ? il->topLevelItemCount() : -1);

        whenModal([want](QWidget *m) {
            QSpinBox *sp = m->findChild<QSpinBox *>();
            if (!sp) { printf("  !! add dialog has no spin box\n"); m->close(); return; }
            // select-all then type: the dialog pre-fills 1, and C11c measured
            // that Qt 6 leaves a pre-filled field selected where Qt 2 did not,
            // so appending would give 15 rather than 5.
            sp->setFocus(); sp->selectAll();
            QTest::keyClick(sp, Qt::Key_Delete);
            QTest::keyClicks(sp, QString::number(want));
            QTest::qWait(40);
            printf("  add dialog: n=%d\n", sp->value());
            if (QDialog *d = qobject_cast<QDialog *>(m)) d->accept();
        }, 6000);
        clickMenu("&Individuals", "&Add");
        QTest::qWait(3000);
        printf("  individuals after Add=%d\n", il ? il->topLevelItemCount() : -1);

        const QString out = scratch() + "/rngseed.exp";
        QFile::remove(out);
        whenModal([out](QWidget *m) {
            if (QFileDialog *fd = qobject_cast<QFileDialog *>(m)) {
                printf("  [savedialog] parentIsTheMainWindow=%d\n",
                       (fd->parentWidget() && fd->parentWidget()->window() == W) ? 1 : 0);
                acceptFileDialog(fd, out);
            }
            else m->close();
        });
        clickMenu("&File", "&Save Experiment");
        QTest::qWait(4000);
        QFile f(out);
        if (!f.open(QIODevice::ReadOnly)) { printf("  !! nothing saved\n"); return 1; }
        const QStringList all = QString::fromLatin1(f.readAll()).split('\n');
        f.close();

        // EXTRACTION RULE, stated so the two sides can align: every line
        // strictly between `PROGRAM BEGIN{' and `}PROGRAM END', trimmed of
        // leading and trailing whitespace, in file order, joined with '\n'.
        // Nothing else -- no dates, no fitness, no headers.
        QStringList instr;
        QList<int> lens;
        bool in = false;
        int thisLen = 0;
        for (const QString &raw : all) {
            const QString t = raw.trimmed();
            if (t.startsWith("PROGRAM BEGIN{")) { in = true; thisLen = 0; continue; }
            if (t.startsWith("}PROGRAM END"))   { if (in) lens << thisLen; in = false; continue; }
            if (in && !t.isEmpty()) { instr << t; ++thisLen; }
        }
        const QByteArray joined = instr.join(QStringLiteral("\n")).toLatin1();
        printf("  programs=%d  instructionLines=%d\n", (int)lens.count(), (int)instr.count());
        QStringList lenTxt;
        for (int n : lens) lenTxt << QString::number(n);
        printf("  perProgramLengths=[%s]\n", qPrintable(lenTxt.join(",")));
        printf("  sha256(instructions)=%s\n",
               QCryptographicHash::hash(joined, QCryptographicHash::Sha256).toHex().constData());
        printf("  first 6 of individual 0: %s\n",
               qPrintable(QStringList(instr.mid(0, 6)).join(" / ")));
        // Both RANDOMSEED keys, because reading the wrong one is the trap.
        for (int i = 0; i + 1 < all.size(); ++i)
            if (all.at(i).trimmed() == QStringLiteral("RANDOMSEED"))
                printf("  RANDOMSEED at line %d = %s\n", i + 1, qPrintable(all.at(i + 1).trimmed()));
        fflush(stdout);
        return 0;
    }

    // --- Individuals > Add ------------------------------------------------
    if (scenario == "add") {
        whenModal([](QWidget *m) {
            describeDialog(m);
            // The Designer spinbox carries no objectName in the converted form,
            // so take the dialog's only QSpinBox rather than looking one up.
            QSpinBox *sp = m->findChild<QSpinBox *>();
            if (sp) {
                sp->setFocus();
                QTest::keyClick(sp, Qt::Key_A, Qt::ControlModifier);  // select all
                QTest::keyClicks(sp, "5");
                QTest::qWait(120);
            }
            printf("  [set spinbox to] %d\n", sp ? sp->value() : -1); fflush(stdout);
            clickDlgButton(m, "OK");
        });
        clickMenu("&Individuals", "&Add");
        QTest::qWait(1500);
        step("after Individuals > Add (5)", false, false, true);
        QTreeWidget *t = indList();
        if (t) {
            int n = t->topLevelItemCount();
            printf("  [tail rows] (last 6 of %d)\n", n);
            for (int i = qMax(0, n - 6); i < n; ++i)
                printf("    row%-3d | %-12s | %-14s | %-6s\n", i,
                       qPrintable(t->topLevelItem(i)->text(0)),
                       qPrintable(t->topLevelItem(i)->text(1)),
                       qPrintable(t->topLevelItem(i)->text(2)));
        }
        return 0;
    }

    // --- Individuals > Delete, both answers -------------------------------
    if (scenario == "delete") {
        QTreeWidget *t = indList();
        if (!t) { printf("!! no individuals list\n"); return 1; }
        clickRow(t, 0);
        clickRow(t, 1, Qt::ControlModifier);
        printf("\n  [selected %d rows before delete]\n", (int)t->selectedItems().count());
        step("2 rows selected", false, false, true);

        whenModal([](QWidget *m) { describeMessageBox(m); clickMsgButton(m, QMessageBox::No); });
        clickMenu("&Individuals", "&Delete");
        QTest::qWait(800);
        step("after Delete answered NO", false, false, true);

        clickRow(t, 0);
        clickRow(t, 1, Qt::ControlModifier);
        whenModal([](QWidget *m) { describeMessageBox(m); clickMsgButton(m, QMessageBox::Yes); });
        clickMenu("&Individuals", "&Delete");
        QTest::qWait(1500);
        step("after Delete answered YES", false, false, true);
        return 0;
    }

    // --- Individuals > Reset ----------------------------------------------
    if (scenario == "reset") {
        QTreeWidget *t = indList();
        printf("\n  [before reset] rows=%d row0=[%s|%s|%s]\n", t->topLevelItemCount(),
               qPrintable(t->topLevelItem(0)->text(0)), qPrintable(t->topLevelItem(0)->text(1)),
               qPrintable(t->topLevelItem(0)->text(2)));
        whenModal([](QWidget *m) { printf("  [unexpected modal on reset]\n");
                                   describeDialog(m); m->close(); }, 2500);
        clickMenu("&Individuals", "Reset");
        QTest::qWait(2000);
        step("after Individuals > Reset", false, false, true);
        return 0;
    }

    // --- the dialogs -------------------------------------------------------
    // --- save, reload, save again: the round trip -------------------------
    if (scenario == "saveload") {
        QString outA = scratch() + "/roundA.exp";
        QString outB = scratch() + "/roundB.exp";
        QFile::remove(outA); QFile::remove(outB);

        // Save the freshly loaded experiment through File > Save Experiment.
        whenModal([outA](QWidget *m) {
            QFileDialog *fd = qobject_cast<QFileDialog *>(m);
            if (!fd) { printf("  !! save modal is not a QFileDialog\n"); m->close(); return; }
            printf("  [savedialog] title=[%s] mode=%d accept=[%s] selectedFile=[%s]\n",
                   qPrintable(fd->windowTitle()), (int)fd->fileMode(),
                   qPrintable(fd->labelText(QFileDialog::Accept)),
                   fd->selectedFiles().isEmpty() ? "" : qPrintable(fd->selectedFiles().first()));
            printf("  [savedialog] parentIsTheMainWindow=%d\n", (fd->parentWidget() && fd->parentWidget()->window() == W) ? 1 : 0);
            QLineEdit *le = fd->findChild<QLineEdit *>("fileNameEdit");
            if (!le) { printf("  !! no fileNameEdit\n"); fd->reject(); return; }
            // 1.3 pre-fills this field but does NOT pre-select it, so typing
            // appends -- unlike the Rename dialog, which does pre-select.
            // Read BEFORE acceptFileDialog touches it, so the default state is
            // what is printed.
            printf("  [savedialog] prefill=[%s] preselected=[%s]\n",
                   qPrintable(le->text()), qPrintable(le->selectedText()));
            fflush(stdout);
            acceptFileDialog(fd, outA);
        });
        clickMenu("&File", "&Save Experiment");
        QTest::qWait(3000);
        printf("  [saved A] exists=%d size=%lld\n", QFile::exists(outA),
               QFileInfo(outA).size());
        step("after File > Save Experiment", true, false);

        // Load the file we just wrote, as a SECOND experiment.
        openExperiment(outA);
        step("after re-opening the saved file", true, true, true);

        // Save that one too, so A and B can be compared byte for byte.
        whenModal([outB](QWidget *m) {
            QFileDialog *fd = qobject_cast<QFileDialog *>(m);
            if (!fd) { m->close(); return; }
            acceptFileDialog(fd, outB);
        });
        clickMenu("&File", "&Save Experiment");
        QTest::qWait(3000);
        printf("  [saved B] exists=%d size=%lld\n", QFile::exists(outB),
               QFileInfo(outB).size());
        return 0;
    }

    if (scenario == "rename") {
        whenModal([](QWidget *m) { describeDialog(m); clickDlgButton(m, "Cancel"); });
        clickMenu("&File", "&Rename Experiment");
        QTest::qWait(600);
        step("rename CANCELLED", true, false);

        whenModal([](QWidget *m) {
            // Converted dialogs carry no objectNames; take the only QLineEdit.
            QLineEdit *le = m->findChild<QLineEdit *>();
            if (le) { le->setFocus(); QTest::keyClicks(le, "renamed"); }
            printf("  [typed] %s\n", le ? qPrintable(le->text()) : "?"); fflush(stdout);
            clickDlgButton(m, "OK");
        });
        printf("\n  >> about to click OK on rename\n"); fflush(stdout);
        clickMenu("&File", "&Rename Experiment");
        QTest::qWait(800);
        printf("  >> rename dialog returned\n"); fflush(stdout);
        step("rename to 'renamed'", true, false);

        return 0;
    }

    if (scenario == "delexp") {
        printf("\n  >> about to open File > Delete Experiment\n"); fflush(stdout);
        whenModal([](QWidget *m) { describeMessageBox(m); describeDialog(m);
                                   clickMsgButton(m, QMessageBox::No); });
        clickMenu("&File", "&Delete Experiment");
        QTest::qWait(1000);
        step("delete experiment answered NO", true, false);
        return 0;
    }

    if (scenario == "quit") {
        printf("\n  >> about to open File > Quit\n"); fflush(stdout);
        whenModal([](QWidget *m) { describeMessageBox(m); describeDialog(m);
                                   clickMsgButton(m, QMessageBox::No); });
        clickMenu("&File", "&Quit");
        QTest::qWait(1000);
        printf("  >> survived Quit-answered-No, app still alive\n"); fflush(stdout);
        step("quit answered NO", true, false);
        return 0;
    }

    // --- context menus -----------------------------------------------------
    if (scenario == "ctxmenu") {
        SIG_ExperimentListView *lv = listView();
        QTreeWidgetItem *top = lv->topLevelItem(0);
        dumpContextMenu(lv->viewport(), lv->visualItemRect(top).center(), "tree: experiment row");

        QTreeWidgetItem *inds = top->child(0);
        dumpContextMenu(lv->viewport(), lv->visualItemRect(inds).center(),
                        "tree: Individuals child row");

        dumpContextMenu(lv->viewport(), QPoint(lv->viewport()->width() / 2,
                                               lv->viewport()->height() - 12),
                        "tree: empty space below rows");

        clickMenu("&View", "&Population");
        QTest::qWait(300);
        QTreeWidget *t = indList();
        if (t) {
            clickRow(t, 0);
            dumpContextMenu(t->viewport(), t->visualItemRect(t->topLevelItem(0)).center(),
                            "individuals: selected row");
            dumpContextMenu(t->viewport(), QPoint(t->viewport()->width() / 2,
                                                  t->viewport()->height() - 8),
                            "individuals: empty space below rows");
        }
        return 0;
    }

    // --- sorting the individuals list by Fitness --------------------------
    // --- the MetaGP disable warning ----------------------------------------
    if (scenario == "metagp") {
        printf("\n  >> MetaGP > Use MetaGP (switch ON)\n"); fflush(stdout);
        whenModal([](QWidget *m) { printf("  [modal on ENABLE]\n"); describeDialog(m);
                                   m->close(); }, 3000);
        clickMenu("&MetaGP", "&Use MetaGP");
        QTest::qWait(1500);
        for (QAction *a : W->menuBar()->actions())
            if (a->text() == "&MetaGP" && a->menu())
                for (QAction *b : a->menu()->actions())
                    if (b->text() == "&Use MetaGP")
                        printf("  [Use MetaGP] checkable=%d checked=%d\n",
                               b->isCheckable(), b->isChecked());
        fflush(stdout);

        printf("\n  >> MetaGP > Use MetaGP again (switch OFF -- the warning)\n");
        fflush(stdout);
        whenModal([](QWidget *m) {
            describeDialog(m);
            QMessageBox *mb = qobject_cast<QMessageBox *>(m);
            if (mb) {
                printf("  [msgbox] icon=%d text=[%s]\n", (int)mb->icon(),
                       qPrintable(mb->text()));
                for (QAbstractButton *b : mb->buttons())
                    printf("    button [%-14s] role=%d default=%d\n",
                           qPrintable(b->text()), (int)mb->buttonRole(b),
                           b == mb->defaultButton() ? 1 : 0);
            }
            fflush(stdout);
            m->close();
        }, 5000);
        clickMenu("&MetaGP", "&Use MetaGP");
        QTest::qWait(2000);
        return 0;
    }

    // --- context menu on genuinely EMPTY space in the individuals list -----
    // With 120 rows the viewport is full, so a click near its bottom edge still
    // lands ON a row and returns the row menu. itemAt() must actually be null,
    // which means shrinking the pool first -- the same thing a user would have
    // to do, and what the 1.3 oracle did.
    if (scenario == "ctxempty") {
        QTreeWidget *t = indList();
        if (!t) { printf("!! no individuals list\n"); return 1; }
        clickRow(t, 5);
        clickRow(t, t->topLevelItemCount() - 1, Qt::ShiftModifier);
        printf("\n  [selected for deletion] %d rows\n", (int)t->selectedItems().count());
        fflush(stdout);
        whenModal([](QWidget *m) { describeMessageBox(m); clickMsgButton(m, QMessageBox::Yes); });
        clickMenu("&Individuals", "&Delete");
        QTest::qWait(4000);
        printf("  [rows now] %d\n", t->topLevelItemCount());

        QTreeWidgetItem *last = t->topLevelItem(t->topLevelItemCount() - 1);
        QRect lr = t->visualItemRect(last);
        QPoint empty(t->viewport()->width() / 2, lr.bottom() + 24);
        printf("  [empty-space probe] point=(%d,%d) viewport=%dx%d itemAt=%s\n",
               empty.x(), empty.y(), t->viewport()->width(), t->viewport()->height(),
               t->itemAt(empty) ? "A ROW (not empty!)" : "null (genuinely empty)");
        fflush(stdout);

        clickRow(t, 0);
        printf("  [selected before empty-space right-click] %d\n",
               (int)t->selectedItems().count());
        dumpContextMenu(t->viewport(), empty, "individuals: EMPTY space");
        printf("  [selected after  empty-space right-click] %d\n",
               (int)t->selectedItems().count());

        dumpContextMenu(t->viewport(), t->visualItemRect(t->topLevelItem(0)).center(),
                        "individuals: a row (for contrast)");
        return 0;
    }

    // --- File > Export > Program for a named individual --------------------
    // Byte-comparable across architectures: it is the individual's program
    // text, not a simulated number, so it is the half of the "same individual"
    // question that i386-x87 vs aarch64 cannot muddy.
    if (scenario == "export") {
        QTreeWidget *t = indList();
        if (!t) { printf("!! no individuals list\n"); return 1; }
        int want = -1;
        for (int i = 0; i < t->topLevelItemCount(); ++i)
            if (t->topLevelItem(i)->text(0) == "55658") { want = i; break; }
        if (want < 0) { printf("!! individual 55658 not found\n"); return 1; }
        clickRow(t, want);
        printf("\n  [exporting] name=%s selected=%d current=%s\n",
               qPrintable(t->topLevelItem(want)->text(0)),
               (int)t->selectedItems().count(),
               t->currentItem() ? qPrintable(t->currentItem()->text(0)) : "(none)");
        fflush(stdout);

        QString out = scratch() + "/prog55658.prg";
        QFile::remove(out);
        whenModal([out](QWidget *m) {
            QFileDialog *fd = qobject_cast<QFileDialog *>(m);
            if (!fd) { printf("  !! not a QFileDialog\n"); m->close(); return; }
            printf("  [exportdialog] title=[%s] filters=[%s] accept=[%s]\n",
                   qPrintable(fd->windowTitle()),
                   qPrintable(fd->nameFilters().join(" ;; ")),
                   qPrintable(fd->labelText(QFileDialog::Accept)));
            acceptFileDialog(fd, out);
            fflush(stdout);
        });
        clickMenu("&File", "Export", "Program");
        QTest::qWait(2500);
        printf("  [exported] exists=%d bytes=%lld\n", QFile::exists(out), QFileInfo(out).size());
        return 0;
    }

    // --- clipcheck: controls the user cannot see or reach -----------------
    // The one real defect SIGEL_SlaveGUI turned up was a group box too small to
    // contain its own children, which hid seven buttons and three readouts. No
    // widget-level probe saw it: every child was present, enabled and correctly
    // sized. Only the containing rect was wrong. This walks every page and tab
    // and reports any widget whose rect leaves its parent's.
    //
    // The oracle ran the equivalent on 1.3 and found nothing clipped at the
    // default 900x750, having first PROVED the check can fire by shrinking the
    // window until Import/Export fell off their parent. Same idea here: the
    // `--selftest' pass below deliberately shrinks a page and must report hits,
    // or a clean result means nothing.
    if (scenario == "clipcheck") {
        auto walk = [&](QWidget *root, const char *where) {
            return clippedWidgets(root, where);
        };

        printf("\n== CLIPCHECK: main window at %dx%d ==\n", W->width(), W->height());
        QStackedWidget *st = W->findChild<QStackedWidget *>();
        int total = 0;
        for (const char *pg : { "&Population", "&Robot", "&Language Parameters",
                                "&GP Parameters", "&Simulation Parameters",
                                "&Environment" }) {
            clickMenu("&View", QString::fromLatin1(pg));
            QTest::qWait(300);
            QWidget *page = st ? st->currentWidget() : nullptr;
            if (!page) { printf("  %-26s NO PAGE\n", pg); continue; }
            printf("  [%s] page %dx%d\n", pg, page->width(), page->height());
            total += walk(page, pg);
            if (QTabWidget *tw = page->findChild<QTabWidget *>()) {
                for (int i = 0; i < tw->count(); ++i) {
                    QTabBar *bar = tw->tabBar();
                    QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier,
                                      bar->tabRect(i).center());
                    QTest::qWait(200);
                    total += walk(tw->currentWidget(),
                                  qPrintable(QString("  tab %1").arg(tw->tabText(i))));
                }
            }
        }
        printf("  TOTAL CLIPPED: %d\n", total);

        // DIALOGS TOO. The first version walked only the View pages and would
        // NOT have caught the movie-settings dialog, whose "File conventions"
        // group hid seven controls -- a static review found that, not this. A
        // check that misses the second instance of the defect it was written
        // for is not finished.
        // The slave window and its movie dialog are checked by the `slavegui'
        // scenario, which is where they exist. A dialog walk was attempted here
        // first and was useless: the movie dialog belongs to the slave window,
        // not to anything the master's menus can open.

        // THE POSITIVE CONTROL. Without it "0 clipped" is not evidence.
        // Shrinking the window does NOT work as a control here, and that is
        // itself a finding: these pages carry real layouts, so a smaller window
        // reflows them instead of clipping. On 1.3 the same shrink DOES clip,
        // because its pages are absolutely positioned -- the oracle used exactly
        // that to validate its own check, and it fired immediately there. So the
        // port is better behaved than 1.3 on window resize, and this check needs
        // a control that does not depend on clipping being reachable.
        // Move one real widget outside its parent and require a report.
        printf("\n  -- selftest: displace a widget and require the check to see it --\n");
        clickMenu("&View", "&GP Parameters");
        QTest::qWait(300);
        int fired = 0;
        if (QWidget *page = st ? st->currentWidget() : nullptr) {
            QWidget *victim = nullptr;
            for (QWidget *w : page->findChildren<QWidget *>())
                if (w->isVisible() && !w->size().isEmpty() && w->parentWidget()) { victim = w; break; }
            if (!victim) { printf("!! no widget to displace\n"); fflush(stdout); return 1; }
            const QRect keep = victim->geometry();
            victim->move(victim->parentWidget()->width() + 40, keep.y());
            QTest::qWait(120);
            fired = walk(page, "with one widget displaced");
            victim->setGeometry(keep);
            QTest::qWait(120);
        }
        printf("  selftest %s\n", fired > 0
               ? "OK -- the check reports a widget that leaves its parent"
               : "!! USELESS -- a displaced widget was not reported; a clean result proves nothing");
        fflush(stdout);
        // Fail on EITHER a real clipped control or a check that cannot detect one.
        return (total == 0 && fired > 0) ? 0 : 1;
    }

    // --- formsize: a form the user can drag smaller than Qt 6 can lay out --
    // The sibling of clipcheck, and it catches what clipcheck structurally
    // cannot. clipcheck walks widgets at ONE size and asks whether any child
    // leaves its parent. This asks a different question: what is the SMALLEST
    // size the form permits, and can Qt lay the form out at that size?
    //
    // An explicit <minimumSize> in the .ui becomes setMinimumSize(), which
    // OVERRIDES minimumSizeHint() -- so a form whose declared minimum is below
    // what its layout needs can be dragged down until its children compress,
    // and no walk at the default size sees it. The declared values are 1.3's,
    // read under Qt 2's smaller default font; Qt 6's larger metrics are what
    // make the layout need more. Raising them RESTORES 1.3's readability.
    //
    // A declared 0 means "unset", and Qt then uses the hint by itself -- those
    // forms are correct as they are and are reported, not failed.
    if (scenario == "formsize") {
        struct Row { const char *name; std::function<QWidget *()> make; };
        const Row forms[] = {
            { "MT_AddConstantsWidgetBase",  [] { return (QWidget *) new MT_AddConstantsWidgetBase; } },
            { "MT_AddIndividualsWidgetBase",[] { return (QWidget *) new MT_AddIndividualsWidgetBase; } },
            { "MT_EstimationWidgetBase",    [] { return (QWidget *) new MT_EstimationWidgetBase; } },
            { "MT_ExperimentWidgetBase",    [] { return (QWidget *) new MT_ExperimentWidgetBase; } },
            { "MT_IndividualsWidgetBase",   [] { return (QWidget *) new MT_IndividualsWidgetBase; } },
            { "MT_PopulationWidgetBase",    [] { return (QWidget *) new MT_PopulationWidgetBase; } },
            { "MT_SearchWidgetBase",        [] { return (QWidget *) new MT_SearchWidgetBase; } },
            { "MT_SelectionWidgetBase",     [] { return (QWidget *) new MT_SelectionWidgetBase; } },
            { "MT_StatisticsWidgetBase",    [] { return (QWidget *) new MT_StatisticsWidgetBase; } },
            { "SIG_EditHostDialogBase",     [] { return (QWidget *) new SIG_EditHostDialogBase; } },
            { "SIG_EnvironmentBase",        [] { return (QWidget *) new SIG_EnvironmentBase; } },
            { "SIG_ExperimentViewBase",     [] { return (QWidget *) new SIG_ExperimentViewBase; } },
            { "SIG_GPParameterBase",        [] { return (QWidget *) new SIG_GPParameterBase; } },
            { "SIG_IndividualListBase",     [] { return (QWidget *) new SIG_IndividualListBase; } },
            { "SIG_IndividualViewBase",     [] { return (QWidget *) new SIG_IndividualViewBase; } },
            { "SIG_LanguageParametersBase", [] { return (QWidget *) new SIG_LanguageParametersBase; } },
            { "SIG_RobotBase",              [] { return (QWidget *) new SIG_RobotBase; } },
            { "SIG_SimulationParameterBase",[] { return (QWidget *) new SIG_SimulationParameterBase; } },
            { "SIG_MovieSettingsDialogBase",[] { return (QWidget *) new SIG_MovieSettingsDialogBase; } },
            { "SIG_SimulationWidgetBase",   [] { return (QWidget *) new SIG_SimulationWidgetBase; } },
        };
        const int nForms = (int)(sizeof(forms) / sizeof(forms[0]));

        // ONE comparison, used by the loop AND by the control below.
        //
        // It was two: the control re-typed the same expression, so breaking the
        // loop's `<' left the control passing and the scenario returning 0.
        // clipcheck next door does this right -- its control calls the same
        // `walk' lambda that produced the real count -- and this regressed from
        // that pattern in the same file. Found by review.
        //
        // PER AXIS, not per form. setMinimumSize clears the explicit-minimum
        // flag for whichever axis is 0, and the layout then supplies that axis
        // by itself -- so an axis declaring 0 cannot be too small, whatever the
        // other axis declares. The first version tested `width==0 && height==0'
        // and would have called a form declaring 240x0 TOO SMALL for a height
        // Qt takes from the hint. No form is in that state today; it is closed
        // before one is.
        enum Verdict { V_NOHINT, V_UNSET, V_TOOSMALL, V_OK };
        auto classify = [](const QSize &dec, const QSize &hint) -> Verdict {
            if (!hint.isValid()) return V_NOHINT;
            const bool wSet = dec.width() > 0, hSet = dec.height() > 0;
            if (!wSet && !hSet) return V_UNSET;
            if ((wSet && dec.width()  < hint.width()) ||
                (hSet && dec.height() < hint.height())) return V_TOOSMALL;
            return V_OK;
        };

        printf("\n== FORM MINIMUMS: declared <minimumSize> against Qt 6's minimumSizeHint ==\n");
        printf("   %d forms; the corpus size AND the number actually compared\n", nForms);
        printf("   are both asserted below.\n\n");
        int tooSmall = 0, unset = 0, noHint = 0, compared = 0, victim = -1;
        for (int k = 0; k < nForms; ++k) {
            QWidget *w = forms[k].make();
            w->ensurePolished();
            const QSize dec = w->minimumSize();
            const QSize hint = w->minimumSizeHint();
            const Verdict v = classify(dec, hint);
            const char *verdict;
            switch (v) {
            case V_NOHINT:   verdict = "NO HINT (no layout)";       ++noHint;   break;
            case V_UNSET:    verdict = "unset -- Qt uses the hint"; ++unset;    break;
            case V_TOOSMALL: verdict = "TOO SMALL";                 ++tooSmall; ++compared; break;
            default:         verdict = "ok";                                    ++compared;
                             if (victim < 0) victim = k;            break;
            }
            printf("  %-30s declared %4dx%-4d  hint %4dx%-4d  %s\n",
                   forms[k].name, dec.width(), dec.height(),
                   hint.width(), hint.height(), verdict);
            delete w;
        }
        printf("\n  TOO SMALL: %d   (compared: %d, unset: %d, no hint: %d, of %d forms)\n",
               tooSmall, compared, unset, noHint, nForms);

        // THE CONTROL, and it runs through classify() on a form that the loop
        // ACTUALLY COMPARED. The first version forced forms[0], which declares
        // no minimum and therefore takes the `unset' early-out in the real loop
        // -- so it exercised a branch the chosen form never reaches there.
        printf("\n  -- selftest: lower a compared form's minimum below its hint --\n");
        int fired = 0;
        if (victim < 0) {
            printf("    no form reached the comparison; there is nothing to control\n");
        } else {
            QWidget *w = forms[victim].make();
            w->ensurePolished();
            const QSize hint = w->minimumSizeHint();
            if (hint.isValid() && hint.width() > 1 && hint.height() > 1) {
                w->setMinimumSize(1, 1);
                const QSize dec = w->minimumSize();
                const QSize h2 = w->minimumSizeHint();
                fired = (classify(dec, h2) == V_TOOSMALL);
                printf("    %s forced to %dx%d against hint %dx%d -> %s\n",
                       forms[victim].name, dec.width(), dec.height(),
                       h2.width(), h2.height(), fired ? "TOO SMALL" : "NOT REPORTED");
            } else {
                printf("    %s has no usable hint; the control could not run\n",
                       forms[victim].name);
            }
            delete w;
        }
        printf("  selftest %s\n", fired
               ? "OK -- the same comparison the loop uses reports it"
               : "!! USELESS -- a forced-small minimum was not reported;"
                 " a clean result proves nothing");

        // WHY `compared' IS ASSERTED, and it is the whole reason this gate is
        // not decorative. `tooSmall == 0' is satisfied equally by "every form
        // is big enough" and by "no form reached the comparison at all" --
        // and 13 of the 20 declare no minimum, so they take the unset
        // early-out. Delete the six <minimumSize> blocks, or break whatever
        // carries them out of the .ui, and every form becomes `unset':
        // tooSmall stays 0, the corpus is still 20, and the control still
        // fires because it sets its own minimum. Exit 0 with all six fixes
        // gone and nothing measured. check.sh:1502 learned this two commits
        // ago for the forms corpus; this is the same assertion.
        // SEVEN forms declare a minimum today. Raise this when one more does;
        // never lower it to make the gate quiet.
        const int wantCompared = 7;
        if (compared < wantCompared)
            printf("!! only %d form(s) reached the comparison, expected at least %d --\n"
                   "!! the declared minimums are not arriving from the .ui\n",
                   compared, wantCompared);
        // A form with no layout reports an invalid hint and is compared against
        // nothing. One is a defect in itself, not a form to skip.
        if (noHint != 0)
            printf("!! %d form(s) reported no minimumSizeHint at all\n", noHint);
        fflush(stdout);
        return (tooSmall == 0 && fired > 0 && nForms == 20
                && noHint == 0 && compared >= wantCompared) ? 0 : 1;
    }

    // --- SIGEL_SlaveGUI: the slave's simulation window ---------------------
    // The last module C11 never drove. It is reachable WITHOUT PVM: the slave's
    // own standalone mode is `sigel_slave -visualize <exp>'
    // (sigel_slave.cpp:143-152), which loads an experiment, takes individual 0's
    // program and opens this window. This scenario does the same thing in
    // process, because guidrive already links SIGEL_SlaveGUI through GUI_LIBS
    // and because a second program would be a harness rather than a scenario.
    //
    // It matters beyond coverage: this widget owns the `visualisation' pointer
    // of PORTING.md's C11 slave-GUI notes, and showAncorPointsCheckBox is one of the
    // fourteen sites that dereference it behind a guard.
    if (scenario == "slavegui") {
        SIG_ExperimentListView *lv = listView();
        lv->setCurrentItem(lv->topLevelItem(0));
        QTest::qWait(400);
        SIG_GUIGPExperiment *ex = lv->currentlySelectedExperiment();
        if (!ex) { printf("!! no experiment selected\n"); fflush(stdout); return 1; }
        if (ex->gpExperiment.population.getSize() < 1) {
            printf("!! population is empty -- nothing to visualise\n");
            fflush(stdout); return 1;
        }

        printf("\n== SLAVE SIMULATION WINDOW ==\n");
        SIGEL_SlaveGUI::SIG_SimulationWindow *sw =
            new SIGEL_SlaveGUI::SIG_SimulationWindow(nullptr, "simWindow");
        sw->setWindowTitle("Simulation Visualisation");   // as sigel_slave.cpp, main
        sw->resize(780, 810);                            // the oracle's 1.3 geometry
        sw->show();
        (void)QTest::qWaitForWindowExposed(sw);
        QTest::qWait(300);

        // THE ROBOT MUST BE PREPARED FIRST, and this is not optional decoration:
        // SIG_RobotRenderer's constructor walks every link's geometry
        // (SIG_RobotRenderer.cpp, buildDisplayLists -> SIG_GeometryIterator::valid ->
        // SIG_Geometry::getNumPolygons), and a link's geometry is null until
        // SIG_Robot::instantiateGeometries() has run. Passing the experiment's
        // robot straight in SEGFAULTS on a null `this'. The standalone slave
        // does not: sigel_slave.cpp:245-269 copies the robot and calls
        // prepareDynaMechs() (or prepareDynaMo()), each of which begins with
        // instantiateGeometries(). Mirrored exactly here, switch and all, so
        // this scenario drives the same path the real slave drives.
        SIGEL_Robot::SIG_Robot *modifiedRobot =
            new SIGEL_Robot::SIG_Robot( ex->gpExperiment.robot );
        try {
            if (ex->gpExperiment.simulationParameter.getSimulationLibrary()
                    == SIGEL_Simulation::SIG_SimulationParameters::DynaMo)
                modifiedRobot->prepareDynaMo();
            else
                modifiedRobot->prepareDynaMechs();
        } catch (SIGEL_Tools::SIG_Exception &e) {
            printf("!! preparing the robot threw: %s\n", qPrintable(e.getMessage()));
            fflush(stdout); return 1;
        }
        printf("  [robot] prepared, library=%d bodies=%d\n",
               (int)ex->gpExperiment.simulationParameter.getSimulationLibrary(),
               (int)ex->gpExperiment.robot.getBodies().size());
        fflush(stdout);

        // Individual 0, exactly as the standalone slave picks it.
        sw->visualizeThis(*modifiedRobot,
                          ex->gpExperiment.environment,
                          ex->gpExperiment.simulationParameter,
                          ex->gpExperiment.population.getIndividual(0).getProgramVar());
        QTest::qWait(600);
        printf("  [window] title=[%s] size=%dx%d visible=%d\n",
               qPrintable(sw->windowTitle()), sw->width(), sw->height(),
               sw->isVisible());
        int swClipped = clippedWidgets(sw, "slave window");

        // Everything with observable state, by objectName so the oracle can be
        // asked about the same control by the same name.
        for (QTabWidget *tw : sw->findChildren<QTabWidget *>()) {
            printf("  [tabs] count=%d current=%d", tw->count(), tw->currentIndex());
            for (int i = 0; i < tw->count(); ++i)
                printf(" [%s]", qPrintable(tw->tabText(i)));
            printf("\n");
        }
        // GEOMETRY IS REPORTED because a control the user cannot see or hit is a
        // usability defect, not a cosmetic one. 1.3 has seven 50x50 navigation
        // buttons inside a 220x331 group (oracle, measured on screen).
        for (QGroupBox *g : sw->findChildren<QGroupBox *>())
            printf("    group   [%-24s] %dx%d at +%d+%d visible=%d\n",
                   qPrintable(g->title()), g->width(), g->height(), g->x(), g->y(),
                   g->isVisible());
        for (QAbstractButton *b : sw->findChildren<QAbstractButton *>()) {
            const char *kind = qobject_cast<QCheckBox *>(b) ? "check" : "button";
            printf("    %-7s [%-24s] %dx%d at +%d+%d icon=%d text=[%s] enabled=%d checked=%d\n",
                   kind, qPrintable(b->objectName()), b->width(), b->height(),
                   b->x(), b->y(), b->icon().isNull() ? 0 : 1,
                   qPrintable(b->text()), b->isEnabled(), b->isChecked());
        }
        for (QSlider *sl : sw->findChildren<QSlider *>())
            printf("    slider  [%-24s] value=%d min=%d max=%d step=%d enabled=%d\n",
                   qPrintable(sl->objectName()), sl->value(), sl->minimum(),
                   sl->maximum(), sl->pageStep(), sl->isEnabled());
        for (QComboBox *cb : sw->findChildren<QComboBox *>()) {
            printf("    combo   [%-24s] current=%d [", qPrintable(cb->objectName()),
                   cb->currentIndex());
            for (int i = 0; i < cb->count(); ++i)
                printf("%s%s", i ? "|" : "", qPrintable(cb->itemText(i)));
            printf("]\n");
        }
        for (QSpinBox *sp : sw->findChildren<QSpinBox *>())
            printf("    spin    [%-24s] value=%d min=%d max=%d\n",
                   qPrintable(sp->objectName()), sp->value(), sp->minimum(), sp->maximum());
        for (QLabel *la : sw->findChildren<QLabel *>())
            if (!la->objectName().isEmpty() && la->objectName().startsWith("TextLabel") == false)
                printf("    label   [%-24s] text=[%s]\n",
                       qPrintable(la->objectName()), qPrintable(la->text()));
        for (QAction *a : sw->findChildren<QAction *>())
            printf("    action  [%-24s] text=[%s] enabled=%d\n",
                   qPrintable(a->objectName()), qPrintable(a->text()), a->isEnabled());
        fflush(stdout);

        // NOTE FOR THE RECORD: offscreen, Qt reports "QOpenGLWidget is not
        // supported on this platform" and "No fbo, cannot render", so paintGL
        // never draws. Everything below drives the WIDGET layer and the slots
        // behind it; nothing here is evidence about what is on screen. The
        // oracle reads the rendered view on 1.3; this does not.
        auto named = [&](const char *n) -> QAbstractButton * {
            for (QAbstractButton *b : sw->findChildren<QAbstractButton *>())
                if (b->objectName() == QLatin1String(n)) return b;
            return nullptr;
        };
        const char *navNames[] = { "forwardPushButton", "backwardPushButton",
                                   "leftPushButton", "rightPushButton",
                                   "upPushButton", "downPushButton",
                                   "centerPushButton" };

        // --- the trace-robot coupling -------------------------------------
        // visualizeThis() ends with slotSetTraceRobot(true), and that slot
        // disables all seven navigation buttons (SIG_SimulationWidget.cpp:238-249):
        // tracing the robot owns the camera, so manual navigation is off. The
        // coupling is the observable, and it is checkable without rendering.
        printf("\n  -- trace-robot / manual-navigation coupling --\n");
        QCheckBox *trace = qobject_cast<QCheckBox *>(named("traceRobotCheckBox"));
        if (!trace) { printf("!! traceRobotCheckBox missing\n"); fflush(stdout); return 1; }
        int enabledBefore = 0;
        for (const char *n : navNames) if (QAbstractButton *b = named(n)) enabledBefore += b->isEnabled();
        QTest::mouseClick(trace, Qt::LeftButton, Qt::NoModifier,
                          QPoint(8, trace->height() / 2));
        QTest::qWait(200);
        int enabledAfter = 0;
        for (const char *n : navNames) if (QAbstractButton *b = named(n)) enabledAfter += b->isEnabled();
        printf("    traceRobot checked=%d -> %d ; nav buttons enabled %d/7 -> %d/7\n",
               1, trace->isChecked(), enabledBefore, enabledAfter);
        if (trace->isChecked() || enabledAfter != 7) {
            printf("!! unchecking Trace Robot did not enable the seven navigation"
                   " buttons (got %d/7)\n", enabledAfter);
            fflush(stdout); return 1;
        }

        // --- every navigation button, which is also the use-after-free path --
        // Each of these slots is one of the fourteen `if (visualisation)' sites
        // of PORTING.md's C11 slave-GUI notes. Clicking them all is the first time that code has
        // ever been executed in this port.
        printf("\n  -- navigation buttons (each dereferences `visualisation') --\n");
        for (const char *n : navNames) {
            QAbstractButton *b = named(n);
            if (!b) { printf("    %-22s MISSING\n", n); continue; }
            QTest::mouseClick(b, Qt::LeftButton, Qt::NoModifier, b->rect().center());
            QTest::qWait(60);
            printf("    %-22s clicked, survived; xyz=[%s|%s|%s]\n", n,
                   qPrintable(sw->findChild<QLabel *>("xPosTextLabel")->text()),
                   qPrintable(sw->findChild<QLabel *>("yPosTextLabel")->text()),
                   qPrintable(sw->findChild<QLabel *>("zPosTextLabel")->text()));
        }
        fflush(stdout);

        // --- the four remaining checkboxes, each a visualisation-> call ------
        printf("\n  -- view checkboxes --\n");
        for (const char *n : { "showAncorPointsCheckBox", "showRobotPathCheckBox",
                               "showGridCheckBox", "showPlaneCheckBox" }) {
            QCheckBox *cb = qobject_cast<QCheckBox *>(named(n));
            if (!cb) { printf("    %-26s MISSING\n", n); continue; }
            // THREE clicks with a generous wait, reporting checkState() rather
            // than isChecked(): a two-click probe with an 80 ms wait reported
            // showAncorPointsCheckBox stuck at checked, and that had to be told
            // apart from a slow slot swallowing the second event. The slot shows
            // one floating label per robot point, so it is not free.
            const int s0 = (int)cb->checkState();
            int seq[3];
            for (int k = 0; k < 3; ++k) {
                QTest::mouseClick(cb, Qt::LeftButton, Qt::NoModifier,
                                  QPoint(8, cb->height() / 2));
                QTest::qWait(400);
                seq[k] = (int)cb->checkState();
            }
            // showAncorPointsCheckBox is TRISTATE and cycles 0 -> 1 -> 2 -> 0
            // where the other three are plain 0 -> 2 -> 0. That is NOT a port
            // defect: `tristate=true' is on this one checkbox in the PRISTINE
            // 2003 form (vendor commit 0516d62) exactly as in the converted one.
            // Preserved, so the expectation is per-widget rather than uniform.
            // Its consequence is 1.3's too: the slot treats state>0 as points
            // visible, but SIG_VisualisationWidget only shows the floating
            // labels at state==2, so the middle state shows points without text.
            const bool tri = cb->isTristate();
            const bool ok = tri ? (seq[0] == 1 && seq[1] == 2 && seq[2] == 0)
                                : (seq[0] != s0 && seq[1] == s0 && seq[2] != s0);
            printf("    %-26s tristate=%d state %d -> %d -> %d -> %d%s\n", n,
                   tri, s0, seq[0], seq[1], seq[2],
                   ok ? "" : "   << UNEXPECTED CYCLE");
        }
        fflush(stdout);

        // --- the four sliders, by groove click ------------------------------
        // A groove click, not arrow keys: QTest posts keys straight at the
        // widget whether or not it has focus, and on 1.3 a groove click does not
        // focus a slider, so a key probe would measure something 1.3 never does.
        printf("\n  -- sliders, groove click --\n");
        // NOT probeSliderClick(): that one is written for the master's
        // horizontal sliders and always clicks (width-6, height/2). Two of
        // these four are VERTICAL, where that point is mid-groove -- and a
        // groove click that lands ON THE THUMB does nothing. The oracle
        // reproduced exactly the symptom seen here (a slider that looks dead)
        // and it was that artifact, so the thumb is located and avoided before
        // any conclusion is drawn.
        for (QSlider *sl : sw->findChildren<QSlider *>()) {
            const bool vert = sl->orientation() == Qt::Vertical;
            const int len = vert ? sl->height() : sl->width();
            // The CROSS axis must come from the groove rect, not from the middle
            // of the widget. A slider with tick marks on one side has its groove
            // off-centre: yawSlider is 20 tall with a groove at y 6..12, and the
            // widget middle, y=10, is outside the band that responds (3..9).
            // Clicking the middle made yaw and pitch look dead, and that was
            // written up as a port divergence before this was found. It was not.
            QStyleOptionSlider go;
            go.initFrom(sl);
            go.minimum = sl->minimum(); go.maximum = sl->maximum();
            go.sliderPosition = sl->value(); go.sliderValue = sl->value();
            go.orientation = sl->orientation(); go.pageStep = sl->pageStep();
            go.tickPosition = (QSlider::TickPosition)sl->tickPosition();
            go.tickInterval = sl->tickInterval();
            go.upsideDown = sl->invertedAppearance();
            const QRect grv = sl->style()->subControlRect(QStyle::CC_Slider, &go,
                                                          QStyle::SC_SliderGroove, sl);
            const int cross = vert ? grv.center().x() : grv.center().y();
            const int span = sl->maximum() - sl->minimum();
            const double frac = span ? double(sl->value() - sl->minimum()) / span : 0.5;
            // Click at whichever end is FARTHER from the thumb, so the press can
            // never land on it. Qt draws vertical sliders with the maximum at
            // the TOP, so the thumb's pixel fraction is inverted there.
            const double thumbFrac = vert ? 1.0 - frac : frac;
            const bool clickHigh = thumbFrac < 0.5;
            const int along = clickHigh ? int(len * 0.90) : int(len * 0.10);
            const QPoint pt = vert ? QPoint(cross, along) : QPoint(along, cross);
            const int start = sl->value();
            QTest::mouseClick(sl, Qt::LeftButton, Qt::NoModifier, pt);
            QTest::qWait(120);
            const int after = sl->value();
            printf("    %-24s %-10s %dx%d thumb@%.0f%% click@%.0f%% : %d -> %d"
                   " (delta %+d, pageStep %d)%s\n",
                   qPrintable(sl->objectName()), vert ? "vertical" : "horizontal",
                   sl->width(), sl->height(), thumbFrac * 100,
                   (clickHigh ? 90.0 : 10.0), start, after, after - start,
                   sl->pageStep(),
                   (after == start) ? "   << DID NOT MOVE" : "");
            sl->setValue(start);
        }
        // If a slider did not move, SWEEP it before concluding anything. The
        // handle rect comes from the style rather than from arithmetic, so
        // "the click missed the thumb" stops being an assumption.
        for (QSlider *sl : sw->findChildren<QSlider *>()) {
            const int start = sl->value();
            QStyleOptionSlider opt;
            opt.initFrom(sl);
            opt.minimum = sl->minimum(); opt.maximum = sl->maximum();
            opt.sliderPosition = sl->value(); opt.sliderValue = sl->value();
            opt.orientation = sl->orientation();
            opt.pageStep = sl->pageStep();
            opt.tickPosition = (QSlider::TickPosition)sl->tickPosition();
            opt.tickInterval = sl->tickInterval();
            opt.upsideDown = sl->invertedAppearance();
            const QRect h = sl->style()->subControlRect(QStyle::CC_Slider, &opt,
                                                        QStyle::SC_SliderHandle, sl);
            const QRect g = sl->style()->subControlRect(QStyle::CC_Slider, &opt,
                                                        QStyle::SC_SliderGroove, sl);
            QString moved;
            for (double f : { 0.05, 0.20, 0.35, 0.65, 0.80, 0.95 }) {
                const bool vert = sl->orientation() == Qt::Vertical;
                const int len = vert ? sl->height() : sl->width();
                const int cross = vert ? g.center().x() : g.center().y();
                const int along = int(len * f);
                const QPoint pt = vert ? QPoint(cross, along) : QPoint(along, cross);
                if (h.contains(pt)) { moved += QString("  %1:ONTHUMB").arg(f); continue; }
                sl->setValue(start);
                QTest::mouseClick(sl, Qt::LeftButton, Qt::NoModifier, pt);
                QTest::qWait(60);
                moved += QString("  %1:%2").arg(f).arg(sl->value() - start);
            }
            sl->setValue(start);
            // Keyboard and setValue as controls: if PageUp moves it but no
            // click does, the widget is live and only its MOUSE handling is
            // inert -- a different fault from a dead widget.
            sl->setValue(start);
            sl->setFocus();
            QTest::keyClick(sl, Qt::Key_PageUp);
            QTest::qWait(60);
            const int byKey = sl->value() - start;
            sl->setValue(start);
            sl->setValue(start + sl->pageStep());
            const int bySet = sl->value() - start;
            sl->setValue(start);
            printf("    sweep %-24s handle=(%d,%d %dx%d) groove=(%d,%d %dx%d)%s"
                   "  | PageUp %+d\n",
                   qPrintable(sl->objectName()), h.x(), h.y(), h.width(), h.height(),
                   g.x(), g.y(), g.width(), g.height(), qPrintable(moved), byKey);
        }
        fflush(stdout);

        // --- render mode and frame delay ------------------------------------
        printf("\n  -- render mode / frame delay --\n");
        if (QComboBox *cb = sw->findChild<QComboBox *>("renderModeComboBox")) {
            for (int i = 0; i < cb->count(); ++i) {
                cb->setCurrentIndex(i);
                QTest::qWait(80);
                printf("    renderMode -> %d [%s] survived\n", i, qPrintable(cb->itemText(i)));
            }
            cb->setCurrentIndex(0);
        }
        if (QSpinBox *sp = sw->findChild<QSpinBox *>("frameDelaySpinBox")) {
            sp->setFocus();
            QTest::keyClick(sp, Qt::Key_A, Qt::ControlModifier);
            QTest::keyClicks(sp, "250");
            QTest::qWait(80);
            printf("    frameDelay typed 250 -> value=%d\n", sp->value());
            // PUT IT BACK. Leaving 250 ms between frames and then watching Play
            // for four seconds gives ~16 steps, about 0.16 simulated seconds,
            // which displays as "0 secs" -- indistinguishable from a Play that
            // does nothing. That is exactly how this probe first read as a
            // divergence against the oracle's minutes-long run.
            sp->setFocus();
            QTest::keyClick(sp, Qt::Key_A, Qt::ControlModifier);
            QTest::keyClicks(sp, "0");
            QTest::qWait(80);
            printf("    frameDelay restored -> value=%d\n", sp->value());
        }
        fflush(stdout);

        // --- the toolbar actions --------------------------------------------
        // Play / Step / Stop actually advance the simulation, which is the one
        // thing here that produces a number rather than a state.
        printf("\n  -- toolbar actions --\n");
        QLabel *simTime = sw->findChild<QLabel *>("simulationTimeTextLabel");
        auto act = [&](const char *n) -> QAction * {
            for (QAction *a : sw->findChildren<QAction *>())
                if (a->objectName() == QLatin1String(n)) return a;
            return nullptr;
        };
        printf("    simulationTime before      [%s]\n",
               simTime ? qPrintable(simTime->text()) : "(no label)");
        if (QAction *a = act("stepAction")) {
            for (int i = 0; i < 3; ++i) { a->trigger(); QTest::qWait(400); }
            printf("    after 3x stepAction        [%s]\n",
                   simTime ? qPrintable(simTime->text()) : "(no label)");
        }
        if (QAction *a = act("playAction")) {
            a->trigger();
            // Sampled, not a single wait: the label is quantised to whole
            // seconds, so a run that IS advancing looks identical to a dead one
            // until the first second lands. Print the whole progression.
            for (int i = 1; i <= 6; ++i) {
                QTest::qWait(5000);
                printf("    play +%2ds                  [%s]\n", i * 5,
                       simTime ? qPrintable(simTime->text()) : "(no label)");
                fflush(stdout);
            }
        }
        if (QAction *a = act("stopAction")) {
            a->trigger(); QTest::qWait(600);
            printf("    after stopAction           [%s]\n",
                   simTime ? qPrintable(simTime->text()) : "(no label)");
        }
        fflush(stdout);

        // --- the movie settings dialog, never opened before ------------------
        printf("\n  -- movie settings dialog --\n");
        int movieClipped = 0;
        whenModal([&movieClipped](QWidget *m) {
            describeDialog(m);
            for (QSpinBox *sp : m->findChildren<QSpinBox *>())
                printf("      spin  [%-22s] value=%d min=%d max=%d\n",
                       qPrintable(sp->objectName()), sp->value(), sp->minimum(), sp->maximum());
            for (QComboBox *cb : m->findChildren<QComboBox *>()) {
                printf("      combo [%-22s] current=%d [", qPrintable(cb->objectName()),
                       cb->currentIndex());
                for (int i = 0; i < cb->count(); ++i)
                    printf("%s%s", i ? "|" : "", qPrintable(cb->itemText(i)));
                printf("]\n");
            }
            for (QLineEdit *le : m->findChildren<QLineEdit *>())
                printf("      edit  [%-22s] text=[%s] validator=%s\n",
                       qPrintable(le->objectName()), qPrintable(le->text()),
                       qPrintable(validatorDesc(le->validator())));
            // This dialog shipped with seven controls hidden behind a group box
            // that had collapsed. The widget dump above listed them all as
            // healthy either way, which is exactly why the containing rect has
            // to be checked as well.
            movieClipped = clippedWidgets(m, "movie settings dialog");
            clickDlgButton(m, "Cancel");
        });
        if (QAction *a = act("alterMovieSettingsAction")) { a->trigger(); QTest::qWait(1200); }
        cancelModalHandler();
        // --- DIAGNOSTIC: discriminate the two leads for the dead sliders ------
        // The oracle measured on 1.3 that slotMouseRotation DOES write back to
        // exactly yawSlider and pitchSlider, and that those same sliders still
        // page-step on a groove click. So being a write-back target is not by
        // itself incompatible with groove clicks, and the question becomes
        // whether THIS port's wiring swallows the events. Two cheap tests:
        //   1. cut every connection off yawSlider and click it again;
        //   2. build a bare QSlider carrying yawSlider's exact properties and
        //      click that -- if a fresh one works, the properties are cleared.
        // Deliberately LAST, so nothing above is measured through a mutation.
        printf("\n  -- diagnostic: why yaw/pitch ignore the mouse --\n");
        auto grooveClick = [&](QSlider *sl, const char *what) {
            const int start = sl->value();
            const bool vert = sl->orientation() == Qt::Vertical;
            const int len = vert ? sl->height() : sl->width();
            const int cross = (vert ? sl->width() : sl->height()) / 2;
            int moved = 0;
            for (double f : { 0.20, 0.35, 0.65, 0.80 }) {
                sl->setValue(start);
                const int along = int(len * f);
                QTest::mouseClick(sl, Qt::LeftButton, Qt::NoModifier,
                                  vert ? QPoint(cross, along) : QPoint(along, cross));
                QTest::qWait(60);
                if (sl->value() != start) ++moved;
            }
            sl->setValue(start);
            printf("    %-44s %d of 4 positions moved it\n", what, moved);
        };
        if (QSlider *yaw = sw->findChild<QSlider *>("yawSlider")) {
            grooveClick(yaw, "yawSlider as wired");
            QObject::disconnect(yaw, nullptr, nullptr, nullptr);
            grooveClick(yaw, "yawSlider with ALL connections cut (lead 1)");

            QSlider *fresh = new QSlider(yaw->orientation(), yaw->parentWidget());
            fresh->setMinimum(yaw->minimum());   fresh->setMaximum(yaw->maximum());
            fresh->setSingleStep(yaw->singleStep());
            fresh->setPageStep(yaw->pageStep());
            fresh->setValue(yaw->value());
            fresh->setTickPosition(yaw->tickPosition());
            fresh->setTickInterval(yaw->tickInterval());
            fresh->setTracking(yaw->hasTracking());
            fresh->setGeometry(yaw->geometry());
            fresh->show();
            QTest::qWait(150);
            grooveClick(fresh, "a FRESH QSlider with yaw's properties (lead 2)");
            delete fresh;
            // Every click so far used the middle of the slider's SHORT side.
            // That axis was never varied. A slider with tick marks puts its
            // groove off-centre, so the middle may miss it. Vary it now.
            QStyleOptionSlider o;
            o.initFrom(yaw);
            o.minimum = yaw->minimum(); o.maximum = yaw->maximum();
            o.sliderPosition = yaw->value(); o.sliderValue = yaw->value();
            o.orientation = yaw->orientation(); o.pageStep = yaw->pageStep();
            const QRect gr = yaw->style()->subControlRect(QStyle::CC_Slider, &o,
                                                          QStyle::SC_SliderGroove, yaw);
            printf("    yaw groove rect = (%d,%d %dx%d), widget %dx%d\n",
                   gr.x(), gr.y(), gr.width(), gr.height(), yaw->width(), yaw->height());
            const int start = yaw->value();
            QString row;
            for (int y = 1; y < yaw->height(); y += 2) {
                yaw->setValue(start);
                QTest::mouseClick(yaw, Qt::LeftButton, Qt::NoModifier,
                                  QPoint(int(yaw->width() * 0.80), y));
                QTest::qWait(40);
                if (yaw->value() != start) row += QString(" y=%1:%2").arg(y).arg(yaw->value() - start);
            }
            yaw->setValue(start);
            printf("    yaw, click at 80%% along, every y:%s\n",
                   row.isEmpty() ? "  NO y VALUE WORKED" : qPrintable(row));
        }
        if (QSlider *pit = sw->findChild<QSlider *>("pitchSlider")) {
            const int start = pit->value();
            QString row;
            for (int x = 1; x < pit->width(); x += 2) {
                pit->setValue(start);
                QTest::mouseClick(pit, Qt::LeftButton, Qt::NoModifier,
                                  QPoint(x, int(pit->height() * 0.80)));
                QTest::qWait(40);
                if (pit->value() != start) row += QString(" x=%1:%2").arg(x).arg(pit->value() - start);
            }
            pit->setValue(start);
            printf("    pitch, click at 80%% along, every x:%s\n",
                   row.isEmpty() ? "  NO x VALUE WORKED" : qPrintable(row));
        }
        fflush(stdout);

        printf("  -- slave window survived the whole battery --\n");
        if (swClipped || movieClipped) {
            printf("!! %d clipped control(s) in the slave window, %d in its dialog\n",
                   swClipped, movieClipped);
            fflush(stdout);
            return 1;
        }
        fflush(stdout);
        return 0;
    }

    // --- Start / Stop an evolution ----------------------------------------
    if (scenario == "evolution" || scenario == "pvmcrash") {
        // --- pvmcrash: PORTING.md 9's "the pvmTasks crash is untried on the
        // --- port". Everything below is the `evolution' scenario; the only
        // --- difference is that a timer armed just before Start opens MetaGP >
        // --- Configure System DURING the run, which is what kills 1.3.
        //
        // stdout is UNBUFFERED here and nowhere else. The expected outcome is
        // that the process dies -- QList::operator[] asserts where Qt 2's
        // QGVector printed a warning and segfaulted -- and a block-buffered
        // transcript of a run that aborts is lost entirely. The same trap the
        // termination assertion above hit, but here it is the whole point.
        const bool crashProbe = (scenario == "pvmcrash");
        if (crashProbe) setvbuf(stdout, nullptr, _IONBF, 0);
        SIG_ExperimentListView *lv = listView();

        // A generation is one evaluation per individual, each a full physics
        // run dispatched to a PVM slave. With the shipped 120 the counter does
        // not move inside any reasonable budget, and "nothing happened" then
        // looks identical to "Start is broken". Shrinking the pool through the
        // GUI first is what makes a generation observable.
        if (qgetenv("SIGEL_SMALL_POOL") == "1") {
            QTreeWidget *il = indList();
            if (il && il->topLevelItemCount() > 8) {
                clickRow(il, 8);
                clickRow(il, il->topLevelItemCount() - 1, Qt::ShiftModifier);
                whenModal([](QWidget *m) { clickMsgButton(m, QMessageBox::Yes); });
                clickMenu("&Individuals", "&Delete");
                QTest::qWait(6000);
                printf("  [pool shrunk to] %d\n", il->topLevelItemCount());
                fflush(stdout);
            }
        }

        // SIGEL_RESET_POOL=1 clears every stored fitness to -1 through the real
        // GUI path (Individuals > Reset -> SIG_GPPopulation::resetPool), so the
        // whole pool is re-evaluated ON THIS MACHINE. Without it the pool still
        // carries the 2003 i386 numbers for whichever individuals survive, and
        // any fitness curve drawn from it mixes two architectures and means
        // nothing -- which is exactly how a first attempt at this went wrong.
        if (qgetenv("SIGEL_RESET_POOL") == "1") {
            clickMenu("&View", "&Population");
            QTest::qWait(400);
            whenModal([](QWidget *m) { clickMsgButton(m, QMessageBox::Yes); });
            clickMenu("&Individuals", "Reset");
            QTest::qWait(3000);
            cancelModalHandler();
            if (SIG_GUIGPExperiment *ex = lv->currentlySelectedExperiment()) {
                int neg = 0, n = ex->gpExperiment.population.getSize();
                for (int i = 0; i < n; ++i)
                    if (ex->gpExperiment.population.getIndividual(i).getFitness() < 0) ++neg;
                printf("  [reset] %d of %d individuals now unevaluated\n", neg, n);
                if (neg != n) {
                    printf("!! reset did not clear the pool -- a curve from it would mix"
                           " 2003 and local fitness values\n");
                    fflush(stdout); return 1;
                }
            }
            fflush(stdout);
        }

        lv->setCurrentItem(lv->topLevelItem(0));
        QTest::qWait(400);
        step("experiment page, before Start", false, true, false, true);

        QStackedWidget *st = W->findChild<QStackedWidget *>();
        QPushButton *start = nullptr, *stop = nullptr;
        for (QPushButton *b : st->currentWidget()->findChildren<QPushButton *>()) {
            if (b->text() == "&Start") start = b;
            if (b->text() == "S&top")  stop  = b;
        }
        if (!start || !stop) { printf("!! no Start/Stop buttons on this page\n"); return 1; }

        // slotStartEvolution() is guarded by three conditions and does nothing
        // at all if any fails -- silently, with no dialog. Print them, so a
        // Start that appears to do nothing can be told apart from one that is
        // correctly declining.
        if (SIG_GUIGPExperiment *ex = lv->currentlySelectedExperiment()) {
            printf("  [start guard] bodies=%d (needs !=0)  population=%d (needs >=4)  "
                   "fitnessName=[%s] (needs non-empty)\n",
                   (int)ex->gpExperiment.robot.getBodies().size(),
                   (int)ex->gpExperiment.population.getSize(),
                   qPrintable(ex->gpExperiment.gpParameter.getFitnessName()));
        } else printf("  [start guard] no experiment selected\n");
        fflush(stdout);

        // EVERY shipped experiment terminates on a DATE in 2001, so Start
        // correctly runs and finishes at once. To see a running evolution the
        // termination has to be changed first -- through the GUI, by clicking
        // the "interprete as duration" radio and typing a duration, exactly as
        // a user would.
        // Terminate by GENERATION, not by duration. Every shipped experiment
        // terminates on a DATE in 2001, so a correct Start runs and finishes at
        // once -- MEASURED here rather than assumed: with the shipped
        // termination this scenario saw signalEvolutionNotRunning emit false
        // then true with the generation LCD unmoved at 136. A generation count
        // is the lever that gives a run which measurably starts, progresses and
        // stops, and it is the same lever the oracle's fifteen reference runs
        // were produced with -- TERMINATIONGENERATIONNO, PORTING.md's C11 evolution notes.
        // qEnvironmentVariableIntValue returns 0 for UNSET and for UNPARSEABLE
        // alike, so SIGEL_GENERATIONS=abc silently fell through to the old
        // date-terminated branch, saved nothing and still exited 0. Tell the
        // two apart and refuse the bad one.
        bool gensOk = true;
        const int wantGens = qEnvironmentVariableIntValue("SIGEL_GENERATIONS", &gensOk);
        if (!qEnvironmentVariableIsEmpty("SIGEL_GENERATIONS") && (!gensOk || wantGens <= 0)) {
            printf("!! SIGEL_GENERATIONS=[%s] is not a positive integer\n",
                   qPrintable(qEnvironmentVariable("SIGEL_GENERATIONS")));
            fflush(stdout);
            return 1;
        }
        // The pool generation BEFORE the run, so the assertion after it can say
        // the run actually advanced rather than that a file merely exists.
        int genBefore = -1;
        if (SIG_GUIGPExperiment *ex0 = lv->currentlySelectedExperiment())
            genBefore = ex0->gpExperiment.population.getPoolGeneration();
        if (wantGens > 0) {
            clickMenu("&View", "&GP Parameters");
            QTest::qWait(400);
            QWidget *gp = st->currentWidget();
            // The page REMEMBERS its tab and the View menu does not reset it,
            // so the tab is chosen BY NAME. "Evolution control" is the tab that
            // holds both the termination combo and the generation spin box
            // (SIG_GPParameterBase.ui:1162).
            QTabWidget *gptabs = gp ? gp->findChild<QTabWidget *>() : nullptr;
            const char *tabbed = "NOT FOUND";
            if (gptabs) for (int i = 0; i < gptabs->count(); ++i)
                if (gptabs->tabText(i) == "Evolution control") {
                    QTabBar *bar = gptabs->tabBar();
                    QTest::mouseClick(bar, Qt::LeftButton, Qt::NoModifier,
                                      bar->tabRect(i).center());
                    QTest::qWait(200);
                    tabbed = "Evolution control";
                    break;
                }
            QComboBox *term = gp ? gp->findChild<QComboBox *>("comboboxTerminationBy") : nullptr;
            QSpinBox  *gens = gp ? gp->findChild<QSpinBox  *>("spinboxByGenerationNumber") : nullptr;
            printf("  [termination] tab=%s combo=%s spin=%s\n", tabbed,
                   term ? "found" : "MISSING", gens ? "found" : "MISSING");
            if (!term || !gens) { printf("!! cannot set termination by generation\n"); return 1; }

            // The combo's items are User / Time / Generation / Time or
            // generation, which is NOT the enum order (byTime, byGeneration,
            // byTimeGeneration, byUser). Index 2 is the one that maps to
            // byGeneration -- SIGEL_MasterGUI/SIG_GPParameter.cpp:155-169.
            // setCurrentIndex rather than a popup click: QTest is not a mouse
            // and a combo popup is where that difference bites hardest.
            term->setCurrentIndex(2);
            QTest::qWait(150);
            gens->setFocus();
            QTest::keyClick(gens, Qt::Key_A, Qt::ControlModifier);
            QTest::keyClicks(gens, QString::number(wantGens));
            QTest::qWait(150);
            printf("  [termination] combo=[%s] index=%d  generations spin=%d\n",
                   qPrintable(term->currentText()), term->currentIndex(), gens->value());
            fflush(stdout);

            lv->setCurrentItem(lv->topLevelItem(0));
            QTest::qWait(400);
            // Widget values only matter once they are IN the experiment.
            // Selecting the experiment runs putAllIntoExperiment()
            // (SIG_ExperimentListView.cpp, slotSaveExperiment) and slotStartEvolution() runs it
            // again, so read the experiment back rather than trusting the
            // widget: 1 is byGeneration.
            if (SIG_GUIGPExperiment *ex = lv->currentlySelectedExperiment()) {
                const int gotModel = (int)ex->gpExperiment.gpParameter.getTerminationModel();
                const int gotGens  = ex->gpExperiment.gpParameter.getTerminationGenerationNo();
                printf("  [termination in experiment] model=%d (1=byGeneration) generationNo=%d\n",
                       gotModel, gotGens);
                // ASSERT rather than merely print. If the widget values did not
                // reach the experiment the run terminates instantly, the counts
                // come out wrong, and a scenario that only printed would have
                // produced a plausible-looking artefact to diff. Fail here, where
                // the reason is still visible.
                if (gotModel != 1 || gotGens != wantGens) {
                    printf("!! termination did not reach the experiment: wanted "
                           "model=1 generationNo=%d\n", wantGens);
                    // Flush BEFORE returning, not after the block. stdout is
                    // block-buffered to a file and tearDownPvm()'s pvm_halt()
                    // blocks in select() for ever, so the process never reaches
                    // exit and never flushes on its own -- an assertion whose
                    // message is lost is worse than no assertion. Measured: this
                    // exact message vanished until the flush moved up here.
                    fflush(stdout);
                    return 1;
                }
            } else { printf("!! no experiment selected\n"); fflush(stdout); return 1; }
            fflush(stdout);
            st = W->findChild<QStackedWidget *>();
            start = stop = nullptr;
            for (QPushButton *b : st->currentWidget()->findChildren<QPushButton *>()) {
                if (b->text() == "&Start") start = b;
                if (b->text() == "S&top")  stop  = b;
            }
            if (!start || !stop) { printf("!! lost Start/Stop\n"); return 1; }
        } else if (qgetenv("SIGEL_RUN_LONGER") == "1") {
            clickMenu("&View", "&GP Parameters");
            QTest::qWait(400);
            QWidget *gp = st->currentWidget();
            QRadioButton *dur = nullptr;
            for (QRadioButton *rb : gp->findChildren<QRadioButton *>())
                if (rb->objectName() == "radiobuttonInterpreteAsDuration") dur = rb;
            QSpinBox *mins = gp->findChild<QSpinBox *>("spinboxByDurationMins");
            printf("  [termination] durationRadio=%s minsSpin=%s\n",
                   dur ? "found" : "MISSING", mins ? "found" : "MISSING");
            if (dur) {
                QTest::mouseClick(dur, Qt::LeftButton, Qt::NoModifier,
                                  QPoint(8, dur->height() / 2));
                QTest::qWait(200);
                printf("  [termination] duration radio checked=%d\n", dur->isChecked());
            }
            if (mins) {
                mins->setFocus();
                QTest::keyClick(mins, Qt::Key_A, Qt::ControlModifier);
                QTest::keyClicks(mins, "5");
                QTest::qWait(150);
                printf("  [termination] minutes spin now %d\n", mins->value());
            }
            fflush(stdout);
            lv->setCurrentItem(lv->topLevelItem(0));
            QTest::qWait(400);
            st = W->findChild<QStackedWidget *>();
            start = stop = nullptr;
            for (QPushButton *b : st->currentWidget()->findChildren<QPushButton *>()) {
                if (b->text() == "&Start") start = b;
                if (b->text() == "S&top")  stop  = b;
            }
            if (!start || !stop) { printf("!! lost Start/Stop\n"); return 1; }
        }

        printf("  [start button] geom=%dx%d+%d+%d visible=%d enabled=%d isDown=%d\n",
               start->width(), start->height(), start->x(), start->y(),
               start->isVisible(), start->isEnabled(),
               start->isDown());
        // THE PREREQUISITE THIS SCENARIO WAS MISSING. Configure System is
        // GREYED until Use MetaGP is on, so the injected click landed on a
        // dead menu item and the run proved nothing. The control run said so
        // in its own output -- "[action changes] &MetaGP/&Configure System
        // greyed" -- and it was read as a clean pass. The oracle's 1.3 crash
        // runs enable MetaGP BEFORE Start, so this also matches the sequence
        // being reproduced rather than merely unblocking the click.
        //
        // Enabling raises no dialog. The information box with three custom
        // buttons appears on DISABLE only -- the metagui scenario covers it.
        if (crashProbe) {
            clickMenu("&MetaGP", "&Use MetaGP");
            QTest::qWait(400);
            bool cfgFound = false, cfgEnabled = false;
            for (QAction *a : W->findChildren<QAction *>())
                if (a->text() == "&Configure System") {
                    cfgFound = true;
                    cfgEnabled = a->isEnabled();
                }
            printf("  [metagp] Use MetaGP clicked; Configure System found=%d enabled=%d\n",
                   (int)cfgFound, (int)cfgEnabled);
            // Fail HERE, not silently later. An injection into a greyed action
            // produces a run that looks like a clean survival, which is the
            // one outcome this scenario must never manufacture.
            if (!cfgFound || !cfgEnabled) {
                printf("!! Configure System is not enabled, so the injected click would\n"
                       "!! hit a dead menu item and this run would prove NOTHING.\n"
                       "!! A missing stdConf.mt in SIGEL_ROOT is the usual cause; the\n"
                       "!! oracle reports that its absence gives a silent permanent\n"
                       "!! stall rather than the crash, so check the file before\n"
                       "!! concluding the crash does not reproduce.\n");
                fflush(stdout);
                return 1;
            }
            fflush(stdout);
        }
        QSignalSpy spy(start, SIGNAL(clicked()));
        // The decisive question is not whether the button works but whether
        // slotStartEvolution ran and DECLINED. This signal is emitted false on
        // start and true when the run ends, so a start-then-immediately-finish
        // is visible even if the buttons are back to their resting state by the
        // time anything samples them.
        SIG_GUIGPExperiment *exp = lv->currentlySelectedExperiment();
        QSignalSpy *evo = exp ? new QSignalSpy(exp, SIGNAL(signalEvolutionNotRunning(bool)))
                              : nullptr;
        // slotStartEvolution() BLOCKS. It calls guiGPManager->start(), which runs
        // the whole evolution inline and returns only when it has stopped
        // (SIG_GUIGPExperiment.cpp, slotRightClick, with slotEvolutionStopped() on the next
        // line). The loop stays responsive only because
        // SIG_GUIGPManager::haveABreak() calls qApp->processEvents(), so a
        // timer armed BEFORE the click fires DURING the run. The sampling loop
        // further down runs after start() has returned and can therefore only
        // ever see the finished state -- which is why a generation counter that
        // moves has to be caught from here.
        QElapsedTimer runClock;
        int lastGen = -1, samples = 0;
        QTimer sampler;
        QObject::connect(&sampler, &QTimer::timeout, [&]() {
            QWidget *cw = st->currentWidget();
            int gen = -1;
            for (QLCDNumber *l : cw->findChildren<QLCDNumber *>())
                if (l->objectName() == "lcdnumberGenerations") gen = (int)l->value();
            ++samples;
            // Print every sample for the first few, then only on a CHANGE, so a
            // long run does not bury the transitions it exists to show.
            if (samples <= 3 || gen != lastGen) {
                printf("  [run t+%5llds] generations=%d Start=%s Stop=%s\n",
                       (long long)(runClock.elapsed() / 1000), gen,
                       start->isEnabled() ? "enabled" : "GREYED",
                       stop->isEnabled()  ? "ENABLED" : "greyed");
                fflush(stdout);
            }
            lastGen = gen;
            // SIGEL's own messages are buffered in a QTextStream; without this
            // they appear only at a clean exit, out of order with everything
            // above, and not at all if the run is killed or aborts.
            if (crashProbe) flushSigelStreams();
        });
        sampler.start(2000);

        // THE INJECTED EVENT. slotStartEvolution() blocks for the whole run and
        // the loop stays responsive only through SIG_GUIGPManager::haveABreak()'s
        // processEvents(), so a single-shot timer armed HERE fires from inside
        // the running evolution -- which is the only way to reach the unchecked
        // pvmTasks[ taskId ] read in checkTask() the way a user's click does.
        //
        // SIGEL_CRASH_AT_MS=0 is the CONTROL: the identical run with no injected
        // event. Without it a crash proves only that the run crashed, not that
        // opening the window is what did it -- the mistake that made the oracle
        // withdraw "MetaGP crashes 1.3" and then "mid-run GUI interaction
        // crashes 1.3", both generalised over a factor that moved with the
        // trigger.
        QTimer inject;
        // Whether the timer FIRED, not whether it was armed. Deriving the
        // verdict from crashAtMs > 0 reported "SURVIVED WITH the event
        // injected" for a run that finished before the timer, or whose event
        // loop never pumped -- which collapses the injected cell into the
        // control cell and destroys the only thing the pair is for.
        bool injected = false;
        // -1 never sampled, 0 greyed, 1 live. The whole outcome of this
        // scenario turns on this one value, so it is recorded at the moment of
        // the click rather than inferred afterwards from a menu that has since
        // been re-enabled by the run ending.
        int cfgEnabledDuringRun = -1;
        // Same three states, sampled again AFTER a tree click. D29 greys the
        // MetaGP actions from SIG_GUIGPExperiment's signalEvolutionNotRunning, but
        // SIG_ExperimentListView::slotSelectionChanged emits actExpChanged() on
        // the very next line and SIG_MainWindow::slotActExpChanged
        // re-enables mtConfigureAction with NO run check at all. So
        // one click in the tree is expected to hand the crash path straight
        // back. Found by review; the scenario sampled only before the click and
        // could not see it.
        int cfgAfterTreeClick = -1;
        // D29's ARMING LINE, and this is the only thing that reaches it.
        // SIG_ExperimentListView::slotSelectionChanged emits
        // evolutionNotRunning( !isRunning() ), and isRunning() is true only
        // after `evolutionRunning = true;' in slotStartEvolution. Delete that line
        // and a tree click mid-run emits TRUE and hands back all 27 locked
        // actions -- 23 appended at SIG_MainWindow.cpp:571-593 plus the four
        // MetaGP ones at :685-688; `evolutionRunningActions.append' appears 27
        // times. `&Save Experiment' is one of the 23 non-MetaGP ones and,
        // unlike the MetaGP four, nothing re-enables it afterwards -- so it
        // reports the arming line and nothing else.
        //
        // The FIRST greying is NOT the arming line: slotStartEvolution
        // emits signalEvolutionNotRunning(false) BEFORE it sets
        // evolutionRunning, so a run with the arming line deleted still
        // greys everything at Start. An earlier version of this scenario
        // claimed the before-sample gated D29's arming line; it did not.
        // Found by review.
        int saveAfterTreeClick = -1;
        const int crashAtMs = qEnvironmentVariableIntValue("SIGEL_CRASH_AT_MS");
        if (crashProbe && crashAtMs > 0) {
            inject.setSingleShot(true);
            QObject::connect(&inject, &QTimer::timeout, [&]() {
                injected = true;
                printf("\n  >> INJECTING MetaGP > Configure System, %d ms into the run\n",
                       crashAtMs);
                // Leave the MetaGP window OPEN, exactly as the oracle's runs did.
                // A handler that closes what it finds closes MT_MainWindow itself
                // -- the mistake C12 records three wrong conclusions from.
                whenModal([](QWidget *m) {
                    printf("  [modal during run] %s [%s]\n",
                           m->metaObject()->className(), qPrintable(m->windowTitle()));
                    if (qobject_cast<QMessageBox *>(m)) { describeMessageBox(m); m->close(); }
                    else printf("    (left open)\n");
                }, 8000);
                // The BEFORE sample. It does NOT reach D29's arming line --
                // SIG_MainWindow.cpp:685-688 puts the four MetaGP actions into
                // evolutionRunningActions and slotStartEvolution greys them
                // at Start, before evolutionRunning is set. Only the
                // after-tree-click sample below reaches the arming line. An
                // earlier version of this comment claimed otherwise, 25 lines
                // from the paragraph withdrawing it. Found by review.
                auto sampleCfg = [&]() {
                    int v = -1;
                    for (QAction *a : W->findChildren<QAction *>())
                        if (a->text() == "&Configure System") { v = a->isEnabled() ? 1 : 0; break; }
                    return v;
                };
                cfgEnabledDuringRun = sampleCfg();
                printf("  [d29] Configure System during the run: enabled=%d"
                       "  (0 means D29's guard is holding)\n", cfgEnabledDuringRun);

                // NOW THE TREE CLICK. currentItemChanged is what reaches
                // slotSelectionChanged, so the current item has to actually
                // CHANGE -- setCurrentItem on the item that is already current
                // emits nothing and would have silently proved the opposite of
                // what this measures.
                QTreeWidgetItem *cur = lv->currentItem();
                QTreeWidgetItem *other = nullptr;
                for (int i = 0; i < lv->topLevelItemCount() && !other; ++i) {
                    QTreeWidgetItem *t = lv->topLevelItem(i);
                    if (t != cur) other = t;
                    for (int j = 0; j < t->childCount() && !other; ++j)
                        if (t->child(j) != cur) other = t->child(j);
                }
                if (!other) {
                    printf("  [d29] no second tree item to click -- cannot test the"
                           " re-enable path in this run\n");
                } else {
                    lv->setCurrentItem(other);
                    QTest::qWait(300);
                    cfgAfterTreeClick = sampleCfg();
                    for (QAction *a : W->findChildren<QAction *>())
                        if (a->text() == "&Save Experiment") {
                            saveAfterTreeClick = a->isEnabled() ? 1 : 0; break;
                        }
                    printf("  [d29] after ONE tree click ([%s]): Configure System"
                           " enabled=%d (1 = the guard was undone mid-run),"
                           " Save Experiment enabled=%d (0 = the arming line held)\n",
                           qPrintable(other->text(0)), cfgAfterTreeClick,
                           saveAfterTreeClick);
                }
                fflush(stdout);

                // Only click when it is live. clickMenu prints `!!' on a greyed
                // item, and check.sh treats `!!' as "this run must not pass" --
                // so the DESIGNED-SUCCESS outcome used to be required to emit
                // the repo's own failure marker. Found by review.
                const int live = (cfgAfterTreeClick >= 0) ? cfgAfterTreeClick
                                                          : cfgEnabledDuringRun;
                // THE THIRD CELL. The injected run does TWO things -- a tree
                // click and a Configure System click -- so on its own it cannot
                // say which one killed the process. SIGEL_TREE_ONLY=1 does the
                // tree click and stops, which is the missing cell. Without it
                // this scenario commits the exact error its own control cell
                // exists to prevent. Found by review.
                if (qgetenv("SIGEL_TREE_ONLY") == "1") {
                    printf("  >> TREE-CLICK-ONLY CELL: not clicking Configure System.\n"
                           "  >> If this run survives, the tree click alone is not what"
                           " kills it.\n");
                } else if (live == 1) {
                    printf("  >> Configure System IS live -- clicking it, which is"
                           " what kills 1.3\n");
                    fflush(stdout);
                    clickMenu("&MetaGP", "&Configure System");
                    printf("  >> the click returned; the process is still alive\n");
                } else {
                    printf("  >> Configure System is greyed, so it is NOT clicked:"
                           " a click on a dead menu item would prove nothing\n");
                }
            });
            inject.start(crashAtMs);
            printf("  [inject] armed for t+%d ms\n", crashAtMs);
        } else if (crashProbe) {
            printf("  [inject] CONTROL RUN -- no event will be injected\n");
        }
        fflush(stdout);

        runClock.start();
        printf("\n  >> clicking Start\n"); fflush(stdout);
        QTest::mouseClick(start, Qt::LeftButton, Qt::NoModifier, start->rect().center());
        if (crashProbe) {
            inject.stop();
            flushSigelStreams();
            if (crashAtMs > 0 && !injected)
                printf("\n  !! the injection timer never fired -- this run is NOT the"
                       " injected cell, whatever else it shows\n");
            printf("  >> SURVIVED: Start returned normally%s\n",
                   crashAtMs <= 0 ? " (control run)"
                                  : (injected ? " WITH the event injected"
                                              : " with NOTHING injected"));
            flushSigelStreams();
        }
        const qint64 runMs = runClock.elapsed();
        sampler.stop();
        // THE ANSWER TO 9's "the pvmTasks crash is untried on the port".
        //
        // WHAT THE ABORT IS, and it is NOT a plain out-of-range index.
        // MT_Controller::configureSystem does, three lines apart:
        //     mainWindow->show();  delete substitution;  substitution = 0;
        // (MT_Controller.cpp:402-404). `substitution' is the MT_Evaluator, and
        // MT_Evaluator inherits SIG_GPFitnessTrainer -- so when the meta system
        // is the Evaluator (stdConf.mt ships usedSystem=1 = EVALUATOR_SUBST),
        // SIG_GPManager's `trainer' IS that object (SIG_GPManager.cpp:59-60,
        // via MT_Controller::getFitnessTrainer which returns `substitution').
        // Opening the window mid-run therefore DELETES THE TRAINER THE RUNNING
        // LOOP IS HOLDING, and the next trainer->checkTask() reads a freed
        // QList whose header is garbage. Bounds-checking the read would fix
        // nothing. The same two lines are in the pristine 1.3 tarball, so this
        // explains the oracle's crashes too. Found by review.
        //
        // WHICH container asserts is NOT identified. Qt 6 collapsed Qt 2's
        // QArray and QGVector into QList, so MT_ResultBuffer,
        // NumOfCorrectEstimation and NumOfMetaEstimation -- the three the 1.3
        // analysis excluded BY TYPE -- now emit the identical message, and the
        // port's assert carries no index. Do not cite a line number for it.
        // 1.3 dies opening MTMainWindow during a run -- `QGVector::operator[]:
        // Index 359 out of range' then its own SIGSEGV handler's `Invalid
        // storage access', four observations by the oracle, the cleanest with
        // MetaGP set BEFORE Start and one injected click. Which container
        // asserts is NOT identified here -- see the note above.
        //
        // D29 greys the four MetaGP actions for the duration of a run, so the
        // click is refused -- UNTIL one click in the experiment tree, which
        // emits actExpChanged() (SIG_ExperimentListView::slotSelectionChanged) into
        // SIG_MainWindow::slotActExpChanged(), which re-enables
        // mtConfigureAction with no run check. That is what the second sample
        // is for. An earlier version of this scenario sampled only before the
        // tree click and reported the door closed. Found by review.
        if (crashProbe && crashAtMs > 0) {
            if (cfgEnabledDuringRun == 1) {
                printf("\n!! D29 DID NOT HOLD AT ALL: Configure System was live during"
                       " the run before any tree click.\n");
                fflush(stdout); return 1;
            }
            if (cfgEnabledDuringRun == -1) {
                printf("\n!! the injection never sampled Configure System, so this run"
                       " says nothing about D29 either way.\n");
                fflush(stdout); return 1;
            }
            if (cfgAfterTreeClick == 1) {
                printf("\n!! D29 IS INCOMPLETE: Configure System was greyed during the"
                       " run and ONE TREE CLICK made it live again.\n"
                       "!! The 1.3 crash path is OPEN on this port by that route.\n"
                       "!! SIG_MainWindow::slotActExpChanged has no run"
                       " check; SIG_ExperimentListView::slotSelectionChanged emits into it.\n");
                fflush(stdout); return 1;
            }
            // The arming line, reported separately because it is a different
            // guard from the one the MetaGP actions need.
            if (saveAfterTreeClick == 1) {
                printf("\n!! D29's ARMING LINE DID NOT HOLD: one tree click re-enabled"
                       " Save Experiment mid-run, so isRunning() was false"
                       " while an evolution was running.\n");
                fflush(stdout); return 1;
            }
            // Only claim the arming line held when the click is PROVEN to have
            // landed. Save Experiment was already greyed at Start, so 0 on its
            // own is equally consistent with a click that reached nothing. The
            // proof is cfgAfterTreeClick flipping 0 -> 1: actExpChanged() has
            // exactly one emit site (SIG_ExperimentListView::slotSelectionChanged), one line
            // after the emit that reaches the arming line, so the flip cannot
            // happen without the evolutionNotRunning emit having run. Found by review.
            if (saveAfterTreeClick == 0 && cfgAfterTreeClick == 1)
                printf("  >> D29's arming line HELD: the tree click provably landed"
                       " (Configure System flipped 0->1, which only"
                       " SIG_ExperimentListView::slotSelectionChanged can do) and it left the 23"
                       " non-MetaGP locked actions greyed.\n");
            else if (saveAfterTreeClick == 0)
                printf("  >> Save Experiment is greyed, but nothing here proves the"
                       " tree click landed, so this says NOTHING about the arming"
                       " line.\n");
            if (cfgAfterTreeClick == 0)
                printf("\n  >> D29 HELD THROUGH A TREE CLICK: Configure System was"
                       " greyed during the run and stayed greyed after one tree"
                       " click, so the click that kills 1.3 is refused.\n"
                       "  >> The fault behind it is untouched.\n");
            else
                printf("\n  >> D29 held before the tree click; the tree-click path was"
                       " NOT exercised in this run, so it is not answered here.\n");
        }

        // The cost of a generation ON THIS MACHINE. PORTING.md 9 records the
        // oracle's ~4.0 min/generation as a measurement of ONE run on 2003
        // i386 hardware at an unrecorded slave count -- the oracle then
        // measured that figure moving 3.2x with slave count alone, so it
        // predicts nothing here. This line is this machine's own number.
        // Only when the generations were actually COMPLETED. A run the
        // watchdog killed mid-generation still divides cleanly and prints a
        // per-generation cost for generations that never finished -- a made-up
        // number in an artefact people read for timings. Found by review.
        if (wantGens > 0)
            printf("  [throughput] Start returned after %lld ms for %d generation(s)"
                   " = %lld ms/generation, %d samples taken during the run"
                   " -- valid ONLY if the artefact check below reports"
                   " POOLGENERATION advancing by %d\n",
                   (long long)runMs, wantGens, (long long)(runMs / wantGens),
                   samples, wantGens);
        fflush(stdout);
        // Sample FAST: this experiment terminates on a DATE that is long past,
        // so a correct Start can run to completion and re-enable itself well
        // inside a 5-second sampling gap. A slow poll cannot tell that apart
        // from a Start that never fired.
        for (int i = 0; i < 50; ++i) {
            QTest::qWait(100);
            bool se = start->isEnabled(), pe = stop->isEnabled();
            static bool first = true;
            if (first || !se || pe) {
                printf("  [t+%4dms] Start=%s Stop=%s clicked=%d\n", (i + 1) * 100,
                       se ? "enabled" : "GREYED", pe ? "ENABLED" : "greyed",
                       (int)spy.count());
                fflush(stdout);
                first = false;
            }
        }
        printf("  [clicked() emitted %d time(s)]\n", (int)spy.count());
        // Everything below used to be PRINTED and never asserted, so a dead
        // Start button produced a full, plausible-looking transcript and exit 0.
        if (wantGens > 0) {
            if (spy.count() != 1) {
                printf("!! Start emitted clicked() %d times, expected 1 --"
                       " the click path is broken\n", (int)spy.count());
                fflush(stdout); return 1;
            }
            if (samples == 0) {
                printf("!! the in-run sampler never fired -- Start did not block,"
                       " so no evolution ran\n");
                fflush(stdout); return 1;
            }
            if (runMs < 1000) {
                printf("!! Start returned in %lld ms; a generation costs minutes."
                       " Nothing was evaluated.\n", (long long)runMs);
                fflush(stdout); return 1;
            }
        }
        if (evo) {
            printf("  [signalEvolutionNotRunning emitted %d time(s)]", (int)evo->count());
            for (int i = 0; i < evo->count(); ++i)
                printf(" %s", evo->at(i).at(0).toBool() ? "true" : "false");
            printf("\n");
        }
        fflush(stdout);

        // Distinguish a dead connect from a guard that declines: invoke the
        // slot directly on the same object the button is connected to. If THIS
        // greys Start, the click path is broken; if it does not, the slot is
        // running and choosing to do nothing.
        // C10 added this to tell a dead connect from a guard that declines. A
        // run that has just moved the generation counter has already answered
        // that, and invoking the slot again starts a SECOND evolution over the
        // population this scenario exists to compare -- so it is skipped
        // whenever a generation count was asked for.
        if (SIG_GUIGPExperiment *ex = wantGens > 0 ? nullptr
                                              : lv->currentlySelectedExperiment()) {
            bool ok = QMetaObject::invokeMethod(ex, "slotStartEvolution",
                                                Qt::DirectConnection);
            QTest::qWait(600);
            printf("  [direct invoke] accepted=%d -> Start=%s Stop=%s\n", ok,
                   start->isEnabled() ? "enabled" : "GREYED",
                   stop->isEnabled()  ? "ENABLED" : "greyed");
            fflush(stdout);
        }
        for (int i = 1; i <= 6; ++i) {
            QTest::qWait(5000);
            printf("  [t+%2ds] Start=%s Stop=%s", i * 5,
                   start->isEnabled() ? "enabled" : "greyed",
                   stop->isEnabled()  ? "enabled" : "greyed");
            for (QLCDNumber *l : st->currentWidget()->findChildren<QLCDNumber *>())
                printf("  %s=%g", qPrintable(l->objectName()), l->value());
            for (QProgressBar *pb : st->currentWidget()->findChildren<QProgressBar *>())
                printf("  progress=%d", pb->value());
            printf("\n"); fflush(stdout);
        }
        step("while the evolution is running", false, false, false, true);

        printf("\n  >> clicking Stop\n"); fflush(stdout);
        QTest::mouseClick(stop, Qt::LeftButton, Qt::NoModifier, stop->rect().center());
        QTest::qWait(8000);
        step("after Stop", false, true, false, true);
        clickMenu("&View", "&Population");
        QTest::qWait(500);
        step("population after the evolution stopped", false, false, true);

        // Write the evolved experiment out. Without this the run leaves nothing
        // to compare: PORTING.md's C11 evolution notes wants the port driven from the same
        // input as the oracle's reference runs and its OUTPUT diffed
        // structurally -- individuals, names, program text, ordering, file
        // shape, never fitness (D26: a 1-ULP change in start height moves
        // fitness 45%, and that box is i386/x87 against this one's aarch64).
        if (wantGens > 0) {
            const QString evolved = scratch() + "/evolved.exp";
            QFile::remove(evolved);
            whenModal([evolved](QWidget *m) {
                QFileDialog *fd = qobject_cast<QFileDialog *>(m);
                if (!fd) { printf("  !! save modal is not a QFileDialog\n"); m->close(); return; }
                acceptFileDialog(fd, evolved);   // never hand-roll a second one
            });
            clickMenu("&File", "&Save Experiment");
            QTest::qWait(8000);
            printf("  [evolved saved] exists=%d size=%lld path=%s\n",
                   QFile::exists(evolved), QFileInfo(evolved).size(),
                   qPrintable(evolved));
            // A run costs minutes; a silently unwritten artefact would waste all
            // of them and look like a comparison that simply had nothing to say.
            if (!QFile::exists(evolved) || QFileInfo(evolved).size() == 0) {
                printf("!! the evolved experiment was not written -- nothing to diff\n");
                fflush(stdout);
                return 1;
            }
            // Read the artefact back and prove the run is IN it. `exists and
            // non-empty' passed for a file that was simply the unevolved
            // experiment saved again.
            QFile f(evolved);
            int gotGen = -1, fitCount = 0, zeroFit = 0;
            if (f.open(QIODevice::ReadOnly)) {
                QTextStream in(&f);
                QString line;
                while (!(line = in.readLine()).isNull()) {
                    if (line.contains("POOLGENERATION=")) {
                        const QString v = line.section('=', 1).section(';', 0, 0).trimmed();
                        if (gotGen < 0) gotGen = v.toInt();
                    } else if (line.contains("FITNESS=")) {
                        ++fitCount;
                        if (line.section('=', 1).section(';', 0, 0).trimmed().toDouble() == 0.0)
                            ++zeroFit;
                    }
                }
            }
            printf("  [artefact] POOLGENERATION=%d (was %d, +%d expected)"
                   "  fitness values=%d of which zero=%d\n",
                   gotGen, genBefore, wantGens, fitCount, zeroFit);
            if (genBefore >= 0 && gotGen != genBefore + wantGens) {
                printf("!! the saved pool generation did not advance by %d --"
                       " the file does not contain the run\n", wantGens);
                fflush(stdout); return 1;
            }
            // NOT a fitness comparison -- a liveness one. SIG_Simulation.cpp:66-70
            // records that under PVM a throw is swallowed by sigel_slave and the
            // individual "scores 0.0 as though it had been evaluated". A whole
            // population of exact zeros is that failure, and it is WORSE than a
            // crash: every `var1 >= var2' is then true, so the run is perfectly
            // deterministic and its output looks like a clean comparison.
            if (fitCount > 0 && zeroFit == fitCount) {
                printf("!! every one of the %d fitness values is exactly 0.0 --"
                       " no individual was really evaluated (swallowed throw?)\n", fitCount);
                fflush(stdout); return 1;
            }
            fflush(stdout);
        }
        fflush(stdout);
        return 0;
    }

    // --- Individuals > Visualize: what the GUI hands the evaluator ---------
    if (scenario == "visualize") {
        QTreeWidget *t = indList();
        if (!t) { printf("!! no individuals list\n"); return 1; }
        // Pick a NAMED individual so the payload is reproducible and so the
        // oracle can be asked about the same one.
        int want = -1;
        for (int i = 0; i < t->topLevelItemCount(); ++i)
            if (t->topLevelItem(i)->text(0) == "55658") { want = i; break; }
        if (want < 0) { printf("!! individual 55658 not in the list\n"); return 1; }
        clickRow(t, want);
        printf("\n  [selected] row=%d name=%s fitness=%s age=%s (selected=%d)\n", want,
               qPrintable(t->topLevelItem(want)->text(0)),
               qPrintable(t->topLevelItem(want)->text(1)),
               qPrintable(t->topLevelItem(want)->text(2)),
               (int)t->selectedItems().count());
        fflush(stdout);

        whenModal([](QWidget *m) {
            printf("  [modal during visualize] %s [%s]\n",
                   m->metaObject()->className(), qPrintable(m->windowTitle()));
            describeMessageBox(m);
            m->close();
        }, 6000);
        clickMenu("&Individuals", "&Visualize");
        QTest::qWait(6000);
        printf("  [visualize returned]\n");
        return 0;
    }

    if (scenario == "sort") {
        SIG_AllIndividualsView *v = W->findChild<SIG_AllIndividualsView *>();
        QTreeWidget *t = v ? v->findChild<QTreeWidget *>() : nullptr;
        if (t) {
            QHeaderView *h = t->header();
            printf("\n  [header] sortIndicatorShown=%d clickable=%d sortingEnabled=%d\n",
                   h->isSortIndicatorShown(), h->sectionsClickable(), t->isSortingEnabled());
            // viewport coordinates: sectionPosition() is in header space and only
            // agrees while nothing scrolls horizontally.
            int x = h->sectionViewportPosition(1) + h->sectionSize(1) / 2;
            QTest::mouseClick(h->viewport(), Qt::LeftButton, Qt::NoModifier,
                              QPoint(x, h->height() / 2));
            QTest::qWait(400);
            printf("  [after 1 click on Fitness] indicatorSection=%d order=%s\n",
                   h->sortIndicatorSection(),
                   h->sortIndicatorOrder() == Qt::AscendingOrder ? "Ascending" : "Descending");
            step("after clicking Fitness header once", false, false, true);
            {   int n = t->topLevelItemCount();
                printf("  [last three]\n");
                for (int i = n - 3; i < n; ++i)
                    printf("    row%-3d | %-12s | %-14s | %-6s\n", i,
                           qPrintable(t->topLevelItem(i)->text(0)),
                           qPrintable(t->topLevelItem(i)->text(1)),
                           qPrintable(t->topLevelItem(i)->text(2)));
                fflush(stdout);
            }
            QTest::mouseClick(h->viewport(), Qt::LeftButton, Qt::NoModifier,
                              QPoint(x, h->height() / 2));
            QTest::qWait(400);
            printf("  [after 2 clicks on Fitness] order=%s\n",
                   h->sortIndicatorOrder() == Qt::AscendingOrder ? "Ascending" : "Descending");
            step("after clicking Fitness header twice", false, false, true);
        }
        return 0;
    }

    // ---------------------------------------------------------------- xtest
    // Real XTEST input, on the port, for the first time. Everything printed
    // here is either a real click's effect or the control that proves the
    // probe could have seen it.
    if (scenario == "xtest") {
        const QString plat = QGuiApplication::platformName();
        // The DISPLAY name is deliberately NOT printed. It is environment,
        // not behaviour, and this output is a baseline: a run by hand on one
        // display would otherwise fail the diff against a baseline captured on
        // another, for no reason a reader could act on.
        printf("\n== REAL X INPUT ==\n  [platform] %s\n", qPrintable(plat));
        // REFUSE rather than pass. Under offscreen there is no X server, every
        // xdotool call fails, and every count below is zero -- which is
        // indistinguishable from "the port ignores real clicks" unless the
        // scenario says so itself. check.sh has been bitten by a check that
        // could only report the absence it was written to detect.
        if (plat != QLatin1String("xcb")) {
            printf("  !! xtest needs QT_QPA_PLATFORM=xcb against a real X server\n"
                   "  !! (run it inside Xvfb; see PORTING.md C10)\n");
            fflush(stdout);
            return 1;
        }
        // A SCALED DISPLAY MIS-TARGETS EVERY CLICK, and it does so quietly.
        // mapToGlobal() returns logical pixels; xdotool takes device pixels.
        // At ratio 1.25 the clicks are real -- the provenance control still
        // says DISCRIMINATES -- but they land 20% away, and the run then
        // reports a false difference from 1.3. QT_SCALE_FACTOR,
        // QT_ENABLE_HIGHDPI_SCALING, QT_FONT_DPI or an Xft.dpi in the resource
        // database all cause it, and check.sh does not use `env -i'. Found by
        // review. check.sh also clears those variables now; this refuses
        // whatever the caller did.
        if (W->devicePixelRatioF() != 1.0) {
            printf("  !! this display has devicePixelRatio %.4g, not 1.\n"
                   "  !! Widget coordinates and xdotool coordinates would\n"
                   "  !! disagree and every click below would miss.\n",
                   W->devicePixelRatioF());
            fflush(stdout);
            return 1;
        }
        QCoreApplication::instance()->installNativeEventFilter(&g_nspy);
        g_qspy.main = W;
        qApp->installEventFilter(&g_qspy);

        QTreeWidget *tree = listView();
        QTreeWidget *ind  = indList();
        if (!tree || !ind) { printf("  !! no tree (%p) or individuals list (%p)\n",
                                    (void *)tree, (void *)ind); return 1; }
        // Somewhere on screen that is NOT the 900x750 window, for parking the
        // pointer between probes and for the outside-click below.
        const QPoint outside(1300, 950);

        // -- 1. coordinates ------------------------------------------------
        // Everything downstream maps a widget point to a global one and hands
        // it to xdotool. If that mapping is off, every later probe misses and
        // reads as "the port did not respond" -- C10 lost time to exactly that
        // shape. So check the mapping FIRST, against the server's own answer.
        printf("\n  -- 1. coordinates --\n");
        const QPoint probe = W->mapToGlobal(QPoint(40, 40));
        xMoveTo(probe); settle(150);
        const QPoint got = QCursor::pos();
        printf("    asked for (%d,%d), server reports (%d,%d) -- %s\n",
               probe.x(), probe.y(), got.x(), got.y(),
               got == probe ? "AGREE" : "DISAGREE");
        // THE MARKER HAS TO START THE LINE. check.sh greps `^ *!!', so a !!
        // in the MIDDLE of a line is invisible to it -- found by review, which
        // showed the stub-xdotool teeth test failing on the DISCRIMINATES grep
        // alone while this file claimed it failed on both.
        if (got != probe) {
            printf("  !! the pointer did not go where it was asked. Every click\n"
                   "  !! below would miss. Two things cause this: an xdotool\n"
                   "  !! that is not delivering input at all, or a scaling\n"
                   "  !! variable in the environment, because mapToGlobal()\n"
                   "  !! gives logical pixels and xdotool takes device pixels.\n");
            fflush(stdout);
            return 1;
        }

        // -- 2. provenance: real vs QTest ----------------------------------
        // The claim this whole scenario rests on, measured both ways round.
        printf("\n  -- 2. provenance --\n");
        const QRect r0 = ind->visualItemRect(ind->topLevelItem(0));
        const QPoint row0 = ind->viewport()->mapToGlobal(r0.center());
        xMoveTo(outside); settle(120);
        resetSpies(); xClickAt(row0); settle();
        spies("real XTEST click");
        const int realNative = g_nspy.press;
        resetSpies();
        QTest::mouseClick(ind->viewport(), Qt::LeftButton, Qt::NoModifier, r0.center());
        settle();
        spies("QTest::mouseClick");
        const bool discriminates = (realNative > 0 && g_nspy.press == 0);
        printf("    native ButtonPress: real=%d QTest=%d -- %s\n", realNative,
               g_nspy.press,
               discriminates ? "DISCRIMINATES (this is the whole point)"
                             : "NO DISCRIMINATION");
        if (!discriminates) {
            printf("  !! no real click was delivered, or QTest produced a native\n"
                   "  !! event. Either way nothing below is a measurement.\n");
            fflush(stdout);
            return 1;
        }

        // -- 3. activation -------------------------------------------------
        // Sampled on both sides of the click, never once. PORTING.md's
        // handover section: an absence is not a measurement.
        //
        // TWO DIFFERENT QUESTIONS, and they have different answers here.
        // Qt's isActiveWindow() is Qt's own bookkeeping; XGetInputFocus is
        // where the server would actually send a keystroke. With no window
        // manager on the nested display -- xdpyinfo reports focus on the ROOT
        // window at rest, exactly as the 1.3 oracle reports for its own
        // WM-less Xephyr -- nobody assigns focus on a click, so the second
        // question is the one that compares against 1.3.
        printf("\n  -- 3. activation --\n");
        xMoveTo(outside); settle(200);
        printf("    before: isActiveWindow=%d activeWindow=%s xfocus=%s\n",
               W->isActiveWindow() ? 1 : 0,
               QApplication::activeWindow()
                   ? QApplication::activeWindow()->metaObject()->className() : "(none)",
               qPrintable(xFocusName()));
        resetSpies(); xClickAt(row0); settle();
        printf("    after : isActiveWindow=%d activeWindow=%s xfocus=%s\n",
               W->isActiveWindow() ? 1 : 0,
               QApplication::activeWindow()
                   ? QApplication::activeWindow()->metaObject()->className() : "(none)",
               qPrintable(xFocusName()));
        spies("after real click");

        // -- 4. pointer grab -----------------------------------------------
        // A QMenu popup takes a real pointer grab on X. QTest cannot produce
        // one, so what a click OUTSIDE an open menu does had never been driven
        // here on either version until the oracle measured 1.3 on 2026-09-07.
        //
        // WHAT 1.3 DOES, measured on the 2003 binary with real XTest, so this
        // probe is built to answer the SAME question rather than a neighbouring
        // one: the File popup holds a genuine active grab -- XGrabPointer from
        // a second client answers AlreadyGrabbed while it is up and
        // GrabSuccess when it is not -- AND the outside click still reaches the
        // widget beneath it. One click both dismisses the menu and selects the
        // row under it; no second click is needed.
        //
        // SO THE TARGET HAS TO BE A ROW WHOSE SELECTION CAN CHANGE. An earlier
        // version clicked whatever sat three-quarters across the window, landed
        // on a QLabel, and could only report that nothing happened -- which is
        // the same shape as a click that was swallowed. The individuals list is
        // used because it is what the oracle used, and the probe refuses rather
        // than guesses if no row of it lies outside the popup.
        printf("\n  -- 4. pointer grab --\n");
        QMenuBar *mb = W->menuBar();
        QAction *fileAct = topAction(QStringLiteral("&File"));
        QTreeWidget *ind4 = indList();
        if (!fileAct || !ind4)
            printf("    !! no &File menu (%p) or individuals list (%p)\n",
                   (void *)fileAct, (void *)ind4);
        else {
            const QPoint fileG =
                mb->mapToGlobal(mb->actionGeometry(fileAct).center());
            printf("    xfocus with no menu up: %s\n", qPrintable(xFocusName()));
            resetSpies(); xClickAt(fileG); settle(400);
            QWidget *pop = QApplication::activePopupWidget();
            printf("    real click on [&File]: popup=%s mouseGrabber=%s "
                   "keyboardGrabber=%s xfocus=%s\n",
                   pop ? pop->metaObject()->className() : "(none)",
                   QWidget::mouseGrabber()
                       ? QWidget::mouseGrabber()->metaObject()->className() : "(none)",
                   QWidget::keyboardGrabber()
                       ? QWidget::keyboardGrabber()->metaObject()->className() : "(none)",
                   qPrintable(xFocusName()));

            // 1.3 does NOT close the menu on a second click of the same
            // menubar item -- the oracle checked the MAP STATE rather than the
            // focus, having been caught by exactly that. Same question here.
            resetSpies(); xClickAt(fileG); settle(400);
            printf("    second click on [&File]: popup still up=%d\n",
                   QApplication::activePopupWidget() != nullptr ? 1 : 0);
            if (!QApplication::activePopupWidget()) {   // reopen for the real test
                xClickAt(fileG); settle(400);
                pop = QApplication::activePopupWidget();
            }

            const QRect popG = pop ? QRect(pop->mapToGlobal(QPoint(0, 0)), pop->size())
                                   : QRect();
            // ROW 3, BY INDEX, exactly as the oracle used on 1.3. It must
            // not be the current row -- an earlier version took the first row
            // the popup did not cover, which was row 0, already selected by
            // probe 2, so "the selection did not change" was guaranteed
            // whatever the port did and it read as a difference.
            //
            // BY INDEX AND NOT BY GEOMETRY, because geometry is not portable.
            // Review measured this: under QT_STYLE_OVERRIDE=Windows the File
            // popup is 228x235 instead of 261x214, which moves both the point
            // a geometry search picks and the row it lands on. Neither number
            // is printed now for the same reason. The list sits to the RIGHT of
            // the popup, so every one of its rows is outside it whatever the
            // style does -- which is checked below rather than assumed.
            const int outRow = 3;
            QTreeWidgetItem *outItem = ind4->topLevelItem(outRow);
            QPoint outPt;
            if (outItem && outItem != ind4->currentItem()) {
                const QRect ir = ind4->visualItemRect(outItem);
                if (ir.isValid() && ir.height() > 0)
                    outPt = ind4->viewport()->mapToGlobal(ir.center());
            }
            if (outPt.isNull() || popG.contains(outPt)) {
                printf("  !! list row %d is unusable as an outside point"
                       " (missing, current, or under the popup).\n", outRow);
                fflush(stdout);
            }
            else {
                QTreeWidgetItem *selBefore = ind4->currentItem();
                const bool popupBefore = QApplication::activePopupWidget() != nullptr;
                printf("    outside point is list row %d, outside the popup=1, "
                       "widgetThere=%s\n", outRow,
                       QApplication::widgetAt(outPt)
                           ? QApplication::widgetAt(outPt)->metaObject()->className()
                           : "(none)");
                resetSpies(); xClickAt(outPt); settle(500);
                // pressOn IS the answer to "was it swallowed": a QMenu alone
                // means the popup ate it, a viewport as well means the widget
                // under it got it too. The selection line says the same thing
                // in SIGEL's own terms, and the two must agree.
                printf("    then a real click on list row %d: popup before=%d "
                       "after=%d, list current before=[%s] after=[%s] xfocus=%s\n",
                       outRow, popupBefore ? 1 : 0,
                       QApplication::activePopupWidget() != nullptr ? 1 : 0,
                       selBefore ? qPrintable(selBefore->text(0)) : "(none)",
                       ind4->currentItem() ? qPrintable(ind4->currentItem()->text(0))
                                           : "(none)",
                       qPrintable(xFocusName()));
                spies("outside click");
                const bool rowMoved = ind4->currentItem() != selBefore;
                // THE CONTROL, and without it the line above is worthless: the
                // very same click, at the very same point, with no menu open.
                // If THAT does not select the row either, the target is wrong
                // and nothing was being swallowed -- which is the reading a
                // first version of this probe would have got away with.
                QTreeWidgetItem *ctlBefore = ind4->currentItem();
                resetSpies(); xClickAt(outPt); settle(400);
                const bool ctlMoved = ind4->currentItem() != ctlBefore;
                printf("    CONTROL, same point with no menu up: list current "
                       "before=[%s] after=[%s] -- %s\n",
                       ctlBefore ? qPrintable(ctlBefore->text(0)) : "(none)",
                       ind4->currentItem() ? qPrintable(ind4->currentItem()->text(0))
                                           : "(none)",
                       ctlMoved ? "row selectable, so the probe can see a change"
                                : "row NOT selectable");
                if (!ctlMoved)
                    printf("  !! the control click did not move the selection"
                           " either, so the target is wrong and the result"
                           " above means nothing.\n");
                spies("control click");
                // popupBefore IS PART OF THE VERDICT. Without it, a run in
                // which the menu failed to reopen would click with no popup up,
                // the row would move, and the line would read AGREE -- a false
                // "no difference" that only the baseline diff would catch.
                // Found by review.
                printf("    vs 1.3: menu closes AND the row under the click is "
                       "selected -- %s\n",
                       (!ctlMoved || !popupBefore)
                           ? "UNDECIDABLE, the probe did not set up the test"
                       : (QApplication::activePopupWidget() == nullptr && rowMoved)
                           ? "AGREE"
                           : "DIFFERS -- the port SWALLOWS the dismissing click");
                if (!popupBefore)
                    printf("  !! there was no menu open when the outside click"
                           " was sent, so this section tested nothing.\n");
            }
            if (QApplication::activePopupWidget()) {
                QApplication::activePopupWidget()->close();
                settle(200);
            }
        }

        // -- 5. double-click synthesis -------------------------------------
        // Two real presses, and Qt deciding for itself whether they are one
        // double click. QTest::mouseDClick posts the DblClick event ready-made,
        // so it never exercises the decision.
        printf("\n  -- 5. double-click synthesis --\n");
        printf("    doubleClickInterval=%d ms\n", QApplication::doubleClickInterval());
        clickMenu("&View", "&Population");
        settle(400);
        QTreeWidget *ind2 = indList();
        if (!ind2) printf("    !! no individuals list\n");
        else {
            auto countViews = []() {
                int n = 0;
                for (QWidget *w : QApplication::topLevelWidgets())
                    if (qobject_cast<SIG_IndividualView *>(w) && w != W) ++n;
                return n;
            };
            auto closeViews = []() {
                for (QWidget *w : QApplication::topLevelWidgets())
                    if (qobject_cast<SIG_IndividualView *>(w) && w != W) w->close();
            };
            closeViews(); settle(200);
            const QRect rr = ind2->visualItemRect(ind2->topLevelItem(0));
            const QPoint rg = ind2->viewport()->mapToGlobal(rr.center());

            // The CONTROL first, so a scenario that opens a view on any two
            // clicks cannot read as a double-click result. 1200 ms is three
            // times Qt's default interval.
            const int before0 = countViews();
            QSignalSpy slow(ind2, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)));
            resetSpies();
            xClickAt(rg); settle(1200); xClickAt(rg); settle(600);
            printf("    two SLOW clicks (1200 ms apart): qtDblClick=%d "
                   "itemDoubleClicked=%d views %d -> %d\n",
                   g_qspy.dbl, (int)slow.count(), before0, countViews());
            closeViews(); settle(200);

            const int before1 = countViews();
            QSignalSpy fast(ind2, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)));
            resetSpies();
            xClickAt(rg, 2, 80); settle(900);
            const int after1 = countViews();
            printf("    two FAST clicks (80 ms apart) : qtDblClick=%d "
                   "itemDoubleClicked=%d views %d -> %d\n",
                   g_qspy.dbl, (int)fast.count(), before1, after1);
            spies("fast pair");
            // SECTION 5 HAD NO SELF-DIAGNOSIS, and a mis-targeted run reported
            // "a fast double click opens nothing" -- a false difference from
            // 1.3, with no marker anywhere. Found by review. Two real presses
            // inside the interval must reach the row; if they did not, the
            // probe missed rather than the port regressing, and either way this
            // run must not be baselined.
            if (fast.count() == 0)
                printf("  !! two clicks %d ms apart produced no"
                       " itemDoubleClicked. The probe missed the row, or double"
                       " click synthesis is broken. Do not baseline this run.\n",
                       80);
            closeViews(); settle(200);
        }

        // -- 6. enter and leave --------------------------------------------
        // Qt computes these from real pointer motion, and no scenario in this
        // file has ever seen a Leave -- but NOT for the reason the C10 note
        // gives. QTest::mouseMove(QWidget *) with no button held does not post
        // an event at all: qtestmouse.h:198-202 calls QCursor::setPos() and
        // processEvents(). That is a real pointer warp through the platform, so
        // on a real X server QTest's own move DOES produce crossing events, as
        // the third line below shows. What could not deliver them was the
        // OFFSCREEN PLATFORM, where there is no pointer to warp -- a platform
        // limit, not a QTest one. The distinction is the finding here.
        printf("\n  -- 6. enter / leave --\n");
        {
            QTreeWidget *a = listView();
            QTreeWidget *b = indList();
            if (!a || !b) printf("    !! need both views\n");
            else {
                const QPoint pa = a->viewport()->mapToGlobal(
                    QPoint(a->viewport()->width() / 2, a->viewport()->height() / 2));
                const QPoint pb = b->viewport()->mapToGlobal(
                    QPoint(b->viewport()->width() / 2, b->viewport()->height() / 2));
                xMoveTo(pa); settle(250);
                // CONTROL: a second move INSIDE the same widget must produce
                // no crossing at all.
                resetSpies();
                xMoveTo(pa + QPoint(0, 12)); settle(250);
                // nativeEnter/nativeLeave are EXPECTED to be 0 on every line
                // here and their being 0 is not a failure: X crossing events
                // are per X window, and Qt 6 gives a top level ONE native
                // window, so moving between two widgets inside it crosses no X
                // boundary at all. Qt computes Enter and Leave itself from the
                // motion, which is what the qt columns show. nativeMotion is
                // the column that carries the real input on these lines.
                printf("    move within one widget : qtEnter=%d qtLeave=%d "
                       "nativeMotion=%d nativeEnter=%d nativeLeave=%d\n",
                       g_qspy.enter, g_qspy.leave, g_nspy.motion,
                       g_nspy.enter, g_nspy.leave);
                resetSpies();
                xMoveTo(pb); settle(250);
                printf("    move across a boundary : qtEnter=%d qtLeave=%d "
                       "nativeMotion=%d nativeEnter=%d nativeLeave=%d\n",
                       g_qspy.enter, g_qspy.leave, g_nspy.motion,
                       g_nspy.enter, g_nspy.leave);
                // And what QTest does for the same motion, which is the
                // comparison the row above exists for.
                resetSpies();
                QTest::mouseMove(a->viewport(),
                                 QPoint(a->viewport()->width() / 2,
                                        a->viewport()->height() / 2));
                settle(250);
                printf("    same motion via QTest  : qtEnter=%d qtLeave=%d "
                       "nativeMotion=%d nativeEnter=%d nativeLeave=%d\n",
                       g_qspy.enter, g_qspy.leave, g_nspy.motion,
                       g_nspy.enter, g_nspy.leave);
            }
        }
        printf("\n");
        fflush(stdout);
        return 0;
    }

    return 0;
}

int main(int argc, char **argv)
{
    const int rc = guidriveMain(argc, argv);
    // Decide the status FIRST, then tear PVM down. The order is the whole
    // point: tearDownPvm() gets this process SIGTERMed whenever it started the
    // daemon, and the handler installed here re-exits with rc instead of 143.
    // Flush FIRST. tearDownPvm() gets this process SIGTERMed and the handler
    // _exit()s, which runs no destructors and no stdio flush -- so a flush
    // placed after the teardown never executes on the one path the teardown
    // comment is about. The watchdog already had this order; main did not.
    // Found by review.
    flushSigelStreams();
    // ONLY when PVM is actually up. The fourteen scenarios check.sh runs never
    // start it, and their stdout is diffed against guibehaviour-baseline.txt --
    // an unconditional line here would have failed all ten baseline scenarios
    // on a cosmetic addition. It says the status is already decided, so a hang
    // in the halt below is visibly cleanup and not the scenario.
    if (g_pvmOurDaemon || g_pvmEnrolled) {
        printf("  [teardown] scenario finished with status %d; halting PVM\n", rc);
        fflush(stdout);
    }
    keepExitCodeThroughPvmShutdown(rc);
    tearDownPvm();
    flushSigelStreams();
    return rc;
}
