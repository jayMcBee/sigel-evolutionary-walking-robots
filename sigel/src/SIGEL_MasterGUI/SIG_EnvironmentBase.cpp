#include "SIGEL_MasterGUI/SIG_EnvironmentBase.h"

SIG_EnvironmentBase::SIG_EnvironmentBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
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
