#ifndef MT_GUI_EXPERIMENT
#define MT_GUI_EXPERIMENT

#include "MT_GUI/MT_ExperimentItem.h"
#include <QTreeWidget>
#include <QQueue>

class MT_ExperimentWidget : public QTreeWidget
{
	Q_OBJECT

public:
	MT_ExperimentWidget(QWidget* parent=0, const char* name=0, Qt::WindowFlags fl=Qt::WindowFlags() );
	~MT_ExperimentWidget();

public slots:
	void lastSelected();
	void slotCurrentChanged(QTreeWidgetItem *newSelection);

private:
	// Qt 2's QQueue held POINTERS and did not own them here -- QGList's
	// del_item defaults to FALSE and nothing calls setAutoDelete. The items
	// belong to the tree, so the element type is spelled out and remove()
	// becomes dequeue().
	QQueue<MT_ExperimentItem *> prevSelectedItems;
};

#endif