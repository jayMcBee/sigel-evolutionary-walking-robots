#include "MT_GUI/MT_ExperimentWidget.h"
#include "MT_GUI/MT_ExperimentItem.h"

#include <QTreeWidget>
#include <QPixmap>
#include <stdlib.h>

MT_ExperimentWidget::MT_ExperimentWidget(QWidget* parent, const char* name, Qt::WindowFlags fl ) 
	: QTreeWidget( parent )
{
	if ( name )
		setObjectName( QString::fromUtf8( name ) );
	if ( fl != Qt::WindowFlags() )
		setWindowFlags( fl );

	// setSorting(-1) is Qt 2 for "no sorting". Qt 6 defaults to false, but
	// this is stated because it is load-bearing: with sorting off, the six
	// pages below appear in insertion order, and Qt 2 inserted by PREPENDING.
	setSortingEnabled(false);
	clear();
	setColumnCount(1);
	setHeaderLabels(QStringList("Meta experiment"));

#ifdef _WINDOWS	
	QString pixPath = ::getenv("SIGEL_ROOT");
#else
	QString pixPath = std::getenv("SIGEL_ROOT");
#endif
	pixPath.append("/pixmaps/");
	
	new MT_ExperimentItem(this, 5, "Statistics", QPixmap( pixPath+"mt_StatSmall.xpm" ));
	new MT_ExperimentItem(this, 4, "Selection", QPixmap( pixPath+"dnaSmall.xpm" ));
	new MT_ExperimentItem(this, 3, "GP Parameters", QPixmap( pixPath+"dnaSmall.xpm" ));
	new MT_ExperimentItem(this, 2, "Population", QPixmap( pixPath+"individualSmall.xpm" ));
	new MT_ExperimentItem(this, 1, "Individual",QPixmap( pixPath+"mt_InstrSmall.xpm" ));
	new MT_ExperimentItem(this, 0, "Strategy",QPixmap( pixPath+"mt_StrategySmall.xpm" ));

	// before showing the window for the first time select the first item
	// in the list
	setCurrentItem(topLevelItem(0));
};

MT_ExperimentWidget::~MT_ExperimentWidget()
{
};

void MT_ExperimentWidget::slotCurrentChanged(QTreeWidgetItem *newSelection)
{
	// remember the previously selected item to allow reselection
	if(!newSelection)
		return;
	prevSelectedItems.enqueue((MT_ExperimentItem*)newSelection);
	if(prevSelectedItems.count() > 2)
		prevSelectedItems.dequeue();
};

void MT_ExperimentWidget::lastSelected()
{
	// head() on an empty queue is undefined and segfaults. Reached on the
	// first refused page switch.
	if(prevSelectedItems.isEmpty())
		return;
	MT_ExperimentItem *stickingItem = prevSelectedItems.head();
	setCurrentItem(stickingItem);
	prevSelectedItems.enqueue(stickingItem);
	prevSelectedItems.dequeue();
}
