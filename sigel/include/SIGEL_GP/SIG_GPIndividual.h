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
#ifndef SIGEL_GP_SIG_GPINDIVIDUAL_H
#define SIGEL_GP_SIG_GPINDIVIDUAL_H

#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Tools/SIG_IO.h"

#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qtextstream.h>
#include <qfile.h>


namespace SIGEL_GP
{

/**
 * This class realizes the GPIndividual object, which is needed as a container for the robot
 * controll program. 
 *
 *
 */
class SIG_GPIndividual{

/**
 * The age of an individal, which depends on the time, counted by the number of played
 * tournaments, an individual is still a  poolmember.
 */
 private:
  long age;

/**
 * The history of an individual, described in this QStringList. Every event-operation adds
 * a string to the history. For detailed describtion see below.
 */
 private:
  QStringList history;

/**
 * The robot controll program, which is the created and modified randomly by the 
 * genetic operations. Its definition is placed in the SIG-Program class.
 */
 private:
  SIGEL_Program::SIG_Program prog;

/**
 * The Name of an individual. The name is created by taking parts from the name of 
 * the experiment, the name of the robot and an continuous number.
 */
 private:
  QString indName;

/**
 * This attribute holds the position of the individual in the data structure of the 
 * individuals pool.
 */
 private:
  int poolPos;


/**
 * This attribute holds the fitnessvalue of the robot controll program. At the 
 * initialization the value is -1 to show that the fitnessvalue is not actual.
 * The fitnessvalue has no max value.
 */
 private:
  double fitnessValue;

  /**
   * The constructor of an individual, used for creating an empty individual.
   * @pre
   * There must be a pool to place the individual.
   * @post
   * The individual is created and it will contain no data.
   */

 /*
 * The constructor of an individual, used for creating an individual randomly.
 * @pre
 * There must be a pool to place the individual.
 * @post
 * The individual object is created correctly, the history is updated and the individual 
 * is placed in the right place.
 * @param param
 * The parameter which contains the probabilities and datas about the individual.
 * @param random
 * A reference to the randomizer module.
 * @param name
 * The name of the new individual.
 * @param birthtime
 * The time of birth of the individual.
 * @param poolpos
 * The position where the individual is to place.
 */

 public:
 SIG_GPIndividual();


 public:
 SIG_GPIndividual( SIG_GPParameter& param, 
		   SIGEL_Tools::SIG_Randomizer& random, 
		   QString name, 
		   QDateTime birthtime, 
		   int poolpos );

 /**
 * The constructor of an individual, used for the creation of an individual after a genetic operation.
 * @pre 
 * There must be a pool to place the individual and a genetic operation occurred.
 * @post
 * An individual object is created correctly and it is placed in the correct position of the pool. The
 * history is composed of the birthtime and the constant reference to the history of the forefather.
 * @param param
 * The constant reference to the gp parameters, needed for the correct creation of the individual.
 * @param history
 * The history of the forefather.
 * @param name
 * The name of the individual, which is to be created.
 * @param birthtime
 * The birthtime of the created individual
 * @param poolpos
 * The position of the individual in the pool.
 */
 public:
 SIG_GPIndividual( SIG_GPParameter const& param, 
		   QStringList const& hist, 
		   QString name, 
		   QDateTime birthtime, 
		   int poolpos );


/**
 * The constructor of an individual that shall contain a randomly generated program.
 * @post
 * An individual is created randomly. This individual will contain a complete program.
 */  

 public:
 SIG_GPIndividual( SIGEL_Tools::SIG_Randomizer& random );

 public:
 SIG_GPIndividual( SIGEL_Tools::SIG_Randomizer &random,
		   SIGEL_GP::SIG_GPParameter& param, 
		   SIGEL_Robot::SIG_LanguageParameters& languageP );

 /**
 * The constructor of an individual, used for adding an individual to the pool, with a special 
 * robot control program from an external data store.
 * @pre
 * There is a pool object and the external robot control program, which has to be inserted 
 * in the pool.
 * @post
 * The new individual is created with the external robot control program and placed placed
 * in the right position of the pool.
 * @param prog
 * The external robot control program.
 * @param name
 * The name of the new individual.
 * @param history
 * The history, which has to be added to the new individual.
 * @param birthtime
 * The birthtime of the new individual.
 * @param poolpos
 * The position in the pool, where the new individual has to be set.
 */
 public:
 SIG_GPIndividual( SIGEL_Program::SIG_Program const& prog, 
		   QString name, 
		   QString hist, 
		   QDateTime birthtime, 
		   int poolpos );

 /**
 * The constructor of an individual, used for initializing an individual with data from a Qstring, 
 * by loading an experiment.
 * @pre
 * There is individual data, encoded in a QString.
 * @post
 * The new individual is created from the data.
 * @param data
 * The data, which describes the individual, which is to create.
 */
 public:
 SIG_GPIndividual(QString data);


 /**
  * The destructor of the GPIndividual.
  */
 public:
 ~SIG_GPIndividual();
 


