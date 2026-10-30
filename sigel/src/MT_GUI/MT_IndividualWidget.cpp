#include <QLocale>
#include <QValidator>
#include "MT_GUI/MT_IndividualWidget.h"
#include "MT_GUI/MT_AddConstantsWidget.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <QSlider>
#include <QSpinBox>
#include <QLineEdit>
#include <QListWidget>
#include <QProgressDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QLCDNumber>
#include <QStringList>

/********************************
 *       general methods 
 ********************************/

MT_IndividualsWidget::MT_IndividualsWidget(QWidget* parent, const char* name, Qt::WindowFlags fl)
	: MT_IndividualsWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	minValue = -10000.0;
	maxValue =  10000.0;
	integer  = false;
	numToCreate = 0;

	// the order of the command probabilities in the randomizer
	commandNames = { "SWT", "CPY", "ADD", "SUB", "MUL", "DIV", "MOD", "MIN", "MAX", "CMP", "JMP", "JLE", "JEQ", "NOP", "LUI", "LUS", "LUD", "LUM" };

	// limit the lineEdit's allowed values to that of the sliders
	validator = new QIntValidator(sliderSWT->minimum(), sliderSWT->maximum(), this);
	editSWT->setValidator(validator);
	editCPY->setValidator(validator);
	editADD->setValidator(validator);
	editSUB->setValidator(validator);
	editMUL->setValidator(validator);
	editDIV->setValidator(validator);
	editMOD->setValidator(validator);
	editMIN->setValidator(validator);
	editMAX->setValidator(validator);
	editCMP->setValidator(validator);
	editJMP->setValidator(validator);
	editJLE->setValidator(validator);
	editJEQ->setValidator(validator);
	editNOP->setValidator(validator);
	editLUI->setValidator(validator);
	editLUS->setValidator(validator);
	editLUD->setValidator(validator);
	editLUM->setValidator(validator);
// Pinned to the C locale: unpinned, "1,000" is ACCEPTABLE while text().toInt()
// returns 0.
{
	QLocale cLocale = QLocale::c();
	cLocale.setNumberOptions(QLocale::RejectGroupSeparator);
	for (QValidator *v : findChildren<QValidator *>())
		v->setLocale(cLocale);
}

	// connect the sliders to the corresponding lineEdits
	connect(sliderSWT, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderCPY, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderADD, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderSUB, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderMUL, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderDIV, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderMOD, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderMIN, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderMAX, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderCMP, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderJMP, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderJLE, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderJEQ, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderNOP, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderLUI, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderLUS, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderLUD, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect(sliderLUM, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));

	// connect the lineEdits to the corresponding sliders
	connect(editSWT, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editCPY, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editADD, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editSUB, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editMUL, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editDIV, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editMOD, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editMIN, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editMAX, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editCMP, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editJMP, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editJLE, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editJEQ, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editNOP, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editLUI, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editLUS, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editLUD, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(editLUM, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));

	delConstAction = new QAction("&Delete", this);
	delConstAction->setToolTip("Delete Constant");
	delConstAction->setStatusTip("Deletes the selected constants.");
	delConstAction->setEnabled(false);

	editConstAction = new QAction("&Edit", this);
	editConstAction->setToolTip("Edit Constant");
	editConstAction->setStatusTip("Edits the current constant.");
	editConstAction->setEnabled(false);

	createContextMenu();

	connect(delConstAction, SIGNAL(triggered()), SLOT(slotDelConst()));
	connect(editConstAction, SIGNAL(triggered()), SLOT(slotEditConst()));

	// connection for the listbox
	constantsListBox->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(constantsListBox, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotRButtonClicked(const QPoint&)));
	connect(constantsListBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(slotSelectionChanged()));
	connect(constantsListBox, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()));
	connect(constantsListBox, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotEditConst(QListWidgetItem*)));

	// connect the buttons
	connect(createButton, SIGNAL(clicked()), SLOT(slotCreateConstants()));
	connect(importButton, SIGNAL(clicked()), SLOT(slotImportConstants()));
	connect(exportButton, SIGNAL(clicked()), SLOT(slotExportConstants()));

	connect(this, SIGNAL(numConstChanged()), SLOT(slotNumConstChanged()));

	// create the editor
	editor = new MT_Editor(constantsListBox);
	connect(editor, SIGNAL(newText(const QString &)), SLOT(slotChangeConstant(const QString &)));
}

void MT_IndividualsWidget::createContextMenu()
{
	constContextMenu = new QMenu(this);
	constContextMenu->setObjectName("mtConstContextMenu");
	constContextMenu->addAction(delConstAction);
	constContextMenu->addAction(editConstAction);
}

