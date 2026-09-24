/*
  The class the 2003 uic generated, in uic3 declaration-mode shape,
  adapted to Qt 6. Qt 6's uic emits only Ui::SIG_LanguageParametersBase,
  a struct with setupUi().

  Global namespace, as uic generated it in 2003.
*/
#ifndef SIGEL_MASTERGUI_SIG_LANGUAGEPARAMETERSBASE_H
#define SIGEL_MASTERGUI_SIG_LANGUAGEPARAMETERSBASE_H

#include "ui_SIG_LanguageParametersBase.h"

#include <QEvent>
#include <QWidget>

class SIG_LanguageParametersBase : public QWidget, public Ui::SIG_LanguageParametersBase
{
    Q_OBJECT

public:
    SIG_LanguageParametersBase(QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_LanguageParametersBase() override;

public slots:
    virtual void slotPushButtonDisallowAllClicked() = 0;
    virtual void slotPushButtonAllowAllClicked() = 0;
    virtual void slotPushButtonEditClicked() = 0;

protected:
    // Qt 3 called languageChange() itself; Qt 6 has no such hook, so it is
    // driven from changeEvent -- otherwise the slot below is dead code.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_LANGUAGEPARAMETERSBASE_H
