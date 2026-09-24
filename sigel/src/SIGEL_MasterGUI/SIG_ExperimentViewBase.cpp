/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "SIGEL_MasterGUI/SIG_ExperimentViewBase.h"

SIG_ExperimentViewBase::SIG_ExperimentViewBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

SIG_ExperimentViewBase::~SIG_ExperimentViewBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_ExperimentViewBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_ExperimentViewBase::languageChange()
{
  retranslateUi( this );
}
