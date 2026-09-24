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
#include "SIGEL_GP/SIG_GPAdaptiveWalkingFitnessFunction.h"

#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Simulation/SIG_EarlyRunTermSimulation.h"


namespace SIGEL_GP
{

  SIG_GPAdaptiveWalkingFitnessFunction::SIG_GPAdaptiveWalkingFitnessFunction( SIGEL_Program::SIG_Program &program,
								  SIGEL_Robot::SIG_Robot &robot,
								  SIGEL_Environment::SIG_Environment &environment,
								  SIGEL_Simulation::SIG_SimulationParameters & simulationParameters )
    : SIG_GPFitnessFunction( program,
			     robot,
			     environment,
			     simulationParameters,
			     "ZorcWalkingFitnessFunction" )
  { };

  SIG_GPAdaptiveWalkingFitnessFunction::~SIG_GPAdaptiveWalkingFitnessFunction()
  { };

  double SIG_GPAdaptiveWalkingFitnessFunction::evalFitness()
  {
    double  fitness   = 0,
            avgHeight = 0,
            avgHeightPercent = 0;
    int     steps     = 0,
            perfSteps = 0;
		char		infStr[256];

    // let SIG_EarlyRunTermSimulation record the positions or our robot, each 100. frame
    SIGEL_GP::SIG_GPFullDataRecorder recorder( 100 );

    // use class SIG_EarlyRunTermSimulation to terminate the simulation when
    // robot height drops below 1/2 of start height to speed up evolutionary progress
    SIGEL_Simulation::SIG_EarlyRunTermSimulation *simulation = new SIGEL_Simulation::SIG_EarlyRunTermSimulation( rob, environment, program, simparameter, recorder );

    // do the simulation
    try
    {  simulation->start();
    }
    catch (SIGEL_Tools::SIG_Exception &e)
    { };

    // calculate how many steps a complete simulation takes
    // (usually number of elements in recorder-obj., but we may have terminated earlier)
    steps = simulation->getMaxRecorderSteps(100);

    DL_vector realStartPosition = normalizeRobotPosition( *recorder.positions.value( 0 ),
							  *recorder.rotations.value( 0 ) );

    DL_vector realEndPosition = normalizeRobotPosition( recorder.endPosition,
							recorder.endRotation );

    double const startHeight = realStartPosition.y;

    DL_vector distanceVector = realStartPosition;
    distanceVector.minusis( &realEndPosition );

    double distance = distanceVector.norm();

    int simulatedSeconds = QTime( 0, 0 ).secsTo( simparameter.getTimeToSimulate() );

    fitness = distance / simulatedSeconds;

    DL_vector *endPosition = new DL_vector();
    *endPosition = recorder.endPosition;

    DL_matrix *endRotation = new DL_matrix();
    *endRotation = recorder.endRotation;

    recorder.positions.append( endPosition );
    recorder.rotations.append( endRotation );

    qsizetype recIdx = 0;
    DL_vector *actPosition = recorder.positions.value( recIdx );
    DL_matrix *actRotation = recorder.rotations.value( recIdx );

        // iterate through recorded positions and evaluate !
    while (actPosition)
		{
			DL_vector actRealPosition = normalizeRobotPosition( *actPosition, *actRotation );

			if ( !(isValid( actRealPosition.x ) && isValid( actRealPosition.y ) && isValid( actRealPosition.z )) )
	  	{
	    	fitness = 0;
	    	break;
	  	}

			// note decrease in height
			if (actRealPosition.y <= startHeight)
			{	//	disallow neg. values
				if (actRealPosition.y > 0)
				{	avgHeight += actRealPosition.y;
				}
			}
			// interprete all above startheight as 100% !
			else
			{ // really hi ?
        if (actRealPosition.y > 1.5*startHeight)
        {   SIGEL_Tools::SIG_IO::cerr << "\to quite high -- " << actRealPosition.y << Qt::endl;
        }

				avgHeight += startHeight;

				// but check for strange effects..
				if (actRealPosition.y > 2.0*startHeight)
				{   SIGEL_Tools::SIG_IO::cerr << "\t## r u ready 2 fly ?  ##  height is > 2*startHeight  (actRealPosition.y=" << actRealPosition.y << " ##" << Qt::endl;
				}
			}

			perfSteps++;
			++recIdx;
			actPosition = recorder.positions.value( recIdx );
			actRotation = recorder.rotations.value( recIdx );
 		}

		// get average height in percent of start height
		avgHeight = avgHeight / (double)steps;
		avgHeightPercent = avgHeight / startHeight;

  	// weight fitness using avg. height, height is more important than distance !
  	fitness = (100*fitness) * avgHeightPercent;

		// give some information
		sprintf(infStr, "fitness: %5.4f  |  avgHeight: %4.1f%%  |  distance: %4.3f  |  steps (calc./total): %d/%d\n", fitness, avgHeightPercent*100.0, distance, perfSteps, steps);
		fprintf(stderr, infStr);

  	delete simulation;

  	return fitness;
  }

}