void MT_IndividualsWidget::onShow(MT_GPManager *manager, MT_EstimationState *estimationState)
{
	randomizer = manager->getRandomizer();

	int *length = nullptr;
	int *numVar = nullptr;
	QList<double> *constants = nullptr;
	QList<double> *functions = nullptr;

	if(randomizer){
		randomizer->returnIndividualsValue(&length, &numVar, &constants, &functions);
		
		// update the GUI-elements
		programLengthSpinBox->setValue(*length);
		numVariablesSpinBox->setValue(*numVar);

		// add the constants to the appropriate listbox
		// Signals stay blocked during clear(). slotSelectionChanged reads
		// currentRow() and must not run while the items are removed.
		{
			const bool wasBlocked = constantsListBox->blockSignals(true);
			constantsListBox->clear();
			constantsListBox->blockSignals(wasBlocked);
		}
		for(int i=0; i<constants->size(); i++){
			constantsListBox->addItem(QString("%1").arg(constants->at(i)));
		}
		emit numConstChanged();
		
		// set the command probabilities
		QString valText;
		QLineEdit *receiverObject;
		double previous = 0.0;
		double actual;
		for(int i=0; i<commandNames.size(); i++){
			actual = functions->at(i);
			valText = QString("%1").arg(static_cast<int>(actual - previous));
			previous = actual;

			receiverObject = findChild<QLineEdit*>("edit" + commandNames.at(i));
			if(receiverObject&& (receiverObject->text() != valText)){
				receiverObject->setText(valText);
			}
		}
	} else
		QMessageBox::critical(this, "Configure MetaGP System", "Couldn't get randomizer to set/get settings.");
}

bool MT_IndividualsWidget::onHide(MT_GPManager *manager, MT_EstimationState *estimationState)
{
	int *length = nullptr;
	int *numVar = nullptr;

	manager->getParent()->setMaxProgLen( programLengthSpinBox->value() );

	QList<double> *constants = nullptr;
	QList<double> *functions = nullptr;
	randomizer->returnIndividualsValue(&length, &numVar, &constants, &functions);

	*length = programLengthSpinBox->value();
	*numVar = numVariablesSpinBox->value();

	// set the constants
	int numConst = constantsListBox->count();
	if(constants->size() != numConst)
		constants->resize(numConst);
	for(int i=0; i<numConst; i++){
		(*constants)[i] = constantsListBox->item(i)->text().toDouble();
	}

	// set the command probabilities
	QLineEdit *receiverObject;
	double previous = 0.0;
	for(int i=0; i<commandNames.size(); i++){
		receiverObject = findChild<QLineEdit*>("edit" + commandNames.at(i));
		if(receiverObject){
			previous += receiverObject->text().toDouble();
		}
		(*functions)[i] = previous;
	}
	
	return true;
}

/********************************
 *    settings for constants       
 ********************************/

void MT_IndividualsWidget::slotImportConstants()
{
	QStringList files( QFileDialog::getOpenFileNames( this, "Import Constants", QString(), "Constants (*.mcon);;All Files (*)"));

	if(files.isEmpty())
		return;
	
	// iterate over all selected files
	for(QStringList::Iterator it = files.begin(); it != files.end(); ++it){
		QFile file(*it);

		if(file.open(QIODevice::ReadOnly)){
			// file was successfully opened
			// so read it and fill the datastructures
			QTextStream stream( &file );
			QString s;
			bool found = false;
			int count=0;
			while( !stream.atEnd() ){			// iterate the whole file
				s = stream.readLine();
				if(!found && s=="[constants]"){
					found = true;
					s = stream.readLine();
				}
				if(found && s=="count"){
					s = stream.readLine();
					int tc = count = s.toInt();
		QProgressDialog progress("Loading constants", QString(), 0, count, this);
		progress.setWindowModality(Qt::ApplicationModal);
					while(tc && !stream.atEnd()){
						progress.setValue(count - tc);
						s = stream.readLine();
						constantsListBox->addItem(QString(s));
						tc--;
						emit numConstChanged();
					}
					progress.setValue(count);
				}
			}
			if(!found || !count){
				QMessageBox::information(this, "Load Constants",
					"Import operation aborted.\n"
					"The loaded file is not a valid constants file.");
			}
			file.close();
		} else {
			QMessageBox::critical(this, "Import Constants",
				"Couldn't open file.\n"
				"Constants won't be loaded.");
		}
	}
}

void MT_IndividualsWidget::slotExportConstants()
{
	QString fileName = QFileDialog::getSaveFileName(this, QString(), QString(), "Constants (*.mcon);;All Files (*)");
	if(fileName.isEmpty()) return;
	if(fileName.right(5) != ".mcon")
		fileName += ".mcon";

	QFile file(fileName);
	
	if(file.open( QIODevice::WriteOnly )){
		QTextStream str( &file );
		int count = constantsListBox->count();
		if(!count){
			QMessageBox::information(this, "Export Constants",
				"The list contains no constants.\n"
				"The constants were not saved.");
			return;
		}
		str << "[constants]\n";
		str << "count\n" << count << Qt::endl;
		for(int i=0; i<count; i++){
			str << constantsListBox->item(i)->text() << Qt::endl;
		}
		str << Qt::endl;
		file.close();
	} else {
		QMessageBox::critical(this, "Export Constants",
			"Couldn't open file for writing.\n"
			"Constants won't be saved.");
	}
}

