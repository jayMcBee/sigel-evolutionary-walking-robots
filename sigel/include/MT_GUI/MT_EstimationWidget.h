#ifndef MT_GUI_ESTIMATIONWIDGET
#define MT_GUI_ESTIMATIONWIDGET

#include "MT_GUI/MT_EstimationWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/DoubleSpinBox.h"
#include "MT_Control/MT_SubstituteKind.h"

class MT_EstimationWidget : public MT_EstimationWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_EstimationWidget(QWidget* parent=nullptr, const char* name=nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

	bool onHide(MT_GPManager *manager, MT_EstimationState *estimationState) override;
	void onShow(MT_GPManager *manager, MT_EstimationState *estimationState) override;

public slots:
	void slotStrategyChanged(int index);

private:
	MT_SubstituteKind usedSys;
	DISpinBox *toleranceSpinBox;
	int lastIndex;
	double tol0, tol1, tol2, tol3, tol4;
};

#endif