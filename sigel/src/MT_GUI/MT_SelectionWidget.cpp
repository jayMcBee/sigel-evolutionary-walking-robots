#include "MT_GUI/MT_SelectionWidget.h"

#include <QComboBox>
#include <QMessageBox>
#include <QLabel>
#include <QSpinBox>

MT_SelectionWidget::MT_SelectionWidget(QWidget* parent, const char* name, Qt::WindowFlags fl)
: MT_SelectionWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	lastParentSize = -1;

	QObject::connect(overProductionSpinBox, SIGNAL(valueChanged(int)), SLOT(slotOverProdChanged(int)));
	QObject::connect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), SLOT(slotTourSizeChanged(const QString&)));
}

void MT_SelectionWidget::updateTSizeList(int pSize, int oSize)
{
	tournamentSizeComboBox->clear();
	tournamentSizeMap.clear();

	int tNum;
	int index = 0;
	for(int tSize=1; tSize<=oSize; tSize++)
	{
		if( (oSize % tSize) == 0)
		{
			tNum = oSize / tSize;		// possible candidate reaches condition 1
			if((tNum != 0)&&((pSize % tNum) == 0))	//     "        "        "        "    1+2
			{
				// candidate found so insert it into the list
				tournamentSizeComboBox->addItem(QString::number(tSize));
				tournamentSizeMap.insert(tSize, index++);
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

	updateTSizeList(pSize, oSize);

	if(tournamentSizeMap.contains(oTSize))
		return oTSize;

	intMap::Iterator it = tournamentSizeMap.begin();
	int lastSize, lastDiff, actDiff;

	actDiff = abs( (it.value()) - oTSize);
	do
	{
		lastDiff = actDiff;
		lastSize = it.value();
		it++;
		if(it == tournamentSizeMap.end())
			break;

		actDiff = abs( (it.value()) - oTSize);
	} while( actDiff < lastDiff );

	return lastSize;
}

void MT_SelectionWidget::onShow(MT_GPManager *manager, MT_EstimationState *estimationState)
{
	int offspringSize;
	int tournamentSize;
	int selMethod;
	MT_FitnessFunction fitFunc;
	int tsetSize;
	int tDuration;

	QObject::disconnect(overProductionSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotOverProdChanged(int)));
	QObject::disconnect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), this, SLOT(slotTourSizeChanged(const QString&)));

	manager->getSelectionValue(&offspringSize, &tournamentSize,
		&selMethod, &fitFunc,
		&tsetSize, &tDuration);

	// set the overproduction factor
	parentSize = manager->getParent()->getSize();
	int overProdFac = offspringSize / parentSize;
	overProductionSpinBox->setValue(overProdFac);

	// rebuild the list of possible tournament sizes
	if(lastParentSize != parentSize)
	{
		updateTSizeList(parentSize, offspringSize);
	}

	// set the tournament size
	if(tournamentSizeMap.contains(tournamentSize))
	{
		int index = tournamentSizeMap.find(tournamentSize).value();
		tournamentSizeComboBox->setCurrentIndex(index);
	}
	else
	{
		QMessageBox::critical(this, "Configure MetaGP System", "The tournament size is invalid.\n"
			"A default value will be used.");
		tournamentSizeComboBox->setCurrentIndex(0);
	}

	// update labels
	int tourNum = offspringSize / tournamentSize;
	poolSizeLabel->setText(QString::number(offspringSize));
	tourNumLabel->setText(QString::number(tourNum));
	winNumLabel->setText(QString::number(parentSize / tourNum));

	selectionMethodComboBox->setCurrentIndex(selMethod - 1);	// set the selection method

	// set the fitness function
	fitnessFunctionComboBox->clear();

	// The evaluator strategies are 1, 2, 9, 10 and 13, the classifier strategies
	// 3 to 6 and 8; "save best" makes the number negative.
	const int strategy = abs( estimationState->strategy );
	if( ( strategy < 3 ) || ( strategy > 8 ) )		// of type evaluator */
	{
		fitnessFunctionComboBox->addItem("Simple error");
		fitnessFunctionComboBox->addItem("Square error");
		fitnessFunctionComboBox->setCurrentIndex(fitFunc == MT_FitnessFunction::EvaluatorSquareError ? 1 : 0);
	}
	else						// of type classifier
	{
		fitnessFunctionComboBox->addItem("Simple");
		fitnessFunctionComboBox->addItem("Weighted");
		fitnessFunctionComboBox->setCurrentIndex(fitFunc == MT_FitnessFunction::ClassifierWeighted ? 1 : 0);
	}

	TSetSizeSpinBox->setValue(tsetSize);	// training set size
	TDurationSpinBox->setValue(tDuration);	// set max interpretation duration

	QObject::connect(overProductionSpinBox, SIGNAL(valueChanged(int)), SLOT(slotOverProdChanged(int)));
	QObject::connect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), SLOT(slotTourSizeChanged(const QString&)));
}

