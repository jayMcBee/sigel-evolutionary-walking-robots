// MT_TrainingCase.cpp: Implementierung der Klasse MT_TrainingCase.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_TrainingCase.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_TrainingCase::MT_TrainingCase(QTextStream &File)
{
	QString TCaseString("TrainingCases:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != TCaseString) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == TCaseString) && !(File.atEnd()))
	{
		Name = (File.readLine()).toInt();
		Fitness = (File.readLine()).toDouble();
		Boundary = (File.readLine()).toInt();

		TranslateIndividual = new MT_TranslatedIndividual(File);	
	}
}

MT_TrainingCase::MT_TrainingCase()
{
	
	Name = -1;
	Fitness = -1.0;
	Boundary = 0;
}

MT_TrainingCase::~MT_TrainingCase()
{
	delete TranslateIndividual;
}

MT_TrainingCase::MT_TrainingCase(double Fit, MT_TranslatedIndividual *TransIndi, int Na, int boun)
{

	Name = Na;
	Fitness = Fit;
	TranslateIndividual = TransIndi;
	Boundary = boun;

}


void MT_TrainingCase::writeToFileTCase(QTextStream &File)
{
	File << ("TrainingCases:\n");
	File << Name << endl;
	File << Fitness << endl;
	File << Boundary << endl << endl;

	TranslateIndividual->writeToFileTransIndi(File);
}

double MT_TrainingCase::getFitness()
{
	return Fitness; 
}


void MT_TrainingCase::setBoundary(int NewBoundary)
{

	Boundary = NewBoundary;
}

void MT_TrainingCase::setName(int NewName)
{

	Name = NewName;
}

MT_TranslatedIndividual * MT_TrainingCase::getIndividual()
{

	return TranslateIndividual;
}




int MT_TrainingCase::getName()
{

	return Name;
}

void MT_TrainingCase::setFitness(double Fit)
{
	Fitness=Fit;

}
