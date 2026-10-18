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
#ifndef SIGEL_VISUALISATION_SIG_SCENEOBJECT_H
#define SIGEL_VISUALISATION_SIG_SCENEOBJECT_H

#include "SIGEL_Tools/SIG_Vector.h"
#include "SIGEL_Tools/SIG_Matrix.h"

namespace SIGEL_Visualisation
{

  /**
   * The position and rotation of one object. SIG_RenderRecorder keeps one for each link
   * and sets it at each recorded step.
   */
  class SIG_SceneObject
    {
    public:

      /**
       * The position starts at the origin and the rotation at the identity.
       */
      SIG_SceneObject(int number);

      /**
       * In SIG_RenderRecorder, the link number, which is also the index in robotLinks. In a
       * SIG_VisualSceneObject, the index in SIG_Renderer::sceneObjects. An anchor point is
       * different: it has the number of its link, which gives its position and rotation.
       */
      int const number;

      SIG_Vector position;

      SIG_Matrix rotation;
    };

}

#endif // SIGEL_VISUALISATION_SIG_SCENEOBJECT_H
