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

#include "SIGEL_Tools/SIG_Vector.h"

namespace SIGEL_Visualisation
{

  /**
   * The camera and the render mode of a view.
   */
  class SIG_ViewSettings
    {
    public:

      /**
       * Starts in wireframe mode, with the eye at (0, 1, 1) and the look point at the origin.
       */
      SIG_ViewSettings();

      /**
       * The eye point in world coordinates: lookPoint + eyePoint when
       * relativeEyePoint is set, else eyePoint.
       */
      SIG_Vector getAbsoluteEyePoint() const;

      /**
       * How the scene is drawn. The simulation view shows shadows only in flatShaded and
       * garoudShaded.
       */
      enum renderModeType
      { 
	/**
	 * Edges only, in each object's colour, without lighting.
	 */
	wireFrame,
	/**
	 * As wireFrame, but an edge behind a polygon is not drawn.
	 */
	hiddenLine,
	/**
	 * Filled, lit polygons; the lighting is the same across each polygon.
	 */
	flatShaded,
	/**
	 * As flatShaded, but the colour is blended across each polygon. Every vertex has its
	 * polygon's normal, so the picture differs little from flatShaded.
	 */
	garoudShaded
      };

      /**
       * The eye's position; relative to lookPoint when relativeEyePoint is set.
       */
      SIG_Vector eyePoint;

      /**
       * The point the eye looks at.
       */
      SIG_Vector lookPoint;

      /**
       * The up vector of the projection.
       *
       * Must not be parallel to the line of sight.
       */
      SIG_Vector up;

      bool relativeEyePoint;

      renderModeType renderMode;

      /**
       * The window's width divided by its height.
       */
      double aspectRatio;
    };

}

#endif // SIGEL_VISUALISATION_SIG_VIEWSETTINGS_H
