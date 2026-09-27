/*
  uic makes only the struct Ui::MT_IndividualsWidgetBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_INDIVIDUALWIDGETBASE_H
#define MT_GUI_MT_INDIVIDUALWIDGETBASE_H

#include "ui_MT_IndividualWidgetBase.h"

#include <QEvent>
#include <QWidget>

class MT_IndividualsWidgetBase : public QWidget, public Ui::MT_IndividualsWidgetBase
{
    Q_OBJECT

public:
    MT_IndividualsWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_INDIVIDUALWIDGETBASE_H
