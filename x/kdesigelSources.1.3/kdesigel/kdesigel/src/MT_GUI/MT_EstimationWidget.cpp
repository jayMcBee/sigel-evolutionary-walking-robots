#include "MT_GUI/MT_EstimationWidget.h"

#include <qmessagebox.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qcheckbox.h>

MT_EstimationWidget::MT_EstimationWidget(QWidget* parent, const char* name, WFlags fl )
	 : MT_EstimationWidgetBase( parent, name, fl), MT_WidgetBase(parent)
{
	lastIndex = 0;
	tol0 = tol1 = 0.0;
	usedSys = evaluator;

	toleranceSpinBox = new DISpinBox(3, (QWidget*) estimationGroupBox, "toleranceSpinBox");
	Layout61->addWidget(toleranceSpinBox, 1, 2);

	QObject::connect(strategyComboBox, SIGNAL(activated(int)), SLOT(slotStrategyChanged(int)));
}

MT_EstimationWidget::~MT_EstimationWidget()
{}

/***
 * enables/disables widgets during evolution
 ***/
void MT_EstimationWidget::evolutionRunning(bool running)
{
	if(running){

	} else {

	}
}

void MT_EstimationWidget::slotStrategyChanged(int index)
{

	// remember the last set value
	if(toleranceSpinBox->getTyp() == INTTYP){
		switch(lastIndex){
		case 0 : tol0 = toleranceSpinBox->dblValue(); break;
		case 1 : tol1 = toleranceSpinBox->dblValue(); break;
		case 2 : tol2 = toleranceSpinBox->dblValue(); break;
		case 3 : tol3 = toleranceSpinBox->dblValue(); break;
		case 4 : tol4 = toleranceSpinBox->dblValue(); break;
		}
	} else {
		switch(lastIndex){
		case 0 : tol0 = toleranceSpinBox->intValue(); break;
		case 1 : tol1 = toleranceSpinBox->intValue(); break;
		case 2 : tol2 = toleranceSpinBox->intValue(); break;
		case 3 : tol3 = toleranceSpinBox->intValue(); break;
		case 4 : tol4 = toleranceSpinBox->intValue(); break;
		}
	}

	lastIndex = index;
	if(usedSys == evaluator){
		switch(index){
		case 0 : 
			toleranceSpinBox->setRange(3, 0.0, 100.0);
			toleranceSpinBox->setDblValue(tol0);
			break;
		case 1 :
			toleranceSpinBox->setRange(3, 0.0, 100.0);
			toleranceSpinBox->setDblValue(tol1);
			break;
		case 2 : 
			toleranceSpinBox->setRange(3, 0.0, 100.0);
			toleranceSpinBox->setDblValue(tol2);
			break;
		case 3 :
			toleranceSpinBox->setRange(3, 0.0, 100.0);
			toleranceSpinBox->setDblValue(tol3);
			break;
		case 4 :
		default:
			toleranceSpinBox->setRange(3, 0.0, 100.0);
			toleranceSpinBox->setDblValue(tol4);
			break;
		}
	} else {
		switch(index){
		case 0 : 
			toleranceSpinBox->setRange(0, 100);
			toleranceSpinBox->setIntValue((int)tol0);
			break;
		case 1 :
			toleranceSpinBox->setRange(0, 100);
			toleranceSpinBox->setIntValue(tol1);
			break;
		case 2 : 
			toleranceSpinBox->setRange(0, 100);
			toleranceSpinBox->setIntValue(tol2);
			break;
		case 3 :
			toleranceSpinBox->setRange(0, 100);
			toleranceSpinBox->setIntValue(tol3);
			break;
		case 4 :
		default: 
			toleranceSpinBox->setRange(0, 100);
			toleranceSpinBox->setIntValue(tol4);
			break;
		}
	}

}

