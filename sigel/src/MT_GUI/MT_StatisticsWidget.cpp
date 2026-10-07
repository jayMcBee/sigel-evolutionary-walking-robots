#include "MT_GUI/MT_StatisticsWidget.h"
#include "MT_GPSystem/MT_Statistics.h"

#include <math.h>
#include <QSlider>
#include <QMessageBox>
#include <QFileDialog>

MT_StatisticsWidget::MT_StatisticsWidget(QMainWindow* parent, const char* name, Qt::WindowFlags fl)
: MT_StatisticsWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
	soFarSigIdx = totalSim = totalEst = 0;
	soFarMetIdx = totalMaxFit = totalAvgFit = totalVarFit = 0;

	evolRunning = false;

	statToolbar = new QToolBar("MT Statistics", parent);
	if(parent)
		parent->addToolBar(Qt::TopToolBarArea, statToolbar);
	statToolbar->hide();

	QString pixPath = std::getenv("SIGEL_ROOT");
	pixPath.append("/pixmaps/");

	QIcon icon_updateAction(QPixmap(pixPath+"mt_UpdateSmall.xpm"));
	icon_updateAction.addPixmap(QPixmap(pixPath+"mt_UpdateSmall.xpm"));
	updateAction = new QAction(icon_updateAction, "Update statistics", this);
	updateAction->setToolTip("Update statistics");
	statToolbar->addAction(updateAction);
	QObject::connect(updateAction, SIGNAL(triggered()), SLOT(slotUpdateGUI()));

	autoUpdateCheckBox = new QCheckBox("Auto update every ", statToolbar);
	intervalSpinBox = new QSpinBox(statToolbar);
	intervalSpinBox->setRange(10, 6000);
	intervalSpinBox->setSingleStep(10);
	intervalLabel = new QLabel(" sec.", statToolbar);
	// A child parented to a toolbar is an overlay unless addWidget is called.
	statToolbar->addWidget(autoUpdateCheckBox);
	statToolbar->addWidget(intervalSpinBox);
	statToolbar->addWidget(intervalLabel);
	updateTimer = new QTimer(this);
	QObject::connect(autoUpdateCheckBox, SIGNAL(toggled(bool)), SLOT(slotAutoUpdateChanged(bool)));
	QObject::connect(updateTimer, SIGNAL(timeout()), SLOT(slotUpdateGUI()));

	QObject::connect(SIGGenSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSigEdit(int)));
	QObject::connect(SIGGenSlider, SIGNAL(valueChanged(int)), SLOT(slotSigSlider(int)));

	QObject::connect(metaGenSpinBox, SIGNAL(valueChanged(int)), SLOT(slotMtEdit(int)));
	QObject::connect(metaGenSlider, SIGNAL(valueChanged(int)), SLOT(slotMtSlider(int)));

	QObject::connect(estGraphButton, SIGNAL(clicked()), SLOT(slotEstGButton()));
	QObject::connect(fitnessGraphButton, SIGNAL(clicked()), SLOT(slotFitnessGButton()));
	QObject::connect(searchEffectsGraphButton, SIGNAL(clicked()), SLOT(slotSearchEffectsGButton()));
	QObject::connect(estDataButton, SIGNAL(clicked()), SLOT(slotEstDButton()));
	QObject::connect(fitnessDataButton, SIGNAL(clicked()), SLOT(slotFitnessDButton()));
	QObject::connect(searchEffectsDataButton, SIGNAL(clicked()), SLOT(slotSearchEffectsDButton()));
	QObject::connect(estPSButton, SIGNAL(clicked()), SLOT(slotEstPSExport()));
	QObject::connect(fitnessPSButton, SIGNAL(clicked()), SLOT(slotFitnessPSExport()));
	QObject::connect(searchEffectsPSButton, SIGNAL(clicked()), SLOT(slotSearchEffectsPSExport()));
}

