#include <QLocale>
#include <QValidator>
#include "MT_GUI/MT_AddConstantsWidget.h"

#include <QLineEdit>
#include <QButtonGroup>
#include <QSpinBox>
#include <QAbstractButton>
#include <QRadioButton>
#include <QRegularExpression>

namespace
{
  // BOTH validator classes are TRANSCRIBED from vendored Qt 2's
  // qvalidator.cpp rather than patched on top of Qt 6's answers, because Qt 6
  // says Invalid in three places Qt 2 said Intermediate and post-processing
  // one of them missed the other two. Qt 2 returns Invalid ONLY when the text
  // is not a number at all; out of range, too many decimals and a bare "-"
  // are all Intermediate, so QLineEdit keeps accepting keystrokes and the
  // range bites on commit instead of on typing.
  //
  // WHY: the parameter pages' spin boxes keep Qt 6's stricter rule, because
  // the changed value shows before saving. Here it does not: accept() copies
  // these two fields into boss->minValue/maxValue, which
  // go straight to randomizer->createConstant() and become the generated
  // constants. 1.3 generates -50000 when min = max = -50000 in integer mode,
  // with no clamping; Qt 6's rule would silently produce different data.
  //
  // Three separate divergences, all measured on Qt 6.10.2, all fixed here:
  //   QIntValidator(-10000,10000)        typed -50000  -> "-5000"  (tenfold)
  //   QDoubleValidator(100000,-100000,4) typed -50000  -> "50000"  (SIGN FLIP)
  //   QDoubleValidator(...,4)            typed 1.23456 -> "1.2345" (precision)
  // The second is the worst: the
  // dialog OPENS in float mode, and 2003 built that validator with bottom
  // above top -- QDoubleValidator(100000.0, -100000.0, 4) -- which makes
  // Qt 6 reject a leading minus outright, where Qt 2 had no such rule.

  // Qt 2: qvalidator.cpp's QIntValidator::validate, verbatim in behaviour.
  class Qt2IntValidator : public QIntValidator
  {
  public:
    Qt2IntValidator( int bottom, int top, QObject *parent )
      : QIntValidator( bottom, top, parent ) {}

    QValidator::State validate( QString &input, int & ) const override
    {
      // "^ *-? *$" -- empty, a lone minus, and either surrounded by spaces.
      if ( QRegularExpression( QStringLiteral( "^ *-? *$" ) )
               .match( input ).hasMatch() )
        return QValidator::Intermediate;
      bool ok = false;
      // toInt, not toLongLong: Qt 2 used QString::toLong, whose max_mult is
      // INT_MAX/base (qstring.cpp), so it reported failure past +/-INT_MAX
      // whatever the width of `long' was on the platform.
      const int tmp = input.toInt( &ok );
      if ( !ok )
        return QValidator::Invalid;
      return ( tmp < bottom() || tmp > top() ) ? QValidator::Intermediate
                                               : QValidator::Acceptable;
    }

    // fixup() MUST BE A NO-OP, and leaving it inherited re-introduces the
    // defect this class removes. Qt 2's QIntValidator and QDoubleValidator do
    // NOT override fixup at all -- the only definition is QValidator::fixup,
    // an empty body (qvalidator.cpp:175) -- and Qt 2's QLineEdit called it
    // only on Return, never from focusOutEvent. Qt 6 overrides it in both
    // validators AND calls it on Return *and* focus-out. Measured: with the
    // float validator at (-10000, 10000, 4), typing 123.456789 and then
    // clicking away rewrites the field to "1.2346e+02", and accept() stores
    // 123.46. So fixup stays empty: without that the precision is lost one focus
    // change later, in the data path this class exists to protect.
    void fixup( QString & ) const override {}

  };

  // Qt 2: qvalidator.cpp's QDoubleValidator::validate, including its exponent
  // handling and its "too many decimals is Intermediate" rule.
  class Qt2DoubleValidator : public QDoubleValidator
  {
  public:
    Qt2DoubleValidator( double bottom, double top, int decimals, QObject *parent )
      : QDoubleValidator( bottom, top, decimals, parent ) {}

    QValidator::State validate( QString &input, int & ) const override
    {
      // "^ *-?\.? *$"
      if ( QRegularExpression( QStringLiteral( "^ *-?\\.? *$" ) )
               .match( input ).hasMatch() )
        return QValidator::Intermediate;

      bool ok = false;
      double tmp = input.toDouble( &ok );   // locale-independent, as Qt 2's was
      if ( !ok )
        {
          // Qt 2 allowed a mantissa followed by a partial exponent, and a
          // string that is nothing but an exponent tail, so that "1e" and
          // "1e-" stay typeable.
          const QRegularExpression tail( QStringLiteral( "e-?\\d*$" ),
                                         QRegularExpression::CaseInsensitiveOption );
          const QRegularExpressionMatch m = tail.match( input );
          const int eeePos = m.hasMatch() ? m.capturedStart() : -1;
          const int nume = input.count( QLatin1Char( 'e' ), Qt::CaseInsensitive );
          if ( eeePos > 0 && nume < 2 )
            {
              tmp = input.left( eeePos ).toDouble( &ok );
              if ( !ok )
                return QValidator::Invalid;
            }
          else if ( eeePos == 0 )
            return QValidator::Intermediate;
          else
            return QValidator::Invalid;
        }

      const int dot = input.indexOf( QLatin1Char( '.' ) );
      if ( dot >= 0 )
        {
          int j = dot + 1;
          while ( j < input.length() && input[ j ].isDigit() )
            ++j;
          if ( j - ( dot + 1 ) > decimals() )
            return QValidator::Intermediate;   // Qt 6 says Invalid here
        }

      return ( tmp < bottom() || tmp > top() ) ? QValidator::Intermediate
                                               : QValidator::Acceptable;
    }

    // fixup() MUST BE A NO-OP, and leaving it inherited re-introduces the
    // defect this class removes. Qt 2's QIntValidator and QDoubleValidator do
    // NOT override fixup at all -- the only definition is QValidator::fixup,
    // an empty body (qvalidator.cpp:175) -- and Qt 2's QLineEdit called it
    // only on Return, never from focusOutEvent. Qt 6 overrides it in both
    // validators AND calls it on Return *and* focus-out. Measured: with the
    // float validator at (-10000, 10000, 4), typing 123.456789 and then
    // clicking away rewrites the field to "1.2346e+02", and accept() stores
    // 123.46. So fixup stays empty: without that the precision is lost one focus
    // change later, in the data path this class exists to protect.
    void fixup( QString & ) const override {}

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
		minValidator = new Qt2DoubleValidator(100000.0, -100000.0, 4, this);
		maxValidator = new Qt2DoubleValidator(-100000.0, 100000.0, 4, this);
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
			minValidator = new Qt2DoubleValidator(-10000.0, 10000.0, 4, this);
			maxValidator = new Qt2DoubleValidator(-10000.0, 10000.0, 4, this);
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
