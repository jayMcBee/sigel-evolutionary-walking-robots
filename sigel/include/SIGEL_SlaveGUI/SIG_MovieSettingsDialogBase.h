/*
  The class the 2003 uic generated, in uic3 declaration-mode shape,
  adapted to Qt 6. Qt 6's uic emits only Ui::SIG_MovieSettingsDialogBase,
  a struct with setupUi().

  Global namespace, as uic generated it in 2003.
*/
#ifndef SIGEL_SLAVEGUI_SIG_MOVIESETTINGSDIALOGBASE_H
#define SIGEL_SLAVEGUI_SIG_MOVIESETTINGSDIALOGBASE_H

#include "ui_SIG_MovieSettingsDialogBase.h"

#include <QEvent>
#include <QDialog>

class SIG_MovieSettingsDialogBase : public QDialog, public Ui::SIG_MovieSettingsDialogBase
{
    Q_OBJECT

public:
    SIG_MovieSettingsDialogBase(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_MovieSettingsDialogBase() override;

public slots:
    virtual void slotToolButtonClicked();

protected:
    // Qt 3 called languageChange() itself; Qt 6 has no such hook, so it is
    // driven from changeEvent -- otherwise the slot below is dead code.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

    virtual void slotChangedAspectRatio(bool);
    virtual void slotSetHeight(int);
    virtual void slotSetWidth(int);


};

#endif // SIGEL_SLAVEGUI_SIG_MOVIESETTINGSDIALOGBASE_H
