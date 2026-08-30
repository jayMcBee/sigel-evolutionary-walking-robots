/*
  The form's base class -- PORTING.md Phase C, step C2. See C1 for why this
  file exists and where its shape comes from: it is Qt 4.8 uic3's declaration
  mode (`uic3 SIG_IndividualListBase.ui`, tools/qtmig), with the Qt 6 adjustments
  C1 lists. Qt 6's uic emits only Ui::SIG_IndividualListBase, a struct with setupUi().

  Global namespace, as uic generated it in 2003.
*/
#ifndef SIGEL_MASTERGUI_SIG_INDIVIDUALLISTBASE_H
#define SIGEL_MASTERGUI_SIG_INDIVIDUALLISTBASE_H

#include "ui_SIG_IndividualListBase.h"

#include <QEvent>
#include <QWidget>

class SIG_IndividualListBase : public QWidget, public Ui::SIG_IndividualListBase
{
    Q_OBJECT

public:
    SIG_IndividualListBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_IndividualListBase() override;

protected:
    // Qt 3 called languageChange() itself; Qt 6 has no such hook, so it is
    // driven from changeEvent -- otherwise the slot below is dead code.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_INDIVIDUALLISTBASE_H
