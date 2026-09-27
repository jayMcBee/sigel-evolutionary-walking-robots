#include "MT_GUI/MT_SearchWidgetBase.h"

MT_SearchWidgetBase::MT_SearchWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

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
