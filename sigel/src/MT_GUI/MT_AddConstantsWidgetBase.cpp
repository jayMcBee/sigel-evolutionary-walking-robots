#include "MT_GUI/MT_AddConstantsWidgetBase.h"

MT_AddConstantsWidgetBase::MT_AddConstantsWidgetBase(QWidget* parent, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  setModal( modal );

  setupUi( this );
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
