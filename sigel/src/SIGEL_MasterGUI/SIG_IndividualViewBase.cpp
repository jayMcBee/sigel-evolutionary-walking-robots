#include "SIGEL_MasterGUI/SIG_IndividualViewBase.h"

SIG_IndividualViewBase::SIG_IndividualViewBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
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
