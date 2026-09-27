#include "SIGEL_MasterGUI/SIG_SimulationParameterBase.h"

SIG_SimulationParameterBase::SIG_SimulationParameterBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

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
