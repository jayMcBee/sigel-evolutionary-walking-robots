// MT_Trainingset.cpp: implementation of class MT_Trainingset.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Trainingset.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Trainingset::MT_Trainingset()
{
	TSetName = 1;
	TSize = 100;
	TCases.resize(TSize);
	PresentTSize= 0;
	FreePosition=0;
	for(int i=0; i<TSize; i++)
		TCases[i]=0;

}

MT_Trainingset::~MT_Trainingset()
{
	for (int i =0; i<PresentTSize; i++)
			delete TCases[i];
}

MT_Trainingset::MT_Trainingset(QTextStream &File)
{

	QString TSetString("Trainingset:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != TSetString) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == TSetString) && !(File.atEnd()))
	{
		TSetName = (File.readLine()).toInt();
		TSize = (File.readLine()).toInt();
		PresentTSize = (File.readLine()).toInt();

		TCases.resize(TSize);
		for(int i=0; i<TSize; i++)
		TCases[i]=0;

		for (int i=0; i<PresentTSize; i++)
			TCases[i] = new MT_TrainingCase(File);
		
		FreePosition=PresentTSize;
		if (FreePosition >=TSize)
			FreePosition=FreePosition-TSize;
	}

}

MT_Trainingset::MT_Trainingset(int TSi, int TNa)
{
	TSetName = TNa;
	TSize = TSi;
	TCases.resize(TSize);
	PresentTSize= 0;
	FreePosition= 0;
	for(int i=0; i<TSize; i++)
		TCases[i]=0;

}

//////////////////////////////////////////////////////////////////////
// weitere Methoden 
//////////////////////////////////////////////////////////////////////

void MT_Trainingset::writeToFileTSet(QTextStream &File)
{
	
	File << ("Trainingset:\n");
	File << TSetName << Qt::endl;
	File << TSize << Qt::endl;
	File << PresentTSize << Qt::endl << Qt::endl;

	for (int i=0; i<PresentTSize; i++)
		if(TCases[i] != NULL)
			(TCases[i])->writeToFileTCase(File);
}

void MT_Trainingset::changeTSize(int NewTSize)
{

	if ((NewTSize<TSize)&&(PresentTSize>NewTSize))
	{
		for (int i=NewTSize; i<PresentTSize; i++)
			delete TCases[i];

		PresentTSize = NewTSize;
	}
	
	if (FreePosition >= NewTSize)
		FreePosition=0;
	
	TCases.resize(NewTSize);

	if (NewTSize > TSize)
		for(int i=TSize; i<NewTSize; i++)
			TCases[i] =0;

	TSize=NewTSize;

}

void MT_Trainingset::updateTSet(QQueue<MT_TrainingCase *> *NewTCases)
{
	int NumOfNew = NewTCases->count();
	if (NumOfNew !=0)
	{
		TSetName++;
		if (NumOfNew > TSize)
		{
			for(int i=0; i<(NumOfNew-TSize);i++)
				delete (NewTCases->dequeue()); 
			
			NumOfNew = NewTCases->count();
		}

		for (int i=0; i<NumOfNew; i++)
			insertTCase(NewTCases->dequeue());
	}

}

void MT_Trainingset::insertTCase(MT_TrainingCase *Tcase)
{

	if (TCases[FreePosition]==0)
	{
		TCases[FreePosition]= Tcase;
		PresentTSize++;
		FreePosition++;
		if (FreePosition >=TSize)
			FreePosition=0;

	}
	else 
	{
		delete TCases[FreePosition];
		TCases[FreePosition]= Tcase;
		FreePosition++;
		if (FreePosition >=TSize)
			FreePosition=0;

	}

}

MT_TrainingCase *MT_Trainingset::getTCase(int PositionTcase)
{

	return 	TCases[PositionTcase];
}

int MT_Trainingset::getPresentTSize()
{
	return PresentTSize;
}

int MT_Trainingset::getName()
{

	return TSetName;
}
