/*
  The class the 2003 uic generated, in uic3 declaration-mode shape,
  adapted to Qt 6. Qt 6's uic emits only Ui::MT_AddConstantsWidgetBase,
  a struct with setupUi().

  Global namespace, as uic generated it in 2003.
*/
#ifndef MT_GUI_MT_ADDCONSTANTSWIDGETBASE_H
#define MT_GUI_MT_ADDCONSTANTSWIDGETBASE_H

#include "ui_MT_AddConstantsWidgetBase.h"

#include <QEvent>
#include <QDialog>

class MT_AddConstantsWidgetBase : public QDialog, public Ui::MT_AddConstantsWidgetBase
{
    Q_OBJECT

public:
    MT_AddConstantsWidgetBase(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MT_AddConstantsWidgetBase() override;

protected:
    // Qt 3 called languageChange() itself; Qt 6 has no such hook, so it is
    // driven from changeEvent -- otherwise the slot below is dead code.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_ADDCONSTANTSWIDGETBASE_H
