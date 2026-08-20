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
#ifndef SIGEL_GP_SIG_GPSIMPLERECORDER_H
#define SIGEL_GP_SIG_GPSIMPLERECORDER_H

#include "SIGEL_Simulation/SIG_Recorder.h"

namespace SIGEL_GP
{

  /**
   * This class records the start and the end positions of the
   * torso element of the robot and calculates its fitness by
   * calculating the absolut fitness.
   */
  class SIG_GPSimpleRecorder : public SIGEL_Simulation::SIG_Recorder
    {

      friend class SIG_GPSimpleFitnessFunction;

    public:

      /**
       * The constructor of the SIG_GPRecorder.
       */
      SIG_GPSimpleRecorder();

      /**
       * Saves the start position of the torso element
       *
       * First it calls the method init of the superclass
       * SIGEL_Simulation::SIG_Recorder.
       */
      void init();

      /**
       * Records the forces of all robot drives.
       *
       * First it calls the method finish of the superclass
       * SIGEL_Simulation::SIG_Recorder.
       */
      void record();

      /**
       * Reads the end position of the torso element
       *
       * First it calls the method finish of the superclass
       * SIGEL_Simulation::SIG_Recorder.
       */
      void finish();
	       
    private:

      /**
       * The starting position
       */
      DL_vector start;
      DL_vector end;

      DL_matrix startRotation;
      DL_matrix endRotation;
    };

}


#endif // SIGEL_GP_SIG_GPSIMPLERECORDER_H
