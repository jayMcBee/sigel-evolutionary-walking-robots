/*
  uic makes only the struct Ui::MT_AddConstantsWidgetBase. This class inherits
  it and calls setupUi() on itself.
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
    MT_AddConstantsWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MT_AddConstantsWidgetBase() override;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_ADDCONSTANTSWIDGETBASE_H
