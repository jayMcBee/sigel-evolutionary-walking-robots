/*
  uic makes only the struct Ui::MT_SearchWidgetBase. This class inherits it and
  calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_SEARCHWIDGETBASE_H
#define MT_GUI_MT_SEARCHWIDGETBASE_H

#include "ui_MT_SearchWidgetBase.h"

#include <QEvent>
#include <QWidget>

class MT_SearchWidgetBase : public QWidget, public Ui::MT_SearchWidgetBase
{
    Q_OBJECT

public:
    MT_SearchWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_SEARCHWIDGETBASE_H
