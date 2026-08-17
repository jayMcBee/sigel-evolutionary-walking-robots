#include "MT_GUI/MT_SelectionWidget.h"

#include <qcombobox.h>
#include <qmessagebox.h>
#include <qlabel.h>
#include <qspinbox.h>

MT_SelectionWidget::MT_SelectionWidget(QWidget* parent, const char* name, WFlags fl)
: MT_SelectionWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	lastParentSize = -1;

	QObject::connect((QObject*)overProductionSpinBox, SIGNAL(valueChanged(int)), SLOT(slotOverProdChanged(int)));
	QObject::connect((QObject*)tourSizeComboBox, SIGNAL(activated(const QString&)), SLOT(slotTourSizeChanged(const QString&)));
}

MT_SelectionWidget::~MT_SelectionWidget()
{}

/***
 * enables/disables widgets during evolution
 ***/
void MT_SelectionWidget::evolutionRunning(bool running)
{
	if(running){

	} else {

	}
}

void MT_SelectionWidget::updateTSizeList(int pSize, int oSize)
{
	tourSizeComboBox->clear();
	tourSizeMap.clear();

	int tNum;
	int index = 0;
	for(int tSize=1; tSize<=oSize; tSize++){
		if( (oSize % tSize) == 0){
			tNum = oSize / tSize;		// possible candidate reaches condition 1
			if((tNum != 0)&&((pSize % tNum) == 0)){	//     "        "        "        "    1+2
				
				// candidate found so insert it into the list
				tourSizeComboBox->insertItem(QString::number(tSize));
				tourSizeMap.insert(tSize, index++);
			}
		}
	}

	lastParentSize = pSize;
}

/***
 * indirectly called from population widget if size of parent population changed
 ***/
int MT_SelectionWidget::calculateTournSize(int pSize, int oSize, int oTSize)
{
	typedef QMap<int,int> intMap;

//	if(pSize != lastParentSize)
		updateTSizeList(pSize, oSize);

	if(tourSizeMap.contains(oTSize))
		return oTSize;
		
	intMap::Iterator it = tourSizeMap.begin();
	int lastSize, lastDiff, actDiff;

	actDiff = abs( (it.data()) - oTSize);
	do {
		lastDiff = actDiff;
		lastSize = it.data();
		it++;
		if(it == tourSizeMap.end())
			break;

		actDiff = abs( (it.data()) - oTSize);
	} while( actDiff < lastDiff );

	return lastSize;
}

void MT_SelectionWidget::onShow(MT_GPManager *manager, subst_cache *subst)
{
	int offspringSize;
	int tournSize;
	int selMethod;
	int fitFunc;
	int tsetSize;
	int tDuration;

	QObject::disconnect((QObject*)overProductionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotOverProdChanged(int)));
	QObject::disconnect((QObject*)tourSizeComboBox, SIGNAL(activated(const QString&)), this, SLOT(slotTourSizeChanged(const QString&)));

	manager->getSelektionValue(&offspringSize, &tournSize, 
		&selMethod, &fitFunc, 
		&tsetSize, &tDuration);

	// set the overproduction factor
	parentSize = manager->getParent()->getSize();
	int overProdFac = offspringSize / parentSize;
	overProductionSpinBox->setValue(overProdFac);

	// rebuild the list of possible tournament sizes
	if(lastParentSize != parentSize){
		updateTSizeList(parentSize, offspringSize);
	}

	// set the tournament size
	if(tourSizeMap.contains(tournSize)){
		int index = tourSizeMap.find(tournSize).data();
		tourSizeComboBox->setCurrentItem(index);
	} else {
		QMessageBox::critical(this, "Configure system", "The tournament size is invalid.\n"
			"A default value will be used.", "Ok");
		tourSizeComboBox->setCurrentItem(0);
	}

	// update labels
	int tourNum = offspringSize / tournSize;
	poolSizeLabel->setText(QString::number(offspringSize));
	tourNumLabel->setText(QString::number(tourNum));
	winNumLabel->setText(QString::number(parentSize / tourNum));

	selectionMethodComboBox->setCurrentItem(selMethod - 1);	// set the selection method

	// set the fitness function
	fitnessFunctionComboBox->clear();
	if(subst->strategy < 3){		// of type evaluator */
	
		fitnessFunctionComboBox->insertItem("simple error");
		fitnessFunctionComboBox->insertItem("square error");
		fitnessFunctionComboBox->setCurrentItem(fitFunc - 1);

	} else {						// of type classifier

		fitnessFunctionComboBox->insertItem("simple");
		fitnessFunctionComboBox->insertItem("weighted");
		fitnessFunctionComboBox->setCurrentItem(fitFunc - 3);

	}

	TSetSizeSpinBox->setValue(tsetSize);	// training set size
	TDurationSpinBox->setValue(tDuration);	// set max interpretation duration

	QObject::connect((QObject*)overProductionSpinBox, SIGNAL(valueChanged(int)), SLOT(slotOverProdChanged(int)));
	QObject::connect((QObject*)tourSizeComboBox, SIGNAL(activated(const QString&)), SLOT(slotTourSizeChanged(const QString&)));
}

bool MT_SelectionWidget::onHide(MT_GPManager *manager, subst_cache *subst)
{
	int offspringSize=0;
	int tournSize=0;
	int selMethod=0;
	int fitFunc=0;
	int tsetSize=0;
	int tDuration=0;

	offspringSize = parentSize * overProductionSpinBox->value();
	tournSize = tourSizeComboBox->currentText().toInt();
	selMethod = selectionMethodComboBox->currentItem() + 1;

	if(subst->strategy < 3){		// of type evaluator */
		fitFunc = fitnessFunctionComboBox->currentItem() + 1;
	} else {						// of type classifier
		fitFunc = fitnessFunctionComboBox->currentItem() + 3;
	}

	tsetSize = TSetSizeSpinBox->value();	// training set size
	tDuration= TDurationSpinBox->value();	// set max interpretation duration

	manager->setSelektionValue(offspringSize, tournSize,
		selMethod, fitFunc,
		tsetSize, tDuration);

	manager->setPopAndTournamentSize(parentSize, tournSize);

	return true;
}

void MT_SelectionWidget::slotOverProdChanged(int nvalue)
{
	int poolSize = parentSize * nvalue;

	int otourn = tourSizeComboBox->currentText().toInt();
	updateTSizeList(parentSize, poolSize);
	int tourn = calculateTournSize(parentSize, poolSize, otourn);

	QObject::disconnect((QObject*)tourSizeComboBox, SIGNAL(activated(const QString&)), this, SLOT(slotTourSizeChanged(const QString&)));

	if(tourSizeMap.contains(tourn)){
		int index = tourSizeMap.find(tourn).data();
		tourSizeComboBox->setCurrentItem(index);
	} else {
		tourSizeComboBox->setCurrentItem(0);
	}

	tourn = tourSizeComboBox->currentText().toInt();

	int tourNum = poolSize / tourn;
	poolSizeLabel->setText(QString::number(poolSize));
	tourNumLabel->setText(QString::number(tourNum));
	winNumLabel->setText(QString::number(parentSize / tourNum));

	QObject::connect((QObject*)tourSizeComboBox, SIGNAL(activated(const QString&)), SLOT(slotTourSizeChanged(const QString&)));
}

void MT_SelectionWidget::slotTourSizeChanged(const QString &text)
{
	int offSize = poolSizeLabel->text().toInt();
	int tSize   = text.toInt();
	int tourNum = offSize / tSize;
	tourNumLabel->setText(QString::number(tourNum));
	winNumLabel->setText(QString::number(parentSize / tourNum));
}