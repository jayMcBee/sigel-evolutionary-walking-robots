#include "MT_GUI/MT_IndividualWidgetBase.h"

MT_IndividualsWidgetBase::MT_IndividualsWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

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
