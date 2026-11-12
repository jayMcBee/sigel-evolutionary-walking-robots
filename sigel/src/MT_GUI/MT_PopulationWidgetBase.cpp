#include "MT_GUI/MT_PopulationWidgetBase.h"

MT_PopulationWidgetBase::MT_PopulationWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );

  // setSortingEnabled leaves the indicator descending, so the order is set here.
  individualListView->sortByColumn( 0, Qt::AscendingOrder );
}

void MT_PopulationWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_PopulationWidgetBase::languageChange()
{
  retranslateUi( this );
}
