/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6. The qWarning stubs warn at runtime when the subclass
  forgot to override a connected slot, as the 2003 uic stubs did.
*/
#include "MT_GUI/MT_ExperimentWidgetBase.h"

MT_ExperimentWidgetBase::MT_ExperimentWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );

  // Qt 2's QListView sorted by column 0 ASCENDING by default (qlistview.cpp:
  // 1836-1837 sets sortcolumn=0, ascending=TRUE in init()). The .ui carries
  // sortingEnabled, but Qt 6's
  // setSortingEnabled(true) leaves the indicator on column 0 DESCENDING --
  // measured. Without this the rows come out reversed wherever column 0 holds
  // text, which it does here.
  MTExperimentListView->sortByColumn( 0, Qt::AscendingOrder );
}

MT_ExperimentWidgetBase::~MT_ExperimentWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
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
