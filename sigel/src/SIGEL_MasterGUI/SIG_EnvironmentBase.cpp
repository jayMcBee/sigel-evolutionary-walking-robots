#include "SIGEL_MasterGUI/SIG_EnvironmentBase.h"

SIG_EnvironmentBase::SIG_EnvironmentBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
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
