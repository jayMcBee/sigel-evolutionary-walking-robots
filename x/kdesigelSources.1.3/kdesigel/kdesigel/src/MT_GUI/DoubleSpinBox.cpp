// DoubleSpinBox.cpp: Implementierung der Klasse DoubleSpinBox.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/DoubleSpinBox.h"

#include <QLineEdit>
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

DISpinBox::DISpinBox(int decimals, QWidget *parent, const char *name) : QSpinBox(parent)
{
	if ( name )
		setObjectName( QString::fromUtf8( name ) );

	typ = -1;
	precision = 0;
	dValidator = 0;
	iValidator = 0;
	iDecimals = decimals;

	dValidator = new QDoubleValidator(this);
	iValidator = new QIntValidator(this);

	// 1.3 was locale-INDEPENDENT and the decimal point was always '.'. Two
	// things made that true: QApplication forced setlocale(LC_NUMERIC, "C")
	// at startup (qapplication_x11.cpp:1389), and Qt 2's QDoubleValidator
	// hard-coded the separator -- input.find('.') (qvalidator.cpp:387).
	// Qt 6's validators follow QLocale::system() while QString::toDouble stays
	// locale-independent, so under a comma-decimal locale the validator would
	// reject "0.375" and accept "0,375", which toDouble then reads as 0. That
	// is worse than the bug the validate() override exists to fix. Pinning the
	// validators to the C locale restores 1.3 in every locale.
	// QLocale::c() alone is not enough: C's GROUP separator is ',', so the
	// validator accepts "0,375" as 375 grouped while QString::toDouble()
	// rejects it and yields 0. Qt 2 fed the whole string to strtod and
	// required it be consumed to the NUL, so a comma was Invalid.
	QLocale cLocale = QLocale::c();
	cLocale.setNumberOptions(QLocale::RejectGroupSeparator);
	dValidator->setLocale(cLocale);
	iValidator->setLocale(cLocale);

	// Qt 2's textChanged only set an `edited' flag (qspinbox.cpp:780-783);
	// valueChanged came solely from setValue. Qt 6 interprets every keystroke
	// unless keyboard tracking is off.
	setKeyboardTracking(false);

	if(decimals != 0){		// create a DoubleSpinBox
		typ = DBLTYP;
		precision = pow(10, decimals);
		dValidator->setRange(0.0, 100.0, decimals);
		// Qt 2's QSpinBox::setValidator forwarded to its internal QLineEdit
		// (qspinbox.cpp:680). Qt 6 has no such method, so the line edit is
		// addressed directly -- which is the same object Qt 2 reached.
		lineEdit()->setValidator(dValidator);
		setRange(0, dValidator->top() * precision);
		setSingleStep(10);
	} else {				// create an IntSpinBox
		typ = INTTYP;
		precision = 1;
		iValidator->setRange(0, 100);
		lineEdit()->setValidator(iValidator);
		setRange(0, iValidator->top());
		setSingleStep(1);
	}
}

DISpinBox::~DISpinBox()
{
	delete dValidator;
	delete iValidator;
}

QValidator::State DISpinBox::validate(QString &input, int &pos) const
{
	const QValidator *v = (typ == INTTYP) ? (const QValidator *) iValidator
	                                      : (const QValidator *) dValidator;
	return v ? v->validate(input, pos) : QValidator::Acceptable;
}

// Qt 2 had no fixup on this path; leaving the text alone is the no-op that
// matches. Without the override, QSpinBox::fixup() rewrites "0.375" to "0".
void DISpinBox::fixup(QString &) const
{
}

int DISpinBox::valueFromText(const QString &t) const
{
	if(typ == INTTYP)
		return int(t.toInt());
	else 
		return int(t.toDouble()*precision);
}

QString DISpinBox::textFromValue(int value) const
{
	if(typ == INTTYP)
		return QString("%1").arg(value);
	else
		return QString::number(value/precision).append(".").append(QString::number(value%precision).rightJustified(iDecimals, '0'));
}


void DISpinBox::setDblValue(double value)
{
	setValue(value * precision);
}

void DISpinBox::setIntValue(int value)
{
	setValue(value);
}

double DISpinBox::dblValue()
{
	return (double)value() / (double)precision;
}

int DISpinBox::intValue()
{
	return value();
}

void DISpinBox::setRange(int minVal, int maxVal)
{
	typ = INTTYP;
	precision = 1;

	setSingleStep(1);
	iValidator->setRange(minVal, maxVal);
	lineEdit()->setValidator(iValidator);
	QSpinBox::setRange(minVal, maxVal);
}

void DISpinBox::setRange(int decimals, double minVal, double maxVal)
{
	typ = DBLTYP;
	iDecimals = decimals;
	precision = pow(10, decimals);

	setSingleStep(10);
	dValidator->setRange(minVal, maxVal, decimals);
	lineEdit()->setValidator(dValidator);
	QSpinBox::setRange(minVal, (int)(maxVal * precision));
}

int DISpinBox::getTyp()
{
	return typ;
}