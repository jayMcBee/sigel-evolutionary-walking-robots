#ifndef MT_GUI_EXPERIMENT
#define MT_GUI_EXPERIMENT

#include "MT_GUI/MT_ExperimentItem.h"
#include <qlistview.h>
#include <qqueue.h>

class MT_ExperimentWidget : public QListView
{
	Q_OBJECT

public:
	MT_ExperimentWidget(QWidget* parent=0, const char* name=0, WFlags fl=0 );
	~MT_ExperimentWidget();

public slots:
	void lastSelected();
	void slotCurrentChanged(QListViewItem *newSelection);

private:
	QQueue<MT_ExperimentItem> prevSelectedItems;
};

#endif