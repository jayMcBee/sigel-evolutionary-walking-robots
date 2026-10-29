#include <QList>
#include <QString>
#include <QTextStream>
#include <memory>
#include "MT_Control/MT_Classifier.h"

MT_Classifier::MT_Classifier(QTextStream &File) : MT_Substitute()
{
	AverageSigelFitness = 0.01;
	CorrectFitness.resize(0);
	AssumedFitness.resize(0);
	MetaProgError =-1.0;
	NumOfMeta =0;

	QString Classifier( "Classifier:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Classifier) && !(File.atEnd()))
	{
		PresentLine = File.readLine();
	}
	if (PresentLine == Classifier)
	{
		EstimationStrategy = (File.readLine()).toInt();
		Tolerance = (File.readLine()).toDouble();
		RefreshInterval = (File.readLine()).toInt();
		GenerationNumber = (File.readLine()).toUInt();

		if(GenerationNumber !=0)
		{
			NumOfCorrectEstimation.resize(GenerationNumber+1);
			NumOfMetaEstimation.resize(GenerationNumber+1);

			for(int i=0; i< GenerationNumber; i++)
			{
				NumOfCorrectEstimation[i] = (File.readLine()).toUInt();
				NumOfMetaEstimation[i] = (File.readLine()).toUInt();
			}
			NumOfCorrectEstimation[GenerationNumber] = 0;
			NumOfMetaEstimation[GenerationNumber] = 0;
		}
		else
		{
			NumOfCorrectEstimation.resize(100);
			NumOfMetaEstimation.resize(100);
			for(int i=0; i< 100; i++)
			{
				NumOfCorrectEstimation[i] = 0;
				NumOfMetaEstimation[i] = 0;
			}
		}
	}
}

void MT_Classifier::loadSetup(QTextStream &File)
{
	QString Classifier( "Classifier:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Classifier) && !(File.atEnd()))
	{
		PresentLine = File.readLine();
	}
	if (PresentLine == Classifier)
	{
		EstimationStrategy = (File.readLine()).toInt();
		Tolerance = (File.readLine()).toDouble();
		RefreshInterval = (File.readLine()).toInt();
	}
}

void MT_Classifier::writeToFile(QTextStream &File)
{
	File << ("Classifier:\n");
	File << EstimationStrategy <<Qt::endl;
	File << Tolerance <<Qt::endl;
	File << RefreshInterval <<Qt::endl;
	File << GenerationNumber <<Qt::endl;

	for(int i=0; i< GenerationNumber; i++)
	{
		File << NumOfCorrectEstimation[i] <<Qt::endl;
		File << NumOfMetaEstimation[i] << Qt::endl;
	}
}

void MT_Classifier::writeToFileSetup(QTextStream &File)
{
	File << ("Evaluator:\n");
	File << EstimationStrategy <<Qt::endl;
	File << Tolerance <<Qt::endl;
	File << RefreshInterval <<Qt::endl;
	int GenNum =0;
	File << GenNum <<Qt::endl;
}

// functionally method

double MT_Classifier::classifier(SIGEL_Program::SIG_Program const * SigProgOne, SIGEL_Program::SIG_Program const * SigProgTwo)
{
	std::unique_ptr< MT_TranslatedIndividual > TransIndi( createDoubleTransIndi(SigProgOne, SigProgTwo) );

	return Interpreter.interpret( TransIndi.get() );
}

void MT_Classifier::createNewTCase(SIGEL_Program::SIG_Program const * SigProgOne, SIGEL_Program::SIG_Program const * SigProgTwo, double FitDifference)
{
	MT_TranslatedIndividual * TransIndi = createDoubleTransIndi(SigProgOne, SigProgTwo);

	// lock the TCaseBuffer so that no TCases could be taken from it
	pthread_mutex_lock(&tCaseBufferMutex);

	TCaseBuffer.enqueue (new MT_TrainingCase(FitDifference, TransIndi, NumOfCorrectEstimation[GenerationNumber], TransIndi->Boundary));

	// create the symmetric TCase
	TransIndi = createDoubleTransIndi(SigProgTwo, SigProgOne);
	TCaseBuffer.enqueue (new MT_TrainingCase(-1.0 * FitDifference, TransIndi, NumOfCorrectEstimation[GenerationNumber], TransIndi->Boundary));

	// unlock the TCaseBuffer
	pthread_mutex_unlock(&tCaseBufferMutex);
}

int MT_Classifier::evaluationTactic(int TournamentCount)
{
	int NumOfClassi=0; //Number of tournaments to be run with the classifier
	double Num =0.0;
	double SigelGeneration = GenerationNumber;
	if (SigelGeneration <1.0)
		SigelGeneration =1.0;

	if (BestMETAProgram ==nullptr)
		return NumOfClassi;

	switch (EstimationStrategy)
	{
	case 3:
		{
			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size(); i++)
					if( ((AssumedFitness[i]>=0.0)&&(CorrectFitness[i]<0.0)) || ((AssumedFitness[i]<0.0)&&(CorrectFitness[i]>=0.0)) )
						MetaProgError++;

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := percentage error
			}

			if(MetaProgError< Tolerance)
			{
				Num = TournamentCount/RefreshInterval;
				if (Num <1.0)
					NumOfMeta=1;
				else
					NumOfMeta = Num;

				NumOfClassi = TournamentCount-NumOfMeta;
			}
		}
		break;

	case 4:
		{
			// Evaluation strategy weighted by SIGEL generation count
			double ToleranceNew = Tolerance;
			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size(); i++)
					if( ((AssumedFitness[i]>=0.0)&&(CorrectFitness[i]<0.0)) || ((AssumedFitness[i]<0.0)&&(CorrectFitness[i]>=0.0)) )
						MetaProgError++;

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := percentage error
			}

			// Tolerance is weighted by generation count
			if (SigelGeneration >5.0 && SigelGeneration <= 150.0)
			{
				double Rate = (40.0 - Tolerance) / 145.0;
				if (Rate >0.0)
				{
					ToleranceNew = 40.0 - Rate*SigelGeneration;
				}
			}

			if(MetaProgError < ToleranceNew)
			{
				Num = TournamentCount/RefreshInterval;
				if (Num <1.0)
					NumOfMeta=1;
				else
					NumOfMeta = Num;

				NumOfClassi = TournamentCount-NumOfMeta;
			}
		}
		break;
	case 5:
		{
			// Evaluation strategy weighted by SIGEL fitness

			double ToleranceNew = Tolerance;

			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size(); i++)
					if( ((AssumedFitness[i]>=0.0)&&(CorrectFitness[i]<0.0)) || ((AssumedFitness[i]<0.0)&&(CorrectFitness[i]>=0.0)) )
						MetaProgError++;

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := percentage error
			}

			// Tolerance weighting: AverageSigelFitness
			if (AverageSigelFitness>0.1 && AverageSigelFitness<1.0)
			{
				double ASigelFit = AverageSigelFitness - 0.1;
				double Rate = (40.0-Tolerance)/0.9;
				if (Rate >0.0)
					ToleranceNew = 40.0 - Rate*ASigelFit;
				if (ToleranceNew<0.0)
					ToleranceNew = 0.0;
			}

			if(MetaProgError < ToleranceNew)
			{
				Num = TournamentCount/RefreshInterval;
				if (Num <1.0)
					NumOfMeta=1;
				else
					NumOfMeta = Num;

				NumOfClassi = TournamentCount-NumOfMeta;
			}
		}
		break;
	case 8:
		{
			NumOfClassi =0;
		}
		break;

	case 6:
		{
			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size(); i++)
					if( ((AssumedFitness[i]>=0.0)&&(CorrectFitness[i]<0.0)) || ((AssumedFitness[i]<0.0)&&(CorrectFitness[i]>=0.0)) )
						MetaProgError++;

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := percentage error
			}

			if(MetaProgError< Tolerance)
				NumOfClassi =-1;
		}
		break;
	}

	return NumOfClassi;
}

