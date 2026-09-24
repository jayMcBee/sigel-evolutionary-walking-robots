/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  with the two Qt 6 adjustments marked below.
*/
#include "SIGEL_MasterGUI/SIG_GPParameterBase.h"

#include <QHeaderView>
#include <QTreeWidget>

/*
 *  Constructs a SIG_GPParameterBase which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
SIG_GPParameterBase::SIG_GPParameterBase( QWidget *parent, const char *name,
                                          Qt::WindowFlags fl )
  // uic3 emits QWidget(parent, name, fl). Qt 6's QWidget has no name argument;
  // it was QT3_SUPPORT and did exactly this.
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );

  // The .ui carries sortingEnabled on all four columns. Qt 2's QListView additionally sorted by column 0 ASCENDING
  // by default (qlistview.cpp:1836-1837); Qt 6 leaves the indicator on column 0
  // DESCENDING -- measured, not assumed. Restored here because nothing in the
  // .ui can express it and nothing in SIGEL calls setSorting on this view.
  // Row order is unaffected on shipped data: column 0 holds a pixmap and no
  // text, so every sort key is empty. What differs is the indicator arrow, and
  // therefore which way the user's first header click sorts.
  listviewHosts->sortByColumn( 0, Qt::AscendingOrder );
}

/*
 *  Destroys the object and frees any allocated resources
 */
SIG_GPParameterBase::~SIG_GPParameterBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_GPParameterBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void SIG_GPParameterBase::languageChange()
{
  retranslateUi( this );
}
