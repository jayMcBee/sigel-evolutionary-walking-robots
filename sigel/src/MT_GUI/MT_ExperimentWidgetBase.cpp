#include "MT_GUI/MT_ExperimentWidgetBase.h"

MT_ExperimentWidgetBase::MT_ExperimentWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );

  // setSortingEnabled leaves the indicator descending, so the order is set here.
  MTExperimentListView->sortByColumn( 0, Qt::AscendingOrder );
}

void MT_ExperimentWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_ExperimentWidgetBase::languageChange()
{
  retranslateUi( this );
}