MT_TranslatedIndividual * MT_Classifier::createDoubleTransIndi(SIGEL_Program::SIG_Program const * SigProgOne, SIGEL_Program::SIG_Program const * SigProgTwo)
{
	int SigProgOneSize = SigProgOne->getProgramLength();
	int SigProgTwoSize = SigProgTwo->getProgramLength();
	int TransIndiSize = SigProgOneSize + SigProgTwoSize;

	QList<int> * Instructions = new QList<int>;
	QList<int> * OperandOne = new QList<int>;
	QList<int> * OperandTwo = new QList<int>;
	QList<int> * MData = new QList<int>;

	(*Instructions).resize(TransIndiSize);
	(*OperandOne).resize(TransIndiSize);
	(*OperandTwo).resize(TransIndiSize);
	(*MData).resize(32);

	(*MData)[0] = SigProgOneSize;
	for(int k=1; k<32; k++)
		(*MData)[k] =0;
	(*MData)[16] = SigProgTwoSize;

	// First SIGEL program is translated
	for (int i=0; i<SigProgOneSize;i++)
	{
		// WARNING: if the SIGEL instruction from SIGProg is JMP X, NOP, Sense ...
		// a 0 is substituted for the missing operand(s). Any alternative?

		SIGEL_Program::SIG_ProgramLine const &programLine = SigProgOne->getLine(i);
		(*OperandOne)[i]= programLine.getElement(0);
		(*OperandTwo)[i]= programLine.getElement(1);

		switch( programLine.getRobotInstructionType() )
		{
		case SIGEL_Program::COPY: (*Instructions)[i]= 1; break;
		case SIGEL_Program::LOAD: (*Instructions)[i]= 2; break;
		case SIGEL_Program::ADD: (*Instructions)[i]= 3;	break;
		case SIGEL_Program::SUB: (*Instructions)[i]= 4; break;
		case SIGEL_Program::MUL: (*Instructions)[i]= 5;	break;
		case SIGEL_Program::DIV: (*Instructions)[i]= 6;  break;
		case SIGEL_Program::MIN: (*Instructions)[i]= 7;	break;
		case SIGEL_Program::MAX: (*Instructions)[i]= 8;	break;
		case SIGEL_Program::CMP: (*Instructions)[i]= 9;	break;
		case SIGEL_Program::JMP: (*Instructions)[i]= 10; break;
		case SIGEL_Program::SENSE: (*Instructions)[i]= 11; break;
		case SIGEL_Program::MOVE:(*Instructions)[i]= 12; break;
		case SIGEL_Program::DELAY:	(*Instructions)[i]= 13;	break;
		case SIGEL_Program::MOD: (*Instructions)[i]= 14; break;
		case SIGEL_Program::NOP: (*Instructions)[i]= 15; break;
		}

		switch( programLine.getRobotInstructionType() )
		{
		case SIGEL_Program::COPY: (*MData)[1] ++ ; break;
		case SIGEL_Program::LOAD: (*MData)[2] ++; break;
		case SIGEL_Program::ADD: (*MData)[3] ++; break;
		case SIGEL_Program::SUB: (*MData)[4] ++; break;
		case SIGEL_Program::MUL: (*MData)[5] ++;	break;
		case SIGEL_Program::DIV: (*MData)[6] ++;  break;
		case SIGEL_Program::MIN: (*MData)[7] ++;	break;
		case SIGEL_Program::MAX: (*MData)[8] ++;	break;
		case SIGEL_Program::CMP: (*MData)[9] ++;	break;
		case SIGEL_Program::JMP: (*MData)[10] ++; break;
		case SIGEL_Program::SENSE: (*MData)[11] ++; break;
		case SIGEL_Program::MOVE: (*MData)[12] ++; break;
		case SIGEL_Program::DELAY: (*MData)[13] ++;	break;
		case SIGEL_Program::MOD: (*MData)[14] ++; break;
		case SIGEL_Program::NOP: (*MData)[15] ++; break;
		}
	}

	// Second SIGEL program is translated
	for (int i=0; i<SigProgTwoSize;i++)
	{
		// WARNING: if the SIGEL instruction from SIGProg is JMP X, NOP, Sense ...
		// a 0 is substituted for the missing operand(s). Any alternative?

		SIGEL_Program::SIG_ProgramLine const &programLine = SigProgTwo->getLine(i);
		(*OperandOne)[i+SigProgOneSize]= programLine.getElement(0);
		(*OperandTwo)[i+SigProgOneSize]= programLine.getElement(1);

		switch( programLine.getRobotInstructionType() )
		{
		case SIGEL_Program::COPY: (*Instructions)[i+SigProgOneSize]= 1; break;
		case SIGEL_Program::LOAD: (*Instructions)[i+SigProgOneSize]= 2; break;
		case SIGEL_Program::ADD: (*Instructions)[i+SigProgOneSize]= 3;	break;
		case SIGEL_Program::SUB: (*Instructions)[i+SigProgOneSize]= 4; break;
		case SIGEL_Program::MUL: (*Instructions)[i+SigProgOneSize]= 5;	break;
		case SIGEL_Program::DIV: (*Instructions)[i+SigProgOneSize]= 6;  break;
		case SIGEL_Program::MIN: (*Instructions)[i+SigProgOneSize]= 7;	break;
		case SIGEL_Program::MAX: (*Instructions)[i+SigProgOneSize]= 8;	break;
		case SIGEL_Program::CMP: (*Instructions)[i+SigProgOneSize]= 9;	break;
		case SIGEL_Program::JMP: (*Instructions)[i+SigProgOneSize]= 10; break;
		case SIGEL_Program::SENSE: (*Instructions)[i+SigProgOneSize]= 11; break;
		case SIGEL_Program::MOVE:(*Instructions)[i+SigProgOneSize]= 12; break;
		case SIGEL_Program::DELAY:	(*Instructions)[i+SigProgOneSize]= 13;	break;
		case SIGEL_Program::MOD: (*Instructions)[i+SigProgOneSize]= 14; break;
		case SIGEL_Program::NOP: (*Instructions)[i+SigProgOneSize]= 15; break;
		}

		switch( programLine.getRobotInstructionType() )
		{
		case SIGEL_Program::COPY: (*MData)[17] ++ ; break;
		case SIGEL_Program::LOAD: (*MData)[18] ++; break;
		case SIGEL_Program::ADD: (*MData)[19] ++; 	break;
		case SIGEL_Program::SUB: (*MData)[20] ++; break;
		case SIGEL_Program::MUL: (*MData)[21]++;	break;
		case SIGEL_Program::DIV: (*MData)[22] ++;  break;
		case SIGEL_Program::MIN: (*MData)[23] ++;	break;
		case SIGEL_Program::MAX: (*MData)[24] ++;	break;
		case SIGEL_Program::CMP: (*MData)[25] ++;	break;
		case SIGEL_Program::JMP: (*MData)[26] ++; break;
		case SIGEL_Program::SENSE: (*MData)[27] ++; break;
		case SIGEL_Program::MOVE: (*MData)[28] ++; break;
		case SIGEL_Program::DELAY: (*MData)[29] ++;	break;
		case SIGEL_Program::MOD: (*MData)[30] ++; break;
		case SIGEL_Program::NOP: (*MData)[31] ++; break;
		}
	}

	MT_TranslatedIndividual * NewTransIndi = new MT_TranslatedIndividual(Instructions,OperandOne,OperandTwo, MData);

	NewTransIndi->Boundary = SigProgOneSize;

	return NewTransIndi;
}

