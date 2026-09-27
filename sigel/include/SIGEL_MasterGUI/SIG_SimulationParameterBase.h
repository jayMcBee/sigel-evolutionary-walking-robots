/*
  uic makes only the struct Ui::SIG_SimulationParameterBase. This class inherits
  it and calls setupUi() on itself.
*/
#ifndef SIGEL_MASTERGUI_SIG_SIMULATIONPARAMETERBASE_H
#define SIGEL_MASTERGUI_SIG_SIMULATIONPARAMETERBASE_H

#include "ui_SIG_SimulationParameterBase.h"

#include <QEvent>
#include <QWidget>

class SIG_SimulationParameterBase : public QWidget, public Ui::SIG_SimulationParameterBase
{
    Q_OBJECT

public:
    SIG_SimulationParameterBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_SimulationParameterBase() override;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_SIMULATIONPARAMETERBASE_H
