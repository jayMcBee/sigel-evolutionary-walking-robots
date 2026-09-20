#ifndef MT_GUI_ESTIMATIONWIDGET
#define MT_GUI_ESTIMATIONWIDGET

#include "MT_GUI/MT_EstimationWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/DoubleSpinBox.h"

class MT_EstimationWidget : public MT_EstimationWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_EstimationWidget(QWidget* parent=0, const char* name=0, Qt::WindowFlags fl = Qt::WindowFlags() );
	~MT_EstimationWidget();

	void evolutionRunning(bool running);
	bool onHide(MT_GPManager *manager, subst_cache *subst);
	void onShow(MT_GPManager *manager, subst_cache *subst);

public slots:
	void slotStrategyChanged(int index);

private:
	enum { evaluator, classifier } usedSys;
	DISpinBox *toleranceSpinBox;
	int lastIndex;
	double tol0, tol1, tol2, tol3, tol4;
};

#endif