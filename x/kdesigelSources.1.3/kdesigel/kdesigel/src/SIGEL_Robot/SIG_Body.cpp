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
                // "Body" wurde bereits in SIG_Robot gelesen.
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
                usedByLinks.setAutoDelete (FALSE);
                delete geometry;
        };
        
        void SIG_Body::addUsingLink (SIG_Link *user)
        {  
                usedByLinks.append (user);
        };

        NEWMAT::Matrix SIG_Body::createRotationMatrix( QArray< float > rotation )
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
                                
                                QArray< float > buffer(4);

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
                                                QVector< DL_vector > vertices( noOfVertices );
                                                vertices.setAutoDelete( true );

                                                for (int i=0; i < noOfVertices; i++) {
                                                        QArray< float > coords(3);
                                                        
                                                        coordinateNode->getPoint( i, coords.data() );
                                                        
                                                        NEWMAT::ColumnVector actVertex(4);
                                                        for (int j=0; j<3; j++)
                                                                actVertex( j+1 ) = coords[j];
                                                        actVertex(4) = 1;
                                                        
                                                        actVertex = transformation * actVertex;
                                                        
                                                        DL_vector *finalVertex = new DL_vector();
                                                        
                                                        for (int k = 0; k<3; k++)
                                                                finalVertex->set( k, actVertex( k+1 ) );
                                                        
                                                        vertices.insert( i, finalVertex );
                                                };
                                                
                                                SIG_Polygon *actPolygon = 0;
                                                
                                                for (int j=0; j < noOfIndices; j++) {
                                                        int actIndex = indexedFaceSetNode->getCoordIndex(j);
                                                        
                                                        if (actIndex < 0)
                                                                actPolygon = 0;
                                                        else {
                                                                if (!actPolygon)
                                                                        actPolygon = new SIG_Polygon( geometry );
                                                                
                                                                actPolygon->appendVertex( *vertices[ actIndex ] );
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
                // Load expects a char*, so a const_cast is necessary.
                
                bodyScene->load( const_cast<char*>( getGeometryFile ().latin1() ) );
                
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
                if (geometryFile.at (0) == '/')
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
































