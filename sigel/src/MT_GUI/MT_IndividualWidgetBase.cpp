#include "MT_GUI/MT_IndividualWidgetBase.h"

MT_IndividualsWidgetBase::MT_IndividualsWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );
}

void MT_IndividualsWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_IndividualsWidgetBase::languageChange()
{
  retranslateUi( this );
}
