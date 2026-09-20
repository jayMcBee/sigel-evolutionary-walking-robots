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
#ifndef SIGEL_GP_SIG_GPPARAMETER_H
#define SIGEL_GP_SIG_GPPARAMETER_H

#include <QList>
#include "SIGEL_GP/SIG_GPPVMHost.h"
#include "SIGEL_Program/SIG_ProgramLine.h"

#include <qstring.h>
#include <qtextstream.h>
#include <qdatetime.h>
#include <qdir.h>
#include <qstringlist.h>
 
namespace SIGEL_GP
{
/**
* This class represents the data structure of the parameters for the
* genetic programs evolution. The parameters for the designers are
* programmed in the code, the parameters for the users are selectable
* from the gui. So every parameter the users has a set-method and 
* the parameters for the genetic programs have only a get-method.
* More flexibility will be implemented in future.
*/
  class SIG_GPParameter{

  public:
    /**
     * The type of termination condition of the GPManager.
     */
  enum terminationType
  {
    /**
     * Termination by time level.
     *
     * The GPManager will terminate all its components when a time level
     * is reached, given by the attribute terminationtime.
     */
    byTime,
    /**
     * Termination by generation.
     *
     * When the option 'byGeneration' is selected, the evolution will be 
     * computed until a selected number of generations are evolved.
     */
    byGeneration, 
    /**
     * Termination by time or by generation.
     *
     * If the option 'byTimeGeneration' is selected, the evolution will be 
     * computed until a special time is reached or a selected number of 
     * generations are evolved.
     */
    byTimeGeneration, 
    /**
     * Termination by user.
     *
     * If the option 'byUser' is selected, the evolution will be computed
     * until the user presses the stop-button.
     */
    byUser
  };

  /**
   * The priority level of the sigel-system.
   * Each level represents a set of configurations. These configurations are only to be set by
   * the function which changes the priority level.
   */

  enum priorityLevel {
    /**
     * passiveTime= 500
     * maxTouchesPerLoop= -1
     * toDoSweepsPerLoop= 1
     */
   veryLow, 

   /**
    * passiveTime= 1000
    * maxTouchesPerLoop= 1
    * toDoSweepsPerLoop= 2
    */
   low, 

   /**
    * passiveTime= 2000
    * maxTouchesPerLoop= 3
    * toDoSweepsPerLoop= 5
    */
   normal,

   /**
    * passiveTime= 5000
    * maxTouchesPerLoop= 5
    * toDoSweepsPerLoop= 7
    */
   high,

   /**
    * passiveTime= 10000
    * maxTouchesPerLoop= 7
    * toDoSweepsPerLoop= 10
    */
   veryHigh
  };


   /**
   *The integer, which is used to initializes the randomizer.
   */
  private:
  int randomSeed;

   /**
   * The minimal length of a robot controll program.
   */
  private:
  int minIndLength;

   /**
   * The maximal length of a robot controll program.
   */
  private:
  int maxIndLength;

  private:
  /**
   * This attribute specifies the time in generations after the fitness value should be reset.
   * The effect of this reseting is a new computation of the fitness value. A value of 0 disables
   * the reseting.
   */
   int resEvGen;

   /*
   * The set of terminals, used for the random creation of a robot controll 
   * program.
   *
   *  private:
   *  QStringList terminalSet;
   * 
   *
   * The set of functions, used for the random creation of a robot controll 
   * program.
   *
   *  private:
   *  QStringList functionSet;
   */

  /**
   * The probability of selecting reproduction for a genetic operation.
   */
  private:
  int reproductionProb;

  /**
  * The probability of selecting crossover for a genetic operation.
  */
  private:
  int xoverProb;

  /**
  * The probability of selecting mutation for a genetic operation.
  */
  private:
  int mutationProb;

  /**
  * The factor, which is multiplies with the number of individuals in a pool
  * to determine the number of tournaments to play in generation.
  */
  private:
  double tournamentsPerGeneration;

  /**
  * The maximal age which a individual can reach before it dies. 
  */
  private:
  long int maxAge;
  
  /**
  * The amount of time in millisecons for the GUI to preform necessary
  * computations.
  */
  private:
  unsigned long passiveTime;

  /**
  * The maximal number of executable fitnesstasks, which will be spawned
  * by PVM out of the 'toSpawnList' in the SIG_GPFitnesstrainer in one 
  * loop pass. 
  */
  private:
  int maxTouchsPerLoop;

