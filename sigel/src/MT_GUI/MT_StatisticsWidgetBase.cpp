#include "MT_GUI/MT_StatisticsWidgetBase.h"

MT_StatisticsWidgetBase::MT_StatisticsWidgetBase(QWidget* parent, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  setupUi( this );
}

void MT_StatisticsWidgetBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_StatisticsWidgetBase::languageChange()
{
  retranslateUi( this );
}