 /**
  * This function increases the age value of an individual at the end of an generation by one.
  * @pre
  * There must be a number of tournaments played, which defines the change of a generation.
  * @post
  * The age of the individual is increased by one.
  */
 public:
 void increaseAge();

 /**
 * This function returns the age of an individual.
 * @return
 * The age of the individual given as a integer.
 */
 public:
  long getAge() const;

/**
 * This function imports a robot control program to individual.
 * @pre
 * There is an individual, with no robot control program or with one which can be overwrite.
 * @post
 * The individual contains the importet robot control program.
 * @param prog
 * The robot control program, which has to be importet to an individual.
 */
 public:
 void importProgram(SIGEL_Program::SIG_Program& prog );

  /**
 * This function returns the name of an individual.
 * @return
 * The name of an individual, given as a QString.
 *
 */
 public:
 QString getName();

  /**
 * This function loads an individual from a set of data which describes an complete individual.
 * @post
 * A new individual is created containing the data given by the parameter.
 * @param data
 * The data which describes an individual which has to be created.
 */
 public:
 void loadIndividual(QString data); 

  /**
 * This function converts an individual object in a QString.
 * @param history
 *	select true if you want to save the history, else select false
 * @return
 * A QString representing the values of the individual object and
 * the returned QString can be used as a parameter for the 
 * loadIndividual function.
 */
 public:
 QString saveIndividual(bool history);
 
 /**
 * This function copies an individual object and sets its position within the 
 * population.
 *
 */
 
 public:
 void copyIndividual(SIGEL_GP::SIG_GPIndividual &cInd,int poolPos);  
 
 /**
 * This function returns the position of an individual in a pool.
 * @pre
 * There is a pool, containing the considered individual
 * @return
 * The position of the individual in the pool given by an integer.
 */

 public:
 int getPoolPos() const;

  /**
 * This function returns the robot control program, whichs is part of
 * the considered individual object.
 * @pre
 * There is an robot control program contained in the considered individual
 * object.
 * @return
 * The robot control program contained in the considered individual
 * object.
 */
 public:
 SIGEL_Program::SIG_Program const& getProgram() const;
 
  /**
 * This function returns the robot control program, whichs is part of
 * the considered individual object, for modifications.
 * @pre
 * There is an robot control program contained in the considered individual
 * object.
 * @return
 * The robot control program contained in the considered individual
 * object.
 */

 public:
 SIGEL_Program::SIG_Program& getProgramVar();



  //SIGEL_Program::SIG_Program const& SIGEL_GP::SIG_GPIndividual::getProgram() const{};

  /**
 * With this function, the fitness value of a robot control program contained 
 * in the considered individual object, can be set.
 * @pre
 * There is a robot control program contained in the considered individual
 * object, which is evalueted by an simulation run. Otherwise the fitness value
 * is set to -1.
 * @post
 * The fitnessvalue of the considered individual is set to the value of the
 * parameter.
 * @param fit
 * The value which has to be set to the fitnessvalue of the considered individual.
 */
 public:
 void setFitness(double fit);

  /**
 * This function returns the value of the fitness of the considered individual
 * object.
 * @return
 * The fitness value of the considered individual given as a double.
 */
 public:
 double getFitness() const;

  /**
 * This function returns the history of the considered individual object.
 * @return
 * The history of the considered individual object.
 */
 public:
 QStringList getHistory() const;

  /**
 * This function adds informations about the mutation of the robot control
 * program of the considered individual object to the object.
 * @pre
 * A mutation of the robot control program of the considered individual has
 * occurred.
 * @post
 * The information is added to the history.
 * @param name
 * The name of the individual.
 * @param time
 * The time of the occurred mutation.
 * @param mutpoint
 * The mutation point, given as an integer.
 */
 public:
 void addMutationInfo(QString name, QDateTime time, int mutpoint);

 /**
  * This function adds an information to the history.
  */

 public:
 void addMutationInfo(QString name, QDateTime time, int mutpoint, double fit);
  /**
 * This function adds informations about the crossover of two robot control
 * programs, of two individual objects to one of them.
 * @pre A crossover of two robot control programs of two individual has
 * occurred.
 * @post
 * The information is added to the history of the considered individual.
 * @param name1
 * The name of the first individual which was involved in the crossover.
 * @param name2
 * The name of the second individual which was involved in the crossover.
 * @param time
 * The time when the crossover happened.
 * @param xoverpnt1
 * The crossover point of the first robot control program
 * @param xoverpnt2
 * The crossoverpoint of the second robot control program.
 */
 public:
 void addCrossOverInfo(QString name1, QString name2, QDateTime time, int xoverpnt1, int xoverpnt2);

 /**
  * This function adds the crossover information to the history. This function
  * also presents the fitness value of the parents, which genetic materials have
  * been recombined.
  */

 public:
 void addCrossOverInfo(QString name1, 
                                                  QString name2, 
                                                  QDateTime time, 
                                                  int xoverpnt1, 
                                                  int xoverpnt2,
						  double fitness1,
						  double fitness2 );



 /**
  * This function will add an info to the history if the program length needed to be increased,
  * e.g. in case of an imported program.
  */

