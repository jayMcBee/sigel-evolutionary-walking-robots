/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "SIGEL_MasterGUI/SIG_SimulationParameterBase.h"

SIG_SimulationParameterBase::SIG_SimulationParameterBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

SIG_SimulationParameterBase::~SIG_SimulationParameterBase()
{
  // no need to delete child widgets, Qt does it all for us
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
