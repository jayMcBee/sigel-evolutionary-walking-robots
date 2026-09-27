#ifndef MT_GUI_EXPERIMENT
#define MT_GUI_EXPERIMENT

#include "MT_GUI/MT_ExperimentItem.h"
#include <QTreeWidget>
#include <QQueue>

class MT_ExperimentWidget : public QTreeWidget
{
	Q_OBJECT

public:
	MT_ExperimentWidget(QWidget* parent=nullptr, const char* name=nullptr, Qt::WindowFlags fl=Qt::WindowFlags() );

public slots:
	void lastSelected();
	void slotCurrentChanged(QTreeWidgetItem *newSelection);

private:
	// The items belong to the tree; the queue does not own them.
	QQueue<MT_ExperimentItem *> prevSelectedItems;
};

#endif