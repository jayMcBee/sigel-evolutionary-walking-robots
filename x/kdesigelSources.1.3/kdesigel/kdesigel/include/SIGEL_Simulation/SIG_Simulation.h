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
#ifndef SIGEL_SIMULATION_SIG_SIMULATION_H
#define SIGEL_SIMULATION_SIG_SIMULATION_H

#include "SIGEL_Simulation/SIG_Recorder.h"
#include "SIGEL_Simulation/SIG_CommandInterface.h"
#include "SIGEL_Simulation/SIG_SimulationData.h"
#include "SIGEL_Simulation/SIG_SimulationQueries.h"
#include "SIGEL_Simulation/SIG_Interpreter.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationCannotSolveException.h"
#include <qstring.h>

/** The Simulation covers the connection to Dynamo and Solid plus an Interpreter.
 *
 * The Simaultion contains several classes to set up and run a simulation of
 * a robot model in a given environment. It also has classes to interprete the
 * robot program and to hold and record the important data for the simulation
 * and the fitness function.
 */

namespace SIGEL_Simulation
{
  
  /**
   * The class corresponding to one Robot-Simulation.
   *
   * One SIG_Simulation object contains the complete information
   * about one simulation run and the method how to record data
   * about the simulation sun in each frame.
   * This implies that two simulation objects constructed
   * with the same parameters should result in the same simulation
   * run and the same set of recorded data.
   */
  class SIG_Simulation : public QObject
    { 
      Q_OBJECT
    public:
      /**
       * The constructor of the SIG_Simulation class.
       *
       * When being constructed the simulation gets all the relevant data neccessary for the
       * the simulation. This includes the robot, the environment, the robot program, the
       * simulation parameters as well as a recorder to record data.
       * @param robot The robot that is to be simulated.
       * @param environment The environment in which the simulation will take place.
       * @param robotProgram The robot program that will be simulated on the robot.
       * @param simulationParameter The parameters that specify the simulation conditions.
       * @param recorder A recorder to record data.
       */
      SIG_Simulation( SIGEL_Robot::SIG_Robot const & robot,
		      SIGEL_Environment::SIG_Environment const & environment,
		      SIGEL_Program::SIG_Program const & robotProgram,
		      SIG_SimulationParameters const & simulationParameter,
		      SIG_Recorder & theRecorder);
		
		/* The destructor of the SIG_Simulation class.
		 */		
		~SIG_Simulation();
      
      /**
       * Starts the simulation.
       *
       * Starts the simulation and calls makeTimeSteps as often as neccessary, so that a complete
       * simulation run is performed. The duration of this run is specified in the simulationParameter
       * object given in the constructor.
       *
       * @exception SIGEL_Tools::SIG_SimulatorCannotSolveException
       *            Some simulation-constraint could'nt be solved by the Dynamo-System.
       *
       * @post The complete simulation is performed.
       */
      void start()
	throw(SIGEL_Simulation::SIG_SimulationCannotSolveException);
      
      /**
       * Performs a certain number of time steps given by numTimeSteps.
       *
       * This function will be called several times by the start-function as long as the elapsed
       * time is less than the amount of time that the simulation should run.
       *
       * @exception SIGEL_Toolss::SIG_SimulatorCannotSolveException
       *            Some simulation-constraint could'nt be solved by the Dynamo-System.
       *
       * @param numTimeSteps The number of time steps that are performed.
       * @post numTimeStep time steps are performed.
       */
      void makeTimeSteps( int numTimeSteps )
	throw(SIGEL_Simulation::SIG_SimulationCannotSolveException);

      /**
       * Inherited classes override this method to prematurely terminate the
       * simulation before the complete simulation been completly computed.
       *
       * Return 'true' if you want the simulation to stop in method start()
       *
       */
      virtual bool prematureTermination( void )  {  return false;  }


    public slots:
      /** This slot will catch a message from Dynamo
       *
       * (which was thrown in SIG_DynaSystem) */
      void slotDynamoMessage(QString theMessage);


    public:

      /**
       * A recorder to record data.
       *
       * A derived class from SIG_Recorder which will record data at the beginning, during and at
       * the end of the simulation.
       */
      SIG_Recorder & recorder;


    protected:

      /**
       * if this boolean is true, the Simulation will stop
       */
      bool stopSimulation;

      /**
       * The commandInterface used for example to propagate move commands to the simulation.
       */
      SIG_CommandInterface *commandInterface;

      /**
       * The representation of the simulation data.
       *
       * The simulation data cannot be access directly by any of the other classes. They have to
       * use the interfaces SIG_SimulationQueries and SIG_CommandInterface to manipulate it.
       */
      SIG_SimulationData *simulationData;

      /**
       * The interface used to query the simulation for example about the actual simulation time.
       */
      SIG_SimulationQueries *simulationQueries;

      /**
       * The interpreter used to interpret the programs.
       */
      SIG_Interpreter *interpreter;
    };

}

#endif // SIGEL_SIMULATION_SIG_SIMULATION_H
