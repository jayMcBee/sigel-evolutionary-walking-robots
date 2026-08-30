// MT_Classifier.cpp: implementation of class MT_Classifier.
//
//////////////////////////////////////////////////////////////////////

#include <QList>
#include <QString>
#include <QTextStream>
#include "MT_Control/MT_Classifier.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Classifier::MT_Classifier(QTextStream &File) : MT_Substitute()
{
	Typ=2;
	AverageSigelFitness = 0.01;
	Interpreter= new MT_Interpreter(10,100); // WARNING: parameters still need changing! setInterpreter(int NumOfVariable, int TimeToInter)
	BestMETAProgram =0;
	CorrectFitness.resize(0);
	AssumedFitness.resize(0);
	MetaProgError =-1.0;
	NumOfMeta =0;
	
	QString Classifier( "Classifier:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Classifier) && !(File.atEnd()))
		PresentLine = File.readLine();
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


MT_Classifier::~MT_Classifier()
{
	
	delete Interpreter;
	if (BestMETAProgram !=0)
		delete BestMETAProgram;

}

void MT_Classifier::loadSetup(QTextStream &File)
{
	
	QString Classifier( "Classifier:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Classifier) && !(File.atEnd()))
		PresentLine = File.readLine();
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

//////////////////////////////////////////////////////////////////////
// functionally method 
//////////////////////////////////////////////////////////////////////

double MT_Classifier::classifier(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo)
{

		double MetaEstimationResult = Interpreter->interpret(createDoubleTransIndi(SigProgOne, SigProgTwo));

		return MetaEstimationResult;

}

void MT_Classifier::createNewTCase(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo, double FitDifference)
{

	MT_TranslatedIndividual * TransIndi = createDoubleTransIndi(SigProgOne, SigProgTwo);

// lock the TCaseBuffer so that no TCases could be taken from it
#ifdef _WINDOWS
					WaitForSingleObject(tCaseBufferMutex, INFINITE);
#else
					pthread_mutex_lock(&tCaseBufferMutex);
#endif
	
					TCaseBuffer.enqueue (new MT_TrainingCase(FitDifference, TransIndi, NumOfCorrectEstimation[GenerationNumber], TransIndi->Boundary));

					// create the symmetric TCase
					TransIndi = createDoubleTransIndi(SigProgTwo, SigProgOne);
					TCaseBuffer.enqueue (new MT_TrainingCase(-1.0 * FitDifference, TransIndi, NumOfCorrectEstimation[GenerationNumber], TransIndi->Boundary));


// unlock the TCaseBuffer
#ifdef _WINDOWS
					ReleaseMutex(tCaseBufferMutex);
#else
					pthread_mutex_unlock(&tCaseBufferMutex);
#endif

}


int MT_Classifier::evaluationTactic(int ToursSize)
{
	int NumOfClassi=0; //Number of tournaments to be run with the classifier 
	double Num =0.0;
	double SigelGeneration = GenerationNumber;
	if (SigelGeneration <1.0)
		SigelGeneration =1.0;

		
	if (BestMETAProgram ==0)
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

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := prozentualer Fehler 
			}
			
			if(MetaProgError< Tolerance)
			{
				Num = ToursSize/RefreshInterval;
				if (Num <1.0)
					NumOfMeta=1;
				else
					NumOfMeta = Num;

				NumOfClassi = ToursSize-NumOfMeta;
			}
	
		}break;
	
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

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := prozentualer Fehler 
			}

			if (SigelGeneration >5.0)
			{
			
				// Tolerance is weighted by generation count
				if (SigelGeneration <= 150.0)
				{
					double Rate = (40.0 - Tolerance) / 145.0;
					if (Rate >0.0)
					{
						ToleranceNew = 40.0 - Rate*SigelGeneration;
					}
				}
			}
		
			if(MetaProgError < ToleranceNew)
			{
				Num = ToursSize/RefreshInterval;
				if (Num <1.0)
					NumOfMeta=1;
				else
					NumOfMeta = Num;

				NumOfClassi = ToursSize-NumOfMeta;
			}

		}break;
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

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := prozentualer Fehler 
			}

		// Tolerance weighting: AverageSigelFitness 
			if (AverageSigelFitness>0.1)
			{
				if(AverageSigelFitness<1.0)
				{
					double ASigelFit = AverageSigelFitness - 0.1;
					double Rate = (40.0-Tolerance)/0.9;
					if (Rate >0.0)
						ToleranceNew = 40.0 - Rate*ASigelFit;
					if (ToleranceNew<0.0)
						ToleranceNew = 0.0;
				}
			}


			if(MetaProgError < ToleranceNew)
			{
				Num = ToursSize/RefreshInterval;
				if (Num <1.0)
					NumOfMeta=1;
				else
					NumOfMeta = Num;

				NumOfClassi = ToursSize-NumOfMeta;
			}

		}break;
	case 8: 
		{
			NumOfClassi =0;

		}break;

// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 

	case 6: 
		{
			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size(); i++)
					if( ((AssumedFitness[i]>=0.0)&&(CorrectFitness[i]<0.0)) || ((AssumedFitness[i]<0.0)&&(CorrectFitness[i]>=0.0)) )
						MetaProgError++;

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := prozentualer Fehler 
			}
			
			if(MetaProgError< Tolerance)
				NumOfClassi =-1;
			
		}break;
	}
	
return NumOfClassi; 
}


MT_TranslatedIndividual * MT_Classifier::createDoubleTransIndi(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo)
{

	int SigProgOneSize = SigProgOne->getProgramLength();
	int SigProgTwoSize = SigProgTwo->getProgramLength();
	int TransIndiSize = SigProgOneSize + SigProgTwoSize;

	QList<int> * Instruktion = new QList<int>;
	QList<int> * OperandOne = new QList<int>;
	QList<int> * OperandTwo = new QList<int>;
	QList<int> * MData = new QList<int>;

	(*Instruktion).resize(TransIndiSize);
	(*OperandOne).resize(TransIndiSize);
	(*OperandTwo).resize(TransIndiSize);
	(*MData).resize(32);

	(*MData)[0] = SigProgOneSize;
	for(int k=1; k<32; k++)
		(*MData)[k] =0;
	(*MData)[16] = SigProgTwoSize;
	
	SIGEL_Program::SIG_ProgramLine * SIG_ProLine;

// First SIGEL program is translated 
	for (int i=0; i<SigProgOneSize;i++)
	{

		// WARNING: if the SIGEL instruction from SIGProg is JMP X, NOP, Sense ...
		// a 0 is substituted for the missing operand(s). Any alternative?

		SIG_ProLine= SigProgOne->getLine(i);
		(*OperandOne)[i]= SIG_ProLine->getElement(0);
		(*OperandTwo)[i]= SIG_ProLine->getElement(1);


		switch( SIG_ProLine->getRobotinstructionType() )
		{
		case SIGEL_Program::COPY: (*Instruktion)[i]= 1; break;
		case SIGEL_Program::LOAD: (*Instruktion)[i]= 2; break;
		case SIGEL_Program::ADD: (*Instruktion)[i]= 3;	break;
		case SIGEL_Program::SUB: (*Instruktion)[i]= 4; break;
		case SIGEL_Program::MUL: (*Instruktion)[i]= 5;	break;
		case SIGEL_Program::DIV: (*Instruktion)[i]= 6;  break;
		case SIGEL_Program::MIN: (*Instruktion)[i]= 7;	break;
		case SIGEL_Program::MAX: (*Instruktion)[i]= 8;	break;
		case SIGEL_Program::CMP: (*Instruktion)[i]= 9;	break;
		case SIGEL_Program::JMP: (*Instruktion)[i]= 10; break;
		case SIGEL_Program::SENSE: (*Instruktion)[i]= 11; break;
		case SIGEL_Program::MOVE:(*Instruktion)[i]= 12; break;
		case SIGEL_Program::DELAY:	(*Instruktion)[i]= 13;	break;
		case SIGEL_Program::MOD: (*Instruktion)[i]= 14; break;
		case SIGEL_Program::NOP: (*Instruktion)[i]= 15; break;
		}

		switch( SIG_ProLine->getRobotinstructionType() )
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

		SIG_ProLine= SigProgTwo->getLine(i);
		(*OperandOne)[i+SigProgOneSize]= SIG_ProLine->getElement(0);
		(*OperandTwo)[i+SigProgOneSize]= SIG_ProLine->getElement(1);


		switch( SIG_ProLine->getRobotinstructionType() )
		{
		case SIGEL_Program::COPY: (*Instruktion)[i+SigProgOneSize]= 1; break;
		case SIGEL_Program::LOAD: (*Instruktion)[i+SigProgOneSize]= 2; break;
		case SIGEL_Program::ADD: (*Instruktion)[i+SigProgOneSize]= 3;	break;
		case SIGEL_Program::SUB: (*Instruktion)[i+SigProgOneSize]= 4; break;
		case SIGEL_Program::MUL: (*Instruktion)[i+SigProgOneSize]= 5;	break;
		case SIGEL_Program::DIV: (*Instruktion)[i+SigProgOneSize]= 6;  break;
		case SIGEL_Program::MIN: (*Instruktion)[i+SigProgOneSize]= 7;	break;
		case SIGEL_Program::MAX: (*Instruktion)[i+SigProgOneSize]= 8;	break;
		case SIGEL_Program::CMP: (*Instruktion)[i+SigProgOneSize]= 9;	break;
		case SIGEL_Program::JMP: (*Instruktion)[i+SigProgOneSize]= 10; break;
		case SIGEL_Program::SENSE: (*Instruktion)[i+SigProgOneSize]= 11; break;
		case SIGEL_Program::MOVE:(*Instruktion)[i+SigProgOneSize]= 12; break;
		case SIGEL_Program::DELAY:	(*Instruktion)[i+SigProgOneSize]= 13;	break;
		case SIGEL_Program::MOD: (*Instruktion)[i+SigProgOneSize]= 14; break;
		case SIGEL_Program::NOP: (*Instruktion)[i+SigProgOneSize]= 15; break;
		}
	
		switch( SIG_ProLine->getRobotinstructionType() )
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


	MT_TranslatedIndividual * NewTransIndi = new MT_TranslatedIndividual(Instruktion,OperandOne,OperandTwo, MData);
	
	NewTransIndi->Boundary = SigProgOneSize;

	return NewTransIndi;

}

bool MT_Classifier::preEvolution(QList<SIGEL_GP::SIG_GPTournament *> *tours, int PosBest)
{
	SIGEL_GP::SIG_GPTournament *Tourna;

	bool Change = false;
	int TourSize = tours->size();


#ifdef _WINDOWS
	WaitForSingleObject(fitnessMutex, INFINITE);
#else
	pthread_mutex_lock(&fitnessMutex);
#endif


	int NumOfClassi = evaluationTactic(TourSize);

#ifdef _WINDOWS
	ReleaseMutex(fitnessMutex);
#else
	pthread_mutex_unlock(&fitnessMutex);
#endif


	if (NumOfClassi > 0)  // NumOfClassi tournament will run by the Classifier
	{

#ifdef _WINDOWS
		WaitForSingleObject(interpreterMutex, INFINITE);
#else
		pthread_mutex_lock(&interpreterMutex);
#endif
		for(int i=0; i<NumOfClassi; i++)
		{	
			// take(i): return the occupant, empty the slot, never delete.
			Tourna = tours->value(i);
			if (i < tours->size()) (*tours)[i] = 0;
			Tourna->classify(this);
			delete Tourna;
			NumOfMetaEstimation[GenerationNumber] += 1;

		}

// unlock the interpreter so that the program can be updated
#ifdef _WINDOWS
		ReleaseMutex(interpreterMutex);
#else
		pthread_mutex_unlock(&interpreterMutex);
#endif
	
		for( int i=0; i<TourSize-NumOfClassi; i++)
		{
			// take(i+NumOfClassi), then insert(i): slot i is already empty, so
			// Qt 2's insert deleted nothing here -- the delete is kept anyway.
			Tourna = tours->value(i+NumOfClassi);
			if (i+NumOfClassi < tours->size()) (*tours)[i+NumOfClassi] = 0;
			if (i < tours->size()) { delete (*tours)[i]; (*tours)[i] = Tourna; }
		}

		// resize(): Qt 2 deleted the truncated tail. Proven all-null here.
		for (qsizetype k = TourSize-NumOfClassi; k < tours->size(); k++)
			delete tours->at(k);
		tours->resize(TourSize-NumOfClassi);
		Change = true;
	}


// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 

// only  tournament, which haven't the best Sig Individual inside 
// will run by the Classifier

	if (NumOfClassi == -1)  
	{
		QList<int> ToursWBestIndi;
		ToursWBestIndi.resize(tours->size());
		for(int i=0; i<ToursWBestIndi.size(); i++)
			ToursWBestIndi[i] =0;

		int NewNumOfClassi = evalNeededTours(tours, & ToursWBestIndi, PosBest);
		if (NewNumOfClassi <=0)			// No tournament is to be classified
			return Change;  

#ifdef _WINDOWS
		WaitForSingleObject(interpreterMutex, INFINITE);
#else
		pthread_mutex_lock(&interpreterMutex);
#endif
		for(int i=0; i<TourSize; i++)
		{	
			if(ToursWBestIndi[i] == 0)
			{
				// take(i): occupant out, slot emptied, no delete.
				Tourna = tours->value(i);
				if (i < tours->size()) (*tours)[i] = 0;
				Tourna->classify(this);
				delete Tourna;
				NumOfMetaEstimation[GenerationNumber] += 1;
			}

		}

// unlock the interpreter so that the program can be updated
#ifdef _WINDOWS
		ReleaseMutex(interpreterMutex);
#else
		pthread_mutex_unlock(&interpreterMutex);
#endif

// for Marco :-)	
// If tournaments were run via the classifier,
// tours should now be shrunk and the tournaments not yet run
// should be carried over, i.e. all those at the start of tours,
// so that tours->resize(TourSize-NumOfClassi) can simply be called 

		int NumOfTour = TourSize - NewNumOfClassi; // Number of tournaments to be run normally
		for( int i=0; i<NumOfTour; i++)
		{
			Tourna = tours->value(i);
			if(Tourna == NULL)
			{
				for(int NextTourPos = i+1; NextTourPos<TourSize; NextTourPos++)
				{
					Tourna = tours->value(NextTourPos);
					if (NextTourPos < tours->size()) (*tours)[NextTourPos] = 0;
					if(Tourna != NULL)
					{
						// insert(i): slot i is null on this branch, so no delete fires.
						if (i < tours->size()) { delete (*tours)[i]; (*tours)[i] = Tourna; }
						break;
					}

				}
			}
		
		}

		// resize(): Qt 2 deleted the truncated tail. Proven all-null here.
		for (qsizetype k = NumOfTour; k < tours->size(); k++)
			delete tours->at(k);
		tours->resize(NumOfTour);

//**************DebugInfo *************************************
for(int d=0; d < tours->size();d++)
{
	Tourna = tours->value(d);
		if(Tourna == NULL)
		{
			int DeugInfo= tours->size();
		}

}
		
//*************************************************************
		Change = true;
	}


	NumOfCorrectEstimation[GenerationNumber]= tours->size();
	return Change;

		


}

// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 
// NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU // NEU NEU NEU NEU NEU 

int MT_Classifier::evalNeededTours(QList<SIGEL_GP::SIG_GPTournament *> *  tours, QList<int> * ToursWBestIndi, int PosBest)
{
	SIGEL_GP::SIG_GPTournament *Tourna =NULL;
	int NumClassi=0;
	int ToursSize = tours->size();

	for(int i=0; i<ToursSize; i++)
	{
		Tourna = tours->value(i);
		for (int k=0; k <Tourna->indis.size();k++)
		{
			if(PosBest == Tourna->indis.value(k)->indNumber){
				// at() was writable on Qt 2's const QArray; QList's is not.
				(*ToursWBestIndi)[i] = 1;
				break;
			}
		}

		if(ToursWBestIndi->at(i) == 0)
			NumClassi ++;

	}


	// If the best SIGEL individual rarely takes part in tournaments, 
	// further tournaments are used for calibration

	if(RefreshInterval == 0)
		RefreshInterval = 10;

	int MinNumTours = (int) (((double)ToursSize)/RefreshInterval);
	if (MinNumTours <= 0)
		MinNumTours =1;

	int NumKTours = ToursSize-NumClassi;

	if (NumKTours<MinNumTours)
	{
		for(int k=0; k< (MinNumTours-NumKTours);k++)
		{
			for(int l=k; l< ToursSize;l++)
			{
				if(ToursWBestIndi->at(l)==0)
				{
					(*ToursWBestIndi)[l] = 1;
					NumClassi--;
					break;

				}
			}
		}
	
	
	}

	return NumClassi;
}