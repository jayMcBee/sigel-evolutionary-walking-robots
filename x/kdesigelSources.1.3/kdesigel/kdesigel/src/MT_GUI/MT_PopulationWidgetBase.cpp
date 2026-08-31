/*
  The form's base class -- PORTING.md Phase C, step C2. See the header, and C1
  for the reasoning. This is Qt 4.8 uic3's implementation mode (`uic3 -impl`)
  with the Qt 6 adjustments C1 lists. The qWarning stubs are uic3's own: a
  connection whose slot the subclass forgot to override warns at runtime
  instead of failing silently, which is what the 2003 build did.
*/
#include "MT_GUI/MT_PopulationWidgetBase.h"

MT_PopulationWidgetBase::MT_PopulationWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );

  // Qt 2's QTreeWidget sorted by column 0 ASCENDING by default (qlistview.cpp:
  // 1836-1837 sets sortcolumn=0, ascending=TRUE in init()). The .ui carries
  // sortingEnabled, converted from Q3Header's clickable columns, but Qt 6's
  // setSortingEnabled(true) leaves the indicator on column 0 DESCENDING --
  // measured. Without this the rows come out reversed wherever column 0 holds
  // text, which it does here. C1 found this for SIG_GPParameterBase and C2's
  // review found it re-created in the four views C2 converted.
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
