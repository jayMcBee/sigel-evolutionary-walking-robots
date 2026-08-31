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

	// limit the lineEdit's allowed values to that of the sliders
	validator = new QIntValidator(slider01->minimum(), slider01->maximum(), this);
	edit01->setValidator(validator);
	edit02->setValidator(validator);
	edit03->setValidator(validator);
	edit04->setValidator(validator);
	edit05->setValidator(validator);
	edit06->setValidator(validator);
	edit07->setValidator(validator);
	edit08->setValidator(validator);
	edit09->setValidator(validator);
	edit10->setValidator(validator);
	edit11->setValidator(validator);
	edit12->setValidator(validator);
	edit13->setValidator(validator);
	edit14->setValidator(validator);
	edit15->setValidator(validator);
	edit16->setValidator(validator);
	edit17->setValidator(validator);
	edit18->setValidator(validator);

	// connect the sliders to the corresponding lineEdits
	connect((const QObject*)slider01, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider02, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider03, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider04, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider05, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider06, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider07, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider08, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider09, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider10, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider11, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider12, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider13, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider14, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider15, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider16, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider17, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));
	connect((const QObject*)slider18, SIGNAL(valueChanged(int)), SLOT(slotChangeEdit(int)));

	// connect the lineEdits to the corresponding sliders
	connect(edit01, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit02, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit03, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit04, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit05, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit06, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit07, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit08, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit09, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit10, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit11, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit12, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit13, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit14, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit15, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit16, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit17, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));
	connect(edit18, SIGNAL(textChanged(const QString&)), SLOT(slotChangeSlider(const QString&)));

	// create context menu
	constContextMenu = new QMenu(this);
	constContextMenu->setObjectName("mtConstContextMenu");
	delConstAction = new QAction("&Delete", this);
	delConstAction->setToolTip("Delete Constant");
	delConstAction->setStatusTip("Deletes the selected constants.");
	delConstAction->setEnabled(false);
	constContextMenu->addAction(delConstAction);

	editConstAction = new QAction("&Edit", this);
	editConstAction->setToolTip("Edit Constant");
	editConstAction->setStatusTip("Edits the current constant.");
	editConstAction->setEnabled(false);
	constContextMenu->addAction(editConstAction);

	connect(delConstAction, SIGNAL(triggered()), SLOT(slotDelConst()));
	connect(editConstAction, SIGNAL(triggered()), SLOT(slotEditConst()));

	// connection for the listbox
	constantsListBox->setContextMenuPolicy(Qt::CustomContextMenu);
	connect((const QObject*) constantsListBox, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(slotRButtonClicked(const QPoint&)));
	connect((const QObject*) constantsListBox, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), SLOT(slotSelectionChanged()));
	connect((const QObject*) constantsListBox, SIGNAL(itemSelectionChanged()), SLOT(slotSelectionChanged()));
	connect((const QObject*) constantsListBox, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(slotEditConst(QListWidgetItem*)));

	// connect the buttons
	connect((const QObject*)createButton, SIGNAL(clicked()), SLOT(slotCreateConstants()));
	connect((const QObject*)importButton, SIGNAL(clicked()), SLOT(slotImportConstants()));
	connect((const QObject*)exportButton, SIGNAL(clicked()), SLOT(slotExportConstants()));

	connect(this, SIGNAL(numConstChanged()), SLOT(slotNumConstChanged()));

	// create the editor
	editor = new MT_Editor(constantsListBox);
	connect(editor, SIGNAL(newText(const QString &)), SLOT(slotChangeConstant(const QString &)));
}

MT_IndividualsWidget::~MT_IndividualsWidget()
{}

/***
 * enables/disables widgets during evolution
 ***/
void MT_IndividualsWidget::evolutionRunning(bool running)
{
	if(running){

	} else {

	}
}

void MT_IndividualsWidget::onShow(MT_GPManager *manager, subst_cache *subst)
{
	randomizer = manager->getRandomizer();

	int *length = 0;
	int *numVar = 0;
	QList<double> *constants = 0;
	QList<double> *functions = 0;

	if(randomizer){
		
		randomizer->returnIndividualsValue(&length, &numVar, &constants, &functions);
		
		// update the GUI-elements
		programLengthSpinBox->setValue(*length);
		numVariablesSpinBox->setValue(*numVar);

		// add the constants to the appropriate listbox
		constantsListBox->clear();
		for(int i=0; i<constants->size(); i++){
			constantsListBox->addItem(QString("%1").arg(constants->at(i)));
		}
		emit numConstChanged();
		
		// set the command probabilities
		QString com1("edit0");
		QString com2("edit");
		QString valText;
		QString receivername;
		QLineEdit *receiverObject;
		double previous = 0.0;
		double actual;
		for(int i=1; i<10; i++){
			receivername = com1 + QString("%1").arg(i);
			actual = functions->at(i-1);
			valText = QString("%1").arg((int)(actual - previous));
			previous = actual;

			receiverObject = findChild<QLineEdit*>(receivername);
			if(receiverObject&& (receiverObject->text() != valText)){
				receiverObject->setText(valText);
			}
		}
		for(int i=10; i<19; i++){
			receivername = com2 + QString("%1").arg(i);
			actual = functions->at(i-1);
			valText = QString("%1").arg((int)(actual - previous));
			previous = actual;

			receiverObject = findChild<QLineEdit*>(receivername);
			if(receiverObject&& (receiverObject->text() != valText)){
				receiverObject->setText(valText);
			}
		}

	} else
		QMessageBox::critical(this, "Configure Meta-System", "Couldn't get randomizer to set/get settings.", "Ok");
}

