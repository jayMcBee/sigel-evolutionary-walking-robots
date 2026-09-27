/*
  uic makes only the struct Ui::MT_StatisticsWidgetBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef MT_GUI_MT_STATISTICSWIDGETBASE_H
#define MT_GUI_MT_STATISTICSWIDGETBASE_H

#include "ui_MT_StatisticsWidgetBase.h"

#include <QEvent>
#include <QWidget>

class MT_StatisticsWidgetBase : public QWidget, public Ui::MT_StatisticsWidgetBase
{
    Q_OBJECT

public:
    MT_StatisticsWidgetBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags());
    ~MT_StatisticsWidgetBase() override;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_STATISTICSWIDGETBASE_H
