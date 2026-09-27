#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialogBase.h"

SIG_MovieSettingsDialogBase::SIG_MovieSettingsDialogBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );
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
