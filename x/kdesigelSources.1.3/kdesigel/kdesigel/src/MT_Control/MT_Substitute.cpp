// MT_Substitute.cpp: Implementierung der Klasse MT_Substitute.
//
//////////////////////////////////////////////////////////////////////

#include "MT_Control/MT_Substitute.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_Substitute::MT_Substitute()
{
 // overloaded methode
#ifdef _WINDOWS
	interpreterMutex = CreateMutex(NULL, false, NULL);
	tCaseBufferMutex = CreateMutex(NULL, false, NULL);
	fitnessMutex	 = CreateMutex(NULL, false, NULL);
#else
	pthread_mutex_init(&interpreterMutex, NULL);
	pthread_mutex_init(&tCaseBufferMutex, NULL);
	pthread_mutex_init(&fitnessMutex, NULL);
#endif
}

MT_Substitute::~MT_Substitute()
{
#ifdef _WINDOWS
	ReleaseMutex(interpreterMutex);
	ReleaseMutex(tCaseBufferMutex);
	ReleaseMutex(fitnessMutex);
	CloseHandle(interpreterMutex);
	CloseHandle(tCaseBufferMutex);
	CloseHandle(fitnessMutex);
#else
	pthread_mutex_unlock(&interpreterMutex);
	pthread_mutex_unlock(&tCaseBufferMutex);
	pthread_mutex_unlock(&fitnessMutex);
#endif
}

void MT_Substitute::changeBest(MT_Program * MetaProg)
{
	if (BestMETAProgram !=0)
		delete BestMETAProgram;
	
	BestMETAProgram = new MT_Program(MetaProg);

	Interpreter->loadProgram(BestMETAProgram);
}

// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 

void MT_Substitute::changeErrorInfo(QArray<double> * OutCome, QArray<double> * CorrectFit)
{
	if (CorrectFitness.size() < CorrectFit->size())
	{
		CorrectFitness.resize(CorrectFit->size());
		AssumedFitness.resize(OutCome->size());
	}

	for (int i=0; i<CorrectFitness.size();i++)
	{
		CorrectFitness[i]=(*CorrectFit)[i];
		AssumedFitness[i]=(*OutCome)[i];
	}

	MetaProgError = -1.0;
}



MT_TranslatedIndividual * MT_Substitute::translatedSIGProg(SIGEL_Program::SIG_Program *SIGProg)
{
	int ProgSize = SIGProg->getProgramLength();

	QArray<int> * Instruktion = new QArray<int>;
	QArray<int> * OperandOne = new QArray<int>;
	QArray<int> * OperandTwo = new QArray<int>;
	QArray<int> * MData = new QArray<int>;

	(*Instruktion).resize(ProgSize);
	(*OperandOne).resize(ProgSize);
	(*OperandTwo).resize(ProgSize);
	(*MData).resize(16);

	(*MData)[0] =ProgSize;
	for (int k=1; k<16; k++)
		(*MData)[k] =0;
	
	SIGEL_Program::SIG_ProgramLine * SIG_ProLine;
		
	for (int i=0; i<ProgSize;i++)
	{
		// ACHTUNG: falls SIGEL Befehl von SIGProg = JMP X, NOP, Sense ...
		// wird für den oder die nicht vorhanden Operanten eine 0 gesetzt - Alternativ?

		SIG_ProLine= SIGProg->getLine(i);
		(*OperandOne)[i]= SIG_ProLine->getElement(0);
		(*OperandTwo)[i]= SIG_ProLine->getElement(1);


		switch( SIG_ProLine->getRobotinstructionType() )
		{
		case SIGEL_Program::COPY: (*Instruktion)[i]= 1; (*MData)[1]++; break;
		case SIGEL_Program::LOAD: (*Instruktion)[i]= 2; (*MData)[2]++; break;
		case SIGEL_Program::ADD: (*Instruktion)[i]= 3; (*MData)[3]++;	break;
		case SIGEL_Program::SUB: (*Instruktion)[i]= 4; (*MData)[4]++; break;
		case SIGEL_Program::MUL: (*Instruktion)[i]= 5; (*MData)[5]++;	break;
		case SIGEL_Program::DIV: (*Instruktion)[i]= 6; (*MData)[6]++;  break;
		case SIGEL_Program::MIN: (*Instruktion)[i]= 7; (*MData)[7]++;	break;
		case SIGEL_Program::MAX: (*Instruktion)[i]= 8; (*MData)[8]++;	break;
		case SIGEL_Program::CMP: (*Instruktion)[i]= 9; (*MData)[9]++;	break;
		case SIGEL_Program::JMP: (*Instruktion)[i]= 10; (*MData)[10]++; break;
		case SIGEL_Program::SENSE: (*Instruktion)[i]= 11; (*MData)[11]++; break;
		case SIGEL_Program::MOVE:(*Instruktion)[i]= 12; (*MData)[12]++; break;
		case SIGEL_Program::DELAY:	(*Instruktion)[i]= 13; (*MData)[13]++;	break;
		case SIGEL_Program::MOD: (*Instruktion)[i]= 14; (*MData)[14]++; break;
		case SIGEL_Program::NOP: (*Instruktion)[i]= 15; (*MData)[15]++; break;
		}

	}

	MT_TranslatedIndividual * NewTransIndi = new MT_TranslatedIndividual(Instruktion,OperandOne,OperandTwo, MData);

	return NewTransIndi;

}