 public:
 void addLengthIncreasedInfo( QDateTime time, long lgth );

 /**
  * This function will add an info to the history if the program length needed to be decreased,
  * e.g. in case of an imported program.
  */

 public:
 void addLengthDecreasedInfo( QDateTime time, long lgth );
 

 /**
  * This constructor imports a program PROG while it is initializing the individual.
  */


 public:
 SIG_GPIndividual( SIGEL_Program::SIG_Program const& prog, 
		   QString name1, 
		   QString name2, 
		   QString name3, 
		   QDateTime birthtime, 
		   int poolpos,
		   QStringList const& hist );

  /**
   * This function adds information about the reproduction of the robot
   * control program of the considered individual object to the object.
   * @pre
   * A reproduction of the robot control program of the considered individual 
   * has occurred.
   * @post
   * The information is added to the history.
   * @param name
   * The name of the individual.
   * @param time
   * The time of the occurred mutation.
   */

 public:
 void addReproductionInfo(QString name, QDateTime time);

 /**
  * This function adds an information to the history about the import of a complete individual.
  */

 public:
 void addImportIndividualInfo(QDateTime time);

 /**
  * This function adds an information to the history about the import of a new program.
  */

 public:
 void addImportProgramInfo(QDateTime time);

 /**
  * This function prepares the history for an import entry. If this preparation is not
  * made, an import is difficult to recognize within the history data.
  */

 public:
 void addPreparationOfHistoryInfo();
 
 /**
  * With this function the status of the fitnessvalue of the individual
  * can be checked.
  * @return
  * The answer of the questions:"Is the fitnessvalue up to date?".
  */

 public:
 bool upToDate() const;

 /**
  * This function returns a pointer to the program that is contained by the individual.
  * @return
  * The pointer (of the contained) program will be returned.
  */

 public:
 SIGEL_Program::SIG_Program *getProgramPointer();

 /**
  * This function sets the positions of an individual in the pool of individuals.
  * @post
  * The individual's pool position is set to the given value.
  */

 public:
 void setPoolPos(int pp);

 /**
  * This function sets the age of an individual.
  * @post
  * The individual's age is set to the given value.
  */

 public:
 void setAge(long a);

 /**
  * This function sets the name of an individual.
  * @post
  * The individual's name is set to the given string.
  */ 

 public:
 void setName(QString n);

 /**
  * This function writes an individual to a QTextStream.
	*	@param &file
	*		specifies a pointer to the output stream
  * @param history
  *		select true if you want to save the history, else select false
  * @post
  * The individual is written to the given QTextStream.
  */ 
 

 public:
 void writeToFile(QTextStream &file,bool _history);

 /**
  * This function reads an individual out of a QString.
  * @post
  * An individual is created as defined in the given QTextStream.
  */ 
 

 public:
 void readFromFile(QString indStr);
 

 /** 
  * This operator (I1=I2) copies an individual I2 into individual I1.
  * @post
  * I1 is equal to I2. I1 is a completely new individual, not only a
  * further pointer to I2.
  */

 public:
 SIGEL_GP::SIG_GPIndividual& operator =(SIGEL_GP::SIG_GPIndividual& ind); 

 public:
 void generateRandomIndividual(SIGEL_Tools::SIG_Randomizer& random,
                               SIGEL_GP::SIG_GPParameter& param, 
                               SIGEL_Robot::SIG_LanguageParameters& languageP);
/**
  * This function append new Information about the current increased Age 
  * of the individual to the History.
  * @post
  * A new Age is increased.
  */

public:
 void increaseAgeInfo();

/**
  * This function append new Information about the current Fitness Value 
  * of the individual to the History.
  * @post
  * A new Fitness is set.
  */

public:
 void setFitnessInfo();

/**
  * This function append new Information about the current positions 
  * of the individual to the History.
  * @post
  * A new pool position is set.
  */

public:
 void setPoolPosInfo();

/**
  * This function append new Information about the current Age 
  * of the individual to the History.
  * @post
  * A new Age is set.
  */

public:
 void setAgeInfo();

/**
  * This function append new Information about the current Name 
  * of the individual to the History.
  * @post
  * A new Name is set.
  */

public:
 void setNameInfo();

 
/**
 * This function imports a complete individual out of an existing file.
 */

public:
 void importIndividual( QString& filename );

/**
 * This function exports a complete individual to an existing file.
 */

public:
 void exportIndividual( QString& filename );

/**
 * This function is only for test purposes and prints the current individual's data
 * to stderr.
 */ 

public:
 void print();

 /**
  * This function imports a program from a file. An already existing program will be
  * replaced by the loaded one.
  */

public:
 void importProgram( QString& filename );

 /**
  * This function exports the program of the current individual to disk.
  */

public:
 void exportProgram( QString& filename );

 /**
  * This function sets the history of the current individual to the given one. This
  * function is only used in combination with genetic operators.
  */

public:
 void setHistory( QStringList hist );

};
}

#endif // SIGEL_GP_SIG_GPINDIVIDUAL_H












