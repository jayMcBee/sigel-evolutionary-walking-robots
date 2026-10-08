/*
  uic makes only the struct Ui::SIG_IndividualViewBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef SIGEL_MASTERGUI_SIG_INDIVIDUALVIEWBASE_H
#define SIGEL_MASTERGUI_SIG_INDIVIDUALVIEWBASE_H

#include "ui_SIG_IndividualViewBase.h"

#include <QEvent>
#include <QWidget>

class SIG_IndividualViewBase : public QWidget, public Ui::SIG_IndividualViewBase
{
    Q_OBJECT

public:
    SIG_IndividualViewBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();
};

#endif // SIGEL_MASTERGUI_SIG_INDIVIDUALVIEWBASE_H
