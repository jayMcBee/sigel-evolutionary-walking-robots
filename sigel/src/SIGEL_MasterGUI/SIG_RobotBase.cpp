/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "SIGEL_MasterGUI/SIG_RobotBase.h"

SIG_RobotBase::SIG_RobotBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

SIG_RobotBase::~SIG_RobotBase()
{
  // no need to delete child widgets, Qt does it all for us
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
