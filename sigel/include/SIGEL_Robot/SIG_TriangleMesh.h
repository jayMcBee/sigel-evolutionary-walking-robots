/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef SIGEL_ROBOT_SIG_TRIANGLEMESH_H
#define SIGEL_ROBOT_SIG_TRIANGLEMESH_H

#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_Tools/SIG_Vector.h"

#include <QList>

namespace SIGEL_Robot
{

  /**
   * The surface of a body as triangles, and the box around them. It tells
   * whether a point is inside the body and how much volume two bodies share.
   * The surface must be closed.
   */
  class SIG_TriangleMesh
  {

  public:

    SIG_TriangleMesh();

    void addTriangle( SIG_Vector a, SIG_Vector b, SIG_Vector c );

    bool contains( const SIG_Vector &point ) const;

    /**
     * The volume inside both meshes, from this many sample points in the box
     * that the two meshes' boxes share.
     */
    double sharedVolume( const SIG_TriangleMesh &other, int samples, SIGEL_Tools::SIG_Randomizer &randomizer ) const;

  private:

    struct Triangle { SIG_Vector a, b, c; };

    void widenBox( SIG_Vector corner );

    QList<Triangle> triangles;
    SIG_Vector lowCorner;
    SIG_Vector highCorner;

  };

}

#endif // SIGEL_ROBOT_SIG_TRIANGLEMESH_H
