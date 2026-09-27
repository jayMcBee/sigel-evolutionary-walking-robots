/*
  uic makes only the struct Ui::SIG_MovieSettingsDialogBase. This class inherits
  it and calls setupUi() on itself.
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
    SIG_MovieSettingsDialogBase(QWidget* parent = nullptr, const char* name = nullptr, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_MovieSettingsDialogBase() override;

public slots:
    virtual void slotToolButtonClicked() = 0;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();


};

#endif // SIGEL_SLAVEGUI_SIG_MOVIESETTINGSDIALOGBASE_H
