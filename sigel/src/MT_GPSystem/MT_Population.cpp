// MT_Population.cpp: implementation of class MT_Population.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Population.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Population::MT_Population()
{
	FirstFreePos=0;
	PopSize =1;
	Individuals.resize(1);
	Individuals[0]=0;
	LastError =0;
	Change =false;

}

MT_Population::~MT_Population()
{
	for (int i=0; i<FirstFreePos; i++)
		delete Individuals[i]; 

}

MT_Population::MT_Population(MT_Randomizer *Randi, int n)
{
	FirstFreePos=0;
	PopSize =0;
	LastError =0;
	Change =false;

	createNewIndis(n, Randi);
	LastError=Randi->getLastError();

}

MT_Population::MT_Population(QTextStream &File)
{
	LastError=0;
	Change = false;
	
	PopSize = (File.readLine()).toInt();
	FirstFreePos =(File.readLine()).toInt();

	Individuals.resize(PopSize);

	for (int i=0; i<FirstFreePos; i++)
		Individuals[i]= new MT_Individual(File);

}

//////////////////////////////////////////////////////////////////////
// load/store method  
//////////////////////////////////////////////////////////////////////

void MT_Population::writeToFilePop(QTextStream &File)
{
	
	File << ("Population:\n");
	File << PopSize << Qt::endl;
	File << FirstFreePos << Qt::endl << Qt::endl;
	
	for (int i=0; i<FirstFreePos; i++)
		if (Individuals[i] != NULL)
			(Individuals[i])->writeToFileIndi(File);
}

int MT_Population::loadPop(QTextStream &File)
{

	QString PopulationString ("Population:");
	QString PresentLine= File.readLine();

	while ((PresentLine != PopulationString) && !(File.atEnd()))
		PresentLine = File.readLine();
	
	if ((PresentLine == PopulationString) && !(File.atEnd()))
	{
		changePopSize(0); // The individuals are deleted here 

		PopSize = (File.readLine()).toInt();
		FirstFreePos =(File.readLine()).toInt();
	
		Individuals.resize(PopSize);

		for (int i=0; i<FirstFreePos; i++)
			Individuals[i]= new MT_Individual(File);

		for (int i=0; i<FirstFreePos; i++)
			Individuals[i]->setNewName(i);
	}
	else
		LastError=321;

	return LastError;
}

int MT_Population::exportPop(QTextStream &File)
{
	writeToFilePop(File);
	return LastError;
}

int MT_Population::importPop(QTextStream &File)
{
		
	QString PopulationString ("Population:");
	QString PresentLine= File.readLine();

	while ((PresentLine != PopulationString) && !(File.atEnd()))
		PresentLine = File.readLine();
	
	if ((PresentLine == PopulationString) && !(File.atEnd()))
	{	
		int ImportSize = (File.readLine()).toInt();
		int ImportFreePos =(File.readLine()).toInt();
		
		PopSize = PopSize + ImportFreePos;
		Individuals.resize(PopSize);

		for (int i = FirstFreePos; i<(FirstFreePos+ImportFreePos); i++)
			insertAtPos(new MT_Individual(File),i);

		FirstFreePos = FirstFreePos+ImportFreePos;
	}
	else
		LastError= 320;


	return LastError;
}

//////////////////////////////////////////////////////////////////////
// further method  
//////////////////////////////////////////////////////////////////////

int MT_Population::getSize()
{

	return PopSize;
}

int MT_Population::getLastError()
{

	return LastError;
}

MT_Individual * MT_Population::getIndividual(int Pos)
{
	if ((Pos<FirstFreePos)&&(Pos >= 0))
		return Individuals[Pos];
	else 
		return 0;
}

// ************************ random generation 

void MT_Population::createNewIndis(int NumberOfNewIndi, MT_Randomizer *Randi)
{
	// denote that PopSize and FirstFreePos dosn't implicit equal!
	
	PopSize = PopSize + NumberOfNewIndi;
	Individuals.resize(PopSize);

	for (int i=FirstFreePos; i<(FirstFreePos+NumberOfNewIndi); i++)
	{
		Individuals[i]= new MT_Individual(i,Randi);
		Individuals[i]->setPosition(i);

	}
		
	FirstFreePos= FirstFreePos+NumberOfNewIndi;

}


int MT_Population::createNewIndi(MT_Randomizer *Randi)
{
	// denote that PopSize and FirstFreePos dosn't implicit equal!
	
	PopSize++;
	Individuals.resize(PopSize);

	Individuals[FirstFreePos]= new MT_Individual(FirstFreePos,Randi);
	Individuals[FirstFreePos]->setPosition(FirstFreePos);

		
	FirstFreePos ++;

	return (FirstFreePos-1);

}


void MT_Population::createNewPop(int PSize, MT_Randomizer *Randi)
{

	for (int i=0; i<FirstFreePos; i++)
		delete Individuals[i]; 
	
	Individuals.resize(0);
	FirstFreePos = 0;
	PopSize = 0;
	createNewIndis(PSize,Randi);

}

int MT_Population::insertIndividual(MT_Individual *NewIndividual)
{
	LastError = 0;

	if (FirstFreePos<PopSize)
	{
		NewIndividual->setPosition(FirstFreePos);
		Individuals[FirstFreePos] = NewIndividual;
		FirstFreePos++;
	}
	else
		LastError = 340;

	return LastError;
}

MT_Individual * MT_Population::insertAtPos(MT_Individual *NewIndividual, int Pos)
{

	MT_Individual *Indi = Individuals[Pos];

	if(NewIndividual != 0){
		NewIndividual->setPosition(Pos);
	}

	Individuals[Pos] = NewIndividual;

	return Indi;
}

int MT_Population::addIndividual(MT_Individual *NewIndividual)
{
	PopSize ++;
	Individuals.resize(PopSize);

	NewIndividual->setPosition(FirstFreePos);

	Individuals[FirstFreePos]=NewIndividual;
	FirstFreePos++;
	
	return LastError;
}

MT_Individual * MT_Population::delIndividual(int Pos)
{
	MT_Individual *Indi = Individuals[Pos];
	
	while (Pos < (FirstFreePos-1))
	{
		Individuals[Pos]=Individuals[Pos+1];
		Individuals[Pos]->setPosition(Pos);
		Pos++;
	}

	Individuals[Pos] =0; 
	
	Individuals.resize(PopSize-1);
	PopSize --;
	FirstFreePos--;

	return Indi;
}

MT_Individual * MT_Population::removeIndividual(int Pos)
{
	MT_Individual *Indi = Individuals[Pos];
	
	while (Pos < (FirstFreePos-1))
	{
		Individuals[Pos]=Individuals[Pos+1];
		Individuals[Pos]->setPosition(Pos);
		Pos++;
	}

	Individuals[Pos] =0; 
	
	FirstFreePos--;
	
	return Indi;
}

void MT_Population::flush()
{
	for (int i=0; i<PopSize; i++)
		Individuals[i] = 0;

	FirstFreePos=0;

}


void MT_Population::setMaxProgLen(int NewLen)
{
	for(int i=0; i<FirstFreePos;i++)
			Individuals[i]->setMaxProgLen(NewLen);
}

void MT_Population::changeMaxNumVariable(int NewNum)
{
	for(int i=0; i<FirstFreePos;i++)
		Individuals[i]->changeMaxNumVariable(NewNum);
}


void MT_Population::setFreePos(int Pos)
{

	FirstFreePos = Pos;
}

int MT_Population::getFreePos()
{

	return FirstFreePos;
	
}

bool MT_Population::changePopSize(int NewPopSize)
{
	Change =false;

	if(NewPopSize != PopSize)
	{
		if (NewPopSize<PopSize)
		{	
			for(int i= NewPopSize; i<PopSize; i++)
			{
				if(Individuals[i] !=0)
				{
					delete Individuals[i];
					Individuals[i] =0;
				}
			}

			Individuals.resize(NewPopSize);
			Change = true;
			PopSize=NewPopSize;
			FirstFreePos = NewPopSize;
		}

		if (NewPopSize>PopSize)
		{	
			Individuals.resize(NewPopSize);
			for(int i=PopSize; i< NewPopSize; i++)
				Individuals[i] =0;
			
			PopSize=NewPopSize;		
			Change = true;
		}
	
	}

	return Change;
}

MT_Individual * MT_Population::getBestIndividual()
{
	double BestFitness =0.0;
	int PositionOfBestIndividual=0;
	
	for (int i=0; i<FirstFreePos; i++)
	{
		if (BestFitness<(Individuals[i]->getFitness()))
		{
			BestFitness=(Individuals[i]->getFitness());
			PositionOfBestIndividual =i;
		}
	}

	return (Individuals[PositionOfBestIndividual]);
}

