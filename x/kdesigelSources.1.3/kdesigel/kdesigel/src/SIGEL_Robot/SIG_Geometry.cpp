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
	  if (polygons.count () >= polygons.size ()) {
	    int newsize = polygons.size () * 2;
	    if (newsize < 16) newsize = 16;
	    polygons.resize (newsize);
	  }
	  polygons.insert( polygons.count(), p );
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
                vertices.resize (geom->vertices.count ());
                for (int i = 0; i < geom->vertices.count (); i++)
                        vertices.insert (i, new DL_vector (geom->vertices.at (i)));

                polygons.resize (geom->polygons.count ());
                for (int j = 0; j < geom->polygons.count (); j++)
                        polygons.insert (j, new SIG_Polygon (this, geom->polygons.at (j)));
        }

        SIG_Geometry::SIG_Geometry (QTextStream & tx)
        {
                QString tmpstr;
                int zahl;
                
                tx >> tmpstr;
                if (tmpstr != "Geometry")
                        // FEHLER
                        ;

                tx >> zahl;
                vertices.resize (zahl);
                for (int i = 0; i < zahl; i++) {
                        DL_vector d = SIG_Robot::streamToVector (tx);
                        vertices.insert (i, new DL_vector (&d));
                }

                tx >> zahl;
                polygons.resize (zahl);
                for (int j = 0; j < zahl; j++)
                        polygons.insert (j, new SIG_Polygon (this, tx));
        }

        SIG_Geometry::~SIG_Geometry (void)
        {
                polygons.setAutoDelete (TRUE);
                vertices.setAutoDelete (TRUE);
                /* If someone insisted on doing it manually, we would be able to do so.
                int i, size;

                size = polygons.count ();
                for (i = 0; i < size; i++)
                        delete polygons [i];
                polygons.clear ();

                size = vertices.count ();
                for (i = 0; i < size; i++)
                        delete vertices [i];
                vertices.clear ();
                */
        }
        
        int SIG_Geometry::getOrAddVertex (DL_vector vertex)
        {
                int appending_position = vertices.count ();
                for (int i = 0; i < appending_position; i++) {
                        if (vertex.equal (vertices [i]))
                                return i;
                }
                if (vertices.count () >= vertices.size ()) {
                        int newsize = vertices.size () * 2;
                        if (newsize < 16) newsize = 16;
                        vertices.resize (newsize);
                }
                vertices.insert (appending_position, new DL_vector (&vertex));
                return appending_position;
        }

        Q2PtrVector<DL_vector> const & SIG_Geometry::getVertices (void) const
        {
                return vertices;
        }
        
        int SIG_Geometry::getNumVertices (void) const
        {
                return vertices.count ();
        }
        
        DL_vector SIG_Geometry::getVertex (int i) const
        {
                return *vertices.at (i);
        }

        int SIG_Geometry::getNumPolygons (void) const
        {
                return polygons.count ();
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
