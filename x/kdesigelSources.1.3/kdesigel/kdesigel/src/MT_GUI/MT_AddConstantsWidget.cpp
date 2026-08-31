#include "MT_GUI/MT_AddConstantsWidget.h"

#include <QLineEdit>
#include <QButtonGroup>
#include <QSpinBox>
#include <QAbstractButton>
#include <QRadioButton>

MT_AddConstantsWidget::MT_AddConstantsWidget(MT_IndividualsWidget *parent, const char *name, bool modal, Qt::WindowFlags fl)
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
	
	// Ids in .ui order, which is the order Qt 2's QButtonGroup auto-assigned.
	// Only the objectName is ever compared, so the values matter merely for
	// being distinct and stable.
	typeButtons = new QButtonGroup(this);
	typeButtons->addButton(intRadioButton, 0);
	typeButtons->addButton(floatRadioButton, 1);

	// QButtonGroup::clicked(int) is gone in Qt 6; the id-carrying signal is
	// idClicked(int). clicked() without an id is still live, which is why only
	// the int overload moves.
	connect(typeButtons, SIGNAL(idClicked(int)), SLOT(slotClicked(int)));
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
	// Qt 2: QButtonGroup::find(int) -> QButton*, QObject::name() -> const char*.
	if(typeButtons->button(id)->objectName() == QString("intRadioButton")){
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