bool MT_EstimationWidget::onHide(MT_GPManager *manager, subst_cache *subst)
{
	int strategy, refInt;
	double tolerance;

	if(usedSys == evaluator){
		switch(strategyComboBox->currentItem()){
		case 0 : strategy = 1; break;
		case 1 : strategy = 2; break;
		case 2 : strategy = 9; break;
		case 3 : strategy = 10; break;
		case 4 : 
		default: strategy = 13;
		}
	}
	else
	{
		switch(strategyComboBox->currentItem()){
		case 0 : strategy = 3; break;
		case 1 : strategy = 4; break;
		case 2 : strategy = 5; break;
		case 3 : strategy = 6; break;
		case 4 : 
		default: strategy = 8;
		}
	}

	refInt = intervalSpinBox->value();

	if(toleranceSpinBox->getTyp() == INTTYP)
		tolerance = (double) toleranceSpinBox->intValue();
	else
		tolerance = toleranceSpinBox->dblValue();

	if(saveBestCheckBox->isChecked())
		strategy *= -1;

	if(subst){
		subst->strategy = strategy;
		subst->tolerance = tolerance;
		subst->refreshInt = refInt;
	}
	else
		QMessageBox::critical(this, "Configuring System", "Couldn't set information to current Meta-System.", "Ok");

	return true;
}

void MT_EstimationWidget::onShow(MT_GPManager *manager, subst_cache *subst)
{
	int strategy = 0;
	int refInt = 0;
	double tolerance = 0.0;

	// read in the values
	if(subst){
		strategy = subst->strategy;
		refInt = subst->refreshInt;
		tolerance = subst->tolerance;
	}
	else {
		QMessageBox::critical(this, "Configuring System", "Couldn't retrieve information of current Meta-System.", "Ok");
		return;
	}

	if(strategy < 0){
		saveBestCheckBox->setChecked(true);
		strategy *= -1;
	} else {
		saveBestCheckBox->setChecked(false);
	}

	// build new dropdownlist depending on the used substituter
	strategyComboBox->clear();
	if(subst->strategy < 3 ||
		subst->strategy > 8)
	{
		// of type evaluator */
		estimationGroupBox->setTitle("Evaluation");
		usedSys = evaluator;
		strategyComboBox->insertItem("mean error");
		strategyComboBox->insertItem("cleaned mean error");
		strategyComboBox->insertItem("adaptive error");
		strategyComboBox->insertItem("adaptive cleaned error");
		strategyComboBox->insertItem("don't use meta");

		switch(strategy){
		case 1 : strategyComboBox->setCurrentItem(0);
			break;
		case 2 : strategyComboBox->setCurrentItem(1);
			break;
		case 9 : strategyComboBox->setCurrentItem(2);
			break;
		case 10 : strategyComboBox->setCurrentItem(3);
			break;
		case 13 :
		default:
			strategyComboBox->setCurrentItem(4);
		}

	} else {						
		// of type classifier
		estimationGroupBox->setTitle("Classification");
		usedSys = classifier;
		strategyComboBox->insertItem("error percentage");
		strategyComboBox->insertItem("adaptive - Generation");
		strategyComboBox->insertItem("adaptive - Fitness");
		strategyComboBox->insertItem("save the best");
		strategyComboBox->insertItem("don't use meta");

		switch(strategy){
		case 3 : strategyComboBox->setCurrentItem(0); break;
		case 4 : strategyComboBox->setCurrentItem(1); break;
		case 5 : strategyComboBox->setCurrentItem(2); break;
		case 6 : strategyComboBox->setCurrentItem(3); break;
		case 8 :
		default: strategyComboBox->setCurrentItem(4);
		}
	}
	slotStrategyChanged(strategy);
	lastIndex = strategyComboBox->currentItem();

	// remember the value been set
	switch(lastIndex){
	case 0 :
		tol0 = tolerance;
		break;
	case 1 :
		tol1 = tolerance;
		break;
	default: break;
	}

	// set the values
	intervalSpinBox->setValue(refInt);
	if(toleranceSpinBox->getTyp() == INTTYP)
		toleranceSpinBox->setIntValue((int)tolerance);
	else
		toleranceSpinBox->setDblValue(tolerance);

}