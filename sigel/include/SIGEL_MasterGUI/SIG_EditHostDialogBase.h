/*
  The class the 2003 uic generated, in uic3 declaration-mode shape,
  adapted to Qt 6. Qt 6's uic emits only Ui::SIG_EditHostDialogBase,
  a struct with setupUi().

  Global namespace, as uic generated it in 2003.
*/
#ifndef SIGEL_MASTERGUI_SIG_EDITHOSTDIALOGBASE_H
#define SIGEL_MASTERGUI_SIG_EDITHOSTDIALOGBASE_H

#include "ui_SIG_EditHostDialogBase.h"

#include <QEvent>
#include <QDialog>

class SIG_EditHostDialogBase : public QDialog, public Ui::SIG_EditHostDialogBase
{
    Q_OBJECT

public:
    SIG_EditHostDialogBase(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_EditHostDialogBase() override;

public slots:
    virtual void slotToolbuttonSlaveDirectoryClicked() = 0;

protected:
    // Qt 3 called languageChange() itself; Qt 6 has no such hook, so it is
    // driven from changeEvent -- otherwise the slot below is dead code.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_EDITHOSTDIALOGBASE_H
