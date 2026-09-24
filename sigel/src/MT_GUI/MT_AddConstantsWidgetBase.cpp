/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "MT_GUI/MT_AddConstantsWidgetBase.h"

MT_AddConstantsWidgetBase::MT_AddConstantsWidgetBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );
  setModal( modal );

  setupUi( this );
}

MT_AddConstantsWidgetBase::~MT_AddConstantsWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void MT_AddConstantsWidgetBase::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_AddConstantsWidgetBase::languageChange()
{
  retranslateUi( this );
}
