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
#ifndef SIGEL_VISUALISATION_SIG_ENVIRONMENTVISUALISATION_H
#define SIGEL_VISUALISATION_SIG_ENVIRONMENTVISUALISATION_H

#include "SIGEL_Visualisation/SIG_Visualisation.h"
#include "SIGEL_Visualisation/SIG_EnvironmentRenderer.h"
#include "SIGEL_Environment/SIG_Environment.h"

namespace SIGEL_Visualisation
{

  /**
   * This class is used to visualize a SIG_Environment object.
   *
   * It contains the appropriate data structures and methods
   * to perform OpenGL commands that render an environment
   * according to current view settings like eyepoint and lookpoint.
   */
  class SIG_EnvironmentVisualisation : public SIG_Visualisation
    {

    public:

      /**
       * The constructor of the SIG_EnvironmentVisualisation.
       *
       * Does some OpenGL initialization stuff, reads the supplied
       * SIG_Environment object and builds up data structures for rendering.
       *
       * @param environment The SIG_Environment object that should be rendered.
       */
#ifdef _WINDOWS
      SIG_EnvironmentVisualisation(SIGEL_Environment::SIG_Environment &environment);
#else
      SIG_EnvironmentVisualisation(SIGEL_Environment::SIG_Environment const &environment);
#endif

      /**
       * The destructor of the SIG_EnvironmentVisualisation.
       *
       * Does some OpenGL clean up.
       */
      ~SIG_EnvironmentVisualisation();

      /**
       * This method finally initiates the calling
       * of certain OpenGL rendering commands.
       *
       * They refresh the active framebuffer of the
       * current OpenGL context according to the
       * current state of the attribute viewSettings.
       */
      void visualize();

    private:

      /**
       * The SIG_EnvironmentRenderer object that is
       * responsible for the rendering of the environment.
       *
       * While this SIG_EnvironmentVisualisation object
       * takes care of all OpenGL related things to be done
       * (framebuffer initialization and setting of
       * OpenGL state variables for example),
       * the final rendering of the environment
       * is done by the environmentRenderer.
       */
      SIG_EnvironmentRenderer environmentRenderer;

    };

}

#endif // SIGEL_VISUALISATION_SIG_ENVIRONMENTVISUALISATION_H
