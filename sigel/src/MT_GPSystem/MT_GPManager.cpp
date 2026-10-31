#include "MT_GPSystem/MT_GPManager.h"
#include <unistd.h>
#include "MT_Control/MT_Substitute.h"
#include "SIGEL_Tools/SIG_IO.h"

// administrative method

MT_GPManager::MT_GPManager(QTextStream &File) : QObject()
{
	/* 
	Order of initialisation:
	Substituter
	GenerationNumber
	BestIndividual
	Randi
	Statistics
	Offspring
	Parent
	Seeker
	Selector
	FitnessTrainer
	*/

	pthread_mutex_init(&evolutionMutex, nullptr);

	sepEvolPossible = false;
	Substituter = nullptr;
	EvolStopped = false;
	LastError = MT_ErrorCode::None;

	QString Generation( "Generation:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Generation) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == Generation) && !(File.atEnd()))
		GenerationNumber = (File.readLine()).toInt();
	else 
		LastError = MT_ErrorCode::GenerationSectionMissing;
	
	if (GenerationNumber !=0)
		BestIndividual = std::make_unique< MT_Individual >(File);
		
	Randi = std::make_unique< MT_Randomizer >(File);
	
	if (GenerationNumber ==0)
		BestIndividual = std::make_unique< MT_Individual >(-1, Randi.get());

	if(BestIndividual->getLastError() != MT_ErrorCode::None)
		LastError = BestIndividual->getLastError();
	if(Randi->getLastError() != MT_ErrorCode::None)
		LastError = Randi->getLastError();

	Statistics = std::make_unique< MT_Statistics >(File);
	
	Offspring = std::make_unique< MT_Population >();
	QString OffspringSize ( "OffspringSize:" );
	while ((PresentLine != OffspringSize) && !(File.atEnd()))
		PresentLine = File.readLine();
	if ((PresentLine == OffspringSize) && !(File.atEnd()))
		Offspring->changePopSize((File.readLine()).toInt());
	else
		LastError = MT_ErrorCode::OffspringSizeSectionMissing;
	
	QString PopulationString ("Population:");
	while ((PresentLine != PopulationString) && !(File.atEnd()))
		PresentLine = File.readLine();
	if ((PresentLine == PopulationString) && !(File.atEnd()))
	{
		if (GenerationNumber !=0)
			Parent = std::make_unique< MT_Population >(File);
		else
		{
			int NewParentSize = (File.readLine()).toInt();
			Parent = std::make_unique< MT_Population >(Randi.get(),NewParentSize);
		}
	}
	else 
		LastError = MT_ErrorCode::ParentPopulationSectionMissing;

	if (Offspring->getSize()< Parent->getSize())
	{
		Offspring->changePopSize(Parent->getSize());
		Randi->setOffspringSize(Parent->getSize());
		LastError = MT_ErrorCode::OffspringSizeRaisedToParentSize;
	}

	Seeker = std::make_unique< MT_Search >(Parent.get(), Offspring.get(), Randi.get(), File);
	if(Seeker->getLastError() != MT_ErrorCode::None)
		LastError = Seeker->getLastError();

	Selector = std::make_unique< MT_TournamentManager >(Parent.get(), Offspring.get(), Randi.get(), File);
	if(Selector->getLastError() != MT_ErrorCode::None)
		LastError = Selector->getLastError();
	FitnessTrainer = std::make_unique< MT_FitnessTrainer >(File);
	if(FitnessTrainer->getLastError() != MT_ErrorCode::None)
		LastError = FitnessTrainer->getLastError();
}

MT_GPManager::MT_GPManager(MT_Substitute *Substitue) : QObject()
{
	// Not usable - use the loading constructor only 
}

MT_GPManager::MT_GPManager() : QObject()
{
}

MT_GPManager::~MT_GPManager()
{	
	pthread_mutex_unlock(&evolutionMutex);
}

void MT_GPManager::loadSetup(QTextStream &File)
{
	/* 
	1) Randi- parameter
	2) Offspring -> Size
	3) Parent -> Size
	4) BrutSize
	5) SMethod/ TournamentSize
	6) FitnessTrainer - Setup
	*/

	Randi->loadSetup(File);

	QString PresentLine = File.readLine();
	QString OffspringSize ("OffspringSize:");
	while ((PresentLine != OffspringSize) && !(File.atEnd()))
		PresentLine = File.readLine();
	
	if ((PresentLine == OffspringSize) && !(File.atEnd()))
		Offspring->changePopSize((File.readLine()).toInt());

	QString ParentString ("Population:");
	while ((PresentLine != ParentString) && !(File.atEnd()))
		PresentLine = File.readLine();
	
	if ((PresentLine == ParentString) && !(File.atEnd()))
		Parent->changePopSize((File.readLine()).toInt());
	else 
		LastError = MT_ErrorCode::ParentPopulationSetupSectionMissing;
	
	Parent->changeMaxNumVariable(Randi->getNumOfVari());

	if (Offspring->getSize()< Parent->getSize())
	{
		Offspring->changePopSize(Parent->getSize());
		Randi->setOffspringSize(Parent->getSize());
		LastError = MT_ErrorCode::OffspringSizeRaisedToParentSize;
	}

	QString BrutString ( "BrutSize:" );
		
	while ( (PresentLine != BrutString) && !(File.atEnd()))
		PresentLine = File.readLine();
		
	if ((PresentLine == BrutString) && !(File.atEnd()))
		Seeker->setBrutSize((File.readLine()).toInt());
	else 
		LastError = MT_ErrorCode::BroodSizeSetupSectionMissing;
	
	QString TManager ( "TournamentManager:");
	while ((PresentLine != TManager) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == TManager) && !(File.atEnd()))
	{
		Selector->setSelectionMethod((File.readLine()).toInt());
		Selector->setTournamentSize((File.readLine()).toInt());
 	}
	else 
		LastError = MT_ErrorCode::TournamentManagerSetupSectionMissing;
	
	FitnessTrainer->loadSetup(File);
	if(FitnessTrainer->getLastError() != MT_ErrorCode::None)
		LastError = FitnessTrainer->getLastError();
}

