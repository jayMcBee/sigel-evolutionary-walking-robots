#include <QQueue>
#include "MT_Control/MT_Substitute.h"

MT_Substitute::MT_Substitute()
	: Interpreter( startNumberOfVariables, startDuration )
{
 // overloaded method
	pthread_mutex_init(&interpreterMutex, nullptr);
	pthread_mutex_init(&tCaseBufferMutex, nullptr);
	pthread_mutex_init(&fitnessMutex, nullptr);
}

MT_Substitute::~MT_Substitute()
{
	pthread_mutex_unlock(&interpreterMutex);
	pthread_mutex_unlock(&tCaseBufferMutex);
	pthread_mutex_unlock(&fitnessMutex);
}

void MT_Substitute::changeBest(MT_Program * MetaProg)
{
	BestMETAProgram = std::make_unique< MT_Program >(MetaProg);

	Interpreter.loadProgram(BestMETAProgram.get());
}

void MT_Substitute::changeErrorInfo(QList<double> * OutCome, QList<double> * CorrectFit)
{
	if (CorrectFitness.size() < CorrectFit->size())
	{
		CorrectFitness.resize(CorrectFit->size());
		AssumedFitness.resize(OutCome->size());
	}

	// CorrectFitness never shrinks, but the training set shrinks when the user
	// lowers the selection size. So the loop stops at the shortest of the three lists.
	const int n = qMin(CorrectFitness.size(),
	                   qMin(CorrectFit->size(), OutCome->size()));
	for (int i=0; i<n; i++)
	{
		CorrectFitness[i]=(*CorrectFit)[i];
		AssumedFitness[i]=(*OutCome)[i];
	}

	MetaProgError = -1.0;
}



MT_TranslatedIndividual * MT_Substitute::translatedSIGProg(SIGEL_Program::SIG_Program const *SIGProg)
{
	int ProgSize = SIGProg->getProgramLength();

	QList<int> * Instruktion = new QList<int>;
	QList<int> * OperandOne = new QList<int>;
	QList<int> * OperandTwo = new QList<int>;
	QList<int> * MData = new QList<int>;

	(*Instruktion).resize(ProgSize);
	(*OperandOne).resize(ProgSize);
	(*OperandTwo).resize(ProgSize);
	(*MData).resize(16);

	(*MData)[0] =ProgSize;
	for (int k=1; k<16; k++)
		(*MData)[k] =0;
	
	SIGEL_Program::SIG_ProgramLine const * SIG_ProLine;
		
	for (int i=0; i<ProgSize;i++)
	{
		// WARNING: if the SIGEL instruction from SIGProg is JMP X, NOP, Sense ...
		// a 0 is substituted for the missing operand(s). Any alternative?

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
	pthread_mutex_lock(&interpreterMutex);

	Interpreter.setVariableNumber(NumOfVariable);
	Interpreter.setDuration(TimeToInter);

	// unlock the interpreter so that interpretation of programs can continue
	pthread_mutex_unlock(&interpreterMutex);
}

QQueue<MT_TrainingCase *> * MT_Substitute::changeTCases()
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


void MT_Substitute::getNumOfEstimation(QList<unsigned int> *MetaEstimation, QList<unsigned int> *CorrectEstimation)
{
	MetaEstimation = &NumOfMetaEstimation;
	CorrectEstimation =  &NumOfCorrectEstimation;
}


void MT_Substitute::loadSetup(QTextStream &File)
{
	// overloaded method

}

void MT_Substitute::writeToFile(QTextStream &File)
{
// overloaded method
}

void MT_Substitute::writeToFileSetup(QTextStream &File)
{	
	// overloaded method

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


