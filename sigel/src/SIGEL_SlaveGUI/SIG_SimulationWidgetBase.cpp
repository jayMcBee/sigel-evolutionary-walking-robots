#include "SIGEL_SlaveGUI/SIG_SimulationWidgetBase.h"

SIG_SimulationWidgetBase::SIG_SimulationWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
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
