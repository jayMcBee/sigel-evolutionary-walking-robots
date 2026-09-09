// MT_StatisticsElement.cpp: implementation of class MT_StatisticsElement.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_StatisticsElement.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_StatisticsElement::MT_StatisticsElement(QTextStream &File)
{

	QString StatisticsE( "StatisticsElement:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != StatisticsE) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == StatisticsE) && !(File.atEnd()))
	{
		Generation = (File.readLine()).toInt();
		Varianz =(File.readLine()).toDouble(); 
		AverageFitness= (File.readLine()).toDouble();
		MaxFitness= (File.readLine()).toDouble();
		
		NumOfSimpleCopyParent=(File.readLine()).toInt();
	
		NumOfElementMutationParent=(File.readLine()).toInt();
		NumOfMutateIndividuals=(File.readLine()).toInt();   
		NumOfMutateImprovingIndividuals=(File.readLine()).toInt();

		CrossoverEventParent.resize(6);
		for (int i=0; i<6; i++)
			CrossoverEventParent[i]=(File.readLine()).toInt();

		NumOfTotalElementMutation=(File.readLine()).toInt();
		NumOfSimpleCopyOffspring=(File.readLine()).toInt(); 
		NumOfCrossoverEvent=(File.readLine()).toInt();
		NumOfMutateOffspring =0;
		int Buffer =(File.readLine()).toInt();
		if (Buffer != 0)
			NumOfMutateOffspring =Buffer;

	}

}


MT_StatisticsElement::MT_StatisticsElement()
{

	Generation = 0;
	Varianz =0.0 ;
	AverageFitness= 0.0;
	MaxFitness= 0.0;
	
	NumOfSimpleCopyParent=0; 
	
	NumOfElementMutationParent=0; 
	NumOfMutateIndividuals=0;    
	NumOfMutateImprovingIndividuals=0;

	CrossoverEventParent.resize(6);
	for (int i=0; i<6; i++)
		CrossoverEventParent[i]=0; 

	NumOfTotalElementMutation=0;
	NumOfSimpleCopyOffspring=0; 
	NumOfCrossoverEvent=0;
	NumOfMutateOffspring =0;

}

MT_StatisticsElement::~MT_StatisticsElement()
{

}



void MT_StatisticsElement::writeToFileElement(QTextStream &File)
{
	
	File << ("StatisticsElement:\n");
	File << Generation <<Qt::endl;
	File << Varianz <<Qt::endl;
	File << AverageFitness <<Qt::endl;
	File << MaxFitness <<Qt::endl;
	File << NumOfSimpleCopyParent <<Qt::endl;
	File << NumOfElementMutationParent <<Qt::endl;
	File << NumOfMutateIndividuals <<Qt::endl;
	File << NumOfMutateImprovingIndividuals <<Qt::endl;
	File << CrossoverEventParent[0] <<Qt::endl;
	File << CrossoverEventParent[1] <<Qt::endl;
	File << CrossoverEventParent[2] <<Qt::endl;
	File << CrossoverEventParent[3] <<Qt::endl;
	File << CrossoverEventParent[4] <<Qt::endl;
	File << CrossoverEventParent[5] <<Qt::endl;
	File << NumOfTotalElementMutation <<Qt::endl;
	File << NumOfSimpleCopyOffspring <<Qt::endl;
	File << NumOfCrossoverEvent <<Qt::endl;
	File << NumOfMutateOffspring <<Qt::endl;
	File << Qt::endl;

}
