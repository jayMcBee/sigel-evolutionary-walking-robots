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
   * simulation parameters, the environment, the program, the robot and the
   * settings of the job. The master and sigel_slave both use it.
   */

  class SIG_GPPVMData {
   public:

    /** The program is not held; it is passed to savePVMDataTransfer and loadPVMDataTransfer. */
    SIG_GPPVMData(SIGEL_Robot::SIG_Robot& robot, SIGEL_Environment::SIG_Environment& environment, SIGEL_Simulation::SIG_SimulationParameters& simulationParameter, QString fitnessName, bool visualize);


   /** Sends str to the PVM task taskId, as message messageId. */
    void sendQStringToPVM(QString str, int taskId, int messageId);

   /** Waits for message messageId from the PVM task taskId and returns its string. */
    QString getQStringFromPVM(int taskId, int messageId);

   /** Reads the lines up to the next "#####" line, which ends a block of the message. */
    QString cutAfterFiveHashes(QTextStream& source);

    /** Reads a received message into the robot, the environment, the simulation parameters, the settings and program. */
    void loadPVMDataTransfer(QTextStream & file,
  						    SIGEL_Program::SIG_Program & program);

    /** Writes the message for a slave, with program as its program. */
    void savePVMDataTransfer(QTextStream & file,
  						    SIGEL_Program::SIG_Program const &program);

    void setVisualize(bool visu);
    bool getVisualize();
    QString getFitnessFunctionName();
    void setFitnessFunctionName( QString name );
    QString getExperimentName();
    void setExperimentName( QString name );
    void setActGeneration( int _actGeneration) { actGeneration = _actGeneration; }
    int getActGeneration() { return actGeneration; }
    void setResetEveryGeneration(int _resetEveryGeneration) { resetEveryGeneration = _resetEveryGeneration; }
    int getResetEveryGeneration() { return resetEveryGeneration; }

   private:

    /** The robot, the environment and the simulation parameters are held by reference; they do not change during an evolution. */
    SIGEL_Robot::SIG_Robot& robot;
    SIGEL_Environment::SIG_Environment& environment;
    SIGEL_Simulation::SIG_SimulationParameters& simulationParameter;

    /** The id of the fitness function, as experiment files store it. */
    QString fitnessName;

    /** True shows the simulation; false computes a fitness. */
    bool visualize;

    /** The pool generation and the reset interval are sent, but no slave code reads them. */
    int actGeneration;
    int resetEveryGeneration;

    /** The name of the experiment, only for a displayed simulation. */
    QString experimentName;

  };
}

#endif // SIGEL_GP_SIG_GPPVMDATA_H

