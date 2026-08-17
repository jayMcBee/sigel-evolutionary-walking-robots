#include "MT_GUI/MT_ExperimentWidget.h"
#include "MT_GUI/MT_ExperimentItem.h"

#include <qlistview.h>
#include <qpixmap.h>
#include <stdlib.h>

MT_ExperimentWidget::MT_ExperimentWidget(QWidget* parent, const char* name, WFlags fl ) 
	: QListView( parent, name, fl)
{
	setSorting(-1);
	clear();
	addColumn("Meta Experiment");

#ifdef _WINDOWS	
	QString pixPath = ::getenv("SIGEL_ROOT");
#else
	QString pixPath = std::getenv("SIGEL_ROOT");
#endif
	pixPath.append("/pixmaps/");
	
	new MT_ExperimentItem(this, 5, "Statistics", QPixmap( pixPath+"mt_StatSmall.xpm" ));
	new MT_ExperimentItem(this, 4, "Selection", QPixmap( pixPath+"dnaSmall.xpm" ));
	new MT_ExperimentItem(this, 3, "GP Parameter", QPixmap( pixPath+"dnaSmall.xpm" ));
	new MT_ExperimentItem(this, 2, "Population", QPixmap( pixPath+"individualSmall.xpm" ));
	new MT_ExperimentItem(this, 1, "Individual",QPixmap( pixPath+"mt_InstrSmall.xpm" ));
	new MT_ExperimentItem(this, 0, "Strategy",QPixmap( pixPath+"mt_StrategySmall.xpm" ));

	// before showing the window for the first time select the first item
	// in the list
	setCurrentItem(firstChild());
};

MT_ExperimentWidget::~MT_ExperimentWidget()
{
};

void MT_ExperimentWidget::slotCurrentChanged(QListViewItem *newSelection)
{
	// remember the previously selected item to allow reselection
	prevSelectedItems.enqueue((MT_ExperimentItem*)newSelection);
	if(prevSelectedItems.count() > 2)
		prevSelectedItems.remove();
};

void MT_ExperimentWidget::lastSelected()
{
	MT_ExperimentItem *stickingItem = prevSelectedItems.head();
	setCurrentItem(stickingItem);
	prevSelectedItems.enqueue(stickingItem);
	prevSelectedItems.remove();
}