bool MT_Classifier::preEvolution(QList<SIGEL_GP::SIG_GPTournament *> *tournaments, int PosBest)
{
	SIGEL_GP::SIG_GPTournament *Tournament;

	bool Change = false;
	int TournamentCount = tournaments->size();

	pthread_mutex_lock(&fitnessMutex);

	int NumOfClassi = evaluationTactic(TournamentCount);

	pthread_mutex_unlock(&fitnessMutex);

	if (NumOfClassi > 0)  // NumOfClassi tournament will run by the Classifier
	{
		pthread_mutex_lock(&interpreterMutex);
		for(int i=0; i<NumOfClassi; i++)
		{
			Tournament = tournaments->value(i);
			if (i < tournaments->size()) (*tournaments)[i] = nullptr;
			Tournament->classify(this);
			delete Tournament;
			NumOfMetaEstimation[GenerationNumber] += 1;
		}

		// unlock the interpreter so that the program can be updated
		pthread_mutex_unlock(&interpreterMutex);

		for( int i=0; i<TournamentCount-NumOfClassi; i++)
		{
			// Move the tournament in slot i+NumOfClassi down to slot i.
			// Slot i is always empty here, so the delete frees nothing.
			Tournament = tournaments->value(i+NumOfClassi);
			if (i+NumOfClassi < tournaments->size()) (*tournaments)[i+NumOfClassi] = nullptr;
			if (i < tournaments->size()) { delete (*tournaments)[i]; (*tournaments)[i] = Tournament; }
		}

		// Every slot in the tail is null here, so this delete loop frees nothing.
		for (qsizetype k = TournamentCount-NumOfClassi; k < tournaments->size(); k++)
			delete tournaments->at(k);
		tournaments->resize(TournamentCount-NumOfClassi);
		Change = true;
	}

	// only  tournament, which haven't the best Sig Individual inside
	// will run by the Classifier

	if (NumOfClassi == -1)
	{
		QList<int> TournamentsWithBestIndividual;
		TournamentsWithBestIndividual.resize(tournaments->size());
		for(int i=0; i<TournamentsWithBestIndividual.size(); i++)
			TournamentsWithBestIndividual[i] =0;

		int NewNumOfClassi = evalNeededTournaments(tournaments, & TournamentsWithBestIndividual, PosBest);
		if (NewNumOfClassi <=0)			// No tournament is to be classified
			return Change;

		pthread_mutex_lock(&interpreterMutex);
		for(int i=0; i<TournamentCount; i++)
		{
			if(TournamentsWithBestIndividual[i] == 0)
			{
				Tournament = tournaments->value(i);
				if (i < tournaments->size()) (*tournaments)[i] = nullptr;
				Tournament->classify(this);
				delete Tournament;
				NumOfMetaEstimation[GenerationNumber] += 1;
			}
		}

		// unlock the interpreter so that the program can be updated
		pthread_mutex_unlock(&interpreterMutex);

		int NumOfTournaments = TournamentCount - NewNumOfClassi; // Number of tournaments to be run normally
		for( int i=0; i<NumOfTournaments; i++)
		{
			Tournament = tournaments->value(i);
			if(Tournament == nullptr)
			{
				for(int NextTournamentPosition = i+1; NextTournamentPosition<TournamentCount; NextTournamentPosition++)
				{
					Tournament = tournaments->value(NextTournamentPosition);
					if (NextTournamentPosition < tournaments->size()) (*tournaments)[NextTournamentPosition] = nullptr;
					if(Tournament != nullptr)
					{
						if (i < tournaments->size()) { delete (*tournaments)[i]; (*tournaments)[i] = Tournament; }
						break;
					}
				}
			}
		}

		// Every slot in the tail is null here, so this delete loop frees nothing.
		for (qsizetype k = NumOfTournaments; k < tournaments->size(); k++)
			delete tournaments->at(k);
		tournaments->resize(NumOfTournaments);

		//**************DebugInfo *************************************
		for(int d=0; d < tournaments->size();d++)
		{
			Tournament = tournaments->value(d);
			if(Tournament == nullptr)
			{
				int DeugInfo= tournaments->size();
			}
		}

		//*************************************************************
		Change = true;
	}

	NumOfCorrectEstimation[GenerationNumber]= tournaments->size();
	return Change;
}

