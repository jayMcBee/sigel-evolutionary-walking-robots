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
#ifndef SIGEL_GP_SIG_GPPVMDATA_H
#define SIGEL_GP_SIG_GPPVMDATA_H

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Program/SIG_Program.h"

#include <qstring.h>

namespace SIGEL_GP
{

  /**
   * Encodes and decodes what the master sends a slave over PVM: the
   * simulation parameters, the environment, the floor, the program, the robot
   * and the settings of the job. The master and sigel_slave both use it.
   */

  class SIG_GPPVMData {
   public:

    /** The program is not held; it is passed to savePVMDataTransfer and loadPVMDataTransfer. */
    SIG_GPPVMData(SIGEL_Robot::SIG_Robot& robot, SIGEL_Environment::SIG_Environment& environment, SIGEL_Simulation::SIG_SimulationParameters& simulationParameter, QString fitnessName, bool visualize);

   /** Sends str to the PVM task taskId, as message messageId. */
    void sendQStringToPVM(QString str, int taskId, int messageId);

   /** Waits for message messageId from the PVM task taskId and returns its string. */
    QString getQStringFromPVM(int taskId, int messageId);

   /** The MD5 over SIGEL's source that this program was built from. A slave sends it with its fitness. */
    static QByteArray sourceHash();

   /** Reads the lines up to the next "#####" line, which ends a block of the message. Throws a SIG_Exception if the message ends first. */
    QString cutAfterFiveHashes(QTextStream& source);

    /** Reads a received message into the robot, the environment and its floor, the simulation parameters, the settings and program. */
    void loadPVMDataTransfer(QTextStream & file,
  						    SIGEL_Program::SIG_Program & program);

    /** Writes the message for a slave, with program as its program. */
    void savePVMDataTransfer(QTextStream & file,
  						    SIGEL_Program::SIG_Program const &program);

    /** The floor the message carries; the master sets it before the first savePVMDataTransfer. */
    void setTerrain(SIGEL_Environment::SIG_Terrain const &newTerrain);

    void setVisualize(bool visu);
    bool getVisualize();
    QString getFitnessFunctionName();
    void setFitnessFunctionName( QString name );
    QString getExperimentName();
    void setExperimentName( QString name );
    QString getIndividualName();
    void setIndividualName( QString name );
    double getIndividualFitness();
    void setIndividualFitness( double fitness );
    void setActGeneration( int newActGeneration) { actGeneration = newActGeneration; }
    int getActGeneration() { return actGeneration; }
    void setFitnessResetInterval(int newFitnessResetInterval) { fitnessResetInterval = newFitnessResetInterval; }
    int getFitnessResetInterval() { return fitnessResetInterval; }

   private:

    /** The robot, the environment and the simulation parameters are held by reference; they do not change during an evolution. */
    SIGEL_Robot::SIG_Robot& robot;
    SIGEL_Environment::SIG_Environment& environment;
    SIGEL_Simulation::SIG_SimulationParameters& simulationParameter;

    /** Sent, so that a slave does not read the floor picture itself. */
    SIGEL_Environment::SIG_Terrain terrain;

    /** The id of the fitness function, as experiment files store it. */
    QString fitnessName;

    /** True shows the simulation; false computes a fitness. */
    bool visualize;

    /** The pool generation and the reset interval are sent, but no slave code reads them. */
    int actGeneration;
    int fitnessResetInterval;

    /** The name of the experiment, only for a displayed simulation. */
    QString experimentName;

    /** The name and the stored fitness of the individual, only for a displayed simulation. */
    QString individualName;
    double individualFitness;
  };

}

#endif // SIGEL_GP_SIG_GPPVMDATA_H
