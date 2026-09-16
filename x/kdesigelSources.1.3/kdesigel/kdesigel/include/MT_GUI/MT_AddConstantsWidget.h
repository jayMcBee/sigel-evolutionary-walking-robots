#ifndef MT_ADDCONSTANTSWIDGET
#define MT_ADDCONSTANTSWIDGET

#include "MT_GUI/MT_AddConstantsWidgetBase.h"
#include "MT_GUI/MT_IndividualWidget.h"

#include <QValidator>

class MT_AddConstantsWidget : public MT_AddConstantsWidgetBase
{
	Q_OBJECT

public:
	MT_AddConstantsWidget(MT_IndividualsWidget *parent=0, const char *name=0, bool modal=true, Qt::WindowFlags fl = Qt::WindowFlags() );
	~MT_AddConstantsWidget();

private:
	// uic emits only the QGroupBox, so the button group is built here; without
	// it find(id) and clicked(int) have nothing to talk to.
	QButtonGroup *typeButtons;

	MT_IndividualsWidget *boss;
	enum type { intType, floatType } selectedType;
	QValidator	*minValidator;
	QValidator	*maxValidator;

public slots:
	void slotClicked(int id);
	void accept();
};

#endif