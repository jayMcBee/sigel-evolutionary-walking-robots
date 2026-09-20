/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6. The qWarning stubs warn at runtime when the subclass
  forgot to override a connected slot, as the 2003 uic stubs did.
*/
#include "SIGEL_MasterGUI/SIG_EnvironmentBase.h"

SIG_EnvironmentBase::SIG_EnvironmentBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

SIG_EnvironmentBase::~SIG_EnvironmentBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_EnvironmentBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_EnvironmentBase::languageChange()
{
  retranslateUi( this );
}

void SIG_EnvironmentBase::slotAlpha()
{
qWarning("SIG_EnvironmentBase::slotAlpha(): Not implemented yet");
}

void SIG_EnvironmentBase::slotFloorSelectionChanged()
{
qWarning("SIG_EnvironmentBase::slotFloorSelectionChanged(): Not implemented yet");
}

void SIG_EnvironmentBase::slotSelectFile()
{
qWarning("SIG_EnvironmentBase::slotSelectFile(): Not implemented yet");
}

void SIG_EnvironmentBase::slotSelectTextureFile()
{
qWarning("SIG_EnvironmentBase::slotSelectTextureFile(): Not implemented yet");
}

void SIG_EnvironmentBase::slotTextureSelect()
{
qWarning("SIG_EnvironmentBase::slotTextureSelect(): Not implemented yet");
}
