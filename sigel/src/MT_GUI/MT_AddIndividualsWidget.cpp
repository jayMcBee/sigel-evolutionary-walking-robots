/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6.
*/
#include "MT_GUI/MT_AddIndividualsWidget.h"

MT_AddIndividualsWidgetBase::MT_AddIndividualsWidgetBase(QWidget* parent, const char* name, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );
  setModal( modal );

  setupUi( this );
}

MT_AddIndividualsWidgetBase::~MT_AddIndividualsWidgetBase()
{
  // no need to delete child widgets, Qt does it all for us
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