  /**
  * The number of sweeps over the 'slaveList' in the SIG_GPFitnesstrainer
  * in one loop pass to determine which PVM-task is finished or not.
  */
  private:
  int toDoSweepsPerLoop;

  /**
  * The date and time of termination of the evolution computation.
  */
  private:
  QDateTime terminationTime;

  private:
  int terminationDurationDays;

  private:
  int terminationDurationHours;

  private:
  int terminationDurationMinutes;

  private:
  int terminationDurationSeconds;

  private:
  bool terminationUsesDate;

  /**
  * The number of generations, which have to evolve until the evolution
  * computation is to terminate.
  */
  private:
  int terminationGenerationNo;

  /**
  * The type of the termination model.
  */
  private:
  terminationType terminationModel;

  /**
   * This attribute indicates the following, if the Sigel-System runs
   * without GUI and the attribute is true, the program breaks up only
   * when a new generation is reached.
   */
  private:
  bool saveExit;

  /**
  * The directory for storing the old individuals, deleted from the pool.
  */
  private:
  QDir graveYardDirectory;

  /**
  * Boolean which indicates if the graveyard option is enabled.
  */
  private:
  bool liveUndead;


  /**
  * The directory, where to place the complete copy of the pool, the poolImages.
  */
  private:
  QDir poolImageDirectory;

 /**
  * The number of generation, which have to occure until a new poolImage will be saved.
  */
  private:
  int poolImageGeneration;

  /**
   * This value describes the level of preformance, which is demanded by the
   * SIGEL program from the system, on which it is executed.
   */
  private:
  priorityLevel priority;
  
  private:
  double maxFitness;

  private:
  bool parsimonyPressure;

  /**
   * Maximum program length during (!) GP runs: If maximumLength=0 is chosen, the length
   * control mechanism will be ignored. WARNING: The program lengths may increase very fast. 
   */

  private:
  long maximumLength;
  
  /**
   * Minimum program length during (!) GP runs: If minimumLength=0 is chosen, the minimum length
   * is set to 3.
   */

  private:
  long minimumLength;

  /**
   * Probability of the instructions (ADD,...) to be created or to be chosen for variation: The 
   * probabilities are given as an integer value that defines the relative probability of this 
   * instruction. The relative probability is dependend on the other instruction probabilities.
   * This value defines the number of places on a roulette wheel. If this value is set to p, and 
   * the total sum of all probabilties is n (= p1+p2+...), the probability of the current
   * instruction is p/n.
   */

  private:
  QList< int > instructionProb;

  private:
  QString fitnessName;

  private:
  QList< SIG_GPPVMHost * > hostList;

  private:
  int timeOutMinutes;

  /* end of the attribute declaration */




 /**
 * The constructor for the parameter.
 * @pre
 * The gpmanager or the experiment is initialized and needs an parameter.
 * @post
 * The parameter is created.
 */
 public:
  SIG_GPParameter();

 /**
 * The constructor for the parameter.
 * @pre
 * The gpmanager or the experiment is initialized and needs an parameter.
 * @post
 * The parameter is created.
 * @param parameter
 * The gpparameter as a QString for the initialization of the SIG_GPParameter
 * object.
 */
 public:
  SIG_GPParameter(QString parameter);

 /**
  *The destructor for a parameter.
  * @pre
  * none
  * @post
  * The parameteronject is destructed.
  */
 public:
  ~SIG_GPParameter();

 /**
  * This sets the randomseed of the parameter.
  * @pre
  * none
  * @post
  * The randomseed is set.
  * @param seed
  * The seed, which is to be set.
  */
 public:
 void setRandomSeed (int seed);

 /**
  * This gets the randomseed of the parameter.
  * @pre
  * none
  * @post
  * The randomseed is returned.
  * @return 
  * The randomseed.
  */
 public:
 int getRandomSeed() const;

 /**
  * This sets the minimal length of an individual of the parameter.
  * @pre
  * none
  * @post
  * The individuallength is set to the new value.
  * @param minIndLen
  * The new value of individual length.
  */
 public:
 void setMinIndLength (int minIndLen);

 /**
 * This gets the minimal length of an individual of the parameter.
 * @pre
 * none
 * @post
 * The minimal length of an individual is returned.
 * @return
 * The minimal length of an individual.
 */
 public:
 int getMinIndLength() const;

