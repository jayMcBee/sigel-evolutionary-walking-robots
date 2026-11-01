#ifndef MT_GUI_DOUBLESPINBOX_H
#define MT_GUI_DOUBLESPINBOX_H

#include <QSpinBox>
#include <QValidator>
#include <QLocale>
#include "MT_GUI/MT_SpinBoxValueType.h"

class DISpinBox : public QSpinBox  
{
public:
	DISpinBox(int decimals, QWidget *parent=nullptr, const char *name=nullptr);
	~DISpinBox() override;

	MT_SpinBoxValueType getType();

	void setDoubleValue(double value);
	double doubleValue();

	void setIntValue(int value);
	int intValue();

	void setRange(int decimals, double minVal, double maxVal);
	void setRange(int minVal, int maxVal);

private:
	QString textFromValue(int value) const override;
	int valueFromText(const QString &t) const override;

	// QSpinBox's own validate is an integer parser and runs before
	// valueFromText, so "0.375" would be rewritten to "0".
	QValidator::State validate(QString &input, int &pos) const override;
	void fixup(QString &input) const override;

	MT_SpinBoxValueType typ;
	int precision;
	int iDecimals;
	QIntValidator *iValidator;
	QDoubleValidator *dValidator;
};

#endif // MT_GUI_DOUBLESPINBOX_H
