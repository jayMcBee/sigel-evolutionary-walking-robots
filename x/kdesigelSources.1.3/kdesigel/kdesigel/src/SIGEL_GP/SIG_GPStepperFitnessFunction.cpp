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
#include "SIGEL_GP/SIG_GPStepperFitnessFunction.h"

#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Simulation/SIG_EarlyRunTermSimulation.h"


namespace SIGEL_GP
{

  SIG_GPStepperFitnessFunction::SIG_GPStepperFitnessFunction( SIGEL_Program::SIG_Program &program,
								  SIGEL_Robot::SIG_Robot &robot,
								  SIGEL_Environment::SIG_Environment &environment,
								  SIGEL_Simulation::SIG_SimulationParameters & simulationParameters )
    : SIG_GPFitnessFunction( program,
			     robot,
			     environment,
			     simulationParameters,
			     "StepperFitnessFunction" )
  { };

  SIG_GPStepperFitnessFunction::~SIG_GPStepperFitnessFunction()
  { };

  double SIG_GPStepperFitnessFunction::evalFitness()
  {
    double  fitness   = 0,
            avgHeight = 0,
            avgHeightPercent = 0,
						avgTD = 0;
    int     steps     = 0,
            perfSteps = 0,
						i,
					 *dummy;
		char	infTxt[256];

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

    DL_vector realStartPosition = normalizeRobotPosition( *recorder.positions.first(),
							  *recorder.rotations.first() );

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

		dummy = new int;
    recorder.positions.append( endPosition );
    recorder.rotations.append( endRotation );
		recorder.touchdowns.append( dummy );

    DL_vector *actPosition = recorder.positions.first();
    DL_matrix *actRotation = recorder.rotations.first();
	  int				*actTD			 = recorder.touchdowns.first();

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
        {   SIGEL_Tools::SIG_IO::cerr << "\to quite high -- " << actRealPosition.y << "\n";
        }

				avgHeight += startHeight;

				// but check for strange effects..
				if (actRealPosition.y > 2.0*startHeight)
				{   SIGEL_Tools::SIG_IO::cerr << "\t## r u ready 2 fly ?  ##  height is > 2*startHeight  (actRealPositionc++.y=" << actRealPosition.y << " ##\n";
				}
			}

			// get number of touchdowns -- 1 link touching ground is best
			if (*actTD < 1)
			{	avgTD += 1.0;
			}
			else
			{ avgTD += 1.0 / (double)(*actTD);
			}

			perfSteps++;
			actPosition = recorder.positions.next();
			actRotation = recorder.rotations.next();
			actTD	= recorder.touchdowns.next();
		}

		// assume the robot touches ground with at least 3 links for the remaining number
		// of steps which haven't been computed due to the premature termination of evaluation
		avgTD += (1.0/3.0) * (double)(steps-perfSteps);

		// get average height in percent of start height
	  avgTD /= (double)steps;
		avgHeight = avgHeight / (double)steps;
		avgHeightPercent = avgHeight / startHeight;

  	// weight fitness using avg. height and avg. number of links touching the floor
  	fitness = fitness*avgTD;

		// give some information
		sprintf(infTxt, "fitness: %5.4f  |  avgTD: %4.2f  |  avgHeight: %4.2f%%  |  distance: %4.2f  |  steps (calc./total): %d/%d\n", fitness, avgTD, avgHeightPercent*100.0, distance, perfSteps, steps);
		fprintf(stderr, infTxt);

  	delete simulation;
  	return fitness;
  }

}
