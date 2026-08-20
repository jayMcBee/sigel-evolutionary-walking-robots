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
#ifndef SIGEL_SIMULATION_SIG_RECORDER_H
#define SIGEL_SIMULATION_SIG_RECORDER_H

#ifdef _WINDOWS
#pragma warning( disable : 4290 )
#endif

#include "SIGEL_Simulation/SIG_SimulationQueries.h"

#include "SIGEL_Simulation/SIG_RecorderNoQueriesSetException.h"
#include "SIGEL_Simulation/SIG_RecorderBadRecordingOrderException.h"

namespace SIGEL_Simulation
{

/**
 * The SIG_Recorder is an abstract class for the classes that record
 * simulation-data each timestep.
 *
 * It contains a pointer to a SIG_SimulationQueries object whose methods can
 * be used to collect information about the simulation state in the current
 * frame.
 * During the simulation run the three methods init, record and finish
 * of the recorder are called by the SIG_Simulation's main methods
 * (makeTimeStep respectively start).
 * Their equivalents in derived class should perform the data-recording.
 * From SIG_Recorder e.g. a fitness-recorder (collecting data used by a
 * fitness function) or a render-recorder (collecting all the data used
 * to graphically render the simulated frame) can be derived.
 */
class SIG_Recorder { 
 public:

  /**
   * The SIG_Recorders constructor. It has to be called by the
   * constructors of derived classes.
   */
  SIG_Recorder();

  /**
   * This method does initialization-work that cannot be done
   * at construction time (because simulationQueries is still 0).
   *
   * @attention The virtual init-method defined in a derived recorder-class
   *            has to call SIG_Recorder::init() as its first statement!
   *
   * @exception SIG_RecorderNoQueriesSetException
   *            simulationQueries points to 0.
   * @exception SIG_RecorderBadRecordingOrderException
   *            init was called more than once.
   *
   * @pre The pointer simlulationsQueries has to be set to the
   *      simulation's SIG_SimulationQueries object (must not be 0).
   * @pre initialized has to be false, because init may only be called
   *      once.
   * @post The bool initialized is set to true.
   * @post The bool finished is (still) false.
   */
  virtual void init();

  /**
   * Will be called in each timeframe by the simulator.
   *
   * Each time the record-method is called it takes the relevant information
   * from the simulation's queries-interface and stored in an appropriate
   * data structure.
   *
   * @attention The virtual record-method defined in a derived recorder-class
   *            has to call SIG_Recorder::record() as its first statement!
   *
   * @exception SIG_RecorderNoQueriesSetException
   *            simulationQueries points to 0.
   * @exception SIG_RecorderBadRecordingOrderException
   *            record was called before the init-call
   * @exception SIG_RecorderBadRecordingOrderException
   *            record was called after the finish-call.
   *
   * @pre The pointer simlulationsQueries has to be set to the
   *      simulation's SIG_SimulationQueries object (must not be 0).
   * @pre initialized has to be true (meaning that init has already
   *      been executed).
   * @pre finished has to be false (meaning that finished hasn't executed yet).
   */
  virtual void record();

  /**
   * Is called after all the data-recording is done (for example
   * to do final preparations on the collected data).
   *
   * @attention The virtual finish-method defined in a derived recorder-class
   *            has to call SIG_Recorder::finish() as its first statement!
   *
   * @exception SIG_RecorderNoQueriesSetException
   *            simulationQueries points to 0.
   * @exception SIG_RecorderBadRecordingOrderException
   *            finish was called more than once.
   * @exception SIG_RecorderBadRecordingOrderException
   *            finish was called before the init-call.
   *
   * @pre The pointer simlulationsQueries has to be set to the
   *      simulation's SIG_SimulationQueries object (must not be 0).
   * @pre initialized has to be true (meaning that init has to be called).
   * @pre finished has to be false (meaning that this is the first time
   *      finishe is called).
   */
  virtual void finish();

  /**
   * This sets the simulationQueries pointer.
   *
   * @param newSimulationQueries reference to the simulator's
   *                             SIG_SimulationQueries object.
   *
   * @post simulationQueries points to the
   *       supplied SIG_SimulationQueries object.
   */
#ifdef _WINDOWS
  void setSimulationQueries(SIG_SimulationQueries &newSimulationQueries);
#else
  void setSimulationQueries(SIG_SimulationQueries const &newSimulationQueries);
#endif

 protected:

  /**
   * The pointer to the SIG_SimulationQueries object.
   *
   * Its methods are the only way for the recorder to collect
   * data about the simulation.
   */
#ifdef _WINDOWS
  SIG_SimulationQueries* simulationQueries;
#else
  SIG_SimulationQueries const* simulationQueries;
#endif

 private:
  /**
   * True if and only if the method init was already executed.
   */
  bool initialized;

  /**
   * True if and only if the method finish was already executed.
   */
  bool finished;
};

}

#endif // SIGEL_SIMULATION_SIG_RECORDER_H