void MT_StatisticsWidget::onShow(MT_GPManager *manager, subst_cache *subst)
{
	statToolbar->show();

	mng = manager;
	stat = manager->getGPStatistics();
	sigStat = subst;

	if(evolRunning && autoUpdateCheckBox->isChecked()){
		intervalSpinBox->setEnabled(false);
		if(!updateTimer->isActive())
			updateTimer->start(intervalSpinBox->value() * 1000);
	}

	updateGUI(-1, -1);
}

bool MT_StatisticsWidget::onHide(MT_GPManager *manager, subst_cache *subst)
{
	if(evolRunning)
		return false;

	updateTimer->stop();
	intervalSpinBox->setEnabled(true);

	statToolbar->hide();
	return true;
}

void MT_StatisticsWidget::updateGUI(int mtGen, int sigGen)
{
	MT_StatisticsElement *el=nullptr;

	QObject::disconnect(SIGGenSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotSigEdit(int)));
	QObject::disconnect(SIGGenSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSigSlider(int)));
	QObject::disconnect(metaGenSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotMtEdit(int)));
	QObject::disconnect(metaGenSlider, SIGNAL(valueChanged(int)), this, SLOT(slotMtSlider(int)));

	/* total page */
	// Simulation/Estimation
	int sigIdx = 0;
	if(sigStat && sigGen != -2){
		int numSIGGenerations = sigStat->genNumber;
		SIGGenSpinBox->setRange(1, numSIGGenerations);
		SIGGenSlider->setRange(1, numSIGGenerations);
		if(sigGen == -1){
			SIGGenSpinBox->setValue(numSIGGenerations);
			SIGGenSlider->setValue(numSIGGenerations);
			sigIdx = numSIGGenerations-1;
		} else {
			SIGGenSpinBox->setValue(sigGen);
			SIGGenSlider->setValue(sigGen);
			sigIdx = sigGen-1;
		}

		numSIGGen->setText(QString::number(numSIGGenerations));
		if(numSIGGenerations != 0){
			numSimulations->setText(QString::number(sigStat->numCorrectEst->at(sigIdx)));
			numEstimations->setText(QString::number(sigStat->numMetaEst->at(sigIdx)));

			// calculate the total numbers of Sim/Est
			// soFarSigIdx is a member: the loop adds the generations not yet in the totals, through generation sigGen; it adds nothing when sigGen is -1.
			while(soFarSigIdx < sigGen){
				totalSim += sigStat->numCorrectEst->at(soFarSigIdx);
				totalEst += sigStat->numMetaEst->at(soFarSigIdx);
				soFarSigIdx++;
			}
		}
		numSimulationsTotal->setText(QString::number(totalSim));
		numEstimationsTotal->setText(QString::number(totalEst));
	}

	// Fitness total
	int metaGens = stat->StatisticsOfGeneration.count();
	if(stat && sigGen == -1 && metaGens > 0){
		generationsTotal->setText(QString::number(metaGens));
		double tmpMax;
		while(soFarMetIdx < metaGens){
			el = stat->getStatisticElement(soFarMetIdx);
			if(el){
				totalVarFit += el->Variance;
				totalAvgFit += el->AverageFitness;
				tmpMax = el->MaxFitness;
				if(totalMaxFit<tmpMax)
					totalMaxFit = tmpMax;
			}
			soFarMetIdx++;
		}
		varFitnessTotal->setText(QString::number(totalVarFit / static_cast<double>(soFarMetIdx)));
		maxFitnessTotal->setText(QString::number(totalMaxFit));
		averageFitnessTotal->setText(QString::number(totalAvgFit / static_cast<double>(soFarMetIdx)));

		// search operator effects
		int total1 = stat->TotalCrossoverEvent[0];
		int total2 = stat->TotalCrossoverEvent[2];
		int total3 = stat->TotalCrossoverEvent[4];
		num1ptXOverTotal->setText(QString::number(total1));
		num2ptXOverTotal->setText(QString::number(total2));
		num3ptXOverTotal->setText(QString::number(total3));
		const double destroyedTotal1 = total1 - stat->TotalCrossoverEvent[1];
		const double destroyedTotal2 = total2 - stat->TotalCrossoverEvent[3];
		const double destroyedTotal3 = total3 - stat->TotalCrossoverEvent[5];
		num1ptXOverDestrTotal->setText(QString::number(destroyedTotal1 / total1 * 100.0 , 'f', 2 ));
		num2ptXOverDestrTotal->setText(QString::number(destroyedTotal2 / total2 * 100.0 , 'f', 2 ));
		num3ptXOverDestrTotal->setText(QString::number(destroyedTotal3 / total3 * 100.0 , 'f', 2 ));

		reproductionsTotal->setText(QString::number(stat->NumOfSimpleCopyParent));
		int mutInd = stat->NumOfMutateIndividuals;
		mutationsTotal->setText(QString::number(mutInd));
		const double mutationsNotImprovingTotal = mutInd - stat->NumOfMutateImprovingIndividuals;
		mutationsDestrTotal->setText(QString::number(mutationsNotImprovingTotal / mutInd * 100.0 , 'f', 2 ));
	}


	/* generational page */
	metaGenSpinBox->setRange(1, metaGens);
	metaGenSlider->setRange(1, metaGens);
	if(mtGen == -1){
		metaGenSpinBox->setValue(metaGens);
		metaGenSlider->setValue(metaGens);
		if(stat && metaGens > 0) 
			el = stat->getStatisticElement(metaGens-1);
	} else {
		metaGenSpinBox->setValue(mtGen);
		metaGenSlider->setValue(mtGen);
		el = stat->getStatisticElement(mtGen-1);
	}

	if(el && mtGen != -2 && metaGens > 0){

		// Fitness
		varFitness->setText(QString::number(el->Variance));
		maxFitness->setText(QString::number(el->MaxFitness));
		averageFitness->setText(QString::number(el->AverageFitness));

		// composition of the parent pool
		reproductionsParents->setText(QString::number(el->NumOfSimpleCopyParent));
//		crossoversParents->setText(QString::number(mng->getParent()->getSize() - el->NumOfSimpleCopyParent - el->NumOfMutateIndividuals));
		crossoversParents->setText(QString::number(el->NumOfCrossoverEvent));
		mutationsParents->setText(QString::number(el->NumOfMutateIndividuals));
		elMutationsParents->setText(QString::number(el->NumOfElementMutationParent));

		// search operator effects
		int total1 = el->CrossoverEventParent[0];
		int total2 = el->CrossoverEventParent[2];
		int total3 = el->CrossoverEventParent[4];
		num1ptXOver->setText(QString::number(total1));
		num2ptXOver->setText(QString::number(total2));
		num3ptXOver->setText(QString::number(total3));
		const double destroyed1 = total1 - el->CrossoverEventParent[1];
		const double destroyed2 = total2 - el->CrossoverEventParent[3];
		const double destroyed3 = total3 - el->CrossoverEventParent[5];
		num1ptXOverDestr->setText(QString::number(safePercentage(destroyed1, total1)));
		num2ptXOverDestr->setText(QString::number(safePercentage(destroyed2, total2)));
		num3ptXOverDestr->setText(QString::number(safePercentage(destroyed3, total3)));

		int XOverOffspring = total1+total2+total3;
		crossovers->setText(QString::number(XOverOffspring));
		reproductions->setText(QString::number(el->NumOfSimpleCopyOffspring));
		int mutOff = el->NumOfMutateOffspring;
		mutations->setText(QString::number(mutOff));
		const double mutationsNotImproving = mutOff - el->NumOfMutateImprovingIndividuals;
		mutationsDestr->setText(QString::number(safePercentage(mutationsNotImproving, mutOff)));
		elMutations->setText(QString::number(el->NumOfTotalElementMutation));
	}

	QObject::connect(SIGGenSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSigEdit(int)));
	QObject::connect(SIGGenSlider, SIGNAL(valueChanged(int)), SLOT(slotSigSlider(int)));
	QObject::connect(metaGenSpinBox, SIGNAL(valueChanged(int)), SLOT(slotMtEdit(int)));
	QObject::connect(metaGenSlider, SIGNAL(valueChanged(int)), SLOT(slotMtSlider(int)));
}

void MT_StatisticsWidget::slotUpdateGUI()
{
	updateGUI(-1, -1);
}

void MT_StatisticsWidget::slotSigEdit(int nval)
{
	QObject::disconnect(SIGGenSlider, SIGNAL(valueChanged(int)), this, SLOT(slotSigSlider(int)));

	SIGGenSlider->setValue(nval);
	updateGUI(-2, nval);

	QObject::connect(SIGGenSlider, SIGNAL(valueChanged(int)), SLOT(slotSigSlider(int)));
}

void MT_StatisticsWidget::slotSigSlider(int nval)
{
	QObject::disconnect(SIGGenSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotSigEdit(int)));

	SIGGenSpinBox->setValue(nval);
	updateGUI(-2, nval);

	QObject::connect(SIGGenSpinBox, SIGNAL(valueChanged(int)), SLOT(slotSigEdit(int)));
}

void MT_StatisticsWidget::slotMtEdit(int nval)
{
	QObject::disconnect(metaGenSlider, SIGNAL(valueChanged(int)), this, SLOT(slotMtSlider(int)));

	metaGenSlider->setValue(nval);
	updateGUI(nval, -2);

	QObject::connect(metaGenSlider, SIGNAL(valueChanged(int)), SLOT(slotMtSlider(int)));
}