bool MT_SelectionWidget::onHide(MT_GPManager *manager, MT_EstimationState *estimationState)
{
	int offspringSize=0;
	int tournamentSize=0;
	int selMethod=0;
	MT_FitnessFunction fitFunc;
	int tsetSize=0;
	int tDuration=0;

	offspringSize = parentSize * overProductionSpinBox->value();
	tournamentSize = tournamentSizeComboBox->currentText().toInt();
	selMethod = selectionMethodComboBox->currentIndex() + 1;

	// The evaluator strategies are 1, 2, 9, 10 and 13, the classifier strategies
	// 3 to 6 and 8; "save best" makes the number negative.
	const int strategy = abs( estimationState->strategy );
	if( ( strategy < 3 ) || ( strategy > 8 ) )		// of type evaluator */
	{
		fitFunc = fitnessFunctionComboBox->currentIndex() == 1 ? MT_FitnessFunction::EvaluatorSquareError : MT_FitnessFunction::EvaluatorSimpleError;
	}
	else						// of type classifier
	{
		fitFunc = fitnessFunctionComboBox->currentIndex() == 1 ? MT_FitnessFunction::ClassifierWeighted : MT_FitnessFunction::ClassifierSimple;
	}

	tsetSize = TSetSizeSpinBox->value();	// training set size
	tDuration= TDurationSpinBox->value();	// set max interpretation duration

	manager->setSelectionValue(offspringSize, tournamentSize,
		selMethod, fitFunc,
		tsetSize, tDuration);

	manager->setPopAndTournamentSize(parentSize, tournamentSize);

	return true;
}

void MT_SelectionWidget::slotOverProdChanged(int nvalue)
{
	int poolSize = parentSize * nvalue;

	int oldTournamentSize = tournamentSizeComboBox->currentText().toInt();
	updateTSizeList(parentSize, poolSize);
	int tournamentSize = calculateTournSize(parentSize, poolSize, oldTournamentSize);

	QObject::disconnect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), this, SLOT(slotTourSizeChanged(const QString&)));

	if(tournamentSizeMap.contains(tournamentSize))
	{
		int index = tournamentSizeMap.find(tournamentSize).value();
		tournamentSizeComboBox->setCurrentIndex(index);
	}
	else
	{
		tournamentSizeComboBox->setCurrentIndex(0);
	}

	tournamentSize = tournamentSizeComboBox->currentText().toInt();

	int tourNum = poolSize / tournamentSize;
	poolSizeLabel->setText(QString::number(poolSize));
	tourNumLabel->setText(QString::number(tourNum));
	winNumLabel->setText(QString::number(parentSize / tourNum));

	QObject::connect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), SLOT(slotTourSizeChanged(const QString&)));
}

void MT_SelectionWidget::slotTourSizeChanged(const QString &text)
{
	int offSize = poolSizeLabel->text().toInt();
	int tSize   = text.toInt();
	int tourNum = offSize / tSize;
	tourNumLabel->setText(QString::number(tourNum));
	winNumLabel->setText(QString::number(parentSize / tourNum));
}