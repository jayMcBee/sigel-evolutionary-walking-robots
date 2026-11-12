#include "SIGEL_MasterGUI/SIG_RobotBase.h"

SIG_RobotBase::SIG_RobotBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
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
