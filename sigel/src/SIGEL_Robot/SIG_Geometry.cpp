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
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_Robot.h"

namespace SIGEL_Robot {
        void SIG_Geometry::addPolygon (SIG_Polygon *p)
        {
	  polygons.append (p);
        }

        SIG_Polygon const *SIG_Geometry::getPolygon (int i) const
        {
                return polygons [i];
        }

        SIG_Geometry::SIG_Geometry (void)
        {
        }

        SIG_Geometry::SIG_Geometry (const SIG_Geometry *geom)
        {
                for (DL_vector *v : geom->vertices)
                        vertices.append (new DL_vector (v));

                for (SIG_Polygon *p : geom->polygons)
                        polygons.append (new SIG_Polygon (this, p));
        }

        SIG_Geometry::SIG_Geometry (QTextStream & tx)
        {
                QString tmpstr;
                int itemCount;
                
                tx >> tmpstr;
                if (tmpstr != "Geometry")
                        // ERROR
                        ;

                tx >> itemCount;
                for (int i = 0; i < itemCount; i++) {
                        DL_vector d = SIG_Robot::streamToVector (tx);
                        vertices.append (new DL_vector (&d));
                }

                tx >> itemCount;
                for (int j = 0; j < itemCount; j++)
                        polygons.append (new SIG_Polygon (this, tx));
        }

        SIG_Geometry::~SIG_Geometry (void)
        {
                qDeleteAll (polygons);
                qDeleteAll (vertices);
        }
        
        int SIG_Geometry::getOrAddVertex (DL_vector vertex)
        {
                int appending_position = vertices.size ();
                for (int i = 0; i < appending_position; i++) {
                        if (vertex.equal (vertices [i]))
                                return i;
                }
                vertices.append (new DL_vector (&vertex));
                return appending_position;
        }

        QList<DL_vector *> const & SIG_Geometry::getVertices (void) const
        {
                return vertices;
        }
        
        int SIG_Geometry::getNumVertices (void) const
        {
                return vertices.size ();
        }
        
        DL_vector SIG_Geometry::getVertex (int i) const
        {
                return *vertices.at (i);
        }

        int SIG_Geometry::getNumPolygons (void) const
        {
                return polygons.size ();
        }
        
        void SIG_Geometry::translate (DL_vector dir)
        {
                int nrofverts = vertices.count ();
                for (int i = 0; i < nrofverts; i++) {
                        vertices [i]->plusis (&dir);
                }
        }
        
        void SIG_Geometry::rotate (DL_matrix mat)
        {
                int nrofverts = vertices.count ();
                for (int i = 0; i < nrofverts; i++) {
                        DL_vector d (vertices [i]);
                        mat.times (&d, vertices [i]);
                }
        }

        void SIG_Geometry::writeToFileTransfer (QTextStream & tx) const
        {
                int nrofverts = vertices.count ();
                tx << "Geometry "
                   << vertices.count () << ' ';
                for (int i = 0; i < nrofverts; i++)
                        SIG_Robot::vectorToStream (tx, *vertices [i]);
                
                int nrofpolys = polygons.count ();
                tx << polygons.count () << '\n';
                for (int j = 0; j < nrofpolys; j++)
                        polygons [j]->writeToFileTransfer (tx);
        }
}
