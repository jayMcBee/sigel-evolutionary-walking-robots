#include "SIGEL_MasterGUI/SIG_GPParameterBase.h"

#include <QHeaderView>
#include <QTreeWidget>

/*
 *  Constructs a SIG_GPParameterBase which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'
 */
SIG_GPParameterBase::SIG_GPParameterBase( QWidget *parent, const char *name,
                                          Qt::WindowFlags fl )
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );

  // setSortingEnabled leaves the indicator descending, so the order is set here.
  // Column 0 holds only a pixmap, so this sets only the arrow and the direction
  // of the first click.
  listviewHosts->sortByColumn( 0, Qt::AscendingOrder );
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