  public:
 QList< SIG_GPPVMHost * > const &getHostList() const;

  public:
  QList< SIG_GPPVMHost * > &getHostList();

 /**
 * This sets the max length of an individual of the parameter.
 * @pre
 * none
 * @post
 * The individuallength is set to the new value.
 * @param maxIndLen
 * The new value of individual length.
 */
 public:
 void setMaxIndLength (int maxIndLen);

/**
* This gets the max length of an individual of the parameter.
* @pre
* none
* @post
* The max length of an individual is returned.
* @return
* The max length of an individual.
*/
 public:
 int getMaxIndLength() const;

 /*
 * This function sets the functionset to its demanded value.
 * @post
 * The attribute functionSet is set to the value of the parameters. 
 * @param fuset
 * The set of functions, which can be used by the 
 *
 * public:
 *  void setFunctionSet(QStringList fuset);
 *
 *
 * This function returns the set of functions, which are used for the 
 * randomcreation of robot control programs. 
 * @return
 * The set of functions, given as a QString.
 *
 *  public:
 *  QStringList getFunctionSet() const;
 *
 *
 * This function will set the set of terminals, which is used for the random
 * creation of robot control programs, to the demanded value.
 * @param teset
 * The set of terminals, given as a QString.
 *
 * public:
 *  void setTerminalSet(QStringList teset);
 *
 *
 * This function returns the set of terminals, which are used for the 
 * randomcreation of robot control programs. 
 * @return
 * The set of terminals, given as a QString.
 *
 *  public:
 *  QStringList getTerminalSet() const;
 */

 /**
  * This function sets the probability of reproduction of the winner of a tournament, 
 * which are used for the randomcreation of tournaments. The probability expressed as a percentage
 * is given through the reproduction probability divided through the sum of 
 * crossover-, mutation- and reproduction probability.
 * @param prob
 * The probability of reproduction, given as a integer. 
 */
 public:
  void  setReproductionProb(int prob);

 /**
 * This function returns the probability of reproduction of the winner of a tournament, 
 * which are used for the randomcreation of tournaments. The probability expressed as a percentage
 * is given through the reproduction probability divided through the sum of 
 * crossover-, mutation- and reproduction probability.
 * @return
 * The probability of reproduction, given as a integer. 
 */
 public:
  int  getReproductionProb() const;

  /**
 * This function sets the probability of crossover of the winners of a tournament, which are used for 
 * the randomcreation of tournaments. The probability expressed as a percentage
 * is given through the crossover probability divided through the sum of 
 * crossover-, mutation- and reproduction probability.
 * @param prob
 * The probability of crossover, given as a integer. 
 */
 public:
  void  setXoverProb(int prob);

  /**
 * This function returns the probability of crossover of the winners of a tournament, which are used for 
 * the randomcreation of tournaments. The probability expressed as a percentage
 * is given through the crossover probability divided through the sum of 
 * crossover-, mutation- and reproduction probability.
 * @return
 * The probability of crossover, given as a integer. 
 */
 public:
  int getXoverProb() const;

 /**
 * This function sets the probability of reproduction of the winner of a tournament, which are used for 
 * the randomcreation of tournaments. The probability expressed as a percentage
 * is given through the mutation probability divided through the sum of 
 * crossover-, mutation- and reproduction probability.
 * @param prob
 * The probability of mutation, given as a integer. 
 */
 public:
  void setMutationProb(int prob);

 /**
 * This function returns the probability of reproduction of the winner of a tournament, which are used for 
 * the randomcreation of tournaments. The probability expressed as a percentage
 * is given through the mutation probability divided through the sum of 
 * crossover-, mutation- and reproduction probability.
 * @return
 * The probability of mutation, given as a integer. 
 */
 public:
  int  getMutationProb() const;

 /**
 * This function sets the number of tournaments, which defines when
 * a new generation level is reached.
 * @param prob
 * The number of tournaments after the number of played generations is increased by one.
 */
 public:
  void  setTournamentsPerGeneration(double prob);

 /**
 * This function returns the factor, which is multiplied with the number of individuals, 
 * to get the number of tournaments, which defines when
 * a new generation level is reached.
 * @return
 * The number of tournaments after the number of played generations is increased by one.
 */
 public:
  double  getTournamentsPerGeneration() const;

