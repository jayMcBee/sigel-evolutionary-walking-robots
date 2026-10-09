#ifndef MT_GUI_SELECTIONWIDGET
#define MT_GUI_SELECTIONWIDGET

#include "MT_GUI/MT_SelectionWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"

#include <QMap>
#include <QString>

class MT_SelectionWidget : public MT_SelectionWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_SelectionWidget(QWidget* parent=nullptr, const char* name=nullptr, Qt::WindowFlags fl = Qt::WindowFlags());

	void onShow(MT_GPManager *manager, MT_EstimationState *estimationState);
	bool onHide(MT_GPManager *manager, MT_EstimationState *estimationState);

	int calculateTournSize(int pSize, int oSize, int oTSize);

private:
	int parentSize;
	int lastParentSize;
	QMap<int, int>	tourSizeMap;
	void updateTSizeList(int pSize, int oSize);

private slots:
	void slotOverProdChanged(int nvalue);
	void slotTourSizeChanged(const QString &text);
};

#endif