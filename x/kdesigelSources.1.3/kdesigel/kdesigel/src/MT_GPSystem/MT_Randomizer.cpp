// MT_Randomizer.cpp: Implementierung der Klasse MT_Randomizer.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Randomizer.h"
#include <iostream.h>

//////////////////////////////////////////////////////////////////////
// administrative methods
//////////////////////////////////////////////////////////////////////


MT_Randomizer::MT_Randomizer()
{
	
	srand( (unsigned)time( NULL ) );

// initialization
	LastError =0;

	RandomXPoints.resize(7);
	for (int i=0; i<7; i++)
		RandomXPoints[i]=0;
	
	ParentSize =10;
	OffspringSize =100;
	NumberOfVariables =10;
	ProgramLengthMax =50;
	TournamentPartition.resize(OffspringSize);

	ProbSearchOperator.resize(3);
	for (i=0; i<3; i++)
		ProbSearchOperator[i]=0;	
		

	ProbMutationPower.resize(2);
	for (i=0; i<2; i++)
		ProbMutationPower[i]=0;

	ProbCrossOverPoints.resize(3);
	for (i=0; i<3; i++)
		ProbCrossOverPoints[i]=0;

	ProbInstruktion.resize(18);
	for (i=0; i<18; i++)
		ProbInstruktion[i]=0;

	createConstant (50, true , 0, 1000);
	


}

MT_Randomizer::~MT_Randomizer()
{


}

MT_Randomizer::MT_Randomizer(QTextStream &File)
{
	// set seed for the random method
	srand( (unsigned)time( NULL ) );

	LastError =0;
	RandomXPoints.resize(7);
	for (int i=0; i<7; i++)
		RandomXPoints[i]=0;

	cerr << ">>> im Randomizer <<<" << endl;
	QString Randomizer( "Randomizer:" );
	QString PresentLine = File.readLine();
	cerr << PresentLine << endl;
	
	while ((PresentLine != Randomizer) && !(File.atEnd())){
		PresentLine = File.readLine();
		cerr << PresentLine << endl;
	}

	if ((PresentLine == Randomizer) && !(File.atEnd()))
	{
		ParentSize = (File.readLine()).toInt();
		OffspringSize = (File.readLine()).toInt();
		NumberOfVariables = (File.readLine()).toInt();
		ProgramLengthMax = (File.readLine()).toInt();
		TournamentPartition.resize(OffspringSize);
		
		ProbSearchOperator.resize(3);
		for (i=0; i<3; i++)
			ProbSearchOperator[i]=(File.readLine()).toDouble();
		
		ProbMutationPower.resize(2);
		for (i=0; i<2; i++)
			ProbMutationPower[i]=(File.readLine()).toDouble();
		
		ProbCrossOverPoints.resize(3);
		for (i=0; i<3; i++)
			ProbCrossOverPoints[i]=(File.readLine()).toDouble();
	
		ProbInstruktion.resize(18);
		for (i=0; i<17; i++)
			ProbInstruktion[i]=(File.readLine()).toDouble();

		PresentLine = File.readLine();
		if (PresentLine.toDouble() >= 1.0)
		{
			ProbInstruktion[17]=PresentLine.toDouble();
			PresentLine = File.readLine();
		}
		else
			ProbInstruktion[17] = ProbInstruktion[16];

		QString ConString( "Constant:" );
		
		while ((PresentLine != ConString) && !(File.atEnd())){
			PresentLine = File.readLine();
		}

		if ((PresentLine == ConString) && !(File.atEnd()))
		{
			PresentLine = File.readLine();

			if (PresentLine == "newConstant")
			{
				// es werden Konstanten neu erzeugt - für Standart Konstruktor 
				PresentLine = File.readLine();
				createConstant (PresentLine.toInt(), true , 0, 1000);
			}
			else
			{
				// sonst beschreibt die betrachtete Zeile die Anzahl der noch zu ladenden Konstanten			
				int NumOfConst = PresentLine.toInt();
				
				Constant.resize(NumOfConst);
				for (i=0; i<NumOfConst; i++)
				{
					PresentLine = File.readLine();	
					Constant[i]= PresentLine.toDouble();
				}
					

			}
				
		}
		else
			LastError =261;
		
	}
	else
		LastError =260;

	
}


