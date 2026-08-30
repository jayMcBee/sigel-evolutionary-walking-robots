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
#ifndef SIGEL_VISUALISATION_SIG_VISUALISATION_H
#define SIGEL_VISUALISATION_SIG_VISUALISATION_H

#include "SIGEL_Visualisation/SIG_ViewSettings.h"
#include "SIGEL_Visualisation/SIG_FloatingText.h"

#include <QList>

#include <newmat.h>

/**
 * This namespace (package) contains all classes that are responsible
 * for the visualisation of robots (class SIGEL_Robot::SIG_Robot),
 * environments (class SIGEL_Environment::SIG_Environment),
 * and simulations (class SIGEL_Simulation::SIG_Simulation).
 *
 * In detail they are responsibel for calling the appropriate
 * OpenGL commands. The GUI-side of the visualisation
 * is found in other packages.
 */
namespace SIGEL_Visualisation
{

  /**
   * This is the superclass of all
   * visualisation classes.
   *
   * This classes task is to encapsulate all OpenGL
   * commands for rendering of different kinds of things
   * (robots, environments, simulations).
   * Subclasses should contain objects inherited from
   * the class SIG_Renderer.
   */
  class SIG_Visualisation
    {
    public:

      /**
       * The constructor of the SIG_Visualisation.
       *
       * Maybe initializes some OpenGL stuff.
       */
      SIG_Visualisation();

      /**
       * The destructor of the SIG_Visualisation.
       *
       * Maybe cleans up some OpenGL stuff.
       */
      virtual ~SIG_Visualisation();

      /**
       * This method sets up the projection/modelview matrices
       * according to the actual viewSettings.
       *
       * The method visualize of subclasses should first call
       * SIG_Visualisation::visualize and after that initiate
       * the final rendering.
       */
      virtual void visualize();

      virtual void setAmbientSceneColor( double red,
					 double green,
					 double blue );

      /**
       * This method recalculates the projection matrix
       * according to the actual aspect ratio.
       *
       * Is called by the constructor and the widget class
       * class. Tha latter needs only to do this in case
       * of changed widget proportions.
       */
      void updateAspectRatio();

      /**
       * The current view settings.
       *
       * These should be used by the method visualize.
       */
      SIG_ViewSettings viewSettings;

      QList< SIG_FloatingText * > floatingTexts;

    protected:
      NEWMAT::ColumnVector ambientSceneColor;

    };

}

#endif // SIGEL_VISUALISATION_SIG_VISUALISATION_H
