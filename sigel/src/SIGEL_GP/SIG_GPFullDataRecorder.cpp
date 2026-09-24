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
#include <exception>
#include "SIGEL_Tools/SIG_IO.h"

#include <cmath>

namespace SIGEL_GP
{

  SIG_GPFullDataRecorder::SIG_GPFullDataRecorder( int recordingFrequency )
    : SIGEL_Simulation::SIG_Recorder(), recordingFrequency( recordingFrequency ),
    frameCounter( 0 )
  {

    if (recordingFrequency <= 0) {
      recordingFrequency = 1;
    }
  };

  // This class owns positions and rotations.
  SIG_GPFullDataRecorder::~SIG_GPFullDataRecorder()
  {
    qDeleteAll( positions );   positions.clear();
    qDeleteAll( rotations );   rotations.clear();
    // listForces is NOT freed here and never was, even in 1.3: the force
    // vectors belong to SIG_GPForceFitnessFunction, which frees them at the
    // end of its evaluation.
    listForces.clear();
  };


  void SIG_GPFullDataRecorder::init()
  {
    // See SIG_GPSimpleRecorder::init -- this one is also reached from
    // SIG_Simulation's constructor, outside every fitness function's try.
    try {

#ifdef _WINDOWS
    SIG_Recorder::init();
#else
    SIGEL_Simulation::SIG_Recorder::init();
#endif

    record();
  
    }
    catch (SIGEL_Simulation::SIG_RecorderNoQueriesSetException &) { throw; }
    catch (SIGEL_Simulation::SIG_RecorderBadRecordingOrderException &) { throw; }
    catch (...) { std::terminate(); }
  };


  void SIG_GPFullDataRecorder::record()
  {
    if (frameCounter == 0) {
        int rootLinkNumber = simulationQueries->getRootNumber();

        // get the current position and rotation of the root link
        DL_vector *newPosition = new DL_vector();
        DL_matrix *newRotation = new DL_matrix();

        *newPosition = simulationQueries->getLinkPosition( rootLinkNumber );
        *newRotation = simulationQueries->getLinkOrientation( rootLinkNumber );

        positions.append( newPosition );
        rotations.append( newRotation );

        // get the used forces of one iteration,
		std::vector<double*>* usedForces = simulationQueries->getUsedForces();
        // append these Forces to a list
        listForces.append(usedForces);
    }

    frameCounter++;
    frameCounter %= recordingFrequency;
  }


void SIG_GPFullDataRecorder::finish()
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
