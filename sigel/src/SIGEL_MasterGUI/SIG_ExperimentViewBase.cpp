#include "SIGEL_MasterGUI/SIG_ExperimentViewBase.h"

SIG_ExperimentViewBase::SIG_ExperimentViewBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
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
