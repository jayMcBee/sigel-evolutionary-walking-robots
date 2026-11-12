#include "MT_GUI/MT_SelectionWidgetBase.h"

MT_SelectionWidgetBase::MT_SelectionWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );
}

void MT_SelectionWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_SelectionWidgetBase::languageChange()
{
  retranslateUi( this );
}
