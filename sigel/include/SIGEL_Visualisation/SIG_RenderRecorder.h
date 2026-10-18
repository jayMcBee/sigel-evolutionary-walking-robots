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
#ifndef SIGEL_VISUALISATION_SIG_RENDERRECORDER_H
#define SIGEL_VISUALISATION_SIG_RENDERRECORDER_H

#include "SIGEL_Visualisation/SIG_SceneObject.h"
#include "SIGEL_Simulation/SIG_Recorder.h"
#include <QList>

namespace SIGEL_Visualisation
{

  /**
   * Records, at each simulation step, each link's position and rotation and the simulated
   * time, for SIG_SimulationVisualisation.
   */
  class SIG_RenderRecorder : public SIGEL_Simulation::SIG_Recorder
    {
      friend class SIG_SimulationVisualisation;

    public:

      /**
       * Creates noOfObjects scene objects in robotLinks, numbered from 0.
       */
      SIG_RenderRecorder(int noOfObjects);

      /**
       * The destructor. It frees the scene objects in robotLinks.
       */
      ~SIG_RenderRecorder() override;
      // This class owns raw pointers in a QList and frees them in its
      // destructor, so a copy would free them twice.
      SIG_RenderRecorder( const SIG_RenderRecorder & ) = delete;
      SIG_RenderRecorder &operator=( const SIG_RenderRecorder & ) = delete;

      /**
       * Records the start state.
       */
      void init() override;

      /**
       * Copies each link's position and rotation and the simulated time from the simulation
       * queries.
       */
      void record() override;

    private:

      /**
       * One scene object per link, at the index of its link number.
       */
      QList<SIG_SceneObject *> robotLinks;

      double currentSimulationSeconds;
    };

}

#endif // SIGEL_VISUALISATION_SIG_RENDERRECORDER_H
