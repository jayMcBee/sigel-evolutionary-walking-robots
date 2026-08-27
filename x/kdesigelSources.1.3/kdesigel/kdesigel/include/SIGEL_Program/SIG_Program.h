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
/* CLASS:     SIG_Programm.h                                               */
/* FUNCTION:  This class contains all functions that are needed to manage  */
/*            program lines.                                               */
/*            export member functions.                                     */
/* ----------------------------------------------------------------------- */



#if !defined(SIGEL_PROGRAMPACKAGE_SIG_PROGRAM_H) 
#define SIGEL_PROGRAMPACKAGE_SIG_PROGRAM_H


#include "compat/q2compat.h"
#include<qtextstream.h>
#include<qstring.h>

#include<vector>

#include "SIG_ProgramLine.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
namespace SIGEL_GP { class SIG_GPParameter; }
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Tools/SIG_IO.h"



/**
 * This namespace represents all classes that are needed to evolve programs with
 * a Genetic Programming System.
 */


namespace SIGEL_Program
   {

/**
 * This class represents a complete program that is evolved in an evolutionary
 * process by a Genetic Programming System. A program consists of program lines
 * that contain the data of the correponding program instructions. The program
 * lines are saved in a (Q)list, so the program supports all neccessary functions
 * to work with such a list (of program lines). All program lines can be accessed
 * with an index. The first line has the index 0.
 */
#ifdef _WINDOWS
using namespace std;
#endif

class SIG_Program
{

 protected:
 std::vector< SIGEL_Program::SIG_ProgramLine* > lines;

/**
 * This operator copies a complete program.
 */
   
   public:
   SIGEL_Program::SIG_Program& operator =(SIGEL_Program::SIG_Program& prg);

/**
 * This constructor creates an empty program (that contains no program lines).
 * @post
 * An empty program will be created and is ready to work with, e.g. for appending
 * program lines etc.
 */
   public:
   SIG_Program();

/**
 * This constructor creates a program. Its properties are dependend on the language and GP parameters.
 * @pre
 * The language and GP parameters must exist.
 * @post
 * A program, which properties are defined by the mentioned parameters, is created.
 */

   public:
   SIG_Program(SIGEL_GP::SIG_GPParameter &param, 
	       SIGEL_Robot::SIG_LanguageParameters &languageP, 
	       SIGEL_Tools::SIG_Randomizer& random);

/**
 * This destructor deletes all program lines and all data that belongs to the current program.
 * @post
 * All data will be deleted.
 */
   public:
   virtual ~SIG_Program();


/**
 * This function returns the list of programlines.
 */
 
 public:
 std::vector< SIGEL_Program::SIG_ProgramLine* > &getPrgLines();


/**
 * This functions write a complete program to a text stream.
 * @pre
 * The QTextStream file must exist.
 * @post
 * The current program will be attached to the given text stream.
 */
  public:
  void writeToFile(QTextStream &file);

/**
 * This function reads a program out of a given text stream. 
 * @pre
 * The QTextStream file must exist, and it has to contain a complete program.
 * Either this text stream has been created by writeToFile, or the stream has
 * been edited by the user. The stream has to have a defined format, which is
 * described more in detail in the corresponding specification.
 * @post
 * The program, which is described in the text stream, has been transfered
 * (and translated) into (new created) program lines. All neccessary data is
 * prepared. WARNING: An existing program will be deleted by calling this
 * function.
 */
   public:
   void readFromFile(QTextStream &file);

/**
 * This function transfers a complete program into a QString. The string can
 * displayed, e.g. with cout.
 * @post
 * The QString contains the complete program.
 */
   public:
   void printToString(QString &str);

/**
 * This function deletes a complete program.
 * @post
 * The program is deleted and contains no program lines.
 */
   public:
   void clear();

/**
 * This function has only been implemented for test purposes. It is obsolete.
 * @post
 * The program will be printed to stdout.
 */
   public:
   void print();


/**
 * This function returns the length of the current program.
 * @return
 * The returned long is the current program length.
 */
   public:
   long getProgramLength();

/**
 * This functions returns a program line.
 * @pre
 * A program (with program lines) must exist.
 * @return
 * A pointer to the corresponding program line will be returned. If this line is
 * not existing, the functions returns 0.
 */
   public:
   SIGEL_Program::SIG_ProgramLine *getLine(long no);

/**
 * This function deletes the line with the index no.
 * @pre
 * A program (with program lines) must exist.
 * @post
 * The line (with index no) will be deleted. The following lines will get a
 * new index (oldindex - 1),.
 */
   public:
   void deleteLine(long no);

/**
 * This function appends a line to program. It can also be the first program
 * line.
 * @pre

 * The program line to append has to be created (with new) before.
 * @post
 * The program line will be appended at the end of the program. The program
 * length will be increased, The new line can now be accessed with an index.
 */
   public:
   void	appendLine(SIGEL_Program::SIG_ProgramLine *l);

/**
 * This function inserts a program line at position no.
 * @pre
 * The program line to insert has to be created (with new) before.
 * @post
 * The program line will be inserted at position no. The program length will
 * be increased. The new line can now be accessed with an index,
 */
   public:
   void insertLine(long no, SIGEL_Program::SIG_ProgramLine *l);


/**
 * This constructor creates a program that contains randomly generated program lines. This constructor
 * needs a randomizer because a program does not have an own randomizer. All parameters are simulated
 * by fixed values, e.g. the length is always set to 15. This constructor should only be used for test
 * purposes.
 * @pre
 * A randomizer must exist and must be initialized.
 * @post
 * A randomly generated program will exist.
 */
   public:
   SIG_Program(int length, int nop, SIGEL_Tools::SIG_Randomizer &r);


/**
  * This function generates a random program using the language and GP definitions.
  * WARNING: An old program will be deleted.
  * @post
  * A new randomly generated program will exist.
  */

   public:
   void generateRandomProgram(SIGEL_GP::SIG_GPParameter &param, 
                              SIGEL_Robot::SIG_LanguageParameters &languageP, 
			      SIGEL_Tools::SIG_Randomizer& random);

/**
 * This function imports a complete program from an existing file.
 */

   public:
   void importProgram( QString& filename );

/**
 * This function exports a complete program to a file.
 */

   public:
   void exportProgram( QString& filename );

/**
 * This function checks if the current program length is too long or too short.
 * In case of a too long program, the obsolete program lines (>maximum) will be
 * deleted. In case of a too short program, the number of program lines will be
 * increased to mininum.
 */
  
   public:
   void checkLength( long minimum,
		     long maximum,
		     SIGEL_Tools::SIG_Randomizer &r, 
		     SIGEL_Robot::SIG_LanguageParameters &languageP,
		     QList< int > &prob,
		     int &historyInfo );
   };

};

#endif /* SIGEL_PROGRAMPACKAGE_SIG_PROGRAM_H */
