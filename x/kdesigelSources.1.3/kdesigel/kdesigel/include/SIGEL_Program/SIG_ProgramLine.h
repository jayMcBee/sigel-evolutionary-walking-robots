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
/* ----------------------------------------------------------------------- */
/* CLASS:     SIG_ProgrammLine.h                                           */
/* FUNCTION:  This class contains all functions that are needed to work    */
/*            with a program line (that's contained by a SIG_Program).     */
/*            Every program line is a sequence of numbers, which are       */
/*            depended on the line's type.                                 */
/* ----------------------------------------------------------------------- */



#if !defined(SIGEL_PROGRAMPACKAGE_SIG_PROGRAMLINE_H) 
#define SIGEL_PROGRAMPACKAGE_SIG_PROGRAMLINE_H

#include<qlist.h>
#include<qarray.h>
#include<qtextstream.h>
#include<qstring.h>

#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

namespace SIGEL_Program
   {

// -----------------------------------------------------------------------
// ENUM DEFINITION
// -----------------------------------------------------------------------

/**
 * This enumeration defines all supported sorts of robot instructions. IMPORTANT NOTE:
 * If a new instruction needs to be added, it is NOT ENOUGH to add the new instruction
 * to this enumeration! Some more steps are reuqired to make the GP system know about
 * a new supported instruction.
 */

enum Robotinstruction {

     COPY,
     LOAD,
     ADD,
     SUB,
     MUL,
     DIV,
     MOD,
     MIN,
     MAX,
     CMP,
     JMP,
     SENSE,
     MOVE,
     DELAY,
     NOP,
};


/**
 * This class represents a program line that contains robot instructions (and more).
 * A program line is a set of elements. The meaning of the elements is dependend on
 * the type of an instruction. Every instruction type defines a fix meaning for
 * every element.
 */

class SIG_ProgramLine
{

/**
 *  The type of the current robot inbstruction, e.g. LOAD, COPY, ...
 */

   protected:
   Robotinstruction instructionType;

/**
 * This array contains the elements of a program line. Elements are coded with values
 * between 0 and MAXINT. The meaning of these elements is dependend on the instruction
 * type. Elements can represent registers or integer values.
 */

   protected:
   QArray< int > element;
   

/**
 * This constructor initializes an empty program line that does not contain any elements.
 * @post
 * An empty line that does not contain any data will be created.
 */

   public:
   SIG_ProgramLine();

/**
 * This constructor generates a random instruction that is dependend on the language 
 * parameters. The probability to be chosen randomly is dependend on the given probabilities (prob).
 */  

   public:
   SIG_ProgramLine( SIGEL_Tools::SIG_Randomizer &r, 
		    SIGEL_Robot::SIG_LanguageParameters &languageP,
		    QArray< int > &prob );
 
/**
 * An empty destructor.
 * @post
 * All data is destructed.
 */

   public:
   virtual ~SIG_ProgramLine();   

					    
/**
 * This function generates a random instruction that is dependend on the language 
 * parameters. The probability to be chosen randomly is dependend on the given probabilities (prob).
 */   

   public:
   void generateRandomRobotInstruction( SIGEL_Robot::SIG_LanguageParameters &languageP,
				        SIGEL_Tools::SIG_Randomizer &r,
                                        QArray< int > &prob  );
          
/**
 * This function deletes the data of a program line completely.
 * @post
 * The program line is deleted completely.
 */
 
   public:
   void clearLine();

/**
 * This instruction copies the program line source into the line destination.
 * @pre
 * The lines source and destination have to exist.
 * @post
 * The content of source has been copied into destination. WARNING: The content of destination
 * will be deleted before the copy process starts.
 */   	
 
   public:
   void copyLine( SIGEL_Program::SIG_ProgramLine *source, 
		  SIGEL_Program::SIG_ProgramLine *destination );
  
/**
 * This function generates randomly a robot instruction. The choice is depended on the
 * current language parameters and the given probabilities (prob).
 */
 
   public:
   void randomRobotinstruction( SIGEL_Robot::SIG_LanguageParameters &languageP,
			        SIGEL_Tools::SIG_Randomizer &r,
			        QArray< int > &prob );
/**
 * This function is only for test purposes and prints a program line.
 */
 
   public:
   void print();

/**
 * This function sets an element (with index no) to the value. value has to be a pre-defined
 * constant value.
 * @post
 * The element (with index no) is set to value.
 */

   public:
   void setElement( int no, int value );

/**
 * This function changes to size of the array that saves the elements.
 * @post
 * The size of the array will be changed. WARNING: Values (above the new size) are
 * deleted (and lost).
 */
 
   public:
   void resizeElements( int size );


/**
 * This function writes a program line into a QTextStream.
 * @post
 * The current program line will be added to the given QTextStream.
 */
 
   public:
   void writeToFile( QTextStream &file );

/**
 * This function will generate a program line from the read QTextStream.It will return
 * true if a robot instruction could be generated. If no defined instruction could
 * be generated, the current read line will be ignored. So comments can be used
 * within a robot program. Comments always have to use a complete line, and
 * comments are not allowed to contain any keywords (=robot instructions, e.g. ADD).
 * @pre
 * The QTextStream file must exist, and it has to contain a complete program line.
 * Either this text stream has been created by writeToFile, or the stream has
 * been edited by the user. The stream has to have a defined format, which is
 * described more in detail in the corresponding specification.
 * @post
 * The program line, which is described in the text stream, has been transfered
 * (and translated) into a (new created) program line. All neccessary data is
 * prepared. WARNING: An existing program line will be deleted by calling this
 * function.
 */
 
   public:
   bool readFromFile( QString &str, 
		      SIGEL_Program::SIG_ProgramLine *line );

/**
 * This function prepares a QString. This QString will contain the current program line.
 * @post
 * The QString will contain the current program line.
 */
   public:
   void printToString(QString &lineStr);

/**
 * This function returns the array of a line's elements
 */

   public:
   QArray< int > getElementsArray();


/**
 * This function returns the type of the current robotinstruction.
 */

   public:
   Robotinstruction getRobotinstructionType();

/**
 * This function returns the element of the current robotinstruction.
 * The first element, e.g. a register, is to be accessed with index 0.
 * In case of an invalid access, e.g. a not defined index, this function returns -1.
 * Example 1: LOAD 1,432 -> getInstructionElement(0)=1,getInstructionElement(1)=432;
 * Example 2: ADD 1,2 -> getInstructionElement(0)=1,getInstructionElement(1)=2;
 * @return
 * This function returns (in case of a valid access) the element with the index no.
 */
 
   public:
   int getInstructionElement(int no);

/**
 * This function returns the element with the index no.
 * @pre
 * The index must be smaller than the number of elements.
 * @return
 * This function returns the element with the index no (in case of a correct index)
 */
 
   public:
   int getElement(int no);

/**
 * This function gives an inormation about the number of elements at all.
 * @return
 * The number of elements will be returned.
 */
 
   public:
   int getNumberOfElements();

/**
 * This operator copies a complete program line.
 * @post
 * The current program line will be copied completely.
 */
 
   SIGEL_Program::SIG_ProgramLine& operator =(SIGEL_Program::SIG_ProgramLine const & prgLine);

/**
 * Sets the robot instrction type, e.g. SIGEL_Program::COPY, ...
 */

  public:
  void setRobotinstructionType( Robotinstruction instr );

/**
 * A robot instruction can be set explicitely by using this function. The robot instruction
 * is set to instructionType. The operands are defined by op1 and op2. If an instruction
 * only needs one operad (op1) the second operand (op2) will be ignored.
 */  

   public:
   void setRobotinstruction(SIGEL_Program::Robotinstruction instr, int op1, int op2);

/**
 * Internal function, only internal usage.
 */

   protected:
   void readTwoRegisters(QString &str,int &reg1,int &reg2);

/**
 * Internal function, only internal usage.
 */

   protected:
   void readOneRegisterAndIntegerConstant(QString &str,int &reg1,int &cnst);

/**
 * Internal function, only internal usage.
 */
 
   protected:
   void readOneRegister(QString &str,int &reg);   
   
   };

};

#endif // SIGEL_PROGRAMPACKAGE_SIG_PROGRAMLINE_H
