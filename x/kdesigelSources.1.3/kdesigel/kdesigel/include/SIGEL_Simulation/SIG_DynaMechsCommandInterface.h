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
#ifndef SIGEL_SIMULATOR_SIG_DYNAMECHSCOMMANDINTERFACE_H
#define SIGEL_SIMULATOR_SIG_DYNAMECHSCOMMANDINTERFACE_H

#include "compat/q2compat.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_Register.h"
#include "SIGEL_Simulation/SIG_CommandInterface.h"

namespace SIGEL_Simulation
{

/**
 * This interface is used to communicate commands to the Simulation-Data.
 * 
 * To perform commands (e.g. enacting forces) in the simulator one has to use
 * this class. This is the class needed to change the simulation.
 */
  class SIG_DynaMechsCommandInterface : public SIG_CommandInterface
{
 private:

  /** The reference to the simulator's simulationData */
  SIG_DynaMechsSimulationData& simulationData;

 public:

  /** The constructor
   * @pre
   *  theSimulationData has been created
   * @post
   *  the SIG_CommandInterface will be created, simulationData will be set
   *  to theSimulationData.
   * @param theSimulationData 
   *  A reference to the simulator's simulationData
   */
  SIG_DynaMechsCommandInterface(SIG_DynaMechsSimulationData& theSimulationData);

  /** Interpretes a "MOVE" from the program.
   * @pre
   *  none
   * @post
   *  new forces will be added to the simulation
   * @param driveNo
   *  Number of the Drive to be modified
   * @param registers
   *  The interpreter's registers.
   *  The Force(s)/Angle(s) of the Drive is/are interpreted values from the
   *  first (n) registers depending of the type of this drive.
   *  For 'tForceMode' drive the register value is interpreted as a force to be added
   *  to the joint to be affected whereas 'tServoSimpleMode' drives interprete the
   *  register value as the new direction the drive should turn.
   *  Other types of drives currently don't work with DynaMechs.
   */
  void moveDrive(int driveNo,
		 QList<SIG_Register> const& registers);

};

}

#endif // SIGEL_SIMULATION_SIG_DYNAMECHSCOMMANDINTERFACE_H


