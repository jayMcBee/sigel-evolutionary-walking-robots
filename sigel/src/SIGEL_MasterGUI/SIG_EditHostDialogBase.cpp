/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6. The qWarning stubs warn at runtime when the subclass
  forgot to override a connected slot, as the 2003 uic stubs did.
*/
#include "SIGEL_MasterGUI/SIG_EditHostDialogBase.h"

SIG_EditHostDialogBase::SIG_EditHostDialogBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );
  setModal( modal );

  setupUi( this );
}

SIG_EditHostDialogBase::~SIG_EditHostDialogBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_EditHostDialogBase::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_EditHostDialogBase::languageChange()
{
  retranslateUi( this );
}

void SIG_EditHostDialogBase::slotToolbuttonSlaveDirectoryClicked()
{
qWarning("SIG_EditHostDialogBase::slotToolbuttonSlaveDirectoryClicked(): Not implemented yet");
}
