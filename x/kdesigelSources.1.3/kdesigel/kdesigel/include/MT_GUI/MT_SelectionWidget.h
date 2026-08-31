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
	MT_SelectionWidget(QWidget* parent=0, const char* name=0, Qt::WindowFlags fl = Qt::WindowFlags());
	~MT_SelectionWidget();

	void evolutionRunning(bool running);
	void onShow(MT_GPManager *manager, subst_cache *subst);
	bool onHide(MT_GPManager *manager, subst_cache *subst);

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