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

#include <qstring.h>

#include "SIGEL_Visualisation/SIG_SceneObject.h"
#include "SIGEL_Visualisation/SIG_FloatingText.h"

#include "pointvector.h"
#include "matrix.h"
#include "GL/gl.h"

namespace SIGEL_Visualisation
{

  /**
   * This class models a renderable object.
   *
   * The inherited attributes are now private.
   * They may be only accessed via the appropriate
   * set- ang get-methods, because their setting influences
   * internal OpenGL-data.
   * Together with color information it contains everything
   * that can change during general visualisation (highlighting for
   * example).
   */
  class SIG_VisualSceneObject : private SIG_SceneObject
    {

    public:

      /**
       * The constructor of the SIG_VisualSceneObject.
       *
       * Initializes the attributes.
       *
       * @param number Initializes the attribute number.
       * @param name Initializes the attribute name.
       * @param position Initializes the attribute position.
       * @param color Initializes the attribute color.
       */
      SIG_VisualSceneObject(int number,
			    QString name,
			    DL_vector position = DL_vector(),
			    DL_matrix rotation = DL_matrix(),
			    DL_vector color = DL_vector());

      /**
       * Gets the objects number.
       */
      int getNumber();

      /**
       * Sets the object's position.
       *
       * Sets the attribute position and
       * updates the transformation matrix transformationInternal.
       * The last task is performed by calling the method
       * updateTransformationInternal.
       */
      void setPosition(DL_vector newPosition);

      /**
       * Gets the object's position.
       */
      DL_vector getPosition() const;

      /**
       * Sets the object's rotation.
       *
       * Sets the attribute rotation and
       * updates the transformation matrix transformationInternal.
       * The last task is performed by calling the method
       * updateTransformationInternal.
       */
      void setRotation(DL_matrix newRotation);

      /**
       * Gets the object's rotation.
       */
      DL_matrix getRotation() const;

      /**
       * Sets the object's color.
       *
       * Sets the attribute color and
       * updates the attribute colorInternal.
       */
      void setColor(DL_vector newColor);

      /**
       * Gets the object's color.
       */
      DL_vector getColor() const;

      /**
       * Executes appropriate OpenGL commands
       * to apply this object's transformation matrix
       * transformationInternal.
       */
      void applyTransformation();

      /**
       * Executes appropriate OpenGL command
       * to apply this object's color
       * by using the attribute coloInternal.
       */
      void applyColor();

      /**
       * Sets the attribute visible.
       */
      void setVisible( bool newVisible );

      /**
       * Gets the attribute visible.
       */
      bool getVisible();

      void setFloatingText( SIG_FloatingText *newFloatingText );

      SIG_FloatingText *getFloatingText();

      /**
       * The name of this SIG_VisualSceneObject.
       */
      QString const name;

    private:

      /**
       * Recalculate the elements of the attribute
       * transformationInternal dependent from
       * the attribute position from the actual
       * position.
       */
      void updatePositionInternal();

      /**
       * Recalculate the elements of the attribute
       * transformationInternal dependent from
       * the attribute rotation from the actual
       * rotation.
       */
      void updateRotationInternal();

      /**
       * Sets the elements of the attribute
       * colorInternal according to the attribute color.
       */
      void updateColorInternal();

      /**
       * The current color of this SIG_VisualSceneObject.
       */
      DL_vector color;

      /**
       * Internal representation of the
       * attributes position and rotation as a
       * homogenous transformation matrix.
       *
       * Used for OpenGL rendering.
       * The QArray contains the matrix columnwise, so it
       * should contain 16 elements.
       *
       * @invariant transformationInternal has to perform the
       *            same transformation as a %rotation about the
       *            attribute rotation and a translation about
       *            the attribute position. This is guaranted
       *            by the method updateTransformationInternal.
       */
      QArray<GLdouble> transformationInternal;

      /**
       * Internal representation the attribute color.
       *
       * Used for OpenGL rendering.
       * The QArray contains the R, G and B components
       * of the object's color.
       */
      QArray<GLdouble> colorInternal;

      /**
       * Controls if the object is rendered.
       */
      bool visible;

      SIG_FloatingText *floatingText;

    };

}

#endif // SIGEL_VISUALISATION_SIG_VISUALSCENEOBJECT_H
