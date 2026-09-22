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
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include <stdio.h>

namespace SIGEL_Robot {
        SIG_Polygon::SIG_Polygon (SIG_Geometry *within, SIG_Polygon *other)
                : myGeometry (within),
                  vertices (0)
        {
                vertices.resize (other->vertices.count ());
                for (int i = 0; i < other->vertices.count (); i++)
                        vertices [i] = other->vertices [i];
        }
        
        SIG_Polygon::SIG_Polygon (SIG_Geometry *within)
                : myGeometry (within),
                  vertices (0)
        {
                within->addPolygon (this);
        }

        SIG_Polygon::SIG_Polygon (SIG_Geometry *mygeom, QTextStream & tx)
                : myGeometry (mygeom)
        {
                QString tmpstr;
                int vertexCount;

                tx >> tmpstr;
                if (tmpstr != "Polygon")
                        // ERROR
                        ;
                
                tx >> vertexCount;
                vertices.resize (vertexCount);
                for (int i = 0; i < vertexCount; i++)
                        tx >> vertices [i];
        }

        SIG_Polygon::~SIG_Polygon (void)
        {
        }

        void SIG_Polygon::appendVertex (DL_vector pt)
        {
                int idx = vertices.size ();
                vertices.resize (idx + 1);
                vertices [idx] = myGeometry->getOrAddVertex (pt);
        }

        int SIG_Polygon::getNumVertices (void) const
        {
                return vertices.size ();
        }

        DL_vector SIG_Polygon::getVertex (int nr) const
        {
                return myGeometry->getVertex (vertices [nr]);
        }

        int SIG_Polygon::getVertexIndex (int nr) const
        {
                return vertices [nr];
        }

        void SIG_Polygon::writeToFileTransfer (QTextStream & tx) const
        {
                int vc = vertices.count ();
                tx << "Polygon " << vc;
                for (int i = 0; i < vc; i++) {
                        tx << ' ' << vertices [i];
                }
                tx << '\n';
                        
        }
}
