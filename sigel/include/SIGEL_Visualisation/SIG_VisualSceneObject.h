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
#ifndef SIGEL_VISUALISATION_SIG_VISUALSCENEOBJECT_H
#define SIGEL_VISUALISATION_SIG_VISUALSCENEOBJECT_H

#include <QList>
#include <qstring.h>

#include "SIGEL_Visualisation/SIG_SceneObject.h"
#include "SIGEL_Visualisation/SIG_FloatingText.h"

#include "SIGEL_Tools/SIG_Vector.h"
#include "SIGEL_Tools/SIG_Matrix.h"
#include "GL/gl.h"

namespace SIGEL_Visualisation
{

  /**
   * An object that a renderer draws: its position, rotation, colour and visibility. The
   * position and rotation are also kept as an OpenGL matrix, so they change only through
   * the setters.
   */
  class SIG_VisualSceneObject : private SIG_SceneObject
    {
    public:

      SIG_VisualSceneObject(int number, QString name, SIG_Vector color = SIG_Vector());

      int getNumber() const { return number; }

      /**
       * Sets the position and its part of transformationInternal.
       */
      void setPosition(SIG_Vector newPosition);

      SIG_Vector getPosition() const { return position; }

      /**
       * Sets the rotation and its part of transformationInternal.
       */
      void setRotation(SIG_Matrix newRotation);

      SIG_Matrix getRotation() const { return rotation; }

      /**
       * Sets the colour and colorInternal.
       */
      void setColor(SIG_Vector newColor);

      SIG_Vector getColor() const { return color; }

      /**
       * Multiplies the current OpenGL matrix by transformationInternal.
       */
      void applyTransformation();

      /**
       * Sets the current OpenGL colour to colorInternal.
       */
      void applyColor();

      void setVisible( bool newVisible );

      bool getVisible() const { return visible; }

      void setFloatingText( SIG_FloatingText *newFloatingText );

      SIG_FloatingText *getFloatingText() const { return floatingText; }

      /**
       * The POV-Ray export uses this name to refer to the object's #declare, so the two
       * names must be the same.
       */
      QString const name;

    private:

      /**
       * Copies position into transformationInternal.
       */
      void updatePositionInternal();

      /**
       * Copies rotation into transformationInternal.
       */
      void updateRotationInternal();

      /**
       * Copies color into colorInternal.
       */
      void updateColorInternal();

      SIG_Vector color;

      /**
       * position and rotation as a 4 × 4 matrix, 16 elements in OpenGL's column order.
       */
      QList<GLdouble> transformationInternal;

      /**
       * color as red, green and blue for OpenGL.
       */
      QList<GLdouble> colorInternal;

      /**
       * Whether the renderer draws this object and the POV-Ray export writes it.
       */
      bool visible;

      /**
       * Not owned: the renderer's floatingTexts deletes it.
       */
      SIG_FloatingText *floatingText;
    };

}

#endif // SIGEL_VISUALISATION_SIG_VISUALSCENEOBJECT_H
