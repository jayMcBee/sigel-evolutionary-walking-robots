/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "MT_GUI/MT_IndividualWidgetBase.h"

MT_IndividualsWidgetBase::MT_IndividualsWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

MT_IndividualsWidgetBase::~MT_IndividualsWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
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
