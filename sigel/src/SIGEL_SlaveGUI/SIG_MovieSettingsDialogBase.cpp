#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialogBase.h"

SIG_MovieSettingsDialogBase::SIG_MovieSettingsDialogBase(QWidget* parent, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  setModal( modal );

  setupUi( this );
}

void SIG_MovieSettingsDialogBase::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_MovieSettingsDialogBase::languageChange()
{
  retranslateUi( this );
}
