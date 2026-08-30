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
#include "SIGEL_Simulation/SIG_SimulationCannotSolveException.h"
#include <QList>
#include <qdatetime.h>

namespace SIGEL_Visualisation
{

  /**
   * This class records the current positions and orientations
   * of the objects that take part in the SIGEL_Simulation::SIG_Simulation
   * contained in the SIG_SimulationVisualisation that created this
   * SIG_RenderRecorder.
   */
  class SIG_RenderRecorder : public SIGEL_Simulation::SIG_Recorder
    {
      friend class SIG_SimulationVisualisation;

    public:

      /**
       * The constructor of the SIG_RenderRecorder.
       *
       * Initiallizes the attribute robotLinks with the
       * supplied number of SIG_SceneObject objects.
       */
      SIG_RenderRecorder(int noOfObjects);

      /**
       * The destructor.
       *
       * robotLinks had setAutoDelete(true), so ~QVector was its only free and
       * this class had no destructor at all. Qt 6's QList frees nothing, so
       * the free is written out -- PORTING.md Phase C, C5.
       */
      ~SIG_RenderRecorder();
      // This class owns raw pointers in a QList and frees them in its
      // destructor, so a copy would free them twice. Qt 2's QGVector copy
      // cleared del_item and freed nothing -- the hazard is new with the
      // conversion, not inherited. D7, D15 and D25c did the same.
      SIG_RenderRecorder( const SIG_RenderRecorder & ) = delete;
      SIG_RenderRecorder &operator=( const SIG_RenderRecorder & ) = delete;


      /**
       * Initalizes the start positions and rotations of
       * the elements of robotLinks taken from the
       * appropriate SIGEL_Simulation::SIG_SimulationQueries.
       *
       * First it calls the method init of the superclass
       * SIGEL_Simulation::SIG_Recorder.
       */
      void init();

      /**
       * Updates the positions and rotations of the robotLinks
       * with the actual data taken from the appropriate
       * SIGEL_Simulation::SIG_SimulationQueries.
       *
       * First it calls the method record of the superclass
       * SIGEL_Simulation::SIG_Recorder.
       */
      void record();

      /**
       * This method does nothing at the moment.
       *
       * It should'nt even called by the simulation,
       * because there is no defined end of a
       * simulation visualisation.
       *
       * First it calls the method finish of the superclass
       * SIGEL_Simulation::SIG_Recorder.
       */
      void finish();

    private:

      /**
       * The data structure used to record the actual
       * positions and rotations of the simulated objects.
       */
      QList<SIG_SceneObject *> robotLinks;

      QTime simulationTime;

    };

}


#endif // SIGEL_VISUALISATION_SIG_RENDERRECORDER_H