void MT_IndividualsWidget::slotEditConst()
{
	slotEditConst(constantsListBox->item(constantsListBox->currentRow()));
}

void MT_IndividualsWidget::slotEditConst(QListWidgetItem *item)
{
	QRect dim(constantsListBox->visualItemRect(item));
	dim.setX(dim.x() + 2);
	dim.setY(dim.y() + 2);
	dim.setWidth(dim.width() + 2);
	dim.setHeight(dim.height() + 2);
	connect(editor, SIGNAL(lostFocus()), SLOT(slotResetFocus()));
	editor->popup(dim, item->text());
}

void MT_IndividualsWidget::slotResetFocus()
{
	editor->disconnect( SIGNAL(lostFocus()) );
	constantsListBox->setFocus();
}

void MT_IndividualsWidget::slotChangeConstant(const QString &ntext)
{
	constantsListBox->item(constantsListBox->currentRow())->setText(ntext);
}

void MT_IndividualsWidget::slotSelectionChanged()
{
	int actItem = constantsListBox->currentRow();

	if((actItem != -1) && constantsListBox->item(actItem)->isSelected()){
		delConstAction->setEnabled(true);
		editConstAction->setEnabled(true);
	} else {
		delConstAction->setEnabled(false);
		editConstAction->setEnabled(false);
	}
}

void MT_IndividualsWidget::slotDelConst()
{
	QListWidgetItem *nextInd=nullptr;

	// iterate over all items, check if they are selected
	// and delete the selected ones
	for(uint actInd=0; actInd<constantsListBox->count(); actInd++){
		if(constantsListBox->item(actInd)->isSelected()){
			delete constantsListBox->takeItem(actInd);	// delete actual constant if selected
			actInd--;		// the indices of the following items were decreased !!
			emit numConstChanged();
		}
	}
}

void MT_IndividualsWidget::slotRButtonClicked(const QPoint &pos)
{
	// customContextMenuRequested gives viewport coordinates, and a click on
	// blank space must clear the selection so Delete greys out.
	if(!constantsListBox->itemAt(pos))
		constantsListBox->clearSelection();
	constContextMenu->popup(constantsListBox->viewport()->mapToGlobal(pos));
}

void MT_IndividualsWidget::slotCreateConstants()
{
	MT_AddConstantsWidget constDialog(this, nullptr, true);

	if(QDialog::Accepted == constDialog.exec() && numToCreate > 0){
		QProgressDialog progress("Generating constants", QString(), 0, numToCreate+1, this);
		progress.setWindowModality(Qt::ApplicationModal);
		progress.setValue(0);

		int *length = nullptr;
		int *numVar = nullptr;
		QList<double> *constants = nullptr;
		QList<double> *functions = nullptr;
		randomizer->createConstant(numToCreate, integer, minValue, maxValue);
		randomizer->returnIndividualsValue(&length, &numVar, &constants, &functions);

		for(int i=0; i<constants->size(); i++){
			progress.setValue(i+1);
			constantsListBox->addItem(QString("%1").arg(constants->at(i)));
			emit numConstChanged();
		}
		progress.setValue(numToCreate+1);
	}
}

void MT_IndividualsWidget::slotNumConstChanged()
{
	numConstantsLCD->display(constantsListBox->count());
}

/********************************
 * instruction set probabilities
 ********************************/

void MT_IndividualsWidget::slotChangeEdit(int value)
{
	// find out who's sending this signal
	const QObject *senderObject = sender();
	QString senderName(senderObject->objectName());

	// get the appropriate receiver
	QString receiverName("edit");
	receiverName += senderName.right(3);

	QLineEdit *receiverObject = findChild<QLineEdit*>(receiverName);

	// transfer value to text and update the widget if necessary
	QString valText(tr("%1").arg(value));
	if(receiverObject && (receiverObject->text() != valText)){
		receiverObject->setText(valText);
	}
}

void MT_IndividualsWidget::slotChangeSlider(const QString &text)
{
	// find out who's sending this signal
	QLineEdit *senderObject = static_cast<QLineEdit*>(sender());
	QString senderName(senderObject->objectName());

	// get the appropriate receiver
	QString receiverName("slider");
	receiverName += senderName.right(3);

	QSlider *receiverObject = findChild<QSlider*>(receiverName);

	// transfer text to value and update the widget if necessary
	int value = text.toInt();

	// if the value is illegal, first set it to a legal value
	// and update the line edit, else update the slider
	if(value > validator->top()){
		senderObject->setText(tr("%1").arg(validator->top()));
	} else if(receiverObject && (receiverObject->value() != value)){
		receiverObject->setValue(value);
	}
}
