#include <QLocale>
#include <QValidator>
#include "MT_GUI/MT_AddConstantsWidget.h"

#include <QLineEdit>
#include <QButtonGroup>
#include <QSpinBox>
#include <QAbstractButton>
#include <QRadioButton>

namespace
{
  // Qt 2's QIntValidator::validate returned INTERMEDIATE for a value outside
  // [bottom,top] (qvalidator.cpp:236), so QLineEdit accepted every digit and
  // the range bit only on commit. Qt 6's returns INVALID once the typed prefix
  // passes the top, and QLineEdit drops the keystroke, leaving a truncated
  // prefix.
  //
  // §9's D28 ACCEPTED that divergence for the five parameter pages' 29 spin
  // boxes: there the differing value is visible in the box before anything is
  // saved, and restoring it would mean owning a custom spin box forever.
  // THIS SITE IS NOT THAT CASE, and the difference is measured rather than
  // argued. On 1.3, integer mode with min = max = -50000 and a count of 3
  // generates three constants of -50000 -- no clamping at any point, confirmed
  // on the running binary, with min set equal to max so the value could not be
  // a random draw. A port holding -5000 generates constants of -5000: a
  // TENFOLD difference in data that reaches the MetaGP population and is
  // invisible from then on.
  //
  // So Qt 2's rule is RESTORED here rather than accepted. That preserves 1.3's
  // behaviour rather than improving on it -- the same argument as pinning
  // these validators to QLocale::c(), which also restores what Qt 2 did.
  class Qt2IntValidator : public QIntValidator
  {
  public:
    Qt2IntValidator( int bottom, int top, QObject *parent )
      : QIntValidator( bottom, top, parent ) {}

    QValidator::State validate( QString &input, int &pos ) const override
    {
      const QValidator::State s = QIntValidator::validate( input, pos );
      if ( s != QValidator::Invalid )
        return s;
      // Qt 6 answers Invalid both for "out of range" and for "not a number";
      // only the first was Intermediate in Qt 2. So they are separated here:
      // anything parsing as an integer is merely out of range and becomes
      // Intermediate, while letters stay Invalid and are still dropped --
      // which is the control that this does not just disable the validator.
      if ( input.isEmpty() || input == QLatin1String( "-" ) )
        return QValidator::Intermediate;
      bool ok = false;
      (void) input.toLongLong( &ok );
      return ok ? QValidator::Intermediate : QValidator::Invalid;
    }
  };
}

MT_AddConstantsWidget::MT_AddConstantsWidget(MT_IndividualsWidget *parent, const char *name, bool modal, Qt::WindowFlags fl)
	: MT_AddConstantsWidgetBase(parent, name, true, fl)
{
	boss = parent;
	if(boss->integer){
		selectedType = intType;
		intRadioButton->setDown(true);
		floatRadioButton->setDown(false);
		minValidator = new Qt2IntValidator(-10000, 10000, this);
		maxValidator = new Qt2IntValidator(-10000, 10000, this);
	} else {
		selectedType = floatType;
		intRadioButton->setDown(false);
		floatRadioButton->setDown(true);
		minValidator = new QDoubleValidator(100000.0, -100000.0, 4, this);
		maxValidator = new QDoubleValidator(-100000.0, 100000.0, 4, this);
	}
	minValueEdit->setValidator(minValidator);
	minValueEdit->setText(tr("%1").arg(boss->minValue));
	maxValueEdit->setValidator(maxValidator);
	maxValueEdit->setText(tr("%1").arg(boss->maxValue));
	// The C locale for both validators -- see MT_IndividualWidget.cpp. Applied
	// at BOTH creation sites: the type radio deletes and rebuilds them, so a
	// constructor-only pinning would be undone the first time a user switches
	// between integer and float constants.
	{
		QLocale cLocale = QLocale::c();
		cLocale.setNumberOptions(QLocale::RejectGroupSeparator);
		for (QValidator *v : findChildren<QValidator *>())
			v->setLocale(cLocale);
	}
	numConstantsSpinBox->setValue(boss->numToCreate);
	
	// Ids in .ui order, which is the order Qt 2's QButtonGroup auto-assigned.
	// Only the objectName is ever compared, so the values matter merely for
	// being distinct and stable.
	typeButtons = new QButtonGroup(this);
	typeButtons->addButton(intRadioButton, 0);
	typeButtons->addButton(floatRadioButton, 1);

	// QButtonGroup::clicked(int) is gone in Qt 6; the id-carrying signal is
	// idClicked(int). clicked() without an id is still live, which is why only
	// the int overload moves.
	connect(typeButtons, SIGNAL(idClicked(int)), SLOT(slotClicked(int)));
}

MT_AddConstantsWidget::~MT_AddConstantsWidget()
{

}

void MT_AddConstantsWidget::accept()
{
	boss->numToCreate = numConstantsSpinBox->text().toInt();
	boss->minValue = minValueEdit->text().toDouble();
	boss->maxValue = maxValueEdit->text().toDouble();
	boss->integer  = (selectedType == intType) ? true : false;

	QDialog::accept();
}

void MT_AddConstantsWidget::slotClicked(int id)
{
	// Qt 2: QButtonGroup::find(int) -> QButton*, QObject::name() -> const char*.
	if(typeButtons->button(id)->objectName() == QString("intRadioButton")){
		if(selectedType != intType){
			selectedType = intType;
			delete minValidator;
			delete maxValidator;
			minValidator = new Qt2IntValidator(-10000, 10000, this);
			maxValidator = new Qt2IntValidator(-10000, 10000, this);
			minValueEdit->setValidator(minValidator);
			maxValueEdit->setValidator(maxValidator);
			minValueEdit->setText(tr("%1").arg((int)minValueEdit->text().toDouble()));
			maxValueEdit->setText(tr("%1").arg((int)maxValueEdit->text().toDouble()));
					{
				QLocale cLocale = QLocale::c();
				cLocale.setNumberOptions(QLocale::RejectGroupSeparator);
				for (QValidator *v : findChildren<QValidator *>())
					v->setLocale(cLocale);
	}
		}
	} else {
		if(selectedType != floatType){
			selectedType = floatType;
			delete minValidator;
			delete maxValidator;
			minValidator = new QDoubleValidator(-10000.0, 10000.0, 4, this);
			maxValidator = new QDoubleValidator(-10000.0, 10000.0, 4, this);
			minValueEdit->setValidator(minValidator);
			maxValueEdit->setValidator(maxValidator);
			minValueEdit->setText(tr("%1").arg((double)minValueEdit->text().toInt()));
			maxValueEdit->setText(tr("%1").arg((double)maxValueEdit->text().toInt()));
					{
				QLocale cLocale = QLocale::c();
				cLocale.setNumberOptions(QLocale::RejectGroupSeparator);
				for (QValidator *v : findChildren<QValidator *>())
					v->setLocale(cLocale);
	}
		}
	} 		
}