 /**
 * This function sets the maximun age of an individual. If the individual is older
 * than this age, it will lose every
 * tournament and so will deleted automaticaly. 
 * @param age
 * The maximum age of an individual.
 */
 public:
  void setMaxAge(long int age);

 /**
 * This function returns the maximun age of an individual. If the individual is older
 * than this age, it will lose every tournament and so will deleted automaticaly. 
 * @param age
 * The maximum age of an individual.
 */
 public:
  long int getMaxAge() const;

 /**
 * This function sets the time in milliseconds, how long the GP-thread will give the
 * controll back to the GUI.
 * @param pTime
 * The time, how long the GP-thread will be passive.
 */
 private:
  void setPassiveTime(unsigned long pTime);

 /**
 * This function returns the passive time, how long the GP-thread shifts the 
 * process control back to the GUI.
 * @return
 * The time, how long the GP-thread will be passive.
 */
 public:
  unsigned long getPassiveTime() const;

 /**
 * This function sets the maximal touches of the toSpawnList-array per loop of the evolution,
 * this means how many fitnesstasks will executed in one round of the evolutionloop.
 * @param touchs
 * The number of touches in the toSpawnList in one round of the evolutionloop.
 */
 private:
  void setMaxTouchsPerLoop(int touchs);

 /**
 * This function sets the maximal touches of the toSpawnList-array per loop of the evolution,
 * this means how many fitnesstasks will executed in one round of the evolutionloop.
 * @return
 * The number of touches in the toSpawnList in one round of the evolutionloop.
 */
 public:
  int getMaxTouchsPerLoop() const;
 
 /**
 * This function will set the sweeps on the tournament array to do per loop of the evolution.
 * One sweep will detect the set of executable tournaments, so the one's who have no 
 * dependencies to other tournaments.
 * @param sweeps
 * The number of runs of the topological sorting in one round of the evolution.
 */
 private:
  void setToDoSweepsPerLoop(int sweeps);

 /**
 * This function will return the sweeps on the tournament array to do per loop of the evolution.
 * One sweep will detect the set of executable tournaments, so the one's who have no 
 * dependencies to other tournaments.
 * @return
 * The number of runs of the topological sorting in one round of the evolution.
 */
 public:
  int getToDoSweepsPerLoop() const;

 /**
 * This function will set the termination time, when the evolutionloop will be stop by
 * the GPManager.
 * @param tTime
 * The time of termination of the evolutionloop.
 */
 public:
  void setTerminationTime(QDateTime tTime);

 /**
 * This function will return the termination time, when the evolutionloop will be stop by
 * the GPManager.
 * @return
 * The time of termination of the evolutionloop.
 */
 public:
  QDateTime getTerminationTime() const;

  public:
  void setTerminationDurationDays( int newValue );

  public:
  int getTerminationDurationDays() const;

  public:
  void setTerminationDurationHours( int newValue );

  public:
  int getTerminationDurationHours() const;

  public:
  void setTerminationDurationMinutes( int newValue );

  public:
  int getTerminationDurationMinutes() const;

  public:
  void setTerminationDurationSeconds( int newValue );

  public:
  int getTerminationDurationSeconds() const;

  public:
  bool getTerminationUsesDate() const;

  public:
  void setTerminationUsesDate( bool newValue );

 /**
 * This function will set the number of generations, when the evolutionloop will be stop by
 * the GPManager.
 * @param tGenNo
 * The generations, after the evolutionloop will be stop.
 */
 public:
  void setTerminationGenerationNo (int tGenNo);

 /**
 * This function will return the number of generations, when the evolutionloop will be stop by
 * the GPManager.
 * @return
 * The generations, after the evolutionloop will be stop.
 */
 public:
  int getTerminationGenerationNo() const;

 /**
 * This function will set the model of termination fo the GP-System
 * @param termi
 * The type of termination.
 */
 public:
  void setTerminationModel(terminationType termi);

 /**
 * This function will returns the type of termination, which indicates on which
 * incident the GP-Sytem will be stopped.
 * @return
 * The type of truncation condition.
 */
 public:
  terminationType getTerminationModel() const;

 /**
 * This function sets the directory, which is used for storing all indiviuals, which are
 * deleted from the pool, if the liveundead attribute is set to true.
 * @param directory
 * The graveyard directory where the deleted individuals are stored for futher examinations.
 */
 public:
  void setGraveYardDirectory(QDir directory);

