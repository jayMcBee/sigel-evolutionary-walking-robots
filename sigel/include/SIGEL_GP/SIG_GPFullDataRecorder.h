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
#ifndef SIGEL_GP_SIG_GPFULLDATARECORDER_H
#define SIGEL_GP_SIG_GPFULLDATARECORDER_H

#include <QList>
#include "SIGEL_Simulation/SIG_Recorder.h"


namespace SIGEL_GP
{

  class SIG_GPFullDataRecorder : public SIGEL_Simulation::SIG_Recorder
    {

      friend class SIG_GPRealSpeedFitnessFunction;
      friend class SIG_GPNiceWalkingFitnessFunction;
      friend class SIG_GPAdaptiveWalkingFitnessFunction;
      friend class SIG_GPStepperFitnessFunction;
      friend class SIG_GPForceFitnessFunction;
      friend class SIG_GPEnergyFitnessFunction;

    public:

      SIG_GPFullDataRecorder( int recordingFrequency );

      /** This recorder owns its positions, rotations and touchdowns. */

      ~SIG_GPFullDataRecorder();

      void init();

      void record();

      void finish();

      QList< DL_vector * > positions;
      QList< DL_matrix * > rotations;
      QList< int * > touchdowns;


    private:

      DL_vector endPosition;
      DL_matrix endRotation;

      QList< std::vector<double*> * > listForces;
      int numLinks;
      int recordingFrequency;
      int frameCounter;
    };

}

#endif // SIGEL_GP_SIG_GPFULLDATARECORDER_H