void MT_Randomizer::loadSetup(QTextStream &File)
{
	
	QString Randomizer( "Randomizer:" );
	QString PresentLine = File.readLine();
	
	while ((PresentLine != Randomizer) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == Randomizer) && !(File.atEnd()))
	{
		ParentSize = (File.readLine()).toInt();
		OffspringSize = (File.readLine()).toInt();
		TournamentPartition.resize(OffspringSize);
		NumberOfVariables = (File.readLine()).toInt();
		ProgramLengthMax = (File.readLine()).toInt();
	
		for (int i=0; i<3; i++)
			ProbSearchOperator[i]=(File.readLine()).toDouble();
		
		for (i=0; i<2; i++)
			ProbMutationPower[i]=(File.readLine()).toDouble();
		
		for (i=0; i<3; i++)
			ProbCrossOverPoints[i]=(File.readLine()).toDouble();
	
		for (i=0; i<17; i++)
			ProbInstruktion[i]=(File.readLine()).toDouble();

		PresentLine = File.readLine();
		if (PresentLine >= 1.0)
		{
			ProbInstruktion[17]=PresentLine.toDouble();
			PresentLine = File.readLine();
		}
		else
			ProbInstruktion[17] = ProbInstruktion[16];

		QString ConString( "Constant:" );
		PresentLine = File.readLine();
		
		while ((PresentLine != ConString) && !(File.atEnd()))
		PresentLine = File.readLine();

		if ((PresentLine == ConString) && !(File.atEnd()))
		{
			PresentLine = File.readLine();
			if (PresentLine == "newConstant")
			{
				PresentLine = File.readLine();
				createConstant (PresentLine.toInt(), true , 0, 1000);
			}
			else
			{
				int NumOfConst = PresentLine.toInt();
				
				Constant.resize(NumOfConst);
				for (i=0; i<NumOfConst; i++)
				{
					PresentLine = File.readLine();	
					Constant[i]= PresentLine.toDouble();
				}
			}
		}
		else
			LastError =263;
	}
	else
		LastError =262;
}



void MT_Randomizer::writeToFileRandi(QTextStream &File)
{

	File << ("Randomizer:\n");
	File << ParentSize << endl;
	File << OffspringSize << endl;
	File << NumberOfVariables << endl;
	File << ProgramLengthMax << endl;
	
	File << ProbSearchOperator[0] << endl;
	File << ProbSearchOperator[1] << endl;
	File << ProbSearchOperator[2] << endl;

	File << ProbMutationPower[0] << endl;
	File << ProbMutationPower[1] << endl;
	
	File << ProbCrossOverPoints[0] << endl;
	File << ProbCrossOverPoints[1] << endl;
	File << ProbCrossOverPoints[2] << endl;

	for (int i=0; i<18; i++)
		File << ProbInstruktion[i] <<endl;
	
	File << endl;

	File << ("Constant:") << endl;
	File << Constant.size() << endl;
	for(i=0; i<Constant.size(); i++)
		File << Constant[i] << endl;

	File << endl;

}

//////////////////////////////////////////////////////////////////////
// normal set/ get method
////////////////////////////////////////////////////////////////////

void MT_Randomizer::returnIndividualsValue(int **Length, int **NumOfVar, QArray<double> **Con, QArray<double> **ProbOfFu)
{

	*Length= &ProgramLengthMax;
	*NumOfVar= &NumberOfVariables;

	*Con = &Constant;
	*ProbOfFu = &ProbInstruktion;

}

void MT_Randomizer::returnSearchValue(QArray<double> **ProbMPower, QArray<double> **ProbSOperator, QArray<double> **ProbXPoints)
{	
	*ProbMPower = &ProbMutationPower; 
	*ProbSOperator = &ProbSearchOperator;
	*ProbXPoints = &ProbCrossOverPoints;
}

int MT_Randomizer::getLastError()
{
	return LastError;
}

void MT_Randomizer::setParentSize(int SizeOfParent)
{
	ParentSize = SizeOfParent; 
}

void MT_Randomizer::setOffspringSize(int OffSize)
{
	OffspringSize = OffSize;
	TournamentPartition.resize(OffspringSize);
}

int MT_Randomizer::getProgramLengthMax()
{
	return ProgramLengthMax;
}