void MT_StatisticsWidget::slotMtSlider(int nval)
{
	QObject::disconnect(metaGenSpinBox, SIGNAL(valueChanged(int)), this, SLOT(slotMtEdit(int)));
		
	metaGenSpinBox->setValue(nval);
	updateGUI(nval, -2);

	QObject::connect(metaGenSpinBox, SIGNAL(valueChanged(int)), SLOT(slotMtEdit(int)));
}

void MT_StatisticsWidget::slotAutoUpdateChanged(bool on)
{
	if(on){
		intervalSpinBox->setEnabled(false);
		if(evolRunning && !updateTimer->isActive())
			updateTimer->start(intervalSpinBox->value() * 1000);
	} else {
		updateTimer->stop();
		intervalSpinBox->setEnabled(true);
	}
}

void MT_StatisticsWidget::evolutionRunning(bool running)
{
	evolRunning = running;

	if(evolRunning){
		if(autoUpdateCheckBox->isChecked()){
			intervalSpinBox->setEnabled(false);
			if(!updateTimer->isActive())
				updateTimer->start(intervalSpinBox->value() * 1000);
		}
	} else {
		updateTimer->stop();
		intervalSpinBox->setEnabled(true);
		updateGUI(-1, -1);
	}
}

/***
 * show and export statistics
 ***/
/***
 * show on screen
 **/
