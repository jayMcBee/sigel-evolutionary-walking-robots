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


#include "SIGEL_Simulation/SIG_EarlyRunTermSimulation.h"


SIGEL_Simulation::SIG_EarlyRunTermSimulation::SIG_EarlyRunTermSimulation(SIGEL_Robot::SIG_Robot const & robot,
						 SIGEL_Environment::SIG_Environment const & environment,
						 SIGEL_Program::SIG_Program const & robotProgram,
						 SIG_SimulationParameters const & simulationParameter,
						 SIG_Recorder & theRecorder) :
  SIGEL_Simulation::SIG_Simulation(robot, environment, robotProgram,simulationParameter,theRecorder)
{
  ertStartHeight = -1;
  ertRobot = const_cast<SIGEL_Robot::SIG_Robot *>(&robot);

  // need a SIG_GPFullDataRecorder type recorder !
  ertRecorder = dynamic_cast<SIGEL_GP::SIG_GPFullDataRecorder *>(&theRecorder);
  if (!ertRecorder)
  {   SIGEL_Tools::SIG_IO::cerr << "SIG_EarlyRunTermSimulation needs a recorder of type SIG_GPFullDataRecorder to work !\n";
  }
}


bool SIGEL_Simulation::SIG_EarlyRunTermSimulation::prematureTermination( void )
{
  // implementing this virtual method is the whole purpose of our class.
  // first we need the startheight; this is computed once when we have the
  // required information in our recorder object
  if (ertStartHeight < 0.0)
  {
    // nothing was computed yet, but do we have anything recorded yet ?
    if (ertRecorder->positions.count() > 0)
    {
       DL_vector realStartPosition = normalizeRobotPosition( *ertRecorder->positions.first(), *ertRecorder->rotations.first() );
       ertStartHeight = realStartPosition.y;
    }
  }

  // valid startheight information, let's check termination criteria
  else
  {
      // check current y-position of robot
      DL_vector *actPosition = ertRecorder->positions.last();
      DL_matrix *actRotation = ertRecorder->rotations.last();
      DL_vector actRobPos = normalizeRobotPosition( *actPosition, *actRotation );

      // lower than 1/2 of startheight ? Let's stop evaluation here!
      if (actRobPos.y < (ertStartHeight/2.0))
      { return true;
      }
  }

  // default: don't stop the computation
  return false;
}


DL_vector SIGEL_Simulation::SIG_EarlyRunTermSimulation::normalizeRobotPosition( DL_vector originalPosition, DL_matrix actualRobotRotation )
{
  DL_vector robotsRealOrigin = ertRobot->initialLocation;
  robotsRealOrigin.timesis( -1 );

  DL_vector normalizedPosition;


  actualRobotRotation.times( &robotsRealOrigin, &normalizedPosition );
  normalizedPosition.plusis( &originalPosition );

  return normalizedPosition;
}



int SIGEL_Simulation::SIG_EarlyRunTermSimulation::getMaxRecorderSteps(int inRecFrequency)
{  QTime zeroHour;
   double tts;
   int    steps;

   // max. ticks in simulation = timeToSimulate / step
   QTime maxT = simulationData->simulationParameter.getTimeToSimulate();

   // get sim.time in seconds
   tts = zeroHour.secsTo(maxT);

   steps = static_cast<int>(tts / (double)(simulationData->simulationParameter.getStepSize()) / (double)inRecFrequency);

   // recorder additionally takes a start and finish frame..
   steps += 2;

   return steps;
}
