/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "SIGEL_MasterGUI/SIG_LanguageParametersBase.h"

SIG_LanguageParametersBase::SIG_LanguageParametersBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );

  // setSortingEnabled leaves the indicator descending, so the order is set here.
  listviewCommands->sortByColumn( 0, Qt::AscendingOrder );
}

SIG_LanguageParametersBase::~SIG_LanguageParametersBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_LanguageParametersBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_LanguageParametersBase::languageChange()
{
  retranslateUi( this );
}
