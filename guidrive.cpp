/*
  Drives the ported SIGEL interface with real Qt input events and prints a
  diffable dump of what the application does. PORTING.md section C10.

  WHAT THIS IS, precisely, because the distinction matters and the record
  should not overstate it: QTest posts QMouseEvent, QKeyEvent and
  QContextMenuEvent through QApplication::notify, so the widgets' own event
  handlers, hit-testing, menu popup logic, item-view selection and every slot
  behind them run exactly as they do under a mouse. It is NOT an X-level
  click: nothing here goes through the platform plugin. That last hop is the
  only part not covered, and on the machine this was written for it could not
  be covered at all -- see C10 for why.

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
#include <QContextMenuEvent>
#include <QFile>
#include <QFileInfo>
extern "C" {
#include "pvm3.h"
}
#include <QTimer>
#include <QtTest/QtTest>
#include <cstdio>
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
static void whenModal(std::function<void(QWidget *)> fn, int budgetMs = 8000)
{
    QTimer *t = new QTimer;
    int *spent = new int(0);
    QObject::connect(t, &QTimer::timeout, [t, fn, spent, budgetMs]() {
        QWidget *m = QApplication::activeModalWidget();
        *spent += 50;
        if (m) {
            t->stop(); t->deleteLater();
            printf("  [modal] class=%s title=[%s]\n",
                   m->metaObject()->className(), qPrintable(m->windowTitle()));
            fflush(stdout);
            fn(m);
            delete spent;
        } else if (*spent > budgetMs) {
            t->stop(); t->deleteLater();
            printf("  [modal] NONE appeared within %d ms\n", budgetMs);
            fflush(stdout);
            delete spent;
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
    if (!ta || !ta->menu()) { printf("  !! no top menu [%s]\n", qPrintable(top)); return false; }
    QTest::mouseClick(mb, Qt::LeftButton, Qt::NoModifier, mb->actionGeometry(ta).center());
    QTest::qWait(120);
    QMenu *m = ta->menu();
    if (!m->isVisible()) { printf("  !! menu [%s] did not open\n", qPrintable(top)); return false; }
    for (QAction *a : m->actions()) {
        if (a->text() != item) continue;
        if (!sub.isEmpty() && a->menu()) {
            QTest::mouseClick(m, Qt::LeftButton, Qt::NoModifier, m->actionGeometry(a).center());
            QTest::qWait(150);
            QMenu *s = a->menu();
            for (QAction *sa : s->actions())
                if (sa->text() == sub) {
                    QTest::mouseClick(s, Qt::LeftButton, Qt::NoModifier, s->actionGeometry(sa).center());
                    QTest::qWait(150); return true;
                }
            s->close(); m->close(); return false;
        }
        if (!a->isEnabled()) {
            printf("  !! item [%s>%s] is GREYED -- click would do nothing\n",
                   qPrintable(top), qPrintable(item));
            m->close(); return false;
        }
        QTest::mouseClick(m, Qt::LeftButton, Qt::NoModifier, m->actionGeometry(a).center());
        QTest::qWait(200);
        return true;
    }
    printf("  !! no item [%s] in menu [%s]\n", qPrintable(item), qPrintable(top));
    m->close();
    return false;
}

// -------------------------------------------------------------------- steps
static void openExperiment(const QString &path)
{
    whenModal([path](QWidget *m) {
        QFileDialog *fd = qobject_cast<QFileDialog *>(m);
        if (!fd) { printf("  !! modal is not a QFileDialog\n"); m->close(); return; }
        printf("  [filedialog] title=[%s] mode=%d nameFilters=[%s] labelAccept=[%s]\n",
               qPrintable(fd->windowTitle()), (int)fd->fileMode(),
               qPrintable(fd->nameFilters().join(" ;; ")),
               qPrintable(fd->labelText(QFileDialog::Accept)));
        QLineEdit *le = fd->findChild<QLineEdit *>("fileNameEdit");
        if (!le) { printf("  !! no fileNameEdit\n"); fd->reject(); return; }
        le->setFocus();
        QTest::keyClicks(le, path);          // real key events into the dialog
        QTest::qWait(120);
        QTest::keyClick(le, Qt::Key_Return); // real Return
        fflush(stdout);
    });
    clickMenu("&File", "&Open Experiment");
    QTest::qWait(800);
}

// ---------------------------------------------------------------------- main
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QString scenario = argc > 1 ? argv[1] : "open";
    QString expFile  = argc > 2 ? argv[2]
        : qEnvironmentVariable("SIGEL_EXP",
              "data-reordered/Experiments/twoBasesSimpleFitness2.exp");

    // sigel.cpp:154 brings PVM up before the window exists. Only the visualize
    // scenario needs it, and starting a daemon for the others would be noise.
    if (scenario == "visualize" || scenario == "evolution") {
        int info = pvm_start_pvmd(0, 0, 0);
        int mytid = pvm_mytid();
        printf("  [pvm] start_pvmd=%d mytid=0x%x\n", info, mytid);
        fflush(stdout);
    }

    SIG_MainWindow w(nullptr, "MainWindow");
    W = &w;
    w.resize(900, 750);
    w.show();
    QTest::qWaitForWindowExposed(&w);

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
        int x = h->sectionPosition(1) + h->sectionSize(1) / 2;
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

        // 3. THE CRASH CASE. Deleting a block big enough that a selected row's
        //    poolPosition exceeds the surviving pool used to reach 1.3's own
        //    getIndividual() exit(1) through Qt 6's unblocked clear().
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
            int x = h->sectionPosition(1) + h->sectionSize(1) / 2;
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
