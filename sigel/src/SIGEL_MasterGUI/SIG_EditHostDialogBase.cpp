#include "SIGEL_MasterGUI/SIG_EditHostDialogBase.h"

SIG_EditHostDialogBase::SIG_EditHostDialogBase(QWidget* parent, bool modal, Qt::WindowFlags fl)
  : QDialog( parent, fl )
{
  setModal( modal );

  setupUi( this );
}

void SIG_EditHostDialogBase::changeEvent( QEvent *e )
{
  QDialog::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_EditHostDialogBase::languageChange()
{
  retranslateUi( this );
}
