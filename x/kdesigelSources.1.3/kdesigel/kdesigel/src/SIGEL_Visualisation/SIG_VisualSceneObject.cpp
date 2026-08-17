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
#include "SIGEL_Visualisation/SIG_VisualSceneObject.h"

namespace SIGEL_Visualisation
{
  SIG_VisualSceneObject::SIG_VisualSceneObject(int number,
					       QString name,
					       DL_vector position,
					       DL_matrix rotation,
					       DL_vector color)
    : SIG_SceneObject(number, position, rotation),
      name(name),
      color(color),
      transformationInternal(16),
      colorInternal(3),
      visible(true),
      floatingText(0)
  {
    for (int j=0; j<3; j++)
      transformationInternal[ (j * 4) + 3 ] = 0;
    transformationInternal[ 15 ] = 1;

    updatePositionInternal();
    updateRotationInternal();
    updateColorInternal();
  };

  int SIG_VisualSceneObject::getNumber()
  {
    return number;
  };

  void SIG_VisualSceneObject::setPosition(DL_vector newPosition)
  {
    position = newPosition;
    updatePositionInternal();
  };

  DL_vector SIG_VisualSceneObject::getPosition() const
  {
    return position;
  };

  void SIG_VisualSceneObject::setRotation(DL_matrix newRotation)
  {
    rotation = newRotation;
    updateRotationInternal();
  };

  DL_matrix SIG_VisualSceneObject::getRotation() const
  {
    return rotation;
  };

  void SIG_VisualSceneObject::setColor(DL_vector newColor)
  {
    color = newColor;
    updateColorInternal();
  };

  DL_vector SIG_VisualSceneObject::getColor() const
  {
    return color;
  };

  void SIG_VisualSceneObject::applyTransformation()
  {
    glMultMatrixd(transformationInternal.data());
  };

  void SIG_VisualSceneObject::applyColor()
  {
    glColor3dv(colorInternal.data());
  };

  void SIG_VisualSceneObject::updatePositionInternal()
  {
    for (int i=0; i<3; i++)
      transformationInternal[ 12 + i ] = static_cast<GLdouble>(position.get(i));
  };

  void SIG_VisualSceneObject::updateRotationInternal()
  {
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	transformationInternal[ i + (4 * j) ] = static_cast<GLdouble>(rotation.get(i,j));
  };

  void SIG_VisualSceneObject::updateColorInternal()
  {
    for (int i=0; i<3; i++)
      colorInternal[ i ] = static_cast<GLdouble>(color.get(i));
  };

  void SIG_VisualSceneObject::setVisible( bool newVisible )
  {
    visible = newVisible;
  };

  bool SIG_VisualSceneObject::getVisible()
  {
    return visible;
  };

  void SIG_VisualSceneObject::setFloatingText( SIG_FloatingText *newFloatingText )
  {
    floatingText = newFloatingText;
  };

  SIG_FloatingText *SIG_VisualSceneObject::getFloatingText()
  {
    return floatingText;
  };

}
