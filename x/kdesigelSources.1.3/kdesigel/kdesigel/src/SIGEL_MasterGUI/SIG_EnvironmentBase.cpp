/*
  The form's base class -- PORTING.md Phase C, step C2. See the header, and C1
  for the reasoning. This is Qt 4.8 uic3's implementation mode (`uic3 -impl`)
  with the Qt 6 adjustments C1 lists. The qWarning stubs are uic3's own: a
  connection whose slot the subclass forgot to override warns at runtime
  instead of failing silently, which is what the 2003 build did.
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
