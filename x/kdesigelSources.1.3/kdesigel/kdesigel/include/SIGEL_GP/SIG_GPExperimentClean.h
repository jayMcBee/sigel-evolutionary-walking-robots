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
#ifndef SIGEL_GP_SIG_GPEXPERIMENT_H
#define SIGEL_GP_SIG_GPEXPERIMENT_H

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_GP/SIG_GPExperimentHistoryEntry.h"

#include "MT_Control/MT_Controller.h"

#include <qstring.h>
#include <qtextstream.h>
#include <qlist.h>

/**
 * The namespace SIGEL_GP represents the classes of genetic programming and
 * artificial evolution. The algorithms in this classes can create and 
 * develope robot control programs in genetic ways. 
 */

namespace SIGEL_GP
{

/**
 * The SIG_GPExperiment represents the main data structure for the evolution
 * preformed with the SIGEL_GP_Package. It contains any nessecary information
 * and data to describe and preform an evolution run.
 *
 */

class SIG_GPExperiment {

  /**
   * The used robot architecture. 
   */
 public:
  SIGEL_Robot::SIG_Robot robot;

  /**

   * The used parameter for the genetic programming.
   */
 public:
  SIG_GPParameter gpParameter;

  /**
   * The name of the experiment.
   */
 public:
  QString experimentName;

  /**
   * The used environment.
   */
 public:
  SIGEL_Environment::SIG_Environment environment;

  /**
   * The name of the used fitnessfunction.
   */
   public:
  QString fitnessFunctionName;

  /**
   * The population of the experiment.
   */
 public:
  SIG_GPPopulation population;

  /**
   * The used parameter for the simulation.
   */

 public:
  SIGEL_Simulation::SIG_SimulationParameters simulationParameter;

  /**
   * This QString data content is a describtion of the experiment.
   */
 public:
  QString comment;

 public:
  QList< SIG_GPExperimentHistoryEntry > experimentHistory;

 private: 	
 	/**
 		*	This variable specifies the path where the autosave function should save the
 		*	experiment
 		*/
 	QString autosavePath;
 	
/**
 * The constructor of an experiment
 * @pre
 *  The data of the experiment is given in a QString.
 * @post
 *  The object of an experiment is created correctly.
 * @param exp
 *  The experiment encoded in a QString.
 */
 public:
 SIG_GPExperiment(QString exp);

/**
 * The constructor for an experiment.
 * @pre
 *  An empty experiment has to be constructed. There is no data given in a
 *  QString. Every component of the experiment will be set from the GUI.
 * @post
 *  The experiment has been constructed and its attributes are empty.
 */
 public:
  SIG_GPExperiment();
  ~SIG_GPExperiment();

 public:
  QString SIGEL_GP::SIG_GPExperiment::cutAfterFiveHashes(QTextStream& source);

  /**
   * This sets experimentdatas from a QDataStream to the attributes of an
   * experimentobject.
   * @pre
   * There are experiment data, encoded in a QDataStream and there is an object of an experiment.
   * @post
   * The loaded experiment is set to the attributes.
   * @param file
   * The data of an experiment, encoded in a QDataStream.
   */

 public:
  void loadExperiment(QTextStream &file);

  /**
   * This encodes the experiment data to a QDataStream.
   * @pre
   * There is an object of an experiment.
   * @param file
   * The data of the experiment, encoded in a QDataStream.
   * @post
   * The experiment data is written, encoded in a QDataStream.
   * 
   */
 public:
  void saveExperiment(QTextStream & file);

 private:
  void writeHistoryToFileTransfer( QTextStream &file );

 private:
  void readHistoryFromFileTransfer( QTextStream &file );

  /** Starts the FitnessCalculation as a local PVM client 
   * @param program
   * The Program to be evaluated.
   * @return
   * the calculated Fitness
   */
 public:
  double calculateFitness(SIGEL_Program::SIG_Program & program);

  /** This operation returns a reference of the gpparameter.
   * @return
   * A reference of the gpgparameter.
   */
 public:
  SIGEL_GP::SIG_GPParameter& getGPParameter();

  /** This operation returns a reference of the population.
   * @return
   * A reference of the population.
   */
 public:
  SIGEL_GP::SIG_GPPopulation& getPopulation();

  /** This operation returns a pointer to the population.
   * @return
   * A pointer to the population.
   */
 public:
  SIGEL_GP::SIG_GPPopulation *getPopulationPointer();

 public:
  void exportExperimentHistoryToGNUPlot( QString fileName );

	
  /** This is the set method for the variable autosavePath. */
  void setPath(QString _autosavePath);

	/** This is the get method for the variable autosavePath. */
	QString getPath();
 };
}

#endif // SIGEL_GP_SIG_GPEXPERIMENT_H

