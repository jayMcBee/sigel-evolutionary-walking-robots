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
#ifndef SIGEL_GP_SIG_GPPOPULATION_H
#define SIGEL_GP_SIG_GPPOPULATION_H

#include <QList>
#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_Tools/SIG_Randomizer.h"
#include"../SIGEL_GP/SIG_GPParameter.h"
#include"SIGEL_Robot/SIG_LanguageParameters.h"


#include <qstring.h>
#include <qtextstream.h>

namespace SIGEL_GP
{
/**
* The SIG_GPPopulation class defines the datastructure of the population of 
* the individuals in the genetic programming system. It uses the randomizer
* for the randomly creation of new individuals. The name of an new individual
* can be taken from the attribute nextidentifier, which contains the
* continuos name for new individuals. There are also other operations like
* sort, which sorts the pool.
*/

class SIG_GPPopulation
  {

    /**
     * This is the main datastructure in the SIG_GPPopulation. This QVector
     * holds the individual objects, which contains the robot control program.
     */
  private:
    QList<SIG_GPIndividual *> pool;

    /**
     * The is a refernce of the SIG_Randomizer object of the GPManager. It is
     * used for the randomly individual creation and the use of the 
     * genetic generation.
     */
  private:
    SIGEL_Tools::SIG_Randomizer *randomizer;

  private:
  	/**
  		*	This attribute holds the information whether the history for each individual should
  		*	be saved whithin the experiment. True enables the history, false disables the history.
  		*/
   bool history;

    /**
     * This attribute contains the next free name for a new individual.
     * It is assembled from the experiment name, an continuos number at
     * the end and other characters, which are needed to build funny
     * individual names.
     */
  public:
    QString nextIdentifier;

    /**
     * The value, which indicates how old a population is. A generation 
     * change occurs after a special number of played tournaments, default
     * is the half of the individuals. Is the number of tournaments reached,
     * the generation value of the population is increased by one and all
     * ages of the individuals are increased by one too.
     */
  public:
    int poolGeneration;
     
     /**
     * The constructor of a population, which builds an empty pool with
     * default value initialisation.
     * @pre
     * A new population is demanded by the user.
     * @post
     * The population object is created and all attributes are set to default 
     * values.
     */
  public:
    SIG_GPPopulation();

    
    /**
     * The constructor of a population, which builds a pool with a number
     * of randomly created individuals.
     * @pre
     * A new population is demanded by the user and he knows how many
     * individuals he wants the population to contain in front.
     * @post
     * The population object is created and contains the demanded number of
     * randomly created individuals.
     * @param size
     * The number of individuals, which have to be constructed by the 
     * construction of the pool.
     */
  public:
    SIG_GPPopulation(int size);

    /**
     * This constructor should be the STANDARD constructor to create a pupoluation consisting of
     * a number of SIZE individuals. All individuals are created as defined in the language and
     * GP parameters.
     * @pre
     * A randomizer, the language and GP parameters must exist.
     * @post
     * A population consisting of SIZE individuals is created. Each individual contains a randomly
     * created program.
     */


  public:
    SIG_GPPopulation(int size, 
		     SIGEL_Tools::SIG_Randomizer &r, 
		     SIGEL_GP::SIG_GPParameter& param, 
		     SIGEL_Robot::SIG_LanguageParameters& languageP);

    /**
     * The constructor of an population, which is constructed from a QString. 
     * @pre
     * The user want to construct a population from a QString.
     * @post
     * The population object is constructed correctly, due to the data from
     * the QString.
     * @param data
     * The data represent a population encoded in a QString.
     */
  public:
    SIG_GPPopulation(QString data);

    /**
     * The destructor of a population object.
     * @pre
     * A termination condition is fulfilled and the genetic programming 
     * system is forced to break down.
     * @post
     * The population object is destructed correctly.
     */
  public:
    ~SIG_GPPopulation();

    /**
     * The operation sorts the pool, due to the value of the fitness of the
     * individuals. The individuals with the highest fitness value is placed
     * on top of the pool, which means position zero, and so on.
     * @post
     * The population is sorted, with the individual with the highest 
     * fitness value on top.
     */
  public:
    void sort();

    /**
     * This operation returns an individual, which is placed in the population
     * on position poolpos.
     * @pre
     * An individual is needed, for example for a genetic operation.
     * @post
     * A refernce to the demanded individual is returned.
     * @param poolpos
     * The position of the demanded individual in the pool.
     */
  public: 
    SIG_GPIndividual& getIndividual(int poolpos);

    /**
     * This operation sets an individual to the demanded position in the 
     * population.
     * @pre
     * A new individual, for example after a genetic operation, have to be
     * set in the population.
     * @post
     * The individual is set to the population. A previous individual at this position will be deleted!
     * @param indi
     * A reference to the individual will be set to the population.
     * @param poolpos
     * The position in the population where the individual has to be set.
     */
  public:
    void setIndividual(SIG_GPIndividual& indi, int poolpos);



    /**
     * This operation returns the number of individuals, which are members of
     * the population.
     * @return
     * The size, which means the number of individuals, of the population.
     */
  public:
    int getSize();
    
