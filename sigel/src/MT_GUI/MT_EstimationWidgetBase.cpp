#include "MT_GUI/MT_EstimationWidgetBase.h"

MT_EstimationWidgetBase::MT_EstimationWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );
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
