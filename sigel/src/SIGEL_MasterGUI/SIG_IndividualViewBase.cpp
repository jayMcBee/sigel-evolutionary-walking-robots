#include "SIGEL_MasterGUI/SIG_IndividualViewBase.h"

SIG_IndividualViewBase::SIG_IndividualViewBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

void SIG_IndividualViewBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_IndividualViewBase::languageChange()
{
  retranslateUi( this );
}
