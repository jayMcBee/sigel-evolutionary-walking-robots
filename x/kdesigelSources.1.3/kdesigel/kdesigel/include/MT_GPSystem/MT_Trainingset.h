// MT_Trainingset.h: Schnittstelle für die Klasse MT_Trainingset.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_TRAININGSET_H__EAB6FC95_1FB5_4AF2_851B_A67AFC617337__INCLUDED_)
#define AFX_MT_TRAININGSET_H__EAB6FC95_1FB5_4AF2_851B_A67AFC617337__INCLUDED_


#include "MT_GPSystem/MT_TranslatedIndividual.h"
#include "MT_GPSystem/MT_TrainingCase.h"
#include <qstring.h>
#include <qtextstream.h>
#include <qqueue.h> 
 


/* The class MT_Trainingset (stenographic T-set) administer the 
* training - cases for the GP-System.
*/
class MT_Trainingset  
{
public:
	int getName();
	
	int getPresentTSize();
	
	/* This function insert a given T-case to the T-set.
	* @ Pre: there is a T-set.
	* @ Post: the TSize is not allowed to change, therefore 
	* perhaps it's necessary to destroy a senior T-case.
	*/
	void insertTCase (MT_TrainingCase *Tcase);

	/* getTCase supply a pointer of the MT_TrainingCase, 
	*which is specify by the given integer (indicate the Position of the T-Case in the T-Set).
	*/
	MT_TrainingCase* getTCase(int PositionTcase);
	

	/* updateTSet receive a pointer of a QQueue, which contain new T-cases. 
	*The new T-cases result from the MT_Substitute. 
	*/
	void updateTSet (QQueue<MT_TrainingCase> *NewTCases);
	
	/* changeTSize change the Size of T-set especially
	* the Size of the QArray and the TSize.
	* This is necessary by a User variance of the T-set-size in the GUI.
	*/
	void changeTSize (int NewTSize);

	/* This function writes the MT_TrainingsSet (T-set) to a QTextStream.
	* @param:
	* @post: The T-Set is written to the given QTextStream.
	*/
	void writeToFileTSet (QTextStream &File);
	
	/* the constructor receive the TName and the TSize usually from the MT_Fitness Trainer. 
	*/
	MT_Trainingset (int TSi, int TNa);
		
	/* The constructor initializing MT_Trainingset with the data from a Qstring,
	* by loading an experiment.
	* @pre: There is TrainingSet data, encoded in a QString.
	* @post: a new TrainingSet is created from the data.
	*/
	MT_Trainingset (QTextStream &File);
	
	MT_Trainingset();
	
	virtual ~MT_Trainingset();

private:
	int FreePosition;

 /* this private function delete the first num TCase in the QArray 
	*	and move up the other TCase;
    * @pre: num<PresentTSize;
	* @post: PresentTSize= PresentTSize-num
	* @post: the first num Tcases will delete
	*/
	

	/* indicate the present number of T-cases in the Array*/ 
	int PresentTSize;
	/* The name serve for identification of the T-Set by fitness estimation*/ 
	int TSetName;
	/* The TCases is a QArray which contain pointers  the current T-cases */
	QArray<MT_TrainingCase *> TCases;
	/* The TSize denote the max number of the T-Cases in the QArray */
	int TSize;
};

#endif // !defined(AFX_MT_TRAININGSET_H__EAB6FC95_1FB5_4AF2_851B_A67AFC617337__INCLUDED_)