void MT_StatisticsWidget::slotEstGButton()
{
	if(sigStat->genNumber < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	plotEstimation(QString());
}

void MT_StatisticsWidget::slotFitnessGButton()
{
	int metaGens = 0;

	if(stat)
		metaGens = stat->StatisticsOfGeneration.count();

	if(!stat || metaGens < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	plotFitness(QString());
}

void MT_StatisticsWidget::slotSearchEffectsGButton()
{
	int metaGens = 0;

	if(stat)
		metaGens = stat->StatisticsOfGeneration.count();

	if(!stat || metaGens < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	plotSearchEffects(QString());
}

/***
 * export data
 **/
void MT_StatisticsWidget::slotEstDButton()
{
	if(sigStat->genNumber < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(),
						   "gnuplot Data Files (*.dat);;All Files (*)" );
	if(fileName.isNull())
		return;
	if(fileName.right(4) != ".dat")
		fileName.append(".dat");

	QFile file(fileName);

	file.open(QIODevice::WriteOnly);
	QTextStream pipeStream(&file);

	// write data
	pipeStream << "# SIGEL-Generation\t\tnum of simulations\t\tnum of estimations\n#\n";
	for(int i=0; i<sigStat->genNumber; i++){
		pipeStream << i << " " 
			<< sigStat->numCorrectEst->at(i) << " "
			<< sigStat->numMetaEst->at(i) << "\n";
	}

	file.close();
}

void MT_StatisticsWidget::slotFitnessDButton()
{
	int metaGens = 0;

	if(stat)
		metaGens = stat->StatisticsOfGeneration.count();

	if(!stat || metaGens < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(),
						   "gnuplot Data Files (*.dat);;All Files (*)" );
	if(fileName.isNull())
		return;
	if(fileName.right(4) != ".dat")
		fileName.append(".dat");

	QFile file(fileName);

	file.open(QIODevice::WriteOnly);
	QTextStream pipeStream(&file);

	// write data
	pipeStream << "# Meta generations\t\tmax fitness\t\taverage fitness\t\tvariance\n#\n";

	MT_StatisticsElement *el=nullptr;
	for(int i=0; i<metaGens; i++){
		el = stat->getStatisticElement(i);
		if(el) pipeStream << i << " " 
			<< el->MaxFitness << " " 
			<< el->AverageFitness << " "
			<< sqrt(el->Variance) << "\n";
	}
	file.close();
}

void MT_StatisticsWidget::slotSearchEffectsDButton()
{
	int metaGens = 0;

	if(stat)
		metaGens = stat->StatisticsOfGeneration.count();

	if(!stat || metaGens < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(),
						   "gnuplot Data Files (*.dat);;All Files (*)" );
	if(fileName.isNull())
		return;
	if(fileName.right(4) != ".dat")
		fileName.append(".dat");

	QFile file(fileName);

	file.open(QIODevice::WriteOnly);
	QTextStream pipeStream(&file);

	// write data
	pipeStream << "#  number of XOver-Events and the percentage of destructive XOvers\n"
		<< "# Generation\t\tnum 1pt\t\tnum 2pt\t\tnum 3pt\t\t%destr 1pt\t\t%destr 2pt\t\t%destr 3pt\n#\n";

	MT_StatisticsElement *el=nullptr;
	for(int i=0; i<metaGens; i++){
		el = stat->getStatisticElement(i);
		int total1, total2, total3;
		if(el){
			total1 = el->CrossoverEventParent[0]; 
			total2 = el->CrossoverEventParent[2];
			total3 = el->CrossoverEventParent[4]; 
			const double destroyed1 = total1 - el->CrossoverEventParent[1];
			const double destroyed2 = total2 - el->CrossoverEventParent[3];
			const double destroyed3 = total3 - el->CrossoverEventParent[5];
			pipeStream << i << " "
				<< total1 << " "
				<< total2 << " "
				<< total3 << " "
				<< safePercentage(destroyed1, total1)  << " "
				<< safePercentage(destroyed2, total2)  << " "
				<< safePercentage(destroyed3, total3)  << "\n";
		}
	}
	file.close();
}

/***
 * export postscripts
 **/
void MT_StatisticsWidget::slotEstPSExport()
{
	if(sigStat->genNumber < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(),
						   "Encapsulated PostScript Files (*.eps);;All Files (*)" );
	if(fileName.isNull())
		return;
	if(fileName.right(4) != ".eps")
		fileName.append(".eps");

	plotEstimation(fileName);
}

void MT_StatisticsWidget::slotFitnessPSExport()
{
	int metaGens = 0;

	if(stat)
		metaGens = stat->StatisticsOfGeneration.count();

	if(!stat || metaGens < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(),
						   "Encapsulated PostScript Files (*.eps);;All Files (*)" );
	if(fileName.isNull())
		return;
	if(fileName.right(4) != ".eps")
		fileName.append(".eps");

	plotFitness(fileName);
}

void MT_StatisticsWidget::slotSearchEffectsPSExport()
{
	int metaGens = 0;

	if(stat)
		metaGens = stat->StatisticsOfGeneration.count();

	if(!stat || metaGens < 2){
		QMessageBox::information(this, "View Graph", "There's not enough data to plot a graph.");
		return;
	}
	QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(),
						   "Encapsulated PostScript Files (*.eps);;All Files (*)" );
	if(fileName.isNull())
		return;
	if(fileName.right(4) != ".eps")
		fileName.append(".eps");
	
	plotSearchEffects(fileName);
}

/***
 * plot the data
 **/
void MT_StatisticsWidget::plotEstimation(QString fileName)
{
	FILE *gnuPlotPipe = popen( "gnuplot -persist -", "w");
	if(!gnuPlotPipe){
		QMessageBox::warning( this, "Error", "Couldn't start gnuplot.");
		return;
	}

	QFile pipeFile;
	pipeFile.open(gnuPlotPipe, QIODevice::WriteOnly);
	QTextStream pipeStream(&pipeFile);

	// fill the stream with gnuplot commands and data
	if(!fileName.isNull()){
		pipeStream << "set terminal postscript eps color\n"
			<< "set output \"" << fileName << "\"\n";
	}

	pipeStream << "set data style lines\n"
		<< "set yrange[0:" << sigStat->numMetaEst->at(0) + sigStat->numCorrectEst->at(0) << "]\n"
		<< "set title \"Simulation - Estimation Relationship\"\n"
		<< "set xlabel 'Generation'\n"
		<< "set ylabel\n"
		<< "plot '-' title 'estimations'\n"; //, '-' title 'simulations'\n";

/*	for(int i=0; i<sigStat->genNumber; i++){
		pipeStream << i << " " << sigStat->numCorrectEst->at(i) << "\n";
	}
	pipeStream << "e\n";*/
	for(int j=0; j<sigStat->genNumber; j++){
		pipeStream << j << " " << sigStat->numMetaEst->at(j) << "\n";
	}
	pipeStream << "e\nquit\n";

	pipeFile.close();
	pclose( gnuPlotPipe );
}

void MT_StatisticsWidget::plotFitness(QString fileName)
{
	FILE *gnuPlotPipe = popen( "gnuplot -persist -", "w");
	if(!gnuPlotPipe){
		QMessageBox::warning( this, "Error", "Couldn't start gnuplot.");
		return;
	}

	QFile pipeFile;
	pipeFile.open(gnuPlotPipe, QIODevice::WriteOnly);
	QTextStream pipeStream(&pipeFile);

	// fill the stream with gnuplot commands and data
	if(!fileName.isNull()){
		pipeStream << "set terminal postscript eps color\n"
			<< "set output \"" << fileName << "\"\n";
	}

	pipeStream 	<< "set title \"Fitness curve\"\n"
		<< "set xlabel 'Generation'\n"
		<< "set ylabel\n"
		<< "plot '-' title 'variance' with yerrorbars, '-' smooth bezier title 'average fitness' with lines, '-' smooth bezier title 'max fitness' with lines\n";

	MT_StatisticsElement *el=nullptr;
	int metaGens = stat->StatisticsOfGeneration.count();
	for(int k=0; k<metaGens; k++){
		el = stat->getStatisticElement(k);
		if(el) pipeStream << k << " " << el->AverageFitness << " " << sqrt(el->Variance) << "\n";
	}
	pipeStream << "e\n";
	for(int j=0; j<metaGens; j++){
		el = stat->getStatisticElement(j);
		if(el) pipeStream << j << " " << el->AverageFitness << "\n";
	}
	pipeStream << "e\n";
	for(int i=0; i<metaGens; i++){
		el = stat->getStatisticElement(i);
		if(el) pipeStream << i << " " << el->MaxFitness << "\n";
	}
	pipeStream << "e\nquit\n";
	// ***

	pipeFile.close();
	pclose( gnuPlotPipe );
}

// A total of 0 gives 0 %.
int MT_StatisticsWidget::safePercentage(double part, int total)
{
	if(total == 0)
		return 0;
	return static_cast<int>(part / total * 100.0);
}

void MT_StatisticsWidget::plotSearchEffects(QString fileName)
{
	FILE *gnuPlotPipe = popen( "gnuplot -persist -", "w");
	if(!gnuPlotPipe){
		QMessageBox::warning( this, "Error", "Couldn't start gnuplot.");
		return;
	}

	QFile pipeFile;
	pipeFile.open(gnuPlotPipe, QIODevice::WriteOnly);
	QTextStream pipeStream(&pipeFile);

	// fill the stream with gnuplot commands and data
	if(!fileName.isNull()){
		pipeStream << "set terminal postscript eps color\n"
			<< "set output \"" << fileName << "\"\n";
	}

	pipeStream 	<< "set title \"Crossover Effects\"\n"
		<< "set xlabel 'Generation'\n"
		<< "set ylabel 'number of XOvers'\n"
		<< "set y2label 'destructive XOver in %'\n"
		<< "set y2range [0:100]\n"
		<< "set y2tics\n"
		<< "set ytics nomirror\n"
		<< "plot '-' axis x1y1 title 'num 1pt' with lines, '-' axis x1y1 title 'num 2pt' with lines, '-' axis x1y1 title 'num 3pt' with lines, '-' axis x1y2 title '1pt destr.' with lines, '-' axis x1y2 title '2pt destr.' with lines, '-' axis x1y2 title '3pt destr.' with lines\n";

	MT_StatisticsElement *el=nullptr;
	int metaGens = stat->StatisticsOfGeneration.count();
	for(int i=0; i<metaGens; i++){
		el = stat->getStatisticElement(i);
		if(el) pipeStream << i << " " << el->CrossoverEventParent[0] << "\n";
	}
	pipeStream << "e\n";
	for(int k=0; k<metaGens; k++){
		el = stat->getStatisticElement(k);
		if(el) pipeStream << k << " " << el->CrossoverEventParent[2] << "\n";
	}
	pipeStream << "e\n";
	for(int j=0; j<metaGens; j++){
		el = stat->getStatisticElement(j);
		if(el) pipeStream << j << " " << el->CrossoverEventParent[4] << "\n";
	}
	pipeStream << "e\n";

	int total=0;
	for(int l=0; l<metaGens; l++){
		el = stat->getStatisticElement(l);
		if(el){
			total = el->CrossoverEventParent[0];
			const double destroyed = total - el->CrossoverEventParent[1];
			pipeStream << l << " " << safePercentage(destroyed, total) << "\n";
		}
	}
	pipeStream << "e\n";
	for(int m=0; m<metaGens; m++){
		el = stat->getStatisticElement(m);
		if(el){
			total = el->CrossoverEventParent[2];
			const double destroyed = total - el->CrossoverEventParent[3];
			pipeStream << m << " " << safePercentage(destroyed, total) << "\n";
		}
	}
	pipeStream << "e\n";
	for(int n=0; n<metaGens; n++){
		el = stat->getStatisticElement(n);
		if(el){
			total = el->CrossoverEventParent[4];
			const double destroyed = total - el->CrossoverEventParent[5];
			pipeStream << n << " " << safePercentage(destroyed, total) << "\n";
		}
	}
	pipeStream << "e\nquit\n";
	// ***

	pipeFile.close();
	pclose( gnuPlotPipe );
}