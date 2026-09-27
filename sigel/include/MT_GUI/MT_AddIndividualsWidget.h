/*
  uic makes only the struct Ui::MT_AddIndividualsWidgetBase. This class inherits
  it and calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_ADDINDIVIDUALSWIDGET_H
#define MT_GUI_MT_ADDINDIVIDUALSWIDGET_H

#include "ui_MT_AddIndividualsWidget.h"

#include <QEvent>
#include <QDialog>

class MT_AddIndividualsWidgetBase : public QDialog, public Ui::MT_AddIndividualsWidgetBase
{
    Q_OBJECT

public:
    MT_AddIndividualsWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MT_AddIndividualsWidgetBase() override;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_ADDINDIVIDUALSWIDGET_H
