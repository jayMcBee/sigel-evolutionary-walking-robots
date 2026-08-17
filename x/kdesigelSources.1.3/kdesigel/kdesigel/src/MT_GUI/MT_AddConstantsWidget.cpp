#include "MT_GUI/MT_AddConstantsWidget.h"

#include <qlineedit.h>
#include <qbuttongroup.h>
#include <qspinbox.h>
#include <qbutton.h>
#include <qradiobutton.h>

MT_AddConstantsWidget::MT_AddConstantsWidget(MT_IndividualsWidget *parent, const char *name, bool modal, WFlags fl)
	: MT_AddConstantsWidgetBase(parent, name, true, fl)
{
	boss = parent;
	if(boss->integer){
		selectedType = intType;
		intRadioButton->setDown(true);
		floatRadioButton->setDown(false);
		minValidator = new QIntValidator(-10000, 10000, this);
		maxValidator = new QIntValidator(-10000, 10000, this);
	} else {
		selectedType = floatType;
		intRadioButton->setDown(false);
		floatRadioButton->setDown(true);
		minValidator = new QDoubleValidator(100000.0, -100000.0, 4, this);
		maxValidator = new QDoubleValidator(-100000.0, 100000.0, 4, this);
	}
	minValueEdit->setValidator(minValidator);
	minValueEdit->setText(tr("%1").arg(boss->minValue));
	maxValueEdit->setValidator(maxValidator);
	maxValueEdit->setText(tr("%1").arg(boss->maxValue));
	numConstantsSpinBox->setValue(boss->numToCreate);
	
	connect((QObject*)typeGroup, SIGNAL(clicked(int)), SLOT(slotClicked(int)));
}

MT_AddConstantsWidget::~MT_AddConstantsWidget()
{

}

void MT_AddConstantsWidget::accept()
{
	boss->numToCreate = numConstantsSpinBox->text().toInt();
	boss->minValue = minValueEdit->text().toDouble();
	boss->maxValue = maxValueEdit->text().toDouble();
	boss->integer  = (selectedType == intType) ? true : false;

	QDialog::accept();
}

void MT_AddConstantsWidget::slotClicked(int id)
{
	if((QString)typeGroup->find(id)->name() == QString("intRadioButton")){
		if(selectedType != intType){
			selectedType = intType;
			delete minValidator;
			delete maxValidator;
			minValidator = new QIntValidator(-10000, 10000, this);
			maxValidator = new QIntValidator(-10000, 10000, this);
			minValueEdit->setValidator(minValidator);
			maxValueEdit->setValidator(maxValidator);
			minValueEdit->setText(tr("%1").arg((int)minValueEdit->text().toDouble()));
			maxValueEdit->setText(tr("%1").arg((int)maxValueEdit->text().toDouble()));
		}
	} else {
		if(selectedType != floatType){
			selectedType = floatType;
			delete minValidator;
			delete maxValidator;
			minValidator = new QDoubleValidator(-10000.0, 10000.0, 4, this);
			maxValidator = new QDoubleValidator(-10000.0, 10000.0, 4, this);
			minValueEdit->setValidator(minValidator);
			maxValueEdit->setValidator(maxValidator);
			minValueEdit->setText(tr("%1").arg((double)minValueEdit->text().toInt()));
			maxValueEdit->setText(tr("%1").arg((double)maxValueEdit->text().toInt()));
		}
	} 		
}
