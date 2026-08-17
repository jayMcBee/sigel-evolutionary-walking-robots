#ifndef MT_ADDCONSTANTSWIDGET
#define MT_ADDCONSTANTSWIDGET

#include "MT_GUI/MT_AddConstantsWidgetBase.h"
#include "MT_GUI/MT_IndividualWidget.h"

#include <qvalidator.h>

class MT_AddConstantsWidget : public MT_AddConstantsWidgetBase
{
	Q_OBJECT

public:
	MT_AddConstantsWidget(MT_IndividualsWidget *parent=0, const char *name=0, bool modal=TRUE, WFlags fl=0 );
	~MT_AddConstantsWidget();

private:
	MT_IndividualsWidget *boss;
	enum type { intType, floatType } selectedType;
	QValidator	*minValidator;
	QValidator	*maxValidator;

public slots:
	void slotClicked(int id);
	void accept();
};

#endif