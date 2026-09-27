/*
  uic makes only the struct Ui::SIG_IndividualListBase. This class inherits it
  and calls setupUi() on itself.
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
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_INDIVIDUALLISTBASE_H
