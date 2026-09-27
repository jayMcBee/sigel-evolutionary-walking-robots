// DoubleSpinBox.cpp: implementation of class DoubleSpinBox.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/DoubleSpinBox.h"

#include <QLineEdit>
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

DISpinBox::DISpinBox(int decimals, QWidget *parent, const char *name) : QSpinBox(parent)
{
	if ( name )
		setObjectName( QString::fromUtf8( name ) );

	typ = -1;
	precision = 0;
	dValidator = nullptr;
	iValidator = nullptr;
	iDecimals = decimals;

	dValidator = new QDoubleValidator(this);
	iValidator = new QIntValidator(this);

	// Pinned to the C locale, and the group separator rejected: C's group
	// separator is ',' so "0,375" would validate and read back as 0.
	QLocale cLocale = QLocale::c();
	cLocale.setNumberOptions(QLocale::RejectGroupSeparator);
	dValidator->setLocale(cLocale);
	iValidator->setLocale(cLocale);

	// valueChanged comes only from setValue or when editing ends, not on
	// every keystroke.
	setKeyboardTracking(false);

	if(decimals != 0){		// create a DoubleSpinBox
		typ = DBLTYP;
		precision = pow(10, decimals);
		dValidator->setRange(0.0, 100.0, decimals);
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

// Leaves the text alone. QSpinBox::fixup() would rewrite "0.375" to "0".
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