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
#ifndef SIGEL_SIMULATION_SIG_DYNAMECHSSIMULATIONQUERIES_H
#define SIGEL_SIMULATION_SIG_DYNAMECHSSIMULATIONQUERIES_H

#include "compat/q2compat.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/SIG_Register.h"
#include "SIGEL_Simulation/SIG_SimulationQueries.h"
#include <qdatetime.h>
#ifdef _WINDOWS
#include <vector>
#else
#include <vector.h>
#endif

namespace SIGEL_Simulation
{

/**
 * This class is used to query actual data about the simulation.
 *
 * Several entities as for example the recorder or the interpreter
 * need to be able to get actual values from the simulator. For this task the
 * class SIG_SimulationQueries is needed. It gives access to relevant
 * simulation data. Its functionality will be expanded as needed by
 * more sophisticated fitness-functions (respectively their recorders).
 * Especially it can be used to query the actual simulation time and the
 * position of each roboter link.
 */
  class SIG_DynaMechsSimulationQueries : public SIG_SimulationQueries
  {
 private:

  /** the reference to the simulator's simulationData
   */
  SIG_DynaMechsSimulationData& simulationData;

 public:

  /** the constructor
   * @pre
   *  the simulator has created theSimulationData
   * @post
   *  SIG_SimulationQueries is created. simulationData is set to
   *  theSimulationData
   * @param theSimulationData
   *  reference to the simulator's simulationData
   */
  SIG_DynaMechsSimulationQueries(SIG_DynaMechsSimulationData& theSimulationData);

  /** Interprets the SENSE command and writes the results into the registers */
  void sense(int sensorNo,Q2PtrVector<SIG_Register> & registers) const;

  /** gets the actual simulation time which is calculated frame*stepsize
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the calculated actual simulation time
   */
  QTime getActualSimulationTime() const;

  /** gets the position of link linkNo
   * @pre
   *  none
   * @post
   *  none
   * @param linkNo
   *  the number of the link. if the link doesn't exist, it is calculated
   *  modulo maxlinks
   * @return
   *  the position of the center of mass of link linkNo
   */
  DL_vector getLinkPosition(int linkNo) const;

  /** gets the orientation of link linkNo
   * @pre
   *  none
   * @post
   *  none
   * @param linkNo
   *  the number of the link. if the link doesn't exist, it is calculated
   *  modulo maxlinks
   * @return
   *  the orientation of the link relativ to the original orientation
   */
  DL_matrix getLinkOrientation(int linkNo) const;

  /** gets the number of the torso link
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the number of the torso link
   */
  int getRootNumber() const;

  /** gets the forces of each link enacted this timeframe
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the forces of each link this frame. The forces are stored in a array of size 6. The first three doubles specify the
   *  moment and the last three doubles are the 3-d force vector.
   *  see also: S. McMillan: Computational Dynamics for Robotic Systems on Land and Under Water, Introduction, page 6.
   *  The double* is the pointer to this array.
   *  for each link the array is stored in the vector. For example: to query the force for link3, you have to write vector[2].
   */
  std::vector<double*>* getUsedForces() const;

  /** gets the number of links
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the number of links
   */
  int getLinkCount() const;

   /**
    *Return the number of links currently touching the floor.
    */
   int getNumberOfTouchdowns( void ) const;

  void checkDynas() const;
};

}

#endif // SIGEL_SIMULATION_SIG_DYNAMECHSSIMULATIONQUERIES_H
