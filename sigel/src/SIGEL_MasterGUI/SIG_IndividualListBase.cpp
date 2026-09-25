/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "SIGEL_MasterGUI/SIG_IndividualListBase.h"

#include <QHeaderView>

SIG_IndividualListBase::SIG_IndividualListBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );

  // setSortingEnabled leaves the indicator descending, so the order is set here.
  listviewIndividuals->sortByColumn( 0, Qt::AscendingOrder );

  // The name takes the free width; fitness, length and age are as wide as
  // their text. No column is narrower than fourteen digits.
  QHeaderView *header = listviewIndividuals->header();
  header->setMinimumSectionSize( header->fontMetrics().horizontalAdvance( QString( 14, '0' ) ) );
  header->setStretchLastSection( false );
  header->setSectionResizeMode( 0, QHeaderView::Stretch );
  header->setSectionResizeMode( 1, QHeaderView::ResizeToContents );
  header->setSectionResizeMode( 2, QHeaderView::ResizeToContents );
  header->setSectionResizeMode( 3, QHeaderView::ResizeToContents );
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
