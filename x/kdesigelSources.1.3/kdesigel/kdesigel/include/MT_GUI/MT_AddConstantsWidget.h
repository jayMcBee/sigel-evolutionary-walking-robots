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
	// Qt 2's QButtonGroup was a QGroupBox that ALSO managed its buttons and
	// their ids. Qt 6 splits those roles and uic emits only the QGroupBox, so
	// the logical half is rebuilt here -- otherwise find(id) and clicked(int)
	// have nothing to talk to. Qt 2's QButtonGroup::init sets radio_excl=TRUE
	// (qbuttongroup.cpp:167), so radios were exclusive; Qt 6's QButtonGroup
	// defaults exclusive=true, which matches.
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