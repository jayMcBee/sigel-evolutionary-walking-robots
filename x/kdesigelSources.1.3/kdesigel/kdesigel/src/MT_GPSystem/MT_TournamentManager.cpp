// MT_TournamentManager.cpp: Implementierung der Klasse MT_TournamentManager.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_TournamentManager.h"
//////////////////////////////////////////////////////////////////////
//administrative method 
//////////////////////////////////////////////////////////////////////

MT_TournamentManager::MT_TournamentManager()
{

}

MT_TournamentManager::MT_TournamentManager(MT_Population *PPop, MT_Population *OPop, MT_Randomizer * _Randi, int Method, int TSize)
{
	LastError=0;
	ParentPop = PPop;
	OffspringPop = OPop;
	Randi = _Randi;
	SMethod =Method;
	NameForNewParent =PPop->getSize();
		
	setTournamentSize(TSize);
	
}

MT_TournamentManager::MT_TournamentManager(MT_Population *PPop, MT_Population *OPop, MT_Randomizer *_Randi, QTextStream &File)
{
	LastError=0;
	ParentPop = PPop;
	OffspringPop = OPop;
	Randi = _Randi;
	TournamentSize =0;
	NameForNewParent =PPop->getSize();
	 
	QString TManager ( "TournamentManager:");
	QString PresentLine = File.readLine();


	while ((PresentLine != TManager) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == TManager) && !(File.atEnd()))
	{
		SMethod = (File.readLine()).toInt();
		setTournamentSize((File.readLine()).toInt());
	}
	else
		LastError=510;
}


MT_TournamentManager::~MT_TournamentManager()
{
	// Achtung: im Array Pointer auf Tournament = Population
	// Population muss clean sein, also keine Individuen 
	// besitzen !!! Verantwortung darfür performaTournaments()!
	if(TournamentSize !=0)
	{
		for(int i=0; i<Tournaments.size(); i++)
		{
			delete Tournaments[i];
		}
	}
}



//////////////////////////////////////////////////////////////////////
//get/set method 
//////////////////////////////////////////////////////////////////////
int MT_TournamentManager::getLastError()
{
	return LastError;
}

int MT_TournamentManager::getTournamentSize()
{

	return TournamentSize;
}

int MT_TournamentManager::getSelectionMethod()
{

	return SMethod;
}


void MT_TournamentManager::setSelectionMethod(int Method)
{
	SMethod = Method;
}

void MT_TournamentManager::setNameForParent(int Na)
{
	NameForNewParent = Na;
}


void MT_TournamentManager::setTournamentSize(int SizeOfT)
{

	if(TournamentSize !=0)
	{
		for(int i=0; i<TournamentNumber; i++)
			delete Tournaments[i];

		// Tournaments müssen immmer vollständig gelöscht werden
		// obwohl evtl. TournamentSize nicht geändert hat 
		// allerdings kann sich ParentSize oder OffspringSize geändert haben!!!

	}

	TournamentSize = SizeOfT;
	TournamentNumber = (OffspringPop->getSize())/TournamentSize;
	if (TournamentNumber <1.0)
		TournamentNumber = 1;

	WinnerNumber = (ParentPop->getSize())/TournamentNumber;
	
	Tournaments.resize(TournamentNumber);
	WinnerLoser.resize(TournamentSize);
	
	createTournaments();

}

void MT_TournamentManager::setTypOfIndividual(int Typ)
{
	for(int i=0; i<TournamentNumber; i++)
		Tournaments[i]->setTypOfIndividual(Typ);
}


//////////////////////////////////////////////////////////////////////
//special method 
//////////////////////////////////////////////////////////////////////

void MT_TournamentManager::createTournaments()
{
	for(int i=0; i<TournamentNumber; i++)
		Tournaments[i] = new MT_Tournament(TournamentSize, WinnerNumber);	
}


void MT_TournamentManager::performTournaments()
{

// ************************ Vorbereitung/ Einteilung der Offspring Individuen in Turniere ************************ //
	int OffspringSize = OffspringPop->getSize();

	for (int i=0; i<Tournaments.size(); i++)
		Tournaments[i]->setFreePos(0);
	
	// fetch a correct partition for separate the offspring into tournaments
	QArray<int>	*Partition = Randi->getRandomTournamentPartition(TournamentNumber);

//int DebugInfo =0;
//for (int g=0; g < OffspringSize;g++)
//	DebugInfo = Partition->at(g);

//MT_Individual *DebugIndi = 0;

	// füge nun alle O-Individuen in ein Turnier ein! 
	for (i=0; i<OffspringSize; i++)
	{
//		DebugIndi = OffspringPop->getIndividual(i); 
		Tournaments[Partition->at(i)]->insertIndividual(OffspringPop->getIndividual(i));
	
	}

//DebugIndi =0;

	OffspringPop->flush();

//for(i=0;i<TournamentNumber; i++)
//		for (g=0; g<WinnerLoser.size();g++)
//			DebugIndi= Tournaments[i]->getIndividual(g);


		
// ************************ Ausführung der Turniere / Aktualisierung der Eltern  ************************ //

	int ParentPos =0;
	MT_Individual * Winner;

	if (SMethod == 1)
	{
		for(i=0;i<TournamentNumber; i++)
		{
			Tournaments[i]->ranking(&WinnerLoser);

			for(int k=0;k<WinnerLoser.size();k++)
			{
				/* if (WinnerLoser[k]==0)
				{
					delete (Tournaments[i]->getIndividual(k));
				}
				else */
			
				if (WinnerLoser[k]!=0)
				{
					// Winner = (Tournaments[i]->getIndividual(k));
					 // Neu: Verantwort auf Winner übertragen;
				
					Winner = Tournaments[i]->insertAtPos(0,k);
				
					// increase the Age of the individual
					Winner->increaseAge();
					// set the position inside the parent population
					Winner->setPosition(ParentPos);
					// set Name;
					if (Winner->getName()<0)
					{
						Winner->setNewName(NameForNewParent);
						NameForNewParent++;
					}
					// insert the winner into the parentpopulation
					ParentPop->insertAtPos(Winner,ParentPos);
					// increase the Index for insert 
					ParentPos++;
				}
				
			}

			for(k=0;k<WinnerLoser.size();k++)
			{
				Winner = Tournaments[i]->insertAtPos(0,k);
				delete Winner; 
			}
		

			(Tournaments[i])->flush();
		}

		ParentPop->setFreePos(ParentPop->getSize());
	}
	else 
	{
		for(i=0;i<TournamentNumber; i++)
		{
			Tournaments[i]->fitnessProp(Randi, &WinnerLoser);
			
			for(int k=0;k<WinnerLoser.size();k++)
			{
				if (WinnerLoser[k]==0)
				{

					delete (Tournaments[i]->getIndividual(k));
				}
				else
				{
					Winner = (Tournaments[i]->getIndividual(k));
		
					// increase the Age of the individual
					Winner->increaseAge();
					// set the position inside the parent population
					Winner->setPosition(ParentPos);
					// set Name;
					if (Winner->getName()<0)
					{
						Winner->setNewName(NameForNewParent);
						NameForNewParent++;
					}
					// insert the winner in the parentpopulation
					ParentPop->insertAtPos(Winner,ParentPos);
					// increase the Index for insert 
					ParentPos++;
				}
			}

			(Tournaments[i])->flush();

		}
		
		ParentPop->setFreePos(ParentPop->getSize());
	}

double DebugFitness = 0.0;

	for ( int g=0; g< ParentPop->getFreePos(); g++)
		DebugFitness = ParentPop->getIndividual(g)->getFitness();



}


void MT_TournamentManager::writeToFileSetup(QTextStream &File)
{

	File << ("TournamentManager:\n");
	File << SMethod << endl;
	File << TournamentSize << endl << endl;

}
