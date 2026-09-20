/*
  The class the 2003 uic generated, in uic3 implementation-mode shape,
  adapted to Qt 6. The qWarning stubs warn at runtime when the subclass
  forgot to override a connected slot, as the 2003 uic stubs did.
*/
#include "SIGEL_MasterGUI/SIG_ExperimentViewBase.h"

SIG_ExperimentViewBase::SIG_ExperimentViewBase(QWidget* parent, const char* name, Qt::WindowFlags fl)
  : QWidget( parent, fl )
{
  if ( name )
    setObjectName( QString::fromUtf8( name ) );

  setupUi( this );
}

SIG_ExperimentViewBase::~SIG_ExperimentViewBase()
{
  // no need to delete child widgets, Qt does it all for us
}

void SIG_ExperimentViewBase::changeEvent( QEvent *e )
{
  QWidget::changeEvent( e );
  if ( e->type() == QEvent::LanguageChange )
    languageChange();
}

void SIG_ExperimentViewBase::languageChange()
{
  retranslateUi( this );
}

void SIG_ExperimentViewBase::slotExportPostScript()
{
qWarning("SIG_ExperimentViewBase::slotExportPostScript(): Not implemented yet");
}

void SIG_ExperimentViewBase::slotHistory(bool)
{
qWarning("SIG_ExperimentViewBase::slotHistory(bool): Not implemented yet");
}

void SIG_ExperimentViewBase::slotIntervallChanged(int)
{
qWarning("SIG_ExperimentViewBase::slotIntervallChanged(int): Not implemented yet");
}

void SIG_ExperimentViewBase::slotShowFitnesscurve()
{
qWarning("SIG_ExperimentViewBase::slotShowFitnesscurve(): Not implemented yet");
}
