/*
  The form's base class.

  Qt 2's uic generated a QWidget-derived class from the .ui and the hand-written
  SIG_GPParameter derived from it. Qt 6's uic emits only Ui::SIG_GPParameterBase,
  a plain struct with setupUi(), so that class has to come from somewhere.

  IT IS NOT HAND-INVENTED. Qt 4.8's uic3 in declaration mode
  (`uic3 SIG_GPParameterBase.ui`) generates exactly this shape --
  the QWidget + Ui:: multiple inheritance, the three-argument constructor, the
  custom slots as virtuals, and languageChange. The custom slots are pure
  virtual here, so SIG_GPParameter must supply each one. Three other things
  needed changing for Qt 6, each marked below.

  Global namespace, as uic generated it in 2003: SIG_GPParameter is in
  namespace SIGEL_MasterGUI and names this class unqualified.
*/
#ifndef SIGEL_MASTERGUI_SIG_GPPARAMETERBASE_H
#define SIGEL_MASTERGUI_SIG_GPPARAMETERBASE_H

#include "ui_SIG_GPParameterBase.h"

#include <QEvent>
#include <QWidget>

class SIG_GPParameterBase : public QWidget, public Ui::SIG_GPParameterBase
{
    Q_OBJECT

public:
    // uic3 emits `Qt::WindowFlags fl = 0`, which Qt 6's QFlags does not accept.
    SIG_GPParameterBase( QWidget *parent = nullptr, const char *name = nullptr,
                         Qt::WindowFlags fl = Qt::WindowFlags() );
    ~SIG_GPParameterBase() override;

public slots:
    virtual void slotAddHost() = 0;
    virtual void slotChangeGraveyardDir() = 0;
    virtual void slotChangePoolImageDir() = 0;
    virtual void slotCrossoverChanged( int ) = 0;
    virtual void slotDeleteHost() = 0;
    virtual void slotDisableAllHosts() = 0;
    virtual void slotEditHost() = 0;
    virtual void slotEnableAllHosts() = 0;
    virtual void slotMutationChanged( int ) = 0;
    virtual void slotTourPerGenChanged( int ) = 0;

protected:
    // Drives languageChange; without it the slot never runs.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();
};

#endif // SIGEL_MASTERGUI_SIG_GPPARAMETERBASE_H
