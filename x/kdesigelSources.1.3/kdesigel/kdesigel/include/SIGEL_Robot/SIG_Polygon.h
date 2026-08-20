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
#ifndef SIGEL_ROBOT_SIG_POLYGON
#define SIGEL_ROBOT_SIG_POLYGON

namespace SIGEL_Robot { class SIG_Polygon; }

#include <pointvector.h>
#include <matrix.h>
#include <qtextstream.h>
#include "SIGEL_Robot/SIG_Geometry.h"

namespace SIGEL_Robot {
        /**
         * A polygon class.
         */
        class SIG_Polygon {
                friend class SIG_Geometry;
                
        private:
                SIG_Geometry *myGeometry;
                Q2Array<int> vertices;

        protected:
                /**
                 * "Unreal" copy constructor.
                 *
                 * Duplicates the array of vertex indexes. This is
                 * intended to be called by SIG_Geometry only.
                 * You probably guess that this could be the reason
                 * why the constructor is protected.
                 */
                SIG_Polygon (SIG_Geometry *within, SIG_Polygon *other);
                
        public:
                /**
                 * Quasi standard constructor.
                 *
                 * This constructor creates an empty polygon.
                 */
                SIG_Polygon (SIG_Geometry *within);
                /**
                 * Unstreaming constructor.
                 *
                 * Reads the polygon data from a stream.
                 */
                SIG_Polygon (SIG_Geometry *mygeom, QTextStream & tx);
                /**
                 * Destructor.
                 */
                ~SIG_Polygon (void);

                /**
                 * Appends a vertex to the polygon.
                 */
                void appendVertex (DL_vector pt);
                /**
                 * Returns the number of vertices.
                 */
                int getNumVertices (void) const;
                /**
                 * Returns the ith vertex.
                 */
                DL_vector getVertex (int nr) const;
                /**
                 * Returns the index of the ith vertex.
                 *
                 * The index is an index into the array of vertices
                 * held in the SIG_Geometry object.
                 */
                int getVertexIndex (int nr) const;

                /**
                 * Writes the polygon to a stream.
                 */
                void writeToFileTransfer (QTextStream & tx) const;
        };
}

#endif
