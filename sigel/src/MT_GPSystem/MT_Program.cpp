/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include "MT_GPSystem/MT_Program.h"

// administrative method

MT_Program::MT_Program()
{
	LastError = MT_ErrorCode::None;
	Length   = 0;
}

MT_Program::~MT_Program()
{
	LastError = MT_ErrorCode::None;
	for(int i=0; i<Length; i++)
		delete Program[i];
}

MT_Program::MT_Program (MT_Randomizer *Randi)
{
	LastError = MT_ErrorCode::None;
	Length   = 0;
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
	LastError = MT_ErrorCode::None;
	Length   = 0;
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
		LastError = MT_ErrorCode::ProgramSectionMissing;
}

MT_Program::MT_Program(MT_Program *Prog)
{
	LastError = MT_ErrorCode::None;
	Length  = 0;
	int nLength = Prog->Length;
	MaxLength = Prog->MaxLength;
	
	resize(nLength);
	for(int i=0; i<Length; i++)
		Program[i] = new MT_Programline(Prog->getProgramLine(i));
}

MT_Program::MT_Program(MT_Program *Prog, int Start, int End)
{
	LastError = MT_ErrorCode::None;
	Length  = 0;
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

	for (int i=0; i<Length; i++)
		if (Program[i] != nullptr)
			(Program[i])->writeToFileProgramLine(File);

	File << Qt::endl;
}

// get method

MT_Programline* MT_Program::getProgramLine(int Index)
{
	if(Index<0 || Index>=Length)
		return nullptr;

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

// get method

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
			Program[i] = nullptr;
		}
		resize(MaxLength);
		Length = MaxLength;
	}
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
		Program[i] = nullptr;
	}
}

MT_ErrorCode MT_Program::getLastError()
{
	return LastError;
}

/***
 * resizes the data-array; deletes the lines that no longer fit
 **/
void MT_Program::resize(int nSize)
{
	for(int i=nSize; i<Length; i++)
		delete Program[i];

	Program.resize(nSize, nullptr);
	Length = nSize;
}
