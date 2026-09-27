#include "SIGEL_MasterGUI/SIG_RobotBase.h"

SIG_RobotBase::SIG_RobotBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

void SIG_RobotBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_RobotBase::languageChange()
{
  retranslateUi( this );
}
