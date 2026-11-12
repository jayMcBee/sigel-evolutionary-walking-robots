#include "SIGEL_MasterGUI/SIG_ExperimentViewBase.h"

SIG_ExperimentViewBase::SIG_ExperimentViewBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
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
