/*
  Drives the ported SIGEL interface with real Qt input events and prints a
  diffable dump of what the application does. PORTING.md section C10.

  WHAT THIS IS, precisely, because the distinction matters and the record
  should not overstate it: QTest posts QMouseEvent, QKeyEvent and
  QContextMenuEvent through QApplication::notify, so the widgets' own event
  handlers, hit-testing, menu popup logic, item-view selection and the slots
  behind them all run. It is NOT the same as a mouse, and the difference is
  more than "one missing hop": bypassing QWindowSystemInterface changes window
  activation, mouse grabs, double-click synthesis and enter/leave delivery.
  Menu navigation is the plain example -- a real mouse presses, drags under a
  popup grab and releases, where this posts two independent clicks that happen
  to reach the same actions. So this proves the application's own logic is
  right; it does not prove the platform layer is. On the machine this was
  written for, no tool could drive that layer at all -- see C10.

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

  Environment:
    SIGEL_ROOT      as the application needs it; must hold sigel_slave for
                    the visualize and evolution scenarios
    SIGEL_EXP       experiment file to open (default the twoBases reference)
    SIGEL_SCRATCH   where save/export scenarios write (default /tmp)
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
#include <QSlider>
#include <QProgressBar>
#include <QComboBox>
#include <QHeaderView>
#include <QTabWidget>
#include <QStyle>
#include <QCompleter>
#include <QStyleOptionSlider>
#include <QTabBar>
#include <QListWidget>
#include <QValidator>
#include <QAbstractSpinBox>
#include <QContextMenuEvent>
#include <QFile>
#include <QFileInfo>
extern "C" {
#include "pvm3.h"
}
#include <QTimer>
#include <QElapsedTimer>
#include <QtTest/QtTest>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <functional>
#include <map>

#include "SIGEL_MasterGUI/SIG_MainWindow.h"
#include "SIGEL_MasterGUI/SIG_ExperimentListView.h"
#include "SIGEL_MasterGUI/SIG_Experiment.h"
#include "SIGEL_MasterGUI/SIG_AllIndividualsView.h"
#include "SIGEL_MasterGUI/SIG_IndividualListItem.h"
#include "SIGEL_MasterGUI/SIG_IndividualView.h"

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
}

// Everything on the current page that carries observable state. Generic on
// purpose: the same dump has to serve the experiment page, the GP page and the
// individuals page, and the oracle reads the same facts off 1.3 by eye.
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

// A scenario that blocks in a modal exec() that never closes would otherwise
// hang forever; check.sh's timeout would kill it and report the wrong reason.
// A modal exec() still runs an event loop, so this timer fires inside exactly
// the case it exists for.
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
        fflush(stdout);
        tearDownPvm();
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
static void probeSpin(QSpinBox *sp)
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
    QTest::keyClick(sp, Qt::Key_Return);
    QTest::qWait(20);
    const int committed = sp->value();
    sp->setValue(start);

    printf("    spin   %-36s v=%-9d [%d..%d] step=%d pre=[%s] suf=[%s] "
           "up=%-9d dn2=%-9d typed(max+1)=[%s]->%d commits=%d enabled=%d\n",
           qPrintable(sp->objectName()), start, sp->minimum(), sp->maximum(),
           sp->singleStep(), qPrintable(sp->prefix()), qPrintable(sp->suffix()),
           up, down, qPrintable(typed), typedVal, committed,
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
    sl->setValue(start);
    printf("    slider %-36s v=%-6d [%d..%d] page=%d up=%-6d pgup=%-6d "
           "lcd=%s %g/%g/%g enabled=%d\n",
           qPrintable(sl->objectName()), start, sl->minimum(), sl->maximum(),
           sl->pageStep(), up, pg, lcd ? qPrintable(lcd->objectName()) : "(none)",
           lcdStart, lcdUp, lcdPg, sl->isEnabled() ? 1 : 0);
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

// -------------------------------------------------------------------- steps
// Type a path into a file dialog and accept it.
//
// THIS WAS THE INTERMITTENT HANG, and the cause is not what it looked like.
// Seen four times while C11a was being written and then not reproducible in a
// dozen runs; it is not new here either -- C10's openExperiment has the same
// shape, so the C10 gate has been quietly flaky since it was written.
//
// TWO WRONG DIAGNOSES, both recorded because each looked right:
//  1. "The completer popup eats the Return." Plausible, and the popup is real,
//     so the first fix escaped it and pressed Return again. That is WORSE than
//     hanging: it reproduced during the fresh-eyes review and the dialog
//     accepted with the completion's filename instead of the typed one, so the
//     experiment silently did not load and the run carried on against an empty
//     tree. A hang is loud; a wrong file is not.
//  2. The actual cause, which only a printed diagnostic showed: QFileDialog
//     NAVIGATES as a path with separators is typed, and strips the directory
//     part out of the field as it goes. The field is then left holding just
//     the basename, and Return resolves it against whichever directory the
//     dialog has got to -- which is a race with the typing.
//
// So: type an ABSOLUTE path, which resolves the same wherever the dialog has
// navigated to, and re-assert it before Return. Both the whole path and the
// bare basename are the dialog behaving normally, so neither prints; anything
// else does. The completer goes too -- one less thing to race with.
static void acceptFileDialog(QFileDialog *fd, const QString &path)
{
    QLineEdit *le = fd->findChild<QLineEdit *>("fileNameEdit");
    if (!le) { printf("  !! no fileNameEdit\n"); fd->reject(); return; }
    const QString abs  = QFileInfo(path).absoluteFilePath();
    const QString base = QFileInfo(abs).fileName();
    le->setCompleter(nullptr);
    le->setFocus();
    le->selectAll();
    QTest::keyClicks(le, abs);           // real key events, one per character
    QTest::qWait(120);
    if (le->text() != abs && le->text() != base)
        printf("  [filedialog] typed [%s] but the field holds [%s]\n",
               qPrintable(abs), qPrintable(le->text()));
    le->setText(abs);                    // unconditional: kills the race
    QTest::keyClick(le, Qt::Key_Return);
    QTest::qWait(150);
    if (!fd->isVisible()) return;

    // If this ever prints, something changed in Qt's dialog and the run should
    // be looked at rather than trusted.
    printf("  [filedialog] still open after Return; clicking accept\n");
    for (QPushButton *b : fd->findChildren<QPushButton *>())
        if (b->isDefault() && b->isEnabled()) {
            QTest::mouseClick(b, Qt::LeftButton, Qt::NoModifier, b->rect().center());
            break;
        }
    QTest::qWait(150);
    if (fd->isVisible()) {
        printf("  [filedialog] STILL open -- rejecting it\n");
        fd->reject();
    }
    fflush(stdout);
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

// ---------------------------------------------------------------------- main
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    // Anything that returns from run() gets PVM torn down; the watchdog does it
    // itself because _exit() runs no destructors.
    struct PvmGuard { ~PvmGuard() { tearDownPvm(); } } pvmGuard;
    QString scenario = argc > 1 ? argv[1] : "open";
    QString expFile  = argc > 2 ? argv[2]
        : qEnvironmentVariable("SIGEL_EXP",
              "data-reordered/Experiments/twoBasesSimpleFitness2.exp");

    // sigel.cpp:154 brings PVM up before the window exists. Only the visualize
    // scenario needs it, and starting a daemon for the others would be noise.
    // Evolution genuinely takes minutes; everything else that runs longer than
    // this is stuck, not busy.
    const bool slow = (scenario == "evolution" || scenario == "visualize");
    armWatchdog(qEnvironmentVariableIntValue("SIGEL_WATCHDOG_MS") > 0
                    ? qEnvironmentVariableIntValue("SIGEL_WATCHDOG_MS")
                    : (slow ? 900000 : 240000));

    if (scenario == "visualize" || scenario == "evolution") {
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

    // --- the gate: one deterministic pass over the behaviour that has an
    // --- oracle reading behind it. Everything here was compared against the
    // --- running 1.3 binary; see PORTING.md's C10 section.
    if (scenario == "gate") {
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
            QLineEdit *le = fd->findChild<QLineEdit *>("fileNameEdit");
            if (!le) { printf("  !! no fileNameEdit\n"); fd->reject(); return; }
            // 1.3 pre-fills this field but does NOT pre-select it, so typing
            // appends -- unlike the Rename dialog, which does pre-select.
            // Printed before selectAll() so the default state is visible.
            printf("  [savedialog] prefill=[%s] preselected=[%s]\n",
                   qPrintable(le->text()), qPrintable(le->selectedText()));
            fflush(stdout);
            le->setFocus();
            le->selectAll();
            QTest::keyClicks(le, outA);
            QTest::qWait(120);
            QTest::keyClick(le, Qt::Key_Return);
            fflush(stdout);
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
            QLineEdit *le = fd->findChild<QLineEdit *>("fileNameEdit");
            if (!le) { fd->reject(); return; }
            le->setFocus(); le->selectAll();
            QTest::keyClicks(le, outB);
            QTest::qWait(120);
            QTest::keyClick(le, Qt::Key_Return);
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
            QLineEdit *le = fd->findChild<QLineEdit *>("fileNameEdit");
            if (!le) { fd->reject(); return; }
            le->setFocus(); le->selectAll();
            QTest::keyClicks(le, out);
            QTest::qWait(120);
            QTest::keyClick(le, Qt::Key_Return);
            fflush(stdout);
        });
        clickMenu("&File", "Export", "Program");
        QTest::qWait(2500);
        printf("  [exported] exists=%d bytes=%lld\n", QFile::exists(out), QFileInfo(out).size());
        return 0;
    }

    // --- Start / Stop an evolution ----------------------------------------
    if (scenario == "evolution") {
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
        if (SIG_Experiment *ex = lv->currentlySelectedExperiment()) {
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
        if (qgetenv("SIGEL_RUN_LONGER") == "1") {
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
        QSignalSpy spy(start, SIGNAL(clicked()));
        // The decisive question is not whether the button works but whether
        // slotStartEvolution ran and DECLINED. This signal is emitted false on
        // start and true when the run ends, so a start-then-immediately-finish
        // is visible even if the buttons are back to their resting state by the
        // time anything samples them.
        SIG_Experiment *exp = lv->currentlySelectedExperiment();
        QSignalSpy *evo = exp ? new QSignalSpy(exp, SIGNAL(signalEvolutionNotRunning(bool)))
                              : nullptr;
        printf("\n  >> clicking Start\n"); fflush(stdout);
        QTest::mouseClick(start, Qt::LeftButton, Qt::NoModifier, start->rect().center());
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
        if (SIG_Experiment *ex = lv->currentlySelectedExperiment()) {
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

    return 0;
}
