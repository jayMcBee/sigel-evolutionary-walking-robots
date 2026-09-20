// MT_Statistics.cpp: implementation of class MT_Statistics.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Statistics.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Statistics::MT_Statistics()
{

	TotalCrossoverEvent.resize(6);

}

MT_Statistics::~MT_Statistics()
{

}

MT_Statistics::MT_Statistics(QTextStream & File)
{
	
	TotalCrossoverEvent.resize(6);

	QString Statistics( "Statistics:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != Statistics) && !(File.atEnd()))
		PresentLine = File.readLine();

	if (PresentLine == Statistics)
	{
		PresentLine = File.readLine();
		int NumOfEntry = PresentLine.toInt();
		
		if ( NumOfEntry == 0)
		{
			NumOfSimpleCopyParent =0;
			NumOfMutateIndividuals =0;
			NumOfMutateImprovingIndividuals =0;
			for (int i=0;i<6;i++)
				TotalCrossoverEvent[i]=0;		
		}
		else 
		{
			NumOfSimpleCopyParent = (File.readLine()).toUInt();
			NumOfMutateIndividuals = (File.readLine()).toUInt();
			NumOfMutateImprovingIndividuals = (File.readLine()).toUInt();
			for (int i=0;i<6;i++)
				TotalCrossoverEvent[i]=(File.readLine()).toUInt();

			for (int i=0;i<NumOfEntry;i++ )
				addStatisticElement(new MT_StatisticsElement(File)); 
						
		}
	}
}

void MT_Statistics::writeToFileMT_Statistics(QTextStream & File)
{

	File << ("Statistics:\n");
	File << (StatisticsOfGeneration.size()) <<Qt::endl;
	File << NumOfSimpleCopyParent <<Qt::endl;
	File << NumOfMutateIndividuals <<Qt::endl;
	File << NumOfMutateImprovingIndividuals <<Qt::endl;
	File << TotalCrossoverEvent[0] <<Qt::endl;
	File << TotalCrossoverEvent[1] <<Qt::endl;
	File << TotalCrossoverEvent[2] <<Qt::endl;
	File << TotalCrossoverEvent[3] <<Qt::endl;
	File << TotalCrossoverEvent[4] <<Qt::endl;
	File << TotalCrossoverEvent[5] <<Qt::endl;
	File << Qt::endl;

	for (int i=0; i<(StatisticsOfGeneration.size());i++)
		if (getStatisticElement(i) != NULL)
			getStatisticElement(i)->writeToFileElement(File);
	
}

void MT_Statistics::addStatisticElement(MT_StatisticsElement *Element)
{

	StatisticsOfGeneration.append(Element);	

}

MT_StatisticsElement * MT_Statistics::getStatisticElement(int ElementOfGeneration)
{

	return StatisticsOfGeneration.value(ElementOfGeneration);

}

int MT_Statistics::updateStatistics()
{
	int Error=0;
	NumOfSimpleCopyParent =0;  
	// Note: NumOfSimpleCopyParent now collects offspring data, not parent data  
	NumOfMutateIndividuals = 0;
	NumOfMutateImprovingIndividuals =0;

	for (int i=0; i<6;i++)
		TotalCrossoverEvent[i] =0;

	MT_StatisticsElement * PresentSElement;


	for (int i=1; i<StatisticsOfGeneration.size(); i++)
	{
	
		PresentSElement= StatisticsOfGeneration.value(i);
	
		NumOfSimpleCopyParent = NumOfSimpleCopyParent + PresentSElement->NumOfSimpleCopyOffspring;

		NumOfMutateIndividuals = NumOfMutateIndividuals + PresentSElement->NumOfMutateOffspring;

		NumOfMutateImprovingIndividuals = NumOfMutateImprovingIndividuals + PresentSElement->NumOfMutateImprovingIndividuals;

		for (int k=0; k<6;k++)
			TotalCrossoverEvent[k]=TotalCrossoverEvent[k]+PresentSElement->CrossoverEventParent[k];
			
	}

	return Error;
}
	
