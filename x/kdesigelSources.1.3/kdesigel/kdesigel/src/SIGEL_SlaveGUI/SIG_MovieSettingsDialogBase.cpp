/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6. The qWarning stubs warn at runtime when the subclass
  forgot to override a connected slot, as the 2003 uic stubs did.
*/
#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialogBase.h"

SIG_MovieSettingsDialogBase::SIG_MovieSettingsDialogBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );
  setModal( modal );

  setupUi( this );
}

SIG_MovieSettingsDialogBase::~SIG_MovieSettingsDialogBase()
{
  // no need to delete child widgets, Qt does it all for us
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

void SIG_MovieSettingsDialogBase::slotChangedAspectRatio(bool)
{
qWarning("SIG_MovieSettingsDialogBase::slotChangedAspectRatio(bool): Not implemented yet");
}

void SIG_MovieSettingsDialogBase::slotSetHeight(int)
{
qWarning("SIG_MovieSettingsDialogBase::slotSetHeight(int): Not implemented yet");
}

void SIG_MovieSettingsDialogBase::slotSetWidth(int)
{
qWarning("SIG_MovieSettingsDialogBase::slotSetWidth(int): Not implemented yet");
}

void SIG_MovieSettingsDialogBase::slotToolButtonClicked()
{
qWarning("SIG_MovieSettingsDialogBase::slotToolButtonClicked(): Not implemented yet");
}
