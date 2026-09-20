/*
  The class the 2003 uic generated, in uic3 declaration-mode shape,
  adapted to Qt 6. Qt 6's uic emits only Ui::SIG_IndividualViewBase,
  a struct with setupUi().

  Global namespace, as uic generated it in 2003.
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
    SIG_IndividualViewBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_IndividualViewBase() override;

protected:
    // Qt 3 called languageChange() itself; Qt 6 has no such hook, so it is
    // driven from changeEvent -- otherwise the slot below is dead code.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_INDIVIDUALVIEWBASE_H