//////////////////////////////////////////////////////////////////////
//special method for supply random
////////////////////////////////////////////////////////////////////

unsigned long MT_Randomizer::getRandomInteger(unsigned long upperBoundary)
{
	unsigned long Random;

	if (upperBoundary>0)
		Random = fabs(rand() % upperBoundary);
	else
		Random = 0;

	return Random;
}


int MT_Randomizer::getProgLength()
{

	double d= ProgramLengthMax * 0.66;
	int StartLength = d;

	return StartLength;
}


void MT_Randomizer::createConstant(int NumOfConstant, bool Integer, double underBoun, double upperBoun)
{
	
	double RangeDouble = upperBoun-underBoun;
	int RangeInt = upperBoun-underBoun;

	Constant.resize(NumOfConstant);

	if (Integer)
	{
		// Erzeuge zufällig Integer Konstanten
		for(int i=0; i<NumOfConstant; i++)
			Constant[i]= underBoun+getRandomInteger(RangeInt);
		
	}
	else
	{
		// Problem falls double aus großen Bereich gewählt wird !!!
		// RangeDouble <65  .... drei Stellen hinter dem Komma)
		for(int i=0; i<NumOfConstant; i++)
		{	
			Constant[i]= (double) underBoun + fabs((double)getRandomInteger(RangeDouble*1000) / 1000.0);
		}
	}
}

bool MT_Randomizer::answerMutateLine()
{
	bool Answer;

	if (getRandomInteger(1000) < ProbMutationPower[0])
		Answer = true;
	else
		Answer = false;

	return Answer;
}

bool MT_Randomizer::answerMutateElement()
{
	bool Answer;

	if (getRandomInteger(1000) < ProbMutationPower[1])
		Answer = true;
	else
		Answer = false;

	return Answer;
}


QArray <int> * MT_Randomizer::getRandomTournamentPartition(int NumberOfTour)
{
	// Array welches sich die Anzahl der momnetanen Spieler pro Turnier merkt
	QArray<int> Index;
	Index.resize(NumberOfTour);
	for (int k =0; k<NumberOfTour; k++)
		Index[k]=0;

	// Angabe des Turniers anwelchem der Speiler teilnehmen soll
	int IndexOfTour =0;
	// Anzahl der Spieler pro Turnier, also Turniergröße !
	int MaxPlayers = (OffspringSize/ NumberOfTour);


	for (int i=0; i<OffspringSize; i++)
	{
		IndexOfTour = getRandomInteger(NumberOfTour);
			
		if (Index[IndexOfTour] < MaxPlayers)
		{
			// im "IndexOfTour" Turnier ist noch ein Platz frei
			TournamentPartition[i]=IndexOfTour;
			Index[IndexOfTour]++;
		
		}
		else 
		{
			// das "IndexOfTour" Turnier ist voll! dann ...
			while (IndexOfTour !=-1)
			{
				IndexOfTour = (IndexOfTour+1)%NumberOfTour;
				if (Index[IndexOfTour] < MaxPlayers)
				{
					TournamentPartition[i]=IndexOfTour;
					Index[IndexOfTour]++;
					IndexOfTour =-1;
				}
			}
		}
	}
	
	return &TournamentPartition;
}

int MT_Randomizer::getProportionalWinner(QArray<int> * Players)
{
	QArray<int> PlayersProportional; 
	PlayersProportional.resize(Players->size());
	int Total = 0;

	for(int i=0; i<(*Players).size();i++)
	{
		Total = Total + (*Players)[i];
		PlayersProportional[i] = Total;
	}
	
	int Winner = getRandomInteger(Total);

	for(i=0; i<(*Players).size();i++)
	{
		if (Winner<=PlayersProportional[i])
		{
			Winner = i; break;
		}
	
	}
	return Winner;
}


