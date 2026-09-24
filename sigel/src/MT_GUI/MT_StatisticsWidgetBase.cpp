/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "MT_GUI/MT_StatisticsWidgetBase.h"

MT_StatisticsWidgetBase::MT_StatisticsWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

MT_StatisticsWidgetBase::~MT_StatisticsWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
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
