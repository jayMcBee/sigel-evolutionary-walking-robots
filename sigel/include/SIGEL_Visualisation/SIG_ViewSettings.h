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
#ifndef SIGEL_VISUALISATION_SIG_VIEWSETTINGS_H
#define SIGEL_VISUALISATION_SIG_VIEWSETTINGS_H

#include "pointvector.h"

namespace SIGEL_Visualisation
{

  /**
   * This class contains all attributes of a
   * current rendering view (Viewers position for example).
   */
  class SIG_ViewSettings
    {
    public:

      /**
       * The constructor of the SIG_ViewSettings.
       *
       * Initializes all attributes with standard values.
       */
      SIG_ViewSettings();

      /**
       * The type of the render Mode.
       */
      enum renderModeType
      { 
	/**
	 * Wireframe rendering.
	 *
	 * The wire's color is just given by the
	 * object's attribute. No lighting is active.
	 */
	wireFrame,
	/**
	 * Wireframe rendering without the hidden lines.
	 *
	 * As wireFrame, but an edge behind a polygon is not drawn.
	 */
	hiddenLine,
	/**
	 * Flat shaded rendering.
	 *
	 * The polygons are filled.
	 * The fill colors are calculated from the object's
	 * atrribute and lighting with some light sources.
	 * Their positions are reatively fixed to the robot's
	 * main link position.
	 * The normal vectors of the vertices of a polygon are set
	 * to the normal vector of the polygon.
	 */
	flatShaded,
	/**
	 * Gouraud shaded rendering.
	 *
	 * The polygons are filled and garoud shaded.
	 * The colors are calculated from the object's
	 * atrribute and lighting with some light sources.
	 * Their positions are reatively fixed to the robot's
	 * main link position.
	 * The normal vectors of the vertices of a polygon are set
	 * to the normal vector of the polygon.
	 */
	garoudShaded
      };

      /**
       * The viewers position.
       */
      DL_vector eyePoint;

      /**
       * The viewers lookpoint.
       */
      DL_vector lookPoint;

      /**
       * The up vector of the projection.
       *
       * Must not be parallel to the line of sight.
       */
      DL_vector up;

      /**
       * If true, the real eyepoint is
       * calculated by adding the DL_vector
       * eyePoint to the DL_vector lookPoint.
       */
      bool relativeEyePoint;

      /**
       * The currently active render mode.
       */
      renderModeType renderMode;

      /**
       * The actual aspect ratio defined by @f$\frac{x}{y}@f$ for
       * the window width x and height y.
       */
      double aspectRatio;

    };

}

#endif // SIGEL_VISUALISATION_SIG_VIEWSETTINGS_H
