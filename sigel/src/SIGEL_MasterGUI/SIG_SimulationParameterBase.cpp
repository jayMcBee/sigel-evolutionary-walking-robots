#include "SIGEL_MasterGUI/SIG_SimulationParameterBase.h"

SIG_SimulationParameterBase::SIG_SimulationParameterBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );
}

void SIG_SimulationParameterBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_SimulationParameterBase::languageChange()
{
  retranslateUi( this );
}
