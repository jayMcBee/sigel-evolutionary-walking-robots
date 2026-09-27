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
