/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6. The qWarning stubs warn at runtime when the subclass
  forgot to override a connected slot, as the 2003 uic stubs did.
*/
#include "MT_GUI/MT_PopulationWidgetBase.h"

MT_PopulationWidgetBase::MT_PopulationWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );

  // setSortingEnabled leaves the indicator descending, so the order is set here.
  individualListView->sortByColumn( 0, Qt::AscendingOrder );
}

MT_PopulationWidgetBase::~MT_PopulationWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
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