    /**
     * This operation returns the name for the next new individual. The 
     * individual names are continous and so, it is necessary to controll
     * the name giver operation to ensure that the right names for new
     * individuals are taken. 
     * @pre
     * In a genetic operation or in the addRandomIndividual operation is
     * a new individual created and therefore  new name is needed.
     * @return
     * A new name for a new individual is returned.
     */
  public:
    QString getNextIdentifier();

    /**
     * This operation sets a new identifier for an experiment. The operation
     * is usefull in the terms of creating a new experiment.
     * @pre
     * A new experiment is created and a new continous name for the individual
     * is needed.
     * @post
     * The attribute NextIdentifier is set to the parameter.
     * @param identifier
     * A set of strings, which is the new base for the individual names.
     */
  public:
    void setNextIdentifier(QString identifier);

    /**
     * This operation returns the number of generations of the population,
     * or how long the pool exists.
     * @return
     * The number of generations, which indicates how long a opoulation exists.
     */
  public:
    int getPoolGeneration();

    /**
     * This operation accepts a qtextstream is an input, to build a new 
     * population.
     * @post
     * The population is rebuilded containing the data of the qtextstream.
     * @param pool
     * The population, which has to be loaded, encoded in a qtextstream.
     */
  public:
    void loadPool(QTextStream & pool);

    /**
     * This operation returns a qtextstream, in which the data of the 
     * population is encoded.
     * @post
     * The data of the population is encoded in a qtextstream.
     * @param pool
     * A qtextstream, which contains the data of a population.
     */
  public:
    void savePool(QTextStream & pool);

    /**
     * This constructor creates a population that contains a number of randomly created individuals.
     * @post
     * A population with the given number of individuals is created. All individuals will also contain
     * complete programs.
     */


  public:
    SIG_GPPopulation(int size, SIGEL_Tools::SIG_Randomizer &r);

    /**
     * This function returns the pointer to an individual.
     * @return
     * The pointer to the current individual will be returned.
     */  


  public:
    SIGEL_GP::SIG_GPIndividual *getIndividualPointer(int poolpos);

    /**
     * This function deletes an individual.
     * @post
     * The individual at position poolpos will be deleted. The individuals at the following pool positions
     * will be moved a position backwards.
     */

  public:
    void deleteIndividual(int poolpos);

    /**
     * This function sets the generation number of a pool.
     * @post
     * The generation number will be set to the given value.
     */

  public:
    void setPoolGeneration(int pGen);

    /**
     * This function sets a randomizer for all random operations.
     * @post
     * The randomizer will be set to the given randomizer
     */    

  public:
    void setRandomizer(SIGEL_Tools::SIG_Randomizer *r);

    /**
     * This function returns the pointer of the randomizer all random operations are connected with.
     * @return
     * The pointer of the randomizer will be returned.
     */


  public:
    SIGEL_Tools::SIG_Randomizer getRandomizer();

    /**
     * This function is obsolete (and will be deleted later)
     */

  public:
    SIGEL_Tools::SIG_Randomizer *getRandomizerPointer();

    /**
     * This function writes a complete population (including all individuals and their programs) to a text stream.
     * @pre
     * The QTextStream file must exist.
     * @post
     * The current population will be written to the given text stream.
     */


  public:
    void readFromFile(QTextStream &file);


    /**
     * This function reads a complete population out of a given text stream.
     * @pre
     * The QTextStream file must exist, and it has to contain a complete population.
     * Either this text stream has been created by writeToFile, or the stream has
     * been edited by the user. The stream has to have a defined format, which is
     * described more in detail in the corresponding specification.
     * @post
     * The population, which is described in the text stream, has been transfered
     * (and translated) into (new created) individuals. All neccessary data is
     * prepared. WARNING: An existing population will be deleted by calling this
     * function.
     */

  public:
    void writeToFile(QTextStream &file); 

    /**
     * This operation adds randomly created individuals to the pool. First
     * the individuals are created and than they are appended to the population
     * of the current experiment.
     * @pre
     * The user wants to add some individuals to the population, which can be
     * empty or already filled with individuals.
     * @post
     * The demanded quantity fo individuals are appended to the population.
     * @param quantity
     * The quantity expresses how many individuals are to be added to the pool.
     */


  public:
    void addRandomIndividuals(int quantity, 
			      SIGEL_GP::SIG_GPParameter& param, 
			      SIGEL_Robot::SIG_LanguageParameters& languageP);
    
    /** 
     * This function searches for the best fitness within the population. It returns the best found fitness and the position
     * of the intdividual the fitness belongs to. HIGH = TRUE defines a high fitness to be a better fitness. Vice versa, 
     * HIGH=FALSE defines a low fitness to be better fitness.
     */

  public:
    double getBestFitness(bool high);

  public:
    double getWorstFitness(bool high);

  public:
    double getAverageFitness();

  public:
    void resetPool();

    /**
     * This function imports a new individual. A new individual will be created automatically before. Hence, the population
     * size will be increased.
     */

  public:
    void importNewIndividual( QString& filename );

    /**
      * This is the set method for the variable history.
      */
    void setHistory(bool _history);
    bool getHistory();
};
}
#endif //  SIGEL_GP_SIG_GPPOPULATION_H