  /**
   * This function returns the value of save exit, which means if its enabled, that an exit in the
   * non GUI task will be only preformed if all tournaments of a generation are play.
   */
 public:
  bool getSaveExit() const;

  /**
   * This function sets the value to the save exit attribute.
   */
 public:
  void setSaveExit(bool sexit);

 /**
 * This function returns the directory, which is used for storing all indiviuals, which are
 * deleted from the pool, if the liveundead attribute is set to true.
 * @return
 * The graveyard directory where the deleted individuals are stored for futher examinations.
 */
 public:
  QDir getGraveYardDirectory() const;

 /**
 * This function will set the attribute liveUndead to the value of the boolean parameter.
 * If liveUndead is true, every individual, which loses an tournament, will be stored
 * after its deletion in this directory.
 * @param grave
 * The value which indicates if liveUndead is true or not.
 */
 public:
  void setLiveUndead(bool grave);

 /**
 * This function will return the value of the attribute liveUndead. 
 * If liveUndead is true, every individual, which loses an tournament, will be stored
 * after its deletion in this directory.
 * @return
 * The value which indicates if liveUndead is true or not.
 */
 public:
  bool getLiveUndead() const;

 /**
 * This function sets the poolimage directory. In this directory images of the pool will saved
 * automaticaly if the value of the attribute poolImageGeneration is higher than zero.
 * @param PIDir
 * The directory, where the poolimages will be saved.
 */
 public:
  void setPoolImageDirectory(QDir PIDir);

 /**
 * This function returns the poolimage directory. In this directory images of the pool will saved
 * automaticaly if the value of the attribute poolImageGeneration is higher than zero.
 * @return
 * The directory, where the poolimages will be saved.
 */
 public:
  QDir getPoolImageDirectory() const;

 /**
 * This function sets the number of generations, which have to be played, before a poolimage
 * will be created.
 * @param gener
 * The number of generations which have to played between each poolimage.
 */
 public:
  void setPoolImageGeneration(int gener);

 /**
 * This function returns the number of generations, which have to be played, before a poolimage
 * will be created.
 * @return
 * The number of generations which have to played between each poolimage.
 */
 public:
  int getPoolImageGeneration() const;

 /**
 * This function sets the priority level, which indicates how much computation power is demanded
 * for the Sigel-system. If the function is executed, it will not only sets the priority attribute,
 * it will set the attributes passsiveTime, maxTouchesPer and toDoSweepsPerLoop to its depending right values.
 * @param prio
 * The level of priority of the Sigel-system.
 */
 public:
  void setPriority(priorityLevel prio);

 /**
 * This function sets the priority level, which indicates how much computation power is demanded
 * for the Sigel-system. If the function is executed, it will not only sets the priority attribute,
 * it will set the attributes passsiveTime, maxTouchesPerLoop and toDoSweepsPerLoop to its depending
 * right values.
 * @return
 * The level of priority of the Sigel-system.
 */
 public:
  priorityLevel getPriority() const;

 /**
 * This command loads a set of parameters, given in a QTextStream, to the attributes of the gpParameter object.
 * @param file
 * The data of a set of parameters, given as a QTextStream.
 */
 public:
  void readFromFile(QTextStream & file);

 /**
 * This function writes to a QTextStream, which contains the whole data of the current experiment.
 * @param file
 * The data of a set of parameters, given as a QTextStream.
 */
 public:
  void writeToFile(QTextStream & file);
  
  public:
  double getMaxFitness() const;
  
  public:
  void setMaxFitness(double maxfit);

  public:
  QString getFitnessName() const;

  public:
  void setFitnessName(QString name);

  public:
  bool getParsimonyPressure();
  
  public:
  void setParsimonyPressure( bool pressure );

  public:
  void setProbability( SIGEL_Program::Robotinstruction instruction, int prob );

  public:
  int  getProbability( SIGEL_Program::Robotinstruction instruction );

  public:
  QList< int > &getInstructionProbabilities();

  public:
  int getTimeOutMinutes();

  public:
  void setTimeOutMinutes( int newValue );


  public:
  void setResetEveryGeneration(int _resEvGen) { resEvGen = _resEvGen; }
  int getResetEveryGeneration() { return resEvGen; }
};
}
#endif //  SIGEL_GP_SIG_GPPARAMETER_H





