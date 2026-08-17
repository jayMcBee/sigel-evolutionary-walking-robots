// MT_Classifier.cpp: Implementierung der Klasse MT_Classifier.
//
//////////////////////////////////////////////////////////////////////

#include "MT_Control/MT_Classifier.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_Classifier::MT_Classifier(QTextStream &File) : MT_Substitute()
{
	Typ=2;
	AverageSigelFitness = 0.01;
	Interpreter= new MT_Interpreter(10,100); // ACHTUNG Parameter müssen noch geändert werden! setInterpreter(int NumOfVariable,  int TimeToInter)
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
	File << EstimationStrategy <<endl;
	File << Tolerance <<endl;
	File << RefreshInterval <<endl;
	File << GenerationNumber <<endl;

	for(int i=0; i< GenerationNumber; i++)
		{
			File << NumOfCorrectEstimation[i] <<endl;
			File << NumOfMetaEstimation[i] << endl;
		}

}

void MT_Classifier::writeToFileSetup(QTextStream &File)
{
	File << ("Evaluator:\n");
	File << EstimationStrategy <<endl;
	File << Tolerance <<endl;
	File << RefreshInterval <<endl;
	int GenNum =0;
	File << GenNum <<endl;

}

//////////////////////////////////////////////////////////////////////
// functionally method 
//////////////////////////////////////////////////////////////////////

double MT_Classifier::classifer(SIGEL_Program::SIG_Program * SigProgOne, SIGEL_Program::SIG_Program * SigProgTwo)
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
	int NumOfClassi=0; //die Anzahl der Turnier die mit dem Classi ausgeführt werden sollen 
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
			// nach Anzahl der SigelGenerationen gewichtete Bewertungsstrategie
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
			
				// nach Anzahl der Generationen wird die Tolerance gewichtet!
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
			// nach Sigel Fitness gewichtete Bewertungsstrategie!

			double ToleranceNew = Tolerance;

			if(MetaProgError == -1.0)
			{
				MetaProgError=0.0;
				for (int i=0; i<CorrectFitness.size(); i++)
					if( ((AssumedFitness[i]>=0.0)&&(CorrectFitness[i]<0.0)) || ((AssumedFitness[i]<0.0)&&(CorrectFitness[i]>=0.0)) )
						MetaProgError++;

				MetaProgError = (MetaProgError/ CorrectFitness.size())*100.0;  //MetaProgError := prozentualer Fehler 
			}

		// Gewichtung  der Tolerance AverageSigelFitness !! 
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

	QArray<int> * Instruktion = new QArray<int>;
	QArray<int> * OperandOne = new QArray<int>;
	QArray<int> * OperandTwo = new QArray<int>;
	QArray<int> * MData = new QArray<int>;

	(*Instruktion).resize(TransIndiSize);
	(*OperandOne).resize(TransIndiSize);
	(*OperandTwo).resize(TransIndiSize);
	(*MData).resize(32);

	(*MData)[0] = SigProgOneSize;
	for(int k=1; k<32; k++)
		(*MData)[k] =0;
	(*MData)[16] = SigProgTwoSize;
	
	SIGEL_Program::SIG_ProgramLine * SIG_ProLine;

// erste Sigel Programm wird übersetzt 
	for (int i=0; i<SigProgOneSize;i++)
	{

		// ACHTUNG: falls SIGEL Befehl von SIGProg = JMP X, NOP, Sense ...
		// wird für den oder die nicht vorhanden Operanten eine 0 gesetzt - Alternativ?

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

// zweites Sigel Programm wird übersetzt 
	for (i=0; i<SigProgTwoSize;i++)
	{
		// ACHTUNG: falls SIGEL Befehl von SIGProg = JMP X, NOP, Sense ...
		// wird für den oder die nicht vorhanden Operanten eine 0 gesetzt - Alternativ?

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

bool MT_Classifier::preEvolution(QVector<SIGEL_GP::SIG_GPTournament> *tours, int PosBest)
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
			Tourna = tours->take(i);
			Tourna->classify(this);
			delete Tourna;
			NumOfMetaEstimation[GenerationNumber] += 1;

		}

// unlock the interpreter so that the rogram can be updated
#ifdef _WINDOWS
		ReleaseMutex(interpreterMutex);
#else
		pthread_mutex_unlock(&interpreterMutex);
#endif
	
		for( i=0; i<TourSize-NumOfClassi; i++)
		{
			Tourna = tours->take(i+NumOfClassi); 
			tours->insert(i,Tourna);
		}

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
		QArray<int> ToursWBestIndi;
		ToursWBestIndi.resize(tours->size());
		for(int i=0; i<ToursWBestIndi.size(); i++)
			ToursWBestIndi[i] =0;

		int NewNumOfClassi = evalNeededTours(tours, & ToursWBestIndi, PosBest);
		if (NewNumOfClassi <=0)			// kein Turnier soll klassifiert werden;
			return Change;  

#ifdef _WINDOWS
		WaitForSingleObject(interpreterMutex, INFINITE);
#else
		pthread_mutex_lock(&interpreterMutex);
#endif
		for(i=0; i<TourSize; i++)
		{	
			if(ToursWBestIndi[i] == 0)
			{
				Tourna = tours->take(i);
				Tourna->classify(this);
				delete Tourna;
				NumOfMetaEstimation[GenerationNumber] += 1;
			}

		}

// unlock the interpreter so that the rogram can be updated
#ifdef _WINDOWS
		ReleaseMutex(interpreterMutex);
#else
		pthread_mutex_unlock(&interpreterMutex);
#endif

// für Marco:  :-)	
// falls Turnier per Klassi ausgeführt wurden,
// soll hier nun tours verkleinert werden, und die noch nicht gelaufenden
// Turnier soll noch übernommen werden, also alle am Anfang von tours stehene
// damit man dann einfach 	tours->resize(TourSize-NumOfClassi); durchführen kann 

		int NumOfTour = TourSize - NewNumOfClassi; // Anzahl der Turnier, die normal ausgeführt werden soll
		for( i=0; i<NumOfTour; i++)
		{
			Tourna = tours->at(i);
			if(Tourna == NULL)
			{
				for(int NextTourPos = i+1; NextTourPos<TourSize; NextTourPos++)
				{
					Tourna = tours->take(NextTourPos); 
					if(Tourna != NULL)
					{
						tours->insert(i,Tourna);
						break;
					}

				}
			}
		
		}

		tours->resize(NumOfTour);

//**************DebugInfo *************************************
for(int d=0; d < tours->size();d++)
{
	Tourna = tours->at(d); 
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

int MT_Classifier::evalNeededTours(QVector<SIGEL_GP::SIG_GPTournament> *  tours, QArray<int> * ToursWBestIndi, int PosBest)
{
	SIGEL_GP::SIG_GPTournament *Tourna =NULL;
	int NumClassi=0;
	int ToursSize = tours->size();

	for(int i=0; i<ToursSize; i++)
	{
		Tourna = tours->at(i);
		for (int k=0; k <Tourna->indis.size();k++)
		{
			if(PosBest == Tourna->indis.at(k)->indNumber){
				ToursWBestIndi->at(i) = 1;
				break;
			}
		}

		if(ToursWBestIndi->at(i) == 0)
			NumClassi ++;

	}


	// falls das beste Sigel Individuum selten an Turnieren teilnimmt 
	// werden weiter Turnier zu Kalibrierung herangezogen

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
					ToursWBestIndi->at(l)= 1;
					NumClassi--;
					break;

				}
			}
		}
	
	
	}

	return NumClassi;
}