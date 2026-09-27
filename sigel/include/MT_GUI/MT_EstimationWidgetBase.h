/*
  uic makes only the struct Ui::MT_EstimationWidgetBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_ESTIMATIONWIDGETBASE_H
#define MT_GUI_MT_ESTIMATIONWIDGETBASE_H

#include "ui_MT_EstimationWidgetBase.h"

#include <QEvent>
#include <QWidget>

class MT_EstimationWidgetBase : public QWidget, public Ui::MT_EstimationWidgetBase
{
    Q_OBJECT

public:
    MT_EstimationWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MT_EstimationWidgetBase() override;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_ESTIMATIONWIDGETBASE_H
