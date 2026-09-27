/*
  uic makes only the struct Ui::MT_SelectionWidgetBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_SELECTIONWIDGETBASE_H
#define MT_GUI_MT_SELECTIONWIDGETBASE_H

#include "ui_MT_SelectionWidgetBase.h"

#include <QEvent>
#include <QWidget>

class MT_SelectionWidgetBase : public QWidget, public Ui::MT_SelectionWidgetBase
{
    Q_OBJECT

public:
    MT_SelectionWidgetBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MT_SelectionWidgetBase() override;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_SELECTIONWIDGETBASE_H
