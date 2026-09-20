// MT_TranslatedIndividual.cpp: implementation of class MT_TranslatedIndividual.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_TranslatedIndividual.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_TranslatedIndividual::MT_TranslatedIndividual()
{
	//Do not use 

	T_Instruktion=0;
	T_Operand1 = 0;
	T_Operand2 =0;
	T_length = 0;
	Boundary = -1;
	MetaData = 0;

}

MT_TranslatedIndividual::~MT_TranslatedIndividual()
{
	delete T_Operand1;
	delete T_Operand2;
	delete T_Instruktion;
	delete MetaData;
		
}

MT_TranslatedIndividual::MT_TranslatedIndividual(QList<int> *T_Instruk, QList<int> *T_OperOne, QList<int> *T_OperTwo, QList<int> *MData )
{

	T_Instruktion= T_Instruk;
	T_Operand1 = T_OperOne;
	T_Operand2 = T_OperTwo;
	T_length = T_Instruktion->size();
	MetaData = MData;
	
	Boundary = -1;
}

MT_TranslatedIndividual::MT_TranslatedIndividual(QTextStream &File)
{
	QString TransIndiString("TranslatedIndividual:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != TransIndiString) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == TransIndiString) && !(File.atEnd()))
	{
		T_length = (File.readLine()).toInt();
		Boundary = (File.readLine()).toInt();

		T_Instruktion = new QList<int>;
		T_Operand1 = new QList<int>;
		T_Operand2 = new QList<int>;
		MetaData = new QList<int>;

		if (Boundary <= 0)
			MetaData->resize(16);
		else
			MetaData->resize(32);			

		T_Instruktion->resize(T_length);
		T_Operand1->resize(T_length);
		T_Operand2->resize(T_length);

		QString PartOfLine;
		// Line layout, e.g. "1234 1234567 1234567" <- length 18  	
		for (int i=0; i<T_length;i++)
		{
			PresentLine = File.readLine();
			PartOfLine = PresentLine.mid(0,4); 
			(*T_Instruktion)[i] = PartOfLine.toInt();
			PartOfLine = PresentLine.mid(4,7);
			(*T_Operand1)[i] = PartOfLine.toInt();
			PartOfLine = PresentLine.mid(11,7);
			(*T_Operand2)[i] = PartOfLine.toInt();
		}
		
		PresentLine = File.readLine();

		if( PresentLine == "MetaData:")
			if(Boundary <=0)
				for(int k=0; k<16; k++)
				{
					PresentLine = File.readLine();
					(*MetaData)[k] = PresentLine.toInt();
				}
			else 
				for(int k=0; k<32; k++)
				{
					PresentLine = File.readLine();
					(*MetaData)[k] = PresentLine.toInt();
				}
					

		else
			// Generate metadata .... 
		{
			for (int i=0; i<MetaData->size(); i++)
				(*MetaData)[i] =0;

			if (Boundary <=0)		// For the evaluator
			{
				(*MetaData)[0] = T_length;
				for (int k=0; k< T_Instruktion->size(); k++)
					(*MetaData)[(*T_Instruktion)[k]] ++;		
			}
			else						// For the classifier
			{
				(*MetaData)[0] = Boundary;
				for (int k=0; k< Boundary; k++)
					(*MetaData)[(*T_Instruktion)[k]] ++;

				(*MetaData)[16] = T_length-Boundary;
				for (int k=Boundary; k< T_Instruktion->size(); k++)
					(*MetaData)[16+(*T_Instruktion)[k]] ++;

			}
		}
	}
}

void MT_TranslatedIndividual::writeToFileTransIndi(QTextStream &File)
{
	File << ("TranslatedIndividual:\n");
	File << T_length << Qt::endl;
	File << Boundary << Qt::endl;

	QString PartOfLine;

	// Line layout, e.g. "123 123456 1234567" <- length 18 


	for (int i=0; i<T_length;i++)
	{
		PartOfLine.setNum((*T_Instruktion)[i]);
		File<<PartOfLine.leftJustified(4,' ', true);

		PartOfLine.setNum((*T_Operand1)[i]);
		File<<PartOfLine.leftJustified(7,' ', true);
		
		PartOfLine.setNum((*T_Operand2)[i]);
		File<<PartOfLine.leftJustified(7,' ', true) << Qt::endl;
		
	}
	File << "MetaData:" << Qt::endl;
	
	for (int i=0; i< MetaData->size(); i++)
		File << (*MetaData)[i] << Qt::endl;
	
	
	File<<Qt::endl;

}
