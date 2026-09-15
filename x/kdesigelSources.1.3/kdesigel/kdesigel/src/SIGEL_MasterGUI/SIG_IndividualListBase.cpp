/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6. The qWarning stubs warn at runtime when the subclass
  forgot to override a connected slot, as the 2003 uic stubs did.
*/
#include "SIGEL_MasterGUI/SIG_IndividualListBase.h"

SIG_IndividualListBase::SIG_IndividualListBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
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
  listviewIndividuals->sortByColumn( 0, Qt::AscendingOrder );
}

SIG_IndividualListBase::~SIG_IndividualListBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_IndividualListBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_IndividualListBase::languageChange()
{
  retranslateUi( this );
}
