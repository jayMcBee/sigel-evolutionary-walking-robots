// MT_Program.cpp: implementation of class MT_Program.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Program.h"

//////////////////////////////////////////////////////////////////////
// administrative method 
//////////////////////////////////////////////////////////////////////

MT_Program::MT_Program()
{
	LastError= 0;
	Program  = 0;
	Length   = 0;
}

MT_Program::~MT_Program()
{
	LastError=0;
	for(int i=0; i<Length; i++)
		delete Program[i];
	delete []Program;
}

MT_Program::MT_Program (MT_Randomizer *Randi)
{
	LastError= 0;
	Length   = 0;
	Program  = 0;
	int nLength = Randi->getProgLength();
	MaxLength = Randi->getProgramLengthMax();
	resize(nLength);
		
	for(int i=0; i<Length; i++)
	{
		Program[i] = new MT_Programline(Randi);
	}

}


MT_Program::MT_Program(QTextStream &File)
{
	LastError= 0;
	Length   = 0;
	Program  = 0;
	int nLength = 0;
	QString ProString ("Program:" );
	QString PresentLine = File.readLine();
	while ((PresentLine != ProString) && !(File.atEnd()))
		PresentLine = File.readLine();
	if ((PresentLine == ProString) && !(File.atEnd()))
	{
		MaxLength= (File.readLine()).toInt();
		nLength = (File.readLine()).toInt();
	
		resize(nLength);
		for(int i=0; i<Length; i++)
		{
			Program[i] = new MT_Programline(File.readLine());
		}
	}
	else
		LastError=360;
	
}

MT_Program::MT_Program(MT_Program *Prog)
{
	LastError=0;
	Length  = 0;
	Program = 0;
	int nLength = Prog->Length;
	MaxLength = Prog->MaxLength;
	
	resize(nLength);
	for(int i=0; i<Length; i++)
		Program[i] = new MT_Programline(Prog->getProgramLine(i));

}



MT_Program::MT_Program(MT_Program *Prog, int Start, int End)
{
	LastError=0;
	Length  = 0;
	Program = 0;
	int nLength = End-Start+1;
	MaxLength = nLength;
	resize(nLength);
	
	for(int i=0; i<Length; i++)
	{
		Program[i] = new MT_Programline(Prog->getProgramLine(i+Start));
	}
}

void MT_Program::writeToFileProgram(QTextStream &File)
{
	File << "Program:\n";
	File << MaxLength << Qt::endl;
	File << Length << Qt::endl;

	if (Program != NULL)
		for (int i=0; i<Length; i++)
			if (Program[i] != NULL)
				(Program[i])->writeToFileProgramLine(File);

	File << Qt::endl;
}

//////////////////////////////////////////////////////////////////////
// get method 
//////////////////////////////////////////////////////////////////////

MT_Programline* MT_Program::getProgramLine(int Index)
{
	if(Index<0 || Index>=Length)
		return NULL;

	return Program[Index];
}

int MT_Program::getLength()
{
	return Length;
}


int MT_Program::getMaxLength()
{
	return MaxLength;
}


//////////////////////////////////////////////////////////////////////
// get method 
//////////////////////////////////////////////////////////////////////

void MT_Program::changeMaxNumVariable(int NewNum)
{	
	for(int i=0; i<Length; i++)
		(*Program[i]).changeMaxNumVariable(NewNum);	
}




void MT_Program::changeMaxProgLen(int NewLen)
{
	MaxLength = NewLen;

// if the MaxLength reduce, the excess programline will delete!
	if (MaxLength < Length)
	{
		for (int i=MaxLength; i<Length; i++) 
		{
			delete Program[i];
			Program[i] = 0;
		}
		resize(MaxLength);
		Length = MaxLength;
	}
//	Program.resize(MaxLength);
}


/* append the given program to the present program.
* If the Lenght of the Program > MaxLen then the program will be reduced
* and the excess programline will delete!
* Attention: Part must (maybe) delete by the called function!
* @post: the Part is emptily!  
*/

void MT_Program::insertProg(MT_Program *Part)
{
	
	int NewLength = Length+Part->getLength();
	if (NewLength <= MaxLength)
	{
		// the whole part-program was the right size to insert.     
		int oldSize = Length;
		resize(NewLength);
		
		for (int i=0; i<Part->getLength(); i++)
			Program[oldSize+i] = Part->getProgramLine(i);
		
		Part->clearProgram();
		Length = NewLength;
	}
	else 
	{
		int oldSize = Length;
		resize(MaxLength);
		NewLength= MaxLength-oldSize; // How many new program lines can be inserted before MaxLength is reached

		for(int i=0; i<NewLength; i++)
			Program[oldSize+i] = Part->getProgramLine(i);
				
		for(int i=NewLength; i<Part->getLength(); i++)
			delete (Part->getProgramLine(i));
		
		Part->clearProgram();
		
	}
}

/***
 * sets all elements to null; doesn't delete the elements!!!
 ***/
void MT_Program::clearProgram()
{

	for(int i=0; i<Length; i++){
		Program[i] = 0;
	}
}

int MT_Program::getLastError()
{
	return LastError;

}

/***
 * resizes the data-array
 **/
void MT_Program::resize(int nSize)
{
	if(nSize != Length)
	{
		MT_Programline **tmpArray = new MT_Programline*[nSize];	// get memory for the resized array

		if(nSize < Length)
		{	
			// delete the data that doesn't fit in the new array
			for(int i=nSize; i<Length; i++)
			{
				delete Program[i];
				Program[i]=0;
			}
			
			Length = nSize; 
			// copy the data to the resized array
			for(int i=0; i<nSize; i++)
			{
				tmpArray[i] = Program[i];
				Program[i] =0;
			}
			
		} 
		else
		{			
			// copy the data to the resized array
			for(int i=0; i<Length; i++)
			{
				tmpArray[i] = Program[i];
				Program[i] =0;
			}
			// initialize the empty slots with null
			for(int i=Length; i<nSize; i++)
				tmpArray[i] = 0;
		}
	
		
		// make the resized array the used array
		delete[]  Program;
		Program = tmpArray;
		Length = nSize;
	}
}