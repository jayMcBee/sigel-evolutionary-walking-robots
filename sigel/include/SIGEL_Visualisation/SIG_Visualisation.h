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
#include <QVector3D>

#include <newmat.h>

/**
 * Draws robots, environments and simulations with OpenGL and writes them as POV-Ray
 * scenes. The windows that show them are in other modules.
 */
namespace SIGEL_Visualisation
{

  /**
   * Base class of the visualisations: the OpenGL state, the camera, the sky and the fog.
   * Subclasses draw their scene with SIG_Renderer objects.
   */
  class SIG_Visualisation
    {
    public:

      /**
       * Sets the OpenGL state: depth test, lights, fog and the projection.
       *
       * @pre An OpenGL context is current.
       */
      SIG_Visualisation();

      virtual ~SIG_Visualisation() = default;

      /**
       * Clears the frame, draws the sky, and sets the camera, the fog, the light at the eye
       * point and the render mode from viewSettings. A subclass's visualize calls this first,
       * then draws its scene.
       */
      virtual void visualize();

      virtual void setAmbientSceneColor( double red, double green, double blue );

      NEWMAT::ColumnVector getAmbientSceneColor() const { return ambientSceneColor; }

      /**
       * Sets the projection matrix for viewSettings.aspectRatio. The widget calls it when its
       * size changes.
       */
      void updateAspectRatio();

      /**
       * The vertical field of view of the projection, in degrees.
       */
      static double const fieldOfView;

      /**
       * The camera and the render mode.
       */
      SIG_ViewSettings viewSettings;

      QList< SIG_FloatingText * > floatingTexts;

    protected:
      NEWMAT::ColumnVector ambientSceneColor;

    private:

      static constexpr QVector3D skyTopColor = QVector3D( 0.70f, 0.80f, 0.95f );

      static constexpr QVector3D skyBottomColor = QVector3D( 1.0f, 1.0f, 1.0f );

      /** The fog starts and ends this far behind the look point. */
      static constexpr float fogStart = 0.0f;
      static constexpr float fogEnd = 22.0f;

      void drawSky() const;

      /** Sets the fog's range and gives it the sky's colour at the horizon. */
      void setFog( SIG_Vector const &eyePoint ) const;
    };

}

#endif // SIGEL_VISUALISATION_SIG_VISUALISATION_H
