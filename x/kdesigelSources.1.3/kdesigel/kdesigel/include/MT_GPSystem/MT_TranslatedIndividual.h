// MT_TranslatedIndividual.h: interface for class MT_TranslatedIndividual.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_TRANSLATEDINDIVIDUAL_H__5C48785A_CF39_45EF_9A94_1214783FF5E3__INCLUDED_)
#define AFX_MT_TRANSLATEDINDIVIDUAL_H__5C48785A_CF39_45EF_9A94_1214783FF5E3__INCLUDED_

#include <qstring.h>
#include <qtextstream.h>
#include <qarray.h> 

/*
* This class is a simplifiy representation of a SIGEL individual
* respectively the robot control program.The main task is to contain
* the robot program for a very fast use through the MT_Interpreter.
*/


class MT_TranslatedIndividual
 
{
public:
	
	/* indicates whether the TIndividual consists of one (-1) or two (>0) robot control programs
	*  in the case of two, the bondary supply the beginning of the second robot control program
	*/
	int Boundary;

	/* The length of a translated SIGEL robot control program */
	int T_length;
	
	/* An Qarray of integer represent the instructions
	* of a SIGEL robot control program without the operands.
	* -1 symbolize a invalid value/ instruction.
	* @pre: the MT_Translator produce this array
	* @post: the destructor of this class must destroy this array
	*/
	QArray<int> * T_Instruktion;
	
	/* An array of integer represent the first operands
	* of a SIGEL robot control program.
	* -1 symbolize a invalid value/ operand.
	* @pre: the MT_Translator produce this array
	* @post: the destructor of this class must destroy this array
	*/
	QArray<int> * T_Operand1;

	/* An array of integer represent the second operands
	* of a SIGEL robot control program.
	* -1 symbolize a invalid value/ operand.
	* @pre: the MT_Translator produce this array
	* @post: the destructor of this class must destroy this array
	*/
	QArray<int> * T_Operand2;


	/* An array of integer represent the MetaData
	* about a SIGEL robot control program
	* Metadata are the lenght of the Sigel Program and the 
	* number of the difference Instructions
	* in the case of the classifier: 
	*/
	QArray<int> * MetaData;

	
	/* constructor
	*/
	MT_TranslatedIndividual();
	
	/* the constructor receive the three array (per pointers) for 
	* representing the SIGEL robot control program thought the MT_Translator 
	* @pre the three arrays must existed 
	*/
	MT_TranslatedIndividual (QArray <int> * T_Instruk, QArray<int> * T_OperOne, QArray<int>  * T_OperTwo, QArray<int>  * MData );

	/* The constructor of a translated individual, used for initializing
	* a translated individual with data from a Qstring, by loading an experiment.
	* @pre: There is individual data, encoded in a QString.
	* @post: the new translated individual is created from the data.
 	*/
	MT_TranslatedIndividual(QTextStream &File);
	

	/* the destructor
	*	the three array must destroy 
	*/
	virtual ~MT_TranslatedIndividual();
	
	/* This function writes an individual to a QTextStream.
	* @param: 
	* @post: The individual is written to the given QTextStream.
	*/
	void writeToFileTransIndi (QTextStream &File);
	
	
};

#endif // !defined(AFX_MT_TRANSLATEDINDIVIDUAL_H__5C48785A_CF39_45EF_9A94_1214783FF5E3__INCLUDED_)
