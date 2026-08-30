/*
  The form's base class -- PORTING.md Phase C, step C2. See the header, and C1
  for the reasoning. This is Qt 4.8 uic3's implementation mode (`uic3 -impl`)
  with the Qt 6 adjustments C1 lists. The qWarning stubs are uic3's own: a
  connection whose slot the subclass forgot to override warns at runtime
  instead of failing silently, which is what the 2003 build did.
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
