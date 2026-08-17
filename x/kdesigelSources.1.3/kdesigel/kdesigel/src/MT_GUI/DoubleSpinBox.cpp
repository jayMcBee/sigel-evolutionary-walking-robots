// DoubleSpinBox.cpp: Implementierung der Klasse DoubleSpinBox.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/DoubleSpinBox.h"

#include <cmath>

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

DISpinBox::DISpinBox(int decimals, QWidget *parent, const char *name) : QSpinBox(parent, name)
{
	typ = -1;
	precision = 0;
	dValidator = 0;
	iValidator = 0;
	iDecimals = decimals;

	dValidator = new QDoubleValidator(this);
	iValidator = new QIntValidator(this);

	if(decimals != 0){		// create a DoubleSpinBox
		typ = DBLTYP;
		precision = pow(10, decimals);
		dValidator->setRange(0.0, 100.0, decimals);
		setValidator(dValidator);
		setRange(0, dValidator->top() * precision);
		setLineStep(10);
	} else {				// create an IntSpinBox
		typ = INTTYP;
		precision = 1;
		iValidator->setRange(0, 100);
		setValidator(iValidator);
		setRange(0, iValidator->top());
		setLineStep(1);
	}
}

DISpinBox::~DISpinBox()
{
	delete dValidator;
	delete iValidator;
}

int DISpinBox::mapTextToValue(bool *ok)
{
	if(typ == INTTYP)
		return int(text().toInt());
	else 
		return int(text().toDouble()*precision);
}

QString DISpinBox::mapValueToText(int value)
{
	if(typ == INTTYP)
		return QString("%1").arg(value);
	else
		return QString::number(value/precision).append(".").append(QString::number(value%precision).rightJustify(iDecimals, '0'));
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

	setLineStep(1);
	iValidator->setRange(minVal, maxVal);
	setValidator(iValidator);
	QSpinBox::setRange(minVal, maxVal);
}

void DISpinBox::setRange(int decimals, double minVal, double maxVal)
{
	typ = DBLTYP;
	iDecimals = decimals;
	precision = pow(10, decimals);

	setLineStep(10);
	dValidator->setRange(minVal, maxVal, decimals);
	setValidator(dValidator);
	QSpinBox::setRange(minVal, (int)(maxVal * precision));
}

int DISpinBox::getTyp()
{
	return typ;
}