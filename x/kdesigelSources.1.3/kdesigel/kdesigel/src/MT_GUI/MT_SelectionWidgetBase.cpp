/*
  The form's base class -- PORTING.md Phase C, step C2. See the header, and C1
  for the reasoning. This is Qt 4.8 uic3's implementation mode (`uic3 -impl`)
  with the Qt 6 adjustments C1 lists. The qWarning stubs are uic3's own: a
  connection whose slot the subclass forgot to override warns at runtime
  instead of failing silently, which is what the 2003 build did.
*/
#include "MT_GUI/MT_SelectionWidgetBase.h"

MT_SelectionWidgetBase::MT_SelectionWidgetBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

MT_SelectionWidgetBase::~MT_SelectionWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
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