void MT_Substitute::setInterpreter(int NumOfVariable, int TimeToInter)
{
	// lock the interpreter so we can safely change the interpreter settings
#ifdef _WINDOWS
	WaitForSingleObject(interpreterMutex, INFINITE);
#else
	pthread_mutex_lock(&interpreterMutex);
#endif

	Interpreter->setVariableNumber(NumOfVariable);
	Interpreter->setDuration(TimeToInter);

	// unlock the interpreter so that interpretation of programs can continue
#ifdef _WINDOWS
	ReleaseMutex(interpreterMutex);
#else
	pthread_mutex_unlock(&interpreterMutex);
#endif
}

QQueue<MT_TrainingCase> * MT_Substitute::changeTCases()
{	
	return &TCaseBuffer;
}

void MT_Substitute::setEstimationParameter(int EStrategy, double Tol, int ReInterval)
{
	EstimationStrategy =EStrategy;
	Tolerance =Tol;
	RefreshInterval=ReInterval;

}

void MT_Substitute::getEstimationParameter(int *EStrategy, double *Tol, int *ReInterval)
{
	*EStrategy =EstimationStrategy;
	*Tol =Tolerance;
	*ReInterval =RefreshInterval;
}


void MT_Substitute::getNumOfEstimation(QArray<unsigned int> *MetaEstimation, QArray<unsigned int> *CorrectEstimation)
{
	MetaEstimation = &NumOfMetaEstimation;
	CorrectEstimation =  &NumOfCorrectEstimation;
}


void MT_Substitute::loadSetup(QTextStream &File)
{
	// overloaded methode

}

void MT_Substitute::writeToFile(QTextStream &File)
{
// overloaded methode
}

void MT_Substitute::writeToFileSetup(QTextStream &File)
{	
	// overloaded methode

}

int MT_Substitute::getTyp()
{
	return Typ;
}

void MT_Substitute::nextSIGGeneration(double AverageSigelFit)
{
	GenerationNumber++;
	AverageSigelFitness = AverageSigelFit;
	if (GenerationNumber>=NumOfCorrectEstimation.size())
	{
		NumOfCorrectEstimation.resize(GenerationNumber +99);
		NumOfMetaEstimation.resize(GenerationNumber +99);
		for(int i=GenerationNumber-1; i<NumOfCorrectEstimation.size();i++)
		{
			NumOfCorrectEstimation[i]=0;
			NumOfMetaEstimation[i]=0;
		}
	}

}


