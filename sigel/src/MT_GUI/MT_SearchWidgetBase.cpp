#include "MT_GUI/MT_SearchWidgetBase.h"

MT_SearchWidgetBase::MT_SearchWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );
}

void MT_SearchWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_SearchWidgetBase::languageChange()
{
  retranslateUi( this );
}
