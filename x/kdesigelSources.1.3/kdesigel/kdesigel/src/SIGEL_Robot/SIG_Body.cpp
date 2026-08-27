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
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include <cmath>
#include "SIGEL_Robot/SIG_GeometryIterator.h"

namespace SIGEL_Robot
{
        SIG_Body::SIG_Body(SIG_Robot *par, QString f, QString d)
                : parent(par),
                  geometryFile(f),
                  directory (d),
                  geometry(0)
        { };
        
        SIG_Body::SIG_Body(SIG_Robot *par, QTextStream & tx)
                : parent (par)
        {
                QString tmpstr;
                // "Body" has already been read in SIG_Robot.
                tx >> geometryFile;
                tx >> directory;
                tx >> tmpstr;
                if (tmpstr == "y") {
                        geometry = new SIG_Geometry (tx);
                } else {
                        geometry = 0;
                }
        };

        SIG_Body::~SIG_Body(void)
        {
                usedByLinks.setAutoDelete (false);
                delete geometry;
        };
        
        void SIG_Body::addUsingLink (SIG_Link *user)
        {  
                usedByLinks.append (user);
        };

        NEWMAT::Matrix SIG_Body::createRotationMatrix( QList< float > rotation )
        {
                NEWMAT::Matrix rotationMatrix(4,4);
                double const x = rotation[0];
                double const y = rotation[1];
                double const z = rotation[2];
                double const a = rotation[3];
                
#ifdef _WINDOWS
                double const sinA = ::sin(a);
                double const cosA = ::cos(a);
#else
                double const sinA = std::sin(a);
                double const cosA = std::cos(a);
#endif
                double const t = 1 - cosA;
                
                rotationMatrix = 0;
                rotationMatrix(1,1) = (t * x * x) + cosA;
                rotationMatrix(1,2) = (t * x * y) + (sinA * z);
                rotationMatrix(1,3) = (t * x * z) - (sinA * y);
                rotationMatrix(2,1) = (t * x * y) - (sinA * z);
                rotationMatrix(2,2) = (t * y * y) + cosA;
                rotationMatrix(2,3) = (t * y * z) + (sinA * x);
                rotationMatrix(3,1) = (t * x * z) + (sinA * y);
                rotationMatrix(3,2) = (t * y * z) - (sinA * x);
                rotationMatrix(3,3) = (t * z * z) + cosA;
                rotationMatrix(4,4) = 1;
                
                return rotationMatrix;
        };
        
        void SIG_Body::readVRMLNode(Node *node, NEWMAT::Matrix transformation)
        {
                if ( node->isGroupNode() || node->isTransformNode() ) {
                        if (node->isTransformNode()) {
                                TransformNode *transformNode = static_cast<TransformNode*> (node);
                                
                                QList< float > buffer(4);

                                NEWMAT::Matrix identity(4,4);
                                identity = 0;
                                for (int i=1; i<=4; i++)
                                        identity(i,i) = 1;

                                NEWMAT::Matrix center(4,4);
                                center = identity;
                                transformNode->getCenter( buffer.data() );
                                for (int j=0; j<3; j++)
                                        center( j+1, 4 ) = buffer[j];
                                
                                NEWMAT::Matrix rotation(4,4);
                                transformNode->getRotation( buffer.data() );
                                rotation = createRotationMatrix( buffer );
                                
                                NEWMAT::Matrix scale(4,4);
                                scale = 0;
                                scale(4,4) = 1;
                                transformNode->getScale( buffer.data() );
                                for (int k=0; k<3; k++)
                                        scale( k+1, k+1 ) = buffer[k];
                                
                                NEWMAT::Matrix scaleOrientation(4,4);
                                transformNode->getScaleOrientation( buffer.data() );
                                scaleOrientation = createRotationMatrix( buffer );
                                
                                NEWMAT::Matrix translation(4,4);
                                translation = identity;
                                transformNode->getTranslation( buffer.data() );
                                for (int l=0; l<3; l++)
                                        translation( l+1, 4 ) = buffer[l];
                                
                                transformation =   transformation
                                        * translation
                                        * center
                                        * rotation
                                        * scaleOrientation
                                        * scale
                                        * scaleOrientation.i()
                                        * center.i();
                        };

                        for (Node *child = node->getChildNodes(); child; child = child->next())
                                readVRMLNode(child, transformation);
                } else if ( node->isShapeNode() ) {
                        ShapeNode *shapeNode = static_cast<ShapeNode*> (node);

                        GeometryNode *geometryNode = shapeNode->getGeometry();

                        if (geometryNode)
                                if (geometryNode->isIndexedFaceSetNode()) {
                                        IndexedFaceSetNode *indexedFaceSetNode
                                                = static_cast<IndexedFaceSetNode*> (geometryNode);

                                        CoordinateNode *coordinateNode = indexedFaceSetNode->getCoordinateNodes();
                                        if (coordinateNode) {
                                                int noOfVertices = coordinateNode->getNPoints();
                                                int noOfIndices = indexedFaceSetNode->getNCoordIndexes();
                                                // Phase D. Was an owning Q2PtrVector of DL_vector*, kept on
                                                // setAutoDelete because the NEWMAT multiply and the SIG_Polygon
                                                // allocations below can throw and a hand-written free would drop
                                                // the unwinding path. There is no polymorphism here, so values
                                                // remove the question entirely -- nothing to own, nothing to free.
                                                QList< DL_vector > vertices( noOfVertices );

                                                for (int i=0; i < noOfVertices; i++) {
                                                        QList< float > coords(3);
                                                        
                                                        coordinateNode->getPoint( i, coords.data() );
                                                        
                                                        NEWMAT::ColumnVector actVertex(4);
                                                        for (int j=0; j<3; j++)
                                                                actVertex( j+1 ) = coords[j];
                                                        actVertex(4) = 1;
                                                        
                                                        actVertex = transformation * actVertex;
                                                        
                                                        for (int k = 0; k<3; k++)
                                                                vertices[ i ].set( k, actVertex( k+1 ) );
                                                };
                                                
                                                SIG_Polygon *actPolygon = 0;
                                                
                                                for (int j=0; j < noOfIndices; j++) {
                                                        int actIndex = indexedFaceSetNode->getCoordIndex(j);
                                                        
                                                        // actIndex comes straight out of the VRML file, and a
                                                        // negative one ends the face. Q2PtrVector::at used to
                                                        // clamp an out-of-range index to 0; QList does not.
                                                        // The check has to come BEFORE the polygon is created:
                                                        // SIG_Polygon self-registers with the geometry in its
                                                        // constructor, so creating one and then skipping every
                                                        // vertex leaves a 0-vertex face behind, which
                                                        // SIG_Mirtich::compFaceNormal reads verts[0..2] from.
                                                        if (actIndex < 0)
                                                                actPolygon = 0;
                                                        else if ( actIndex < vertices.size() ) {
                                                                if (!actPolygon)
                                                                        actPolygon = new SIG_Polygon( geometry );
                                                                actPolygon->appendVertex( vertices[ actIndex ] );
                                                        };
                                                };
                                        };
                                };
                };
        };
        
        void SIG_Body::load()
        {
                // If there is already a geometry, don't create another!
                // To Do: Replace the simple return with an exception.
                if (geometry) return;
                
                geometry = new SIG_Geometry ();
                
                SceneGraph *bodyScene = new SceneGraph();
                
                QByteArray geometryFileBytes = getGeometryFile ().toUtf8();
                bodyScene->load( geometryFileBytes.data() );
                
                NEWMAT::Matrix identity(4,4);
                identity = 0;
                for (int i=1; i<=4; i++)
                        identity(i,i) = 1;

                for (Node *actNode = bodyScene->getNodes(); actNode; actNode = actNode->next())
                        readVRMLNode( actNode,  identity);
        }

        void SIG_Body::setGeometry (SIG_Geometry *geo)
        {
                geometry = geo;
        };

        SIG_Geometry const *SIG_Body::getGeometry (void) const
        {
                return geometry;
        };

        QString SIG_Body::getGeometryFile (void) const
        {
                // Qt 2's QString::at(uint) was bounds-safe -- it returned QChar::null past
		// the end (qstring.h:483). Qt 6's asserts, and on a null string it
		// dereferences a null pointer. SIG_Robot.cpp:365 produces a null
		// geometryFile from a truncated stream. (D13)
		if (!geometryFile.isEmpty() && geometryFile.at (0) == '/')
                        return geometryFile;
                else {
                        QString retval;
                        retval = directory;
                        retval.append (geometryFile);
                        return retval;
                }
        }

        QString SIG_Body::getName (void) const 
        { 
                return geometryFile;
        }

        void SIG_Body::writeToFileTransfer (QTextStream & tx)
        {
                tx << "Body " << geometryFile << ' ' << directory << ' ';
                if (geometry) {
                        tx << "y\n";
                        geometry->writeToFileTransfer (tx);
                } else
                        tx << "n\n";
        };
}
