void MT_GPManager::writeToFileGPSystem(QTextStream &File)
{
	File << Qt::endl <<Qt::endl;
	File << ("Generation:\n");
	File << GenerationNumber << Qt::endl << Qt::endl;

	BestIndividual->writeToFileIndi(File);

	Randi->writeToFileRandi(File);

	Statistics->writeToFileMT_Statistics(File);

	File << ("OffspringSize:\n");
	File << (Offspring->getSize()) << Qt::endl << Qt::endl;

	Parent->writeToFilePop(File);

	File << ("BrutSize:\n");
	File << (Seeker->getBrutSize()) << Qt::endl << Qt::endl;

	Selector->writeToFileSetup(File);

	FitnessTrainer->writeToFileTrainer(File);

	//**************************
	// Additional information about the fitness computation ... is not loaded  
	//**************************
	if(!GenerationNumber)
		return;

	int PresentTSize = FitnessTrainer->getPresentTSize();
	MT_Individual * NewBestIndi= Parent->getBestIndividual();

	File << "best fitness before calculation: " << NewBestIndi->getFitness() << Qt::endl;
	NewBestIndi->setTrainingsSet(-1);
	MT_FitnessFunction fitFct;
	int tDur, tSize;
	FitnessTrainer->getSelectionValue(&fitFct, &tDur, &tSize);
	double corFitValue = NewBestIndi->getFitness();
	double ErrorCorrect =0.0;

	NewBestIndi->setTrainingsSet(-1);

	switch (fitFct)
	{
		case MT_FitnessFunction::EvaluatorSimpleError:
			{
				FitnessTrainer->calculateFitness(NewBestIndi);
				ErrorCorrect = ((0.1/NewBestIndi->getFitness())/PresentTSize);
			}
			break;
			
		case MT_FitnessFunction::EvaluatorSquareError:
			{
				FitnessTrainer->setSelectionValue(MT_FitnessFunction::EvaluatorSimpleError, tDur, tSize);
				FitnessTrainer->calculateFitness(NewBestIndi);
				ErrorCorrect = ((0.1/NewBestIndi->getFitness())/PresentTSize);
			}
			break;
			
		case MT_FitnessFunction::ClassifierSimple:
			{
				FitnessTrainer->calculateFitness(NewBestIndi);
				ErrorCorrect = (NewBestIndi->getFitness()/10.0);
			}
			break;

		case MT_FitnessFunction::ClassifierWeighted:
			{
				FitnessTrainer->setSelectionValue(MT_FitnessFunction::ClassifierSimple, tDur, tSize);
				FitnessTrainer->calculateFitness(NewBestIndi);
				ErrorCorrect = (NewBestIndi->getFitness()/10.0);
			}
			break;
	}

	QList<double> * Outcome =FitnessTrainer->getResultIstArray();
	QList<double> * CorrectFit =FitnessTrainer->getResultArray();
	int NumOfPositiveSigelFit =0; 
	int FitArrayLEnght = Outcome->count();

	for (int k=0; k<Outcome->count(); k++)
	{
		if ((*CorrectFit)[k]>=0.0)
			NumOfPositiveSigelFit ++;
		
		File << "No.: " << k << Qt::endl;
		File << " Sigel Fitness/Winner : " << (*CorrectFit)[k] << Qt::endl;
		File << " Meta Prediction      : " << (*Outcome)[k] << Qt::endl;
	}
	
	File << "#CorrectFit >=0 : " << NumOfPositiveSigelFit << Qt::endl << Qt::endl;

	File << "Generation: "<< GenerationNumber <<Qt::endl;
	File << "max.Fitness   : " << corFitValue << Qt::endl; 
	File << "simple max.Fit: " << NewBestIndi->getFitness() << Qt::endl;
	File << "avg. Fitness: " << Statistics->getStatisticElement(GenerationNumber-2)->AverageFitness << Qt::endl;
	File << "average Error/ Percent of Correct Estimation: " << ErrorCorrect << Qt::endl;

	FitnessTrainer->setSelectionValue(fitFct, tDur, tSize);
	NewBestIndi->setFitness(corFitValue);
}

void MT_GPManager::writeToFileSetup(QTextStream &File)
{
	Randi->writeToFileRandi(File);

	File << ("OffspringSize:\n");
	File << (Offspring->getSize()) << Qt::endl << Qt::endl;

	File << ("Population:\n");
	File << (Parent->getSize()) << Qt::endl << Qt::endl;

	File << ("BrutSize:\n");
	File << (Seeker->getBrutSize()) << Qt::endl << Qt::endl;

	Selector->writeToFileSetup(File);

	FitnessTrainer->writeToFileSetup(File);
}

void MT_GPManager::writeToFilePop(QTextStream &File)
{
	Parent->writeToFilePop(File);
}

// set/ get  method

MT_Statistics * MT_GPManager::getGPStatistics()
{
	return Statistics.get();
}

MT_Population * MT_GPManager::getParent()
{
	return Parent.get();
}

MT_Randomizer * MT_GPManager::getRandomizer()
{
	return Randi.get();
}

void MT_GPManager::setSelectionValue(int OffspringSize, int TournamentSize, int SMethod, MT_FitnessFunction FitnessFunction, int TrainingSetSize, int TrainingDuration)
{
	if ((OffspringSize !=0)&&(TournamentSize!=0) && (TrainingSetSize !=0))
	{
		if (OffspringSize != Offspring->getSize())
		{
			Randi->setOffspringSize(OffspringSize);
			Offspring->changePopSize(OffspringSize);
		}
		if ( OffspringSize < Parent->getSize())
		{
			Offspring->changePopSize(Parent->getSize());
			Randi->setOffspringSize(Parent->getSize());
			LastError = MT_ErrorCode::OffspringSizeRaisedToParentSize;
		}

		Selector->setTournamentSize(TournamentSize); // does more!!
		Selector->setSelectionMethod(SMethod);
		FitnessTrainer->setSelectionValue(FitnessFunction, TrainingDuration, TrainingSetSize);
	}
	else
		LastError = MT_ErrorCode::SizeIsZero;
}

void MT_GPManager::getSelectionValue(int *OffspringSize, int *TournamentSize, int *SMethod, MT_FitnessFunction *FitnessFunction, int *TrainingSetSize, int *TrainingDuration)
{
	*OffspringSize = Offspring->getSize();
	*TournamentSize = Selector->getTournamentSize();
	*SMethod = Selector->getSelectionMethod();

	 FitnessTrainer->getSelectionValue(FitnessFunction, TrainingDuration, TrainingSetSize);
}

void MT_GPManager::setBrutSize(int NewSize)
{
	if(NewSize !=0)
		Seeker->setBrutSize(NewSize);
	else
		LastError = MT_ErrorCode::SizeIsZero;
}

int MT_GPManager::getBrutSize()
{
	return (Seeker->getBrutSize());
}

void MT_GPManager::setPopAndTournamentSize(int NewPopSize, int NewTournamentSize)
{
	if ((NewPopSize !=0)&&(NewTournamentSize !=0))
	{
		Randi->setParentSize(NewPopSize);
		Selector->setTournamentSize(NewTournamentSize);
	}
	else
		LastError = MT_ErrorCode::SizeIsZero;
}

// special method

void MT_GPManager::startEvolution(MT_Substitute *Substitute)
{
/*************************** Initialisation ***************************/
	Substituter = Substitute;
	// The interpreter's parameters are now updated by the substituter
	if (Substituter !=nullptr)
		Substitute->setInterpreter(Randi->getNumOfVari(), FitnessTrainer->getTDuration());
	else
		LastError = MT_ErrorCode::SubstituteMissing;

	int GreatestName =0; 
	for (int i=0; i<Parent->getSize(); i++)
		if(GreatestName<Parent->getIndividual(i)->getName())
			GreatestName = Parent->getIndividual(i)->getName();

	Selector->setNameForParent(GreatestName);

	Parent->changeMaxNumVariable(Randi->getNumOfVari());
	if (BestIndividual !=nullptr)
		BestIndividual->changeMaxNumVariable(Randi->getNumOfVari());

	FitnessTrainer->setNumberOfVariables(Randi->getNumOfVari());

	int trainingDuration =0;
	int trainingSetSize =0;
	MT_FitnessFunction FitFunction = MT_FitnessFunction::EvaluatorSimpleError;
	FitnessTrainer->getSelectionValue(&FitFunction, &trainingDuration, &trainingSetSize);
	
	if ((FitFunction == MT_FitnessFunction::EvaluatorSimpleError) || (FitFunction == MT_FitnessFunction::EvaluatorSquareError))
		Selector->setTypeOfIndividual(0); // Evaluator Meta System;
	else 
		Selector->setTypeOfIndividual(1); // Classifier Meta System;

	bool TSetOK = false;

	int StartTSetSize =0;
	int TenthOfTSet = FitnessTrainer->getResultArray()->size();
	TenthOfTSet = TenthOfTSet/10.0;

	// set the best program
	// only useful when continuing (loading) a previous run
	exchangeBest();
	
	// tell the system that the evolution starts
	emit metaEvolutionRunning(true);

	if (Substituter !=nullptr)
	{
		while ((EvolStopped != true) && (TSetOK != true))
		{
			StartTSetSize= checkForNewTCase();
			if ((StartTSetSize >=10) && (StartTSetSize >TenthOfTSet)) {
				TSetOK = true;
			}
			else {
				SIGEL_Tools::SIG_IO::cerr << "MetaGP: training set below 10 cases or a tenth of the result array; next check in 10000000 s (about 115 days)." << Qt::endl;
				sleep(10000000);
			}
		}
	}
	
/*************************** Evolution loop ***************************/

	/* obtain the evolution lock to prevent concurrent saving during the evolution
	   (multi threaded!!!)
	 */

	while (EvolStopped != true)
	{
	pthread_mutex_lock(&evolutionMutex);

		SElement = new MT_StatisticsElement();
	
		GenerationNumber++;

		Seeker->startMatingProcess();

		FitnessTrainer->calculateFitness(Offspring.get());

		collectOffspringParameter(SElement);

		Selector->performTournaments();

		exchangeBest();

		checkForNewTCase();

		collectParentParameter( SElement);

		Statistics->addStatisticElement(SElement);
	
	pthread_mutex_unlock(&evolutionMutex);
	}
	
//*************************** Post-processing   ***************************

	emit metaEvolutionRunning(false);

	LastError = Statistics->updateStatistics();
	
	EvolStopped = false;
}

