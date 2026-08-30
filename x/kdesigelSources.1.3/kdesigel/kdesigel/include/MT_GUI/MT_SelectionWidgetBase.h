/*
  The form's base class -- PORTING.md Phase C, step C2. See C1 for why this
  file exists and where its shape comes from: it is Qt 4.8 uic3's declaration
  mode (`uic3 MT_SelectionWidgetBase.ui`, tools/qtmig), with the Qt 6 adjustments
  C1 lists. Qt 6's uic emits only Ui::MT_SelectionWidgetBase, a struct with setupUi().

  Global namespace, as uic generated it in 2003.
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
    // Qt 3 called languageChange() itself; Qt 6 has no such hook, so it is
    // driven from changeEvent -- otherwise the slot below is dead code.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // MT_GUI_MT_SELECTIONWIDGETBASE_H
