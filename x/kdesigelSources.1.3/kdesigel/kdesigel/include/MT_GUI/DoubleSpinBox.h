// DoubleSpinBox.h: Schnittstelle für die Klasse DoubleSpinBox.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOUBLESPINBOX_H__B948F50E_5AB1_4AD5_8DC5_52FFC791595D__INCLUDED_)
#define AFX_DOUBLESPINBOX_H__B948F50E_5AB1_4AD5_8DC5_52FFC791595D__INCLUDED_

#include <QSpinBox>
#include <QValidator>
#include <QLocale>

#define INTTYP 0
#define DBLTYP 1

class DISpinBox : public QSpinBox  
{
public:
	DISpinBox(int decimals, QWidget *parent=0, const char *name=0);
	virtual ~DISpinBox();

	int getTyp();

	void setDblValue(double value);
	double dblValue();

	void setIntValue(int value);
	int intValue();

	void setRange(int decimals, double minVal, double maxVal);
	void setRange(int minVal, int maxVal);

private:
	// Qt 2's QSpinBox had virtual mapValueToText/mapTextToValue; Qt 6 calls
	// them textFromValue/valueFromText and both are const. valueFromText is
	// handed the text Qt 2 fetched itself with text().
	QString textFromValue(int value) const override;
	int valueFromText(const QString &t) const override;

	// Qt 6's QSpinBox::validate()/fixup() are an INTEGER parser and run from
	// QAbstractSpinBoxPrivate::interpret() BEFORE the virtual valueFromText, so
	// a typed "0.375" is rejected and rewritten to "0" and the override never
	// sees the fraction. Qt 2 had no such gate: interpretText() called
	// mapTextToValue unconditionally (qspinbox.cpp:725-741) and the QLineEdit's
	// validator was the only constraint on typing. Deferring to that same
	// validator restores 1.3's behaviour.
	QValidator::State validate(QString &input, int &pos) const override;
	void fixup(QString &input) const override;

	int typ;
	int precision;
	int iDecimals;
	QIntValidator *iValidator;
	QDoubleValidator *dValidator;
};

#endif // !defined(AFX_DOUBLESPINBOX_H__B948F50E_5AB1_4AD5_8DC5_52FFC791595D__INCLUDED_)
