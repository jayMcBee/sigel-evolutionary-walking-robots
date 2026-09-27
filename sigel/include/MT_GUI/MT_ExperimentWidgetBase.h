/*
  uic makes only the struct Ui::MT_ExperimentWidgetBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_EXPERIMENTWIDGETBASE_H
#define MT_GUI_MT_EXPERIMENTWIDGETBASE_H

#include "ui_MT_ExperimentWidgetBase.h"

#include <QEvent>
#include <QWidget>

class MT_ExperimentWidgetBase : public QWidget, public Ui::MT_ExperimentWidgetBase
{
    Q_OBJECT

public:
    MT_ExperimentWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_EXPERIMENTWIDGETBASE_H
