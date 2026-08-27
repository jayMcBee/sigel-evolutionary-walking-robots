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
#ifndef SIGEL_SIMULATION_SIG_INTERPRETER_H
#define SIGEL_SIMULATION_SIG_INTERPRETER_H

#include <QList>
#include "SIGEL_Simulation/SIG_Interpreter.h"
#include "SIGEL_Simulation/SIG_CommandInterface.h"
#include "SIGEL_Simulation/SIG_SimulationQueries.h"
#include "SIGEL_Simulation/SIG_Register.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

namespace SIGEL_Simulation
{

/**
 * This class interpretes the robot-program.
 *
 * For this task it uses the classes SIG_CommandInterface and
 * SIG_SimulationQueries to interact with the simulator.
 * SIG_CommandInterface is used to communicate move-commands to the
 * simulator. The class SIG_SimulationQueries is used to receive
 * information about the actual simulation state out of the simulator
 * (like sensor-results).
 * The program of the robot is represented as a SIG_RobotProgram-object.
 * Also known to this class is the language parameters in form of a
 * SIG_LanguageParameters-object. For example they contain the set of forbidden
 * commands.
 * The interpretation works of an internal model of the register-machine
 * (consisting of its registers for example).
 */
class SIG_Interpreter
{
 public:

  /**
   * The constructor of a SIG_Interpreter object.
   *
   * @param langParams        The language-parameters of the robot to control.
   * @param robotProgram      The robot-program to interprete.
   * @param commandInterface  The object offering the methods to manipulate
   *                          the simulation.
   * @param simulationQueries The object offering the methods to query
   *                          actual simulation information.
   */
  SIG_Interpreter(SIGEL_Robot::SIG_LanguageParameters const &langParams,
		  SIGEL_Program::SIG_Program const &robotProgram,
		  SIG_CommandInterface &commandInterface,
		  SIG_SimulationQueries const &simulationQueries);

  /**
   * Interpretes the program until the executes robot-commands
   * have exceeded the time supplied in timeAccountSize.
   *
   * The SIG_Interpreter resumes its work at the program position
   * refered to by the program counter. Every executed command decreases
   * the remaining time to interprete about the amount specified in the
   * language parameters.
   * During the interpretation the register's state may change, the simulation
   * may be manipulated through the commandInterface and queried through
   * the simulationQueries.
   *
   * @param timeAccountSize The amount of model-time that the interpretation
   *                        may consume until the control is returned to the
   *                        calling method. Usually this is the stepsize
   *                        of the simulation.
   */
  void interprete(double timeAccountSize);

 private:

  /**
   * The robot-program to interprete.
   */
  SIGEL_Program::SIG_Program const &robotProgram;

  /**
   * The language-parameters of the robot to control
   * (set of forbidden commands for example).
   */
  SIGEL_Robot::SIG_LanguageParameters const &langParams;

  /**
   * The object that contains the methos used to
   * manipulate the simulation (setting of joint coordinates for example).
   */
  SIG_CommandInterface &commandInterface;

  /**
   * The object containing the methods used to query information
   * about the current simulation state.
   */
  SIG_SimulationQueries const &simulationQueries;

  /**
   * The time at which the last command's execution that exceeded the supplied
   * time account of the previous interprete-methodcall is finished.
   */
  double remainingLastCommandTime;

  /**
   * The set of registers modeling the robots register-machine.
   */
  QList<SIG_Register> registers;

  /**
   * The number of the next command to execute.
   */
  long programCounter;

  /**
   * Contains the results of the last comparison done by a CMP-command.
   */
  bool compareFlag;
};

}

#endif // SIGEL_SIMULATION_SIG_INTERPRETER_H
