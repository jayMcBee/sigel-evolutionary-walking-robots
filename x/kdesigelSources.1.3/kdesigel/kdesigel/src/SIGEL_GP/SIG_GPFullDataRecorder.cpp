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

#include "SIGEL_GP/SIG_GPFullDataRecorder.h"
#include "SIGEL_Tools/SIG_IO.h"

#include <cmath>

namespace SIGEL_GP
{

  SIG_GPFullDataRecorder::SIG_GPFullDataRecorder( int recordingFrequency )
    : SIGEL_Simulation::SIG_Recorder(), recordingFrequency( recordingFrequency ),
    frameCounter( 0 )
  {
    positions.setAutoDelete( true );
    rotations.setAutoDelete( true );
    touchdowns.setAutoDelete( true );

    if (recordingFrequency <= 0) {
      recordingFrequency = 1;
    }
  };


  void SIG_GPFullDataRecorder::init()
    throw (SIGEL_Simulation::SIG_RecorderNoQueriesSetException,
	   SIGEL_Simulation::SIG_RecorderBadRecordingOrderException)
  {
#ifdef _WINDOWS
    SIG_Recorder::init();
#else
    SIGEL_Simulation::SIG_Recorder::init();
#endif

    record();
  };


  void SIG_GPFullDataRecorder::record()
    throw (SIGEL_Simulation::SIG_RecorderNoQueriesSetException, SIGEL_Simulation::SIG_RecorderBadRecordingOrderException)
  {
    int *curTD;

    if (frameCounter == 0) {
        int rootLinkNumber = simulationQueries->getRootNumber();

        // get the current position and rotation of the root link
        DL_vector *newPosition = new DL_vector();
        DL_matrix *newRotation = new DL_matrix();

        *newPosition = simulationQueries->getLinkPosition( rootLinkNumber );
        *newRotation = simulationQueries->getLinkOrientation( rootLinkNumber );

        positions.append( newPosition );
        rotations.append( newRotation );

        // get the number of links touching the floor
        curTD = new int;
        *curTD = simulationQueries->getNumberOfTouchdowns();

        // get the used forces of one iteration,
		std::vector<double*>* usedForces = simulationQueries->getUsedForces();
        // append these Forces to a list
        listForces.append(usedForces);

        // SIGEL_Tools::SIG_IO::cerr << "Touchdowns: " << *curTD << "\n";
        touchdowns.append(curTD);
    }

    frameCounter++;
    frameCounter %= recordingFrequency;
  }


void SIG_GPFullDataRecorder::finish()
  throw (SIGEL_Simulation::SIG_RecorderNoQueriesSetException,
	 SIGEL_Simulation::SIG_RecorderBadRecordingOrderException)
{
#ifdef _WINDOWS
  SIG_Recorder::finish();
#else
  SIGEL_Simulation::SIG_Recorder::finish();
#endif

  int rootLinkNumber = simulationQueries->getRootNumber();

  endPosition = simulationQueries->getLinkPosition( rootLinkNumber );
  endRotation = simulationQueries->getLinkOrientation( rootLinkNumber );
};

}
