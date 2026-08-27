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
#ifndef SIGEL_SIMULATION_SIG_SIMULATIONQUERIES_H
#define SIGEL_SIMULATION_SIG_SIMULATIONQUERIES_H

#include <QList>
#include "SIGEL_Simulation/SIG_Register.h"
#include <pointvector.h>
#include <matrix.h>
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
class SIG_SimulationQueries { 

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
  SIG_SimulationQueries();

  /** Interprets the SENSE command and writes the results into the registers */
  virtual void sense(int sensorNo,QList<SIG_Register> & registers) const = 0;

  /** gets the actual simulation time which is calculated frame*stepsize
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the calculated actual simulation time
   */
  virtual QTime getActualSimulationTime() const = 0;

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
  virtual DL_vector getLinkPosition(int linkNo) const = 0;

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
  virtual DL_matrix getLinkOrientation(int linkNo) const = 0;

  /** gets the number of the torso link
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the number of the torso link
   */
  virtual int getRootNumber() const = 0;

  /** gets the forces for each link enacted this timeframe
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the forces of each link this frame
   */
  virtual std::vector<double*>* getUsedForces() const = 0;

		/**
		*	Return the number of links currently touching the floor.
		*/
			virtual 	int	 getNumberOfTouchdowns( void ) const = 0;

  /** gets the number of links
   * @pre
   *  none
   * @post
   *  none
   * @return
   *  the number of links
   */
  virtual int getLinkCount() const = 0;

  virtual void checkDynas() const = 0;
};

}

#endif // SIGEL_SIMULATION_SIG_SIMULATIONQUERIES_H