MT_Instruction MT_Randomizer::getRandomInstruktion()
{
	
	int Choice =1;
	MT_Instruction NewInstruction;

//	ProbInstruktion[17] = ProbInstruktion[16]+1000.0;

	unsigned long  Random = getRandomInteger(ProbInstruktion[17]);

	for (int i=0; i<18; i++)
	{
		if (Random<ProbInstruktion[i])
		{
			Choice =i;
			break;
		}
	}

	switch (Choice)
	{
		case 0: {NewInstruction = swi;} break;
		case 1: {NewInstruction = cop;} break;
		case 2: {NewInstruction = add;} break;
		case 3: {NewInstruction = sub;} break;
		case 4: {NewInstruction = mul;} break;
		case 5: {NewInstruction = mtd;} break;
		case 6: {NewInstruction = mod;} break;
		case 7: {NewInstruction = mmin;} break;
		case 8: {NewInstruction = mmax;} break;
		case 9: {NewInstruction = cmp;} break;
		case 10: {NewInstruction = jmp;} break;
		case 11: {NewInstruction = jle;} break;
		case 12: {NewInstruction = jeq;} break;
		case 13: {NewInstruction = nop;} break;
		case 14: {NewInstruction = lui;} break;
		case 15: {NewInstruction = lus;} break;
		case 16: {NewInstruction = lud;} break;
		case 17: {NewInstruction = lum;} break;
	}

	return NewInstruction;
}


int MT_Randomizer::getRandomSOperator()
{
	int Choice =0;
	unsigned int Random = getRandomInteger(ProbSearchOperator[2]);
	for (int i=0; i<3; i++)
	{
		if (Random<ProbSearchOperator[i])
		{
			Choice =i+1;
			break;
		}
	}

	return Choice;
}

QArray <int> * MT_Randomizer::getRandomXPoints(int SizeIndi1, int SizeIndi2)
{	
	int NumberofXPoints;
	unsigned int Random = getRandomInteger(ProbCrossOverPoints[2]);

	if ((SizeIndi1>3)&&(SizeIndi2>3))
	{
		for (int i=0; i<3; i++)
		{
			if (Random<ProbCrossOverPoints[i])
			{
				NumberofXPoints =i;
				break;
			}
		}
	}
	else 
		NumberofXPoints =0; // da die Individuen so klein sind, wird nur ein XPunkt gewählt


	RandomXPoints[0]=NumberofXPoints;

	switch (NumberofXPoints)
	{
		case 0: 
			{
				// 1 XPoint
				RandomXPoints[1]= getRandomInteger(SizeIndi1)+1;
				RandomXPoints[2]= getRandomInteger(SizeIndi2)+1;

			}break;
		case 1: 
			{
				// 2 XPoints
				RandomXPoints[1]= getRandomInteger(SizeIndi1-1)+1;
				RandomXPoints[2]= getRandomInteger(SizeIndi2-1)+1;

				RandomXPoints[3]= RandomXPoints[1] + 1 + getRandomInteger(SizeIndi1-RandomXPoints[1]-1);
				RandomXPoints[4]= RandomXPoints[2] + 1 + getRandomInteger(SizeIndi2-RandomXPoints[2]-1);

			}break;
		case 2:
			{
				// 3 XPoints
				RandomXPoints[1]= getRandomInteger(SizeIndi1-2)+1;
				RandomXPoints[2]= getRandomInteger(SizeIndi2-2)+1;

				RandomXPoints[3]= RandomXPoints[1] + 1 + getRandomInteger(SizeIndi1-RandomXPoints[1]-2);
				RandomXPoints[4]= RandomXPoints[2] + 1 + getRandomInteger(SizeIndi2-RandomXPoints[2]-2);

				RandomXPoints[5]= RandomXPoints[3] + 1 + getRandomInteger(SizeIndi1-RandomXPoints[3]-1);
				RandomXPoints[6]= RandomXPoints[4] + 1 + getRandomInteger(SizeIndi2-RandomXPoints[4]-1);

			}break;
	}



	return &RandomXPoints;
}


MT_Operand *  MT_Randomizer::getRandomOperand()
{
	MT_Operand *NewOperand;

	int Terminal = getRandomInteger(NumberOfVariables+Constant.size());

	if (Terminal<NumberOfVariables)
		NewOperand = new MT_Operand(1,-1,Terminal);
	else 
		NewOperand = new MT_Operand(0,Constant[Terminal-NumberOfVariables],-1);

	return NewOperand;
}



MT_Operand * MT_Randomizer::getRandomVariable()
{
	MT_Operand *NewOperand;
	NewOperand = new MT_Operand(1,-1.0,getRandomInteger(NumberOfVariables));
	return NewOperand;
}


int MT_Randomizer::getNumOfVari()
{
	return NumberOfVariables;
}
