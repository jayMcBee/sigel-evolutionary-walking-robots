/*
  uic makes only the struct Ui::SIG_GPParameterBase. This class inherits it and
  calls setupUi() on itself.

  The custom slots are pure virtual, so SIG_GPParameter must supply each one.
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
    SIG_GPParameterBase( QWidget *parent = nullptr, const char *name = nullptr,
                         Qt::WindowFlags fl = Qt::WindowFlags() );

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
