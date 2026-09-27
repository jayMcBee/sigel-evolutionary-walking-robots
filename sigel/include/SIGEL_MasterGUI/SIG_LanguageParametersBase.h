/*
  uic makes only the struct Ui::SIG_LanguageParametersBase. This class inherits
  it and calls setupUi() on itself.
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
    SIG_LanguageParametersBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());
    ~SIG_LanguageParametersBase() override;

public slots:
    virtual void slotPushButtonDisallowAllClicked() = 0;
    virtual void slotPushButtonAllowAllClicked() = 0;
    virtual void slotPushButtonEditClicked() = 0;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();

};

#endif // SIGEL_MASTERGUI_SIG_LANGUAGEPARAMETERSBASE_H
