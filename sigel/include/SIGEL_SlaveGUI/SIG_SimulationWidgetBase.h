/*
  uic makes only the struct Ui::SIG_SimulationWidgetBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef SIGEL_SLAVEGUI_SIG_SIMULATIONWIDGETBASE_H
#define SIGEL_SLAVEGUI_SIG_SIMULATIONWIDGETBASE_H

#include "ui_SIG_SimulationWidgetBase.h"

#include <QEvent>
#include <QWidget>

class SIG_SimulationWidgetBase : public QWidget, public Ui::SIG_SimulationWidgetBase
{
    Q_OBJECT

public:
    SIG_SimulationWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_SLAVEGUI_SIG_SIMULATIONWIDGETBASE_H
