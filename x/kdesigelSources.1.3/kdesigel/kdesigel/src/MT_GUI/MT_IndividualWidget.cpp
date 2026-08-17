#include "MT_GUI/MT_IndividualWidget.h"
#include "MT_GUI/MT_AddConstantsWidget.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <qslider.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qprogressdialog.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qlcdnumber.h>
#include <qstringlist.h>

/********************************
 *       general methods 
 ********************************/

MT_IndividualsWidget::MT_IndividualsWidget(QWidget* parent, const char* name, WFlags fl)
	: MT_IndividualsWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	minValue = -10000.0;
	maxValue =  10000.0;
	integer  = false;
	numToCreate = 0;

	// limit the lineEdit's allowed values to that of the sliders
	validator = new QIntValidator(slider01->minValue(), slider01->maxValue(), this);
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
	constContextMenu = new QPopupMenu(this, "mtConstContextMenu");
	delConstAction = new QAction("Delete Constant", "&Delete", 0, this);
	delConstAction->setStatusTip("Deletes the selected constants.");
	delConstAction->setEnabled(false);
	delConstAction->addTo(constContextMenu);

	editConstAction = new QAction("Edit Constant", "&Edit", 0, this);
	editConstAction->setStatusTip("Edits the current constant.");
	editConstAction->setEnabled(false);
	editConstAction->addTo(constContextMenu);

	connect(delConstAction, SIGNAL(activated()), SLOT(slotDelConst()));
	connect(editConstAction, SIGNAL(activated()), SLOT(slotEditConst()));

	// connection for the listbox
	connect((const QObject*) constantsListBox, SIGNAL(rightButtonClicked(QListBoxItem*, const QPoint&)), SLOT(slotRButtonClicked(QListBoxItem*, const QPoint&)));
	connect((const QObject*) constantsListBox, SIGNAL(currentChanged(QListBoxItem*)), SLOT(slotSelectionChanged()));
	connect((const QObject*) constantsListBox, SIGNAL(selectionChanged()), SLOT(slotSelectionChanged()));
	connect((const QObject*) constantsListBox, SIGNAL(selected(QListBoxItem*)), SLOT(slotEditConst(QListBoxItem*)));

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
	QArray<double> *constants = 0;
	QArray<double> *functions = 0;

	if(randomizer){
		
		randomizer->returnIndividualsValue(&length, &numVar, &constants, &functions);
		
		// update the GUI-elements
		programLengthSpinBox->setValue(*length);
		numVariablesSpinBox->setValue(*numVar);

		// add the constants to the appropriate listbox
		constantsListBox->clear();
		for(int i=0; i<constants->size(); i++){
			constantsListBox->insertItem(QString("%1").arg(constants->at(i)));
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
		for(i=1; i<10; i++){
			receivername = com1 + QString("%1").arg(i);
			actual = functions->at(i-1);
			valText = QString("%1").arg((int)(actual - previous));
			previous = actual;

			receiverObject = (QLineEdit*) child(receivername, "QLineEdit");
			if(receiverObject&& (receiverObject->text() != valText)){
				receiverObject->setText(valText);
			}
		}
		for(i=10; i<19; i++){
			receivername = com2 + QString("%1").arg(i);
			actual = functions->at(i-1);
			valText = QString("%1").arg((int)(actual - previous));
			previous = actual;

			receiverObject = (QLineEdit*) child(receivername, "QLineEdit");
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

	QArray<double> *constants = 0;
	QArray<double> *functions = 0;
	randomizer->returnIndividualsValue(&length, &numVar, &constants, &functions);

	*length = programLengthSpinBox->value();
	*numVar = numVariablesSpinBox->value();

	// set the constants
	int numConst = constantsListBox->count();
	if(constants->size() != numConst)
		constants->resize(numConst);
	for(int i=0; i<numConst; i++){
		constants->at(i) = constantsListBox->text(i).toDouble();
	}

	// set the command probabilities
	QString com1("edit0");
	QString com2("edit");
	QString valText;
	QString receivername;
	QLineEdit *receiverObject;
	double previous = 0.0;
	for(i=1; i<10; i++){
		receivername = com1 + QString("%1").arg(i);
		receiverObject = (QLineEdit*) child(receivername, "QLineEdit");
		if(receiverObject){
			previous += receiverObject->text().toDouble();
		}
		functions->at(i-1) = previous;
	}
	for(i=10; i<19; i++){
		receivername = com2 + QString("%1").arg(i);
		receiverObject = (QLineEdit*) child(receivername, "QLineEdit");
		if(receiverObject){
			previous += receiverObject->text().toDouble();
		}
		functions->at(i-1) = previous;
	}
	
	return true;
}

/********************************
 *    settings for constants       
 ********************************/

void MT_IndividualsWidget::slotImportConstants()
{
	QStringList files( QFileDialog::getOpenFileNames( "Constants(*.mcon);;All Files(*)",
		0, this, "ConImpDialog", "Import Constants"));


	if(files.isEmpty())
		return;
	
	// iterate over all selected files
	for(QStringList::Iterator it = files.begin(); it != files.end(); ++it){
		QFile file(*it);

		if(file.open(IO_ReadOnly)){

			// file was successfully opened
			// so read it and fill the datastructures
			QTextStream stream( &file );
			QString s;
			bool found = false;
			int count=0;
			while( !stream.eof() ){			// iterate the whole file
				s = stream.readLine();
				if(!found && s=="[constants]"){
					found = true;
					s = stream.readLine();
				}
				if(found && s=="count"){
					s = stream.readLine();
					int tc = count = s.toInt();
					QProgressDialog progress("Loading constants", 0, count, this, 0, true);
					while(tc && !stream.eof()){
						progress.setProgress(count - tc);
						s = stream.readLine();
						constantsListBox->insertItem(QString(s));
						tc--;
						emit numConstChanged();
					}
					progress.setProgress(count);
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
	QString fileName = QFileDialog::getSaveFileName(0, "Constants(*.mcon);;All Files(*)", this);
	if(fileName.isEmpty()) return;
	if(fileName.right(5) != ".mcon")
		fileName += ".mcon";

	QFile file(fileName);
	if(file.exists()){
		if(0 == QMessageBox::warning(this, "Save constants", "There is another file with this name. This will overwrite\n"
			"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
			return;
	}
	
	if(file.open( IO_WriteOnly )){
		QTextStream str( &file );
		int count = constantsListBox->count();
		if(!count){
			QMessageBox::information(this, "Export Constants",
				"Listbox doesn't contain constants.\n"
				"Couldn't save constants.", 1, 0);
			return;
		}
		str << "[constants]\n";
		str << "count\n" << count << endl;
		for(int i=0; i<count; i++){
			str << constantsListBox->text(i) << endl;
		}
		str << endl;
		file.close();
	} else {
		QMessageBox::critical(this, "Export Constants",
			"Couldn't open file for writing.\n"
			"Constant won't be saved.", 1, 0);
	}
}

void MT_IndividualsWidget::slotEditConst()
{
	slotEditConst(constantsListBox->item(constantsListBox->currentItem()));
}

void MT_IndividualsWidget::slotEditConst(QListBoxItem *item)
{
	QRect dim(constantsListBox->itemRect(item));
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
	constantsListBox->changeItem(ntext, constantsListBox->currentItem());
}

void MT_IndividualsWidget::slotSelectionChanged()
{
	int actItem = constantsListBox->currentItem();

	if((actItem != -1) && constantsListBox->isSelected(actItem)){
		delConstAction->setEnabled(true);
		editConstAction->setEnabled(true);
	} else {
		delConstAction->setEnabled(false);
		editConstAction->setEnabled(false);
	}
}

void MT_IndividualsWidget::slotDelConst()
{
	QListBoxItem *nextInd=0;

	// iterate over all items, check if they are selected
	// and delete the selected ones
	for(uint actInd=0; actInd<constantsListBox->count(); actInd++){
		if(constantsListBox->isSelected(actInd)){
			constantsListBox->removeItem(actInd);	// delete actual constant if selected
			actInd--;		// the indices of the following items were decreased !!
			emit numConstChanged();
		}
		//numConstants->setMinValue(constantsListBox->count());
	}
}

void MT_IndividualsWidget::slotRButtonClicked(QListBoxItem *item, const QPoint &point)
{
	constContextMenu->popup(point);
}

void MT_IndividualsWidget::slotCreateConstants()
{
	MT_AddConstantsWidget constDialog(this, 0, true);

	if(QDialog::Accepted == constDialog.exec()){

		if(numToCreate > 0){
		
			QProgressDialog progress("Generating constants", 0, numToCreate+1, this, 0, true);
			progress.setProgress(0);

			int *length = 0;
			int *numVar = 0;
			QArray<double> *constants = 0;
			QArray<double> *functions = 0;
			randomizer->createConstant(numToCreate, integer, minValue, maxValue);
			randomizer->returnIndividualsValue(&length, &numVar, &constants, &functions);

			for(int i=0; i<constants->size(); i++){
				progress.setProgress(i+1);
				constantsListBox->insertItem(QString("%1").arg(constants->at(i)));
				emit numConstChanged();
			}
			progress.setProgress(numToCreate+1);
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
	QString senderName(senderObject->name());

	// get the appropriate receiver
	QString receiverName("edit");
	receiverName += senderName.right(2);

	QLineEdit *receiverObject = (QLineEdit*) child(receiverName, "QLineEdit");

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
	QString senderName(senderObject->name());

	// get the appropriate receiver
	QString receiverName("slider");
	receiverName += senderName.right(2);

	QSlider *receiverObject = (QSlider*) child(receiverName, "QSlider");

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
