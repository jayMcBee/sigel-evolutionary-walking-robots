// DoubleSpinBox.h: Schnittstelle für die Klasse DoubleSpinBox.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DOUBLESPINBOX_H__B948F50E_5AB1_4AD5_8DC5_52FFC791595D__INCLUDED_)
#define AFX_DOUBLESPINBOX_H__B948F50E_5AB1_4AD5_8DC5_52FFC791595D__INCLUDED_

#include <qspinbox.h>
#include <qvalidator.h>

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
	QString mapValueToText(int value);
	int mapTextToValue(bool *ok);

	int typ;
	int precision;
	int iDecimals;
	QIntValidator *iValidator;
	QDoubleValidator *dValidator;
};

#endif // !defined(AFX_DOUBLESPINBOX_H__B948F50E_5AB1_4AD5_8DC5_52FFC791595D__INCLUDED_)
