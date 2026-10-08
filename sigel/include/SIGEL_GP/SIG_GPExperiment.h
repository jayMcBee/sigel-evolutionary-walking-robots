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

#include <QList>
#include <memory>
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_GP/SIG_GPPopulation.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_GP/SIG_GPExperimentHistoryEntry.h"

#include "MT_Control/MT_Controller.h"

#include <qstring.h>
#include <qtextstream.h>

/**
 * The namespace SIGEL_GP holds the classes of genetic programming and
 * artificial evolution. They create and develop robot control programs.
 */

namespace SIGEL_GP
{

/**
 * SIG_GPExperiment is the main data structure of an evolution run. It holds
 * all the data that describe the run.
 */

class SIG_GPExperiment {
 public:
  /**
   * Builds an empty experiment.
   * @param exp
   *  Is not used.
   */
  SIG_GPExperiment(QString exp);

  /**
   * Builds an empty experiment.
   */
  SIG_GPExperiment();
  ~SIG_GPExperiment();

  QString cutAfterFiveHashes(QTextStream& source);

  /**
   * Reads the experiment from a text stream.
   * @param file
   *  The stream that holds a saved experiment.
   */
  void loadExperiment(QTextStream &file);

  /**
   * Writes the experiment to a text stream.
   * @param file
   *  The stream that receives the experiment.
   */
  void saveExperiment(QTextStream & file);

  /**
   * Calculates nothing.
   * @param program
   *  Is not used.
   * @return
   *  Always 0.
   */
  double calculateFitness(SIGEL_Program::SIG_Program & program);

  /**
   * @return
   *  A reference to the GP parameters.
   */
  SIGEL_GP::SIG_GPParameter& getGPParameter();

  /**
   * @return
   *  A reference to the population.
   */
  SIGEL_GP::SIG_GPPopulation& getPopulation();

  /**
   * @return
   *  A pointer to the population.
   */
  SIGEL_GP::SIG_GPPopulation *getPopulationPointer();

  void exportExperimentHistoryToGNUPlot( QString fileName );

  /** Sets autosavePath. */
  void setPath(QString path);

  /** Returns autosavePath. */
  QString getPath();

  /**
   * The robot of the experiment.
   */
  SIGEL_Robot::SIG_Robot robot;

  /**
   * The parameters of the genetic programming.
   */
  SIG_GPParameter gpParameter;

  /**
   * The name of the experiment.
   */
  QString experimentName;

  /**
   * The environment of the experiment.
   */
  SIGEL_Environment::SIG_Environment environment;

  /**
   * The name of the fitness function.
   */
  QString fitnessFunctionName;

  /**
   * The population of the experiment.
   */
  SIG_GPPopulation population;

  /**
   * The parameters of the simulation.
   */
  SIGEL_Simulation::SIG_SimulationParameters simulationParameter;

  /**
   * A description of the experiment.
   */
  QString comment;

  QList< SIG_GPExperimentHistoryEntry * > experimentHistory;

 private:
  void writeHistoryToFileTransfer( QTextStream &file );
  void readHistoryFromFileTransfer( QTextStream &file );

  /**
   * The file the experiment is saved to. It is "new" until the experiment
   * has a file.
   */
  QString autosavePath;

 public:
  /**
   * The controller of the MetaGP system. It is the last member, so it is
   * destroyed first. SIG_GPExperimentClean.h has no such member.
   */
  std::unique_ptr< MT_Controller > mtController;
};

}

#endif // SIGEL_GP_SIG_GPEXPERIMENT_H
