/*
  uic makes only the struct Ui::SIG_EnvironmentBase. This class inherits it and
  calls setupUi() on itself.
*/
#ifndef SIGEL_MASTERGUI_SIG_ENVIRONMENTBASE_H
#define SIGEL_MASTERGUI_SIG_ENVIRONMENTBASE_H

#include "ui_SIG_EnvironmentBase.h"

#include <QEvent>
#include <QWidget>

class SIG_EnvironmentBase : public QWidget, public Ui::SIG_EnvironmentBase
{
    Q_OBJECT

public:
    SIG_EnvironmentBase(QWidget* parent = nullptr, const char* name = nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

public slots:
    virtual void slotAlpha() = 0;
    virtual void slotCenterOnTerrain() = 0;
    virtual void slotFloorSelectionChanged() = 0;
    virtual void slotSelectFile() = 0;
    virtual void slotSelectTextureFile() = 0;
    virtual void slotTextureSelect() = 0;

protected:
    // Qt does not call languageChange() itself.
    // changeEvent calls it on a language change.
    void changeEvent( QEvent *e ) override;

protected slots:
    virtual void languageChange();
};

#endif // SIGEL_MASTERGUI_SIG_ENVIRONMENTBASE_H