void MT_GPManager::stopEvolution()
{
	EvolStopped =true;
}

void MT_GPManager::exchangeBest()
{	
	if (Substituter !=nullptr)
	{
		bool ChangeNecessary = false;

		QList<double> * Outcome =FitnessTrainer->getResultIstArray();
		QList<double> * CorrectFit =FitnessTrainer->getResultArray();
		int PresentTSize = FitnessTrainer->getPresentTSize();

		// BestMETAProgram can only be replaced once the training set is completely filled
		// until then BestMETAProgram in the substituter points to 0, so no estimation happens
		// PresentTSize := current number of training cases in the training set
		// Outcome->size() := maximum permitted number of training cases in the training set 

		if (PresentTSize==Outcome->size()) 	
		{
			MT_Individual * NewBestIndi= Parent->getBestIndividual();
			
			ChangeNecessary = FitnessTrainer->calculateFitness(BestIndividual.get());

			if ((BestIndividual->getFitness())<(NewBestIndi->getFitness()))
			{
				BestIndividual->setNewProgram(NewBestIndi->getProgram());
				BestIndividual->setFitness(NewBestIndi->getFitness());
				BestIndividual->setNewAge(0);
		
				// Recompute so that Outcome/CorrectFit hold correct values, not those of the previous bestIndividual 
				BestIndividual->setTrainingsSet(-1);
				FitnessTrainer->calculateFitness(BestIndividual.get());

// lock the interpreter so we can safely exchange the program
	pthread_mutex_lock(&(Substituter->interpreterMutex));
				
				Substituter->changeBest(BestIndividual->getProgram());

// unlock the interpreter so that interpretation of programs can continue
	pthread_mutex_unlock(&(Substituter->interpreterMutex));
	pthread_mutex_lock(&(Substituter->fitnessMutex));

				Substituter->changeErrorInfo(Outcome ,CorrectFit);
	pthread_mutex_unlock(&(Substituter->fitnessMutex));
			}
			else 
			{
				BestIndividual->increaseAge();

				// lock the interpreter so we can safely exchange the program
	pthread_mutex_lock(&(Substituter->fitnessMutex));

				if (ChangeNecessary == true)
					Substituter->changeErrorInfo(Outcome ,CorrectFit);

				// unlock the interpreter so that interpretation of programs can continue
	pthread_mutex_unlock(&(Substituter->fitnessMutex));
			}
		}
	}
}

int  MT_GPManager::checkForNewTCase()
{
	int PresentTSetSize =0;

	if (Substituter !=nullptr){
		pthread_mutex_lock(&(Substituter->tCaseBufferMutex));

	PresentTSetSize = FitnessTrainer->insertNewTCases(Substituter->changeTCases());

		pthread_mutex_unlock(&(Substituter->tCaseBufferMutex));
	}

	return PresentTSetSize ;
}

