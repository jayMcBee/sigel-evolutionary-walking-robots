/*
  uic makes only the struct Ui::SIG_ExperimentViewBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef SIGEL_MASTERGUI_SIG_EXPERIMENTVIEWBASE_H
#define SIGEL_MASTERGUI_SIG_EXPERIMENTVIEWBASE_H

#include "ui_SIG_ExperimentViewBase.h"

#include <QEvent>
#include <QWidget>

class SIG_ExperimentViewBase : public QWidget, public Ui::SIG_ExperimentViewBase
{
    Q_OBJECT

public:
    SIG_ExperimentViewBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

public slots:
    virtual void slotExportPostScript() = 0;
    virtual void slotHistory(bool) = 0;
    virtual void slotAutosaveChanged(int) = 0;
    virtual void slotShowFitnesscurve() = 0;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_EXPERIMENTVIEWBASE_H
