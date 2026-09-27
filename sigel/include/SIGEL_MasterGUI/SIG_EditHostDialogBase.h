/*
  uic makes only the struct Ui::SIG_EditHostDialogBase. This class inherits it
  and calls setupUi() on itself.
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
    SIG_EditHostDialogBase(QWidget* parent = nullptr, const char* name = nullptr, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_EditHostDialogBase() override;

public slots:
    virtual void slotToolbuttonSlaveDirectoryClicked() = 0;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_EDITHOSTDIALOGBASE_H