bool MT_IndividualsWidget::onHide(MT_GPManager *manager, subst_cache *subst)
{
	int *length = 0;
	int *numVar = 0;

	manager->getParent()->setMaxProgLen( programLengthSpinBox->value() );

	QList<double> *constants = 0;
	QList<double> *functions = 0;
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
	QString com1("edit0");
	QString com2("edit");
	QString valText;
	QString receivername;
	QLineEdit *receiverObject;
	double previous = 0.0;
	for(int i=1; i<10; i++){
		receivername = com1 + QString("%1").arg(i);
		receiverObject = findChild<QLineEdit*>(receivername);
		if(receiverObject){
			previous += receiverObject->text().toDouble();
		}
		(*functions)[i-1] = previous;
	}
	for(int i=10; i<19; i++){
		receivername = com2 + QString("%1").arg(i);
		receiverObject = findChild<QLineEdit*>(receivername);
		if(receiverObject){
			previous += receiverObject->text().toDouble();
		}
		(*functions)[i-1] = previous;
	}
	
	return true;
}

/********************************
 *    settings for constants       
 ********************************/

void MT_IndividualsWidget::slotImportConstants()
{
	QStringList files( QFileDialog::getOpenFileNames( this, "Import Constants", QString(), "Constants(*.mcon);;All Files(*)"));


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
					// Qt 2: (label, cancelText, totalSteps, creator, name, modal).
		// Qt 6: (label, cancelText, minimum, maximum, parent); modality separate.
		// cancelText was 0 in 1.3, i.e. NO Cancel button -- QString() keeps that.
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
					"The loaded file is not a correct constants-file.",	1, 0);
			}
			file.close();

		} else {
			QMessageBox::critical(this, "Import Constants",
				"Couldn't open file.\n"
				"Constants won't be loaded.", 1, 0);
		}
	}
}

void MT_IndividualsWidget::slotExportConstants()
{
	QString fileName = QFileDialog::getSaveFileName(this, QString(), QString(), "Constants(*.mcon);;All Files(*)");
	if(fileName.isEmpty()) return;
	if(fileName.right(5) != ".mcon")
		fileName += ".mcon";

	QFile file(fileName);
	if(file.exists()){
		if(0 == QMessageBox::warning(this, "Save constants", "There is another file with this name. This will overwrite\n"
			"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
			return;
	}
	
	if(file.open( QIODevice::WriteOnly )){
		QTextStream str( &file );
		int count = constantsListBox->count();
		if(!count){
			QMessageBox::information(this, "Export Constants",
				"Listbox doesn't contain constants.\n"
				"Couldn't save constants.", 1, 0);
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
			"Constant won't be saved.", 1, 0);
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
	QListWidgetItem *nextInd=0;

	// iterate over all items, check if they are selected
	// and delete the selected ones
	for(uint actInd=0; actInd<constantsListBox->count(); actInd++){
		if(constantsListBox->item(actInd)->isSelected()){
			delete constantsListBox->takeItem(actInd);	// delete actual constant if selected
			actInd--;		// the indices of the following items were decreased !!
			emit numConstChanged();
		}
		//numConstants->setMinValue(constantsListBox->count());
	}
}

void MT_IndividualsWidget::slotRButtonClicked(const QPoint &pos)
{
	// Qt 2's rightButtonClicked is gone. Two things it did must be restored by
	// hand. (1) It delivered a GLOBAL position (viewport()->mapToGlobal, see
	// qlistview.cpp:3396); customContextMenuRequested delivers viewport
	// coordinates. (2) When the click MISSED an item, Qt 2 called
	// clearSelection() before emitting (qlistview.cpp:3390) -- which is why
	// 1.3 greys out Delete on blank space: clearing fires selectionChanged,
	// and slotSelectionChanged disables the action. Qt 6 does neither.
	// Confirmed on the running 1.3: on a row all four entries are enabled; on
	// blank space Delete is greyed and the other three are not.
	if(!constantsListBox->itemAt(pos))
		constantsListBox->clearSelection();
	constContextMenu->popup(constantsListBox->viewport()->mapToGlobal(pos));
}

void MT_IndividualsWidget::slotCreateConstants()
{
	MT_AddConstantsWidget constDialog(this, 0, true);

	if(QDialog::Accepted == constDialog.exec()){

		if(numToCreate > 0){
		
			// Qt 2: (label, cancelText, totalSteps, creator, name, modal).
			// Qt 6: (label, cancelText, minimum, maximum, parent).
			QProgressDialog progress("Generating constants", QString(), 0, numToCreate+1, this);
			progress.setWindowModality(Qt::ApplicationModal);
			progress.setValue(0);

			int *length = 0;
			int *numVar = 0;
			QList<double> *constants = 0;
			QList<double> *functions = 0;
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
}

void MT_IndividualsWidget::slotNumConstChanged()
{
	numConstantsLCD->display((int) constantsListBox->count());
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
	receiverName += senderName.right(2);

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
	QLineEdit *senderObject = (QLineEdit*) sender();
	QString senderName(senderObject->objectName());

	// get the appropriate receiver
	QString receiverName("slider");
	receiverName += senderName.right(2);

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
