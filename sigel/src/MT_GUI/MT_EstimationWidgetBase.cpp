/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "MT_GUI/MT_EstimationWidgetBase.h"

MT_EstimationWidgetBase::MT_EstimationWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

MT_EstimationWidgetBase::~MT_EstimationWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void MT_EstimationWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_EstimationWidgetBase::languageChange()
{
  retranslateUi( this );
}
