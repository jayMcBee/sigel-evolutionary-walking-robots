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
  // Own validators, not Qt's. Qt's reject some keystrokes, so typing -50000
  // leaves "-5000". The text goes into the generated constants unchecked.

  // Keeps any whole number that is typed. Out of range is Intermediate, not
  // Invalid, so the keystroke stays.
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
      // toInt, not toLongLong: a number past INT_MAX must be Invalid, so it
      // cannot reach the generated constants.
      const int tmp = input.toInt( &ok );
      if ( !ok )
        return QValidator::Invalid;
      return ( tmp < bottom() || tmp > top() ) ? QValidator::Intermediate
                                               : QValidator::Acceptable;
    }

    // Must stay empty. QLineEdit calls fixup on focus-out, and the inherited
    // one rewrites the text in its own number format.
    void fixup( QString & ) const override {}

  };

  // Keeps any number that is typed, also with an exponent. Out of range or too
  // many decimals is Intermediate, not Invalid, so the keystroke stays.
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
      double tmp = input.toDouble( &ok );   // C format, whatever the system locale
      if ( !ok )
        {
          // A partial exponent after the number, such as "1e" or "1e-", stays
          // typeable. So does a string that is only an exponent, such as "e-".
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

    // Must stay empty. Qt 6 calls fixup on focus-out, and the inherited one
    // rewrites 123.456789 to "1.2346e+02".
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
	
	// slotClicked compares only the objectName, so the ids only have to differ.
	typeButtons = new QButtonGroup(this);
	typeButtons->addButton(intRadioButton, 0);
	typeButtons->addButton(floatRadioButton, 1);

	connect(typeButtons, SIGNAL(idClicked(int)), SLOT(slotClicked(int)));
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