int MT_Classifier::evalNeededTournaments(QList<SIGEL_GP::SIG_GPTournament *> *  tournaments, QList<int> * TournamentsWithBestIndividual, int PosBest)
{
	SIGEL_GP::SIG_GPTournament *Tournament =nullptr;
	int NumClassi=0;
	int TournamentCount = tournaments->size();

	for(int i=0; i<TournamentCount; i++)
	{
		Tournament = tournaments->value(i);
		for (int k=0; k <Tournament->indis.size();k++)
		{
			if(PosBest == Tournament->indis.value(k)->indNumber){
				(*TournamentsWithBestIndividual)[i] = 1;
				break;
			}
		}

		if(TournamentsWithBestIndividual->at(i) == 0)
			NumClassi ++;
	}

	// If the best SIGEL individual rarely takes part in tournaments,
	// further tournaments are used for calibration

	if(RefreshInterval == 0)
		RefreshInterval = 10;

	const double tournamentsPerRefresh = static_cast<double>(TournamentCount) / RefreshInterval;
	int MinNumTournaments = static_cast<int>(tournamentsPerRefresh);
	if (MinNumTournaments <= 0)
		MinNumTournaments =1;

	int NumKTournaments = TournamentCount-NumClassi;

	if (NumKTournaments<MinNumTournaments)
	{
		for(int k=0; k< (MinNumTournaments-NumKTournaments);k++)
		{
			for(int l=k; l< TournamentCount;l++)
			{
				if(TournamentsWithBestIndividual->at(l)==0)
				{
					(*TournamentsWithBestIndividual)[l] = 1;
					NumClassi--;
					break;
				}
			}
		}
	}

	return NumClassi;
}