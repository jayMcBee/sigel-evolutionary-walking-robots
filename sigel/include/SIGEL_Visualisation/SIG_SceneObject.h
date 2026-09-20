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

#include "pointvector.h"
#include "matrix.h"

namespace SIGEL_Visualisation
{

  /**
   * This class contains the basic information set
   * about one object that can change in each
   * simulation step.
   *
   * This is the type of information that is recorded
   * in each time step by the SIG_RenderRecorder.
   */
  class SIG_SceneObject
    {
    public:

      /**
       * The constructor of the SIG_SceneObject.
       *
       * @param number Initializes the attribute number.
       * @param position Initializes the attribute position.
       * @param rotation Initializes the attribute rotation.
       */
      SIG_SceneObject(int number,
		      DL_vector position = DL_vector(),
		      DL_matrix rotation = DL_matrix());

      /**
       * The number of this SIG_SceneObject.
       *
       * Should be equal to the position of the respective
       * SIG_VisualSceneObject in the QList sceneObjects in
       * the class SIG_Renderer.
       * It makes sense if this number also equals some enumeration
       * in the source datastructure (SIG_Link objects in the SIG_Robot
       * for example).
       */
      int const number;

      /**
       * The current position of the particular object.
       */
      DL_vector position;

      /**
       * The current rotation of the particular object.
       */
      DL_matrix rotation;

    };

}

#endif // SIGEL_VISUALISATION_SIG_SCENEOBJECT_H