void MT_GPManager::collectOffspringParameter(MT_StatisticsElement * SElement)
{
	for (int i=0; i<Offspring->getSize(); i++)
	{
		MT_GenesisType TypOfGenesis = (Offspring->getIndividual(i))->getTypOfGenesis();
		if (TypOfGenesis >= MT_GenesisType::Mutation)
		{
			SElement->NumOfTotalElementMutation = SElement->NumOfTotalElementMutation +(static_cast<int>(TypOfGenesis)-static_cast<int>(MT_GenesisType::Mutation));
			SElement->NumOfMutateOffspring ++;
			
			if ( ((Offspring->getIndividual(i))->getFitness())>=((Offspring->getIndividual(i))->getFitnessOfParent()) )
				SElement->NumOfMutateImprovingIndividuals ++;
		}
		else 
		{
			switch (TypOfGenesis)
			{
			case MT_GenesisType::OnePointCrossover:
				{
					SElement->CrossoverEventParent[0]++;
					if ( ((Offspring->getIndividual(i))->getFitness()) >= ((Offspring->getIndividual(i))->getFitnessOfParent()))
						SElement->CrossoverEventParent[1] ++;
				}
				break;
			case MT_GenesisType::TwoPointCrossover:
				{	SElement->CrossoverEventParent[2]++;
					if ( ((Offspring->getIndividual(i))->getFitness())>=((Offspring->getIndividual(i))->getFitnessOfParent()))
						SElement->CrossoverEventParent[3]++;
				}
				break;
			case MT_GenesisType::ThreePointCrossover:
				{	SElement->CrossoverEventParent[4]++;
					if ( ((Offspring->getIndividual(i))->getFitness())>=((Offspring->getIndividual(i))->getFitnessOfParent()))
						SElement->CrossoverEventParent[5]++;
				}
				break;

			case MT_GenesisType::Reproduction:
				{
					SElement->NumOfSimpleCopyOffspring++;
				}
				break;
			case MT_GenesisType::CarriedOverParent:
				{
					SElement->NumOfSimpleCopyOffspring++;
				}
				break;
			default:
				break;
			}
		}
	}
}

void MT_GPManager::collectParentParameter(MT_StatisticsElement * SElement)
{
	SElement->Generation = GenerationNumber;
	SElement->MaxFitness = (Parent->getBestIndividual())->getFitness();

	int ParentSize = Parent->getSize();
	MT_GenesisType TypOfGenesis;
	
	for (int i=0; i<ParentSize; i++)
	{
		TypOfGenesis = Parent->getIndividual(i)->getTypOfGenesis();
		if (TypOfGenesis >= MT_GenesisType::Mutation)
		{
			SElement->NumOfMutateIndividuals++;
			SElement->NumOfElementMutationParent = SElement->NumOfElementMutationParent + static_cast<int>(TypOfGenesis) - static_cast<int>(MT_GenesisType::Mutation);		
		}
		else
			if ((TypOfGenesis>=MT_GenesisType::OnePointCrossover) &&(TypOfGenesis<=MT_GenesisType::ThreePointCrossover))
				SElement->NumOfCrossoverEvent++;
			else
				SElement->NumOfSimpleCopyParent++;
		
		SElement->AverageFitness= SElement->AverageFitness + (Parent->getIndividual(i))->getFitness();
	}

	SElement->AverageFitness= (SElement->AverageFitness)/Parent->getSize();

	for (int i=0; i<ParentSize; i++)
		SElement->Variance = SElement->Variance + (((Parent->getIndividual(i))->getFitness()) - SElement->AverageFitness)*(((Parent->getIndividual(i))->getFitness()) - SElement->AverageFitness);

	SElement->Variance = SElement->Variance/ Parent->getSize();
}

void MT_GPManager::setInterpreterNumVar(int NewSize)
{
	FitnessTrainer->setNumberOfVariables(NewSize);
	if (Substituter !=nullptr)
		Substituter->setInterpreter(NewSize, FitnessTrainer->getTDuration());
}

MT_ErrorCode MT_GPManager::getLastError()
{
	return LastError;
}

bool MT_GPManager::separateEvolutionAllowed()
{
	if (FitnessTrainer->getPresentTSize() >= FitnessTrainer->getResultArray()->size())
		sepEvolPossible = true;
	else
		sepEvolPossible = false;

	return sepEvolPossible;
}