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
#ifndef SIGEL_ROBOT_SIG_BODY_H
#define SIGEL_ROBOT_SIG_BODY_H

namespace SIGEL_Robot { class SIG_Body; }
#include <qstring.h>
#include <CyberVRML97.h>
#include <newmat.h>
#include <cstdlib>
#include <qlist.h>
#include <qvector.h>
#include <pointvector.h>
#include <matrix.h>
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Geometry.h"

namespace SIGEL_Robot
{
        /**
         * The class SIG_Body represents the geometrical data
         * of a link.
         * 
         * Methods to analize and normalize the geometrical
         * data, such as calculation of the inertia tensor,
         * and transformation to the coordinate system that
         * Dynamo needs.
         */
        class SIG_Body {
        private:
                SIG_Robot *parent;
                QList<SIG_Link> usedByLinks;
                QString geometryFile, directory;
                SIG_Geometry *geometry;
   
        public:
                /**
                 * Nonstandard standard constructor.
                 */
                SIG_Body (SIG_Robot *par, QString f, QString d);
                /**
                 * Unstreaming constructor.
                 *
                 * Reads the geometry and its physical attributes from
                 * a stream.
                 */
                SIG_Body (SIG_Robot *par, QTextStream & tx);
                /**
                 * Destructor.
                 */
                ~SIG_Body (void);
                
                /**
                 * Register a link as being formed like the surface described in this
                 * body object.
                 */
                void addUsingLink (SIG_Link* user);

                /**
                 * Loads the geometry from the VRML file.
                 *
                 * The load-method loads the VRML file given in the constructor
                 * into memory using CyberVRML '97 and, assuming it finds a polygon soup,
                 * detects the incidences and contructs a Polyhedron object.
                 */
                void load ();
                
                /**
                 * Help method to create the affine transformation matrix
                 * that rotates a point as specified in the argument vector.
                 *
                 * @param rotation The rotation given in VRML notation.
                 */
                NEWMAT::Matrix SIG_Body::createRotationMatrix( QArray< float > rotation );
                
                /**
                 * Help method to traverse the VRML scene graph.
                 *
                 * @param node Pointer to the node where to start the Depth-Search.
                 * @param transformation The affine transformation matrix which transforms points
                 *                       described due to the local coordinate system
                 *                       into the calling node's coordinate system
                 *                       (if there no such then transformation should be the identity).
                 */
                void readVRMLNode(Node *node, NEWMAT::Matrix transformation);

                /**
                 * Setting the geometry.
                 *
                 * In normal operation, this should not be used. Generally,
                 * the geometry information will be loaded by SIG_Body
                 * itself from a DXF file.
                 */
                void setGeometry (SIG_Geometry *geo);
                
                /**
                 * Getting access to the surface of the body.
                 */
                SIG_Geometry const *getGeometry (void) const;
                
                /**
                 * Write the SIG_Body object and the geometry information
                 * into a stream.
                 */
                void writeToFileTransfer (QTextStream & tx);
                
                /**
                 * Get the name of the geometry file.
                 */
                QString getGeometryFile (void) const;
                
                /**
                 * Get the name of the geometry file as specified in the
                 * robot description.
                 */
                QString getName (void) const;
        };
}

#endif // SIGEL_ROBOT_SIG_BODY_H
