/*
  The form's base class -- PORTING.md Phase C, step C2. See the header, and C1
  for the reasoning. This is Qt 4.8 uic3's implementation mode (`uic3 -impl`)
  with the Qt 6 adjustments C1 lists. The qWarning stubs are uic3's own: a
  connection whose slot the subclass forgot to override warns at runtime
  instead of failing silently, which is what the 2003 build did.
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
