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

  struct miscParameter {
    /** The name of the fitnessfunction, only necessary for computation of a
     *  fitnessvalue.. It doesn't be change in one evolution,
     * therefor it is given as a reference.
     */
    QString fitnessName;
    /** A flag, which indicates if the simulation have to be displayed or not.
     *  TRUE means show the simulation without fitness computation,
     *  FALSE means compute fitnessvalue.
    */
    bool visualize;
    int actGeneration;
    int resetEveryGeneration;
  };

  /**
   * This class represents the functionality of data encoding and decoding
   * for data transfer issues relating to pvm. There is a special need for 
   * transforming methodes, to bring a textstream to a string and than 
   * to bring this string in the right order, so that it can be transfered
   * through pvm to the sigel-slave program. The SIG_GPFitnesstrainer and 
   * sigel-slave uses this class. They need it to decode and encode the
   * to be send or received pvm data.
   */

  class SIG_GPPVMData {

    /** The constructor of a PVMDataobject with all attributes as parameters, without the program
     * of the individual, which will be given as a parameter.
     * @pre
     * The GPFitnesstrainer or the sigel-slave is created and needs a PVMDataobject.
     */
   public:
    SIG_GPPVMData(SIGEL_Robot::SIG_Robot& robot, SIGEL_Environment::SIG_Environment& environment, SIGEL_Simulation::SIG_SimulationParameters& simulationParameter, QString fitnessName, bool visualize);

    /** The destructor of the PVMDataobject.
     */
   public:
    ~SIG_GPPVMData();

   /** A function to send QStrings via PVM.
    * @param str
    * The string which is to send.
    * @param taskId
    * The id of the pvmtask, who is the demanded receiver of the string.
    * @param messageId
    * The id of the message channel, on which the data have to be send.
    */
   public:
    void sendQStringToPVM(QString str, int taskId, int messageId);

   /** A function to read QStrings via PVM.
    * @param taskId
    * The id of the pvmtask, who is the sender of the string.
    * @param messageId
    * The id of the message channel, on which the data have to be received.*/
   public:
    QString getQStringFromPVM(int taskId, int messageId);

   public:
    QString cutAfterFiveHashes(QTextStream& source);

    /*
     * With this operation, the Sigel_slave loads pvmdata from a received textstream and put it to the attributes of the
     * SIG_GPPVMData-object. This has to happen in the construction of the PVMData-object.
     * @param file
     * The received textstream with the data for the simulation run.
     * @param program
     * The robot control program, which has to be tested for its new fitnessvalue.
     *
     */
   public:
    void SIGEL_GP::SIG_GPPVMData::loadPVMDataTransfer(QTextStream & file,
  						    SIGEL_Program::SIG_Program & program);

    /**
     * With this operation, the SIG_GPFitnessTrainer saves pvmdata to textstream, make it ready for transfer
     * to the Sigel-slave.
     * @param file
     * The textstream to which the data have to encoded.
     * @param program
     * The program of the individual, which is different from individual to individual and so
     * have to be there as a parameter, all other needed datas are given as referneces.
     */
   public:
    void SIGEL_GP::SIG_GPPVMData::savePVMDataTransfer(QTextStream & file,
  						    SIGEL_Program::SIG_Program const &program);

    /**
     * This operation sets the visulize-flag to its right value.
     * @param visu
     * The value of visualize.
     */
   public:
    void SIGEL_GP::SIG_GPPVMData::setVisualize(bool visu);

    /**
     * This operation gets the value of the visualize-flag.
     * @return
     * The value of the visualize-flag.
     */
   public:
    bool SIGEL_GP::SIG_GPPVMData::getVisualize();

   public:
    QString getFitnessFunctionName();

   public:
    void setFitnessFunctionName( QString name );
    void setActGeneration( int _actGeneration) { miscParam.actGeneration = _actGeneration; }
    int getActGeneration() { return miscParam.actGeneration; }
    void setResetEveryGeneration(int _resetEveryGeneration) { miscParam.resetEveryGeneration = _resetEveryGeneration; }
    int getResetEveryGeneration() { return miscParam.resetEveryGeneration; }
    struct miscParameter getMiscParam() { return miscParam; }

    /** The robot for the simulation run. It doesn't be change in one evolution,
     * therefor it is given as a reference.
     */
   private:
    SIGEL_Robot::SIG_Robot& robot;

    /** The environment for the simulation. It doesn't be change in one evolution,
     * therefor it is given as a reference.
     */
   private:
    SIGEL_Environment::SIG_Environment& environment;

    /** The simulation parameters for the simulation. It doesn't be change in one evolution,
     * therefor it is given as a reference.
     */
   private:
    SIGEL_Simulation::SIG_SimulationParameters& simulationParameter;

  /**
   *  This variable holds the actual Generation. It can be used for calculating a fitness value, which depends
   *  on the actual fitness-function. For an example look at the forceFitnessFunction.
   */
    private:
      struct miscParameter miscParam;

  };
}

#endif // SIGEL_GP_SIG_GPPVMDATA_H

