/*
  uic makes only the struct Ui::MT_PopulationWidgetBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_POPULATIONWIDGETBASE_H
#define MT_GUI_MT_POPULATIONWIDGETBASE_H

#include "ui_MT_PopulationWidgetBase.h"

#include <QEvent>
#include <QWidget>

class MT_PopulationWidgetBase : public QWidget, public Ui::MT_PopulationWidgetBase
{
    Q_OBJECT

public:
    MT_PopulationWidgetBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MT_PopulationWidgetBase() override;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_POPULATIONWIDGETBASE_H
