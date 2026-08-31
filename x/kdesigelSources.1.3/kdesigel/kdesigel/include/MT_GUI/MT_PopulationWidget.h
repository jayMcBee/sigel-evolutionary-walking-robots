#ifndef MT_GUI_POPULATIONWIDGET
#define MT_GUI_POPULATIONWIDGET

#include "MT_GUI/MT_PopulationWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/MT_PopListViewItem.h"
#include "MT_GPSystem/MT_Population.h"
#include "MT_GPSystem/MT_GPManager.h"

#include <QAction>
#include <QToolBar>
#include <QTreeWidget>
#include <QMenu>

class MT_MainWindow;

class MT_PopulationWidget : public MT_PopulationWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	virtual bool onHide(MT_GPManager *manager, subst_cache *subst);
	virtual void onShow(MT_GPManager *manager, subst_cache *subst);
	void evolutionRunning(bool running);
	MT_PopulationWidget(QMainWindow* parent=0, const char* name=0, Qt::WindowFlags fl = Qt::WindowFlags());
	~MT_PopulationWidget();

private:
	int oldPopSize;
	QMenu *popContextMenu;
	QToolBar *popToolBar;
	QAction	*addIndAction;
	QAction	*delIndAction;
	QAction *impIndAction;
	QAction *expIndAction;
	QAction	*savePopAction;
	QAction	*loadPopAction;
	MT_Population *population;
	MT_GPManager  *gpManager;
	MT_MainWindow *boss;
	QList<MT_PopListViewItem *>* getSelectedItems();

private slots:
	void slotRButtonClicked(const QPoint &pos);
	void slotCurrentChanged(QTreeWidgetItem *item);
	void slotSelectionChanged();
	void slotAddInd();
	void slotDelInd();
	void slotSavePop();
	void slotLoadPop();
	void slotExpInd();
	void slotImpInd();
	void slotNumChanged();

signals:
	void numChanged();
};

#endif