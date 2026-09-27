#include "MT_GUI/MT_AddIndividualsWidget.h"

MT_AddIndividualsWidgetBase::MT_AddIndividualsWidgetBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );
  setModal( modal );

  setupUi( this );
}

void MT_AddIndividualsWidgetBase::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void MT_AddIndividualsWidgetBase::languageChange()
{
  retranslateUi( this );
}
