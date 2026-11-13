/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "MT_GUI/MT_SelectionWidget.h"
#include "MT_Control/MT_Substitute.h"

#include <QComboBox>
#include <QMessageBox>
#include <QLabel>
#include <QSpinBox>

#include <algorithm>

MT_SelectionWidget::MT_SelectionWidget(QWidget* parent, Qt::WindowFlags fl)
: MT_SelectionWidgetBase(parent, fl), MT_WidgetBase(parent)
{
	lastParentSize = -1;

	QObject::connect(overProductionSpinBox, SIGNAL(valueChanged(int)), SLOT(slotOverProdChanged(int)));
	QObject::connect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), SLOT(slotTournamentSizeChanged(const QString&)));
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
int MT_SelectionWidget::calculateTournamentSize(int pSize, int oSize, int oTSize)
{
	updateTSizeList(pSize, oSize);

	// a still valid old size has distance 0 and wins
	QList<int> sizes = tournamentSizeMap.keys();
	return *std::min_element( sizes.begin(), sizes.end(),
		[oTSize]( int left, int right )
		{
			return abs( left - oTSize ) < abs( right - oTSize );
		} );
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
	QObject::disconnect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), this, SLOT(slotTournamentSizeChanged(const QString&)));

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
	int tournamentCount = offspringSize / tournamentSize;
	poolSizeLabel->setText(QString::number(offspringSize));
	tournamentCountLabel->setText(QString::number(tournamentCount));
	winNumLabel->setText(QString::number(parentSize / tournamentCount));

	selectionMethodComboBox->setCurrentIndex(selMethod - 1);	// set the selection method

	// set the fitness function
	fitnessFunctionComboBox->clear();

	if( MT_Substitute::kindOfStrategy( estimationState->strategy ) == MT_SubstituteKind::Evaluator )
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
	QObject::connect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), SLOT(slotTournamentSizeChanged(const QString&)));
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

	if( MT_Substitute::kindOfStrategy( estimationState->strategy ) == MT_SubstituteKind::Evaluator )
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
	int tournamentSize = calculateTournamentSize(parentSize, poolSize, oldTournamentSize);

	QObject::disconnect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), this, SLOT(slotTournamentSizeChanged(const QString&)));

	tournamentSizeComboBox->setCurrentIndex( tournamentSizeMap.value( tournamentSize ) );

	int tournamentCount = poolSize / tournamentSize;
	poolSizeLabel->setText(QString::number(poolSize));
	tournamentCountLabel->setText(QString::number(tournamentCount));
	winNumLabel->setText(QString::number(parentSize / tournamentCount));

	QObject::connect(tournamentSizeComboBox, SIGNAL(textActivated(const QString&)), SLOT(slotTournamentSizeChanged(const QString&)));
}

void MT_SelectionWidget::slotTournamentSizeChanged(const QString &text)
{
	int offSize = poolSizeLabel->text().toInt();
	int tSize   = text.toInt();
	int tournamentCount = offSize / tSize;
	tournamentCountLabel->setText(QString::number(tournamentCount));
	winNumLabel->setText(QString::number(parentSize / tournamentCount));
}