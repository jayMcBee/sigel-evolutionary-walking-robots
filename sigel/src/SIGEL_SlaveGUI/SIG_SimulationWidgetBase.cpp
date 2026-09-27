#include "SIGEL_SlaveGUI/SIG_SimulationWidgetBase.h"

SIG_SimulationWidgetBase::SIG_SimulationWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

void SIG_SimulationWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_SimulationWidgetBase::languageChange()
{
  retranslateUi( this );
}
