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
#include <QList>
#include "SIGEL_Tools/SIG_FileUtils.h"
#include "SIGEL_Robot/SIG_VRMLUtils.h"

namespace SIGEL_Robot
{

	SIG_Body::SIG_Body( SIG_Robot *parent, QString geometryFile, QString directory )
		: parent( parent ),
		  geometryFile( geometryFile ),
		  directory( directory ),
		  geometry( nullptr )
	{
	}

	SIG_Body::SIG_Body( SIG_Robot *par, QTextStream &tx )
		: parent( par )
	{
		QString tmpstr;

		// "Body" has already been read in SIG_Robot.
		geometryFile = SIGEL_Tools::SIG_FileUtils::readWordOrQuoted( tx );
		directory = SIGEL_Tools::SIG_FileUtils::readWordOrQuoted( tx );
		tx >> tmpstr;
		if( tmpstr == "y" )
		{
			geometry = new SIG_Geometry( tx );
		}
		else
		{
			geometry = nullptr;
		}
	}

	SIG_Body::~SIG_Body()
	{
		delete geometry;
	}

	QList< SIG_Vector > SIG_Body::transformedVertices( CoordinateNode *coordinateNode, NEWMAT::Matrix const &transformation ) const
	{
		int noOfVertices = coordinateNode->getNPoints();

		// Held by value, not as owned SIG_Vector pointers: the NEWMAT multiply can
		// throw, and values need no free on the unwinding path.
		QList< SIG_Vector > vertices( noOfVertices );
		for( int i = 0; i < noOfVertices; i++ )
		{
			QList< float > coords( 3 );

			coordinateNode->getPoint( i, coords.data() );

			NEWMAT::ColumnVector currentVertex( 4 );
			for( int j = 0; j < 3; j++ )
			{
				currentVertex( j + 1 ) = coords[j];
			}
			currentVertex( 4 ) = 1;

			currentVertex = transformation * currentVertex;

			for( int k = 0; k < 3; k++ )
			{
				vertices[i].set( k, currentVertex( k + 1 ) );
			}
		}
		return vertices;
	}

	void SIG_Body::appendFaces( IndexedFaceSetNode *indexedFaceSetNode, QList< SIG_Vector > const &vertices )
	{
		int noOfIndices = indexedFaceSetNode->getNCoordIndexes();
		SIG_Polygon *currentPolygon = nullptr;

		for( int i = 0; i < noOfIndices; i++ )
		{
			int currentIndex = indexedFaceSetNode->getCoordIndex( i );
			if( currentIndex < 0 )
			{
				currentPolygon = nullptr;
			}
			else if( currentIndex < vertices.size() )
			{
				// Making a SIG_Polygon adds it to the geometry. Wait for the first usable vertex,
				// so no empty face is added.
				if( !currentPolygon )
				{
					currentPolygon = new SIG_Polygon( geometry );
				}
				currentPolygon->appendVertex( vertices[currentIndex] );
			}
		}
	}

	void SIG_Body::appendBoxFaces( BoxNode *boxNode, NEWMAT::Matrix const &transformation )
	{
		float const x = boxNode->getX() / 2;
		float const y = boxNode->getY() / 2;
		float const z = boxNode->getZ() / 2;

		CoordinateNode corners;
		for( int i = 0; i < 8; i++ )
		{
			corners.addPoint( i & 1 ? x : -x, i & 2 ? y : -y, i & 4 ? z : -z );
		}
		QList< SIG_Vector > vertices = transformedVertices( &corners, transformation );

		// Counterclockwise seen from outside, as an IndexedFaceSet with ccw TRUE.
		int const faces[6][4] = { { 0, 2, 3, 1 }, { 4, 5, 7, 6 }, { 0, 1, 5, 4 }, { 2, 6, 7, 3 }, { 0, 4, 6, 2 }, { 1, 3, 7, 5 } };
		for( int f = 0; f < 6; f++ )
		{
			SIG_Polygon *polygon = new SIG_Polygon( geometry );
			for( int c = 0; c < 4; c++ )
			{
				polygon->appendVertex( vertices[ faces[f][c] ] );
			}
		}
	}

	void SIG_Body::readVRMLShape( ShapeNode *shapeNode, NEWMAT::Matrix const &transformation )
	{
		GeometryNode *geometryNode = shapeNode->getGeometry();

		if( geometryNode && geometryNode->isIndexedFaceSetNode() )
		{
			IndexedFaceSetNode *indexedFaceSetNode = static_cast<IndexedFaceSetNode *>( geometryNode );

			CoordinateNode *coordinateNode = indexedFaceSetNode->getCoordinateNodes();
			if( coordinateNode )
			{
				appendFaces( indexedFaceSetNode, transformedVertices( coordinateNode, transformation ) );
			}
		}
		else if( geometryNode && geometryNode->isBoxNode() )
		{
			appendBoxFaces( static_cast<BoxNode *>( geometryNode ), transformation );
		}
	}

	void SIG_Body::readVRMLNode( Node *node, NEWMAT::Matrix transformation )
	{
		if( node->isGroupingNode() || node->isLodNode() || node->isSwitchNode() )
		{
			if( node->isTransformNode() )
			{
				transformation = transformation * SIG_VRMLUtils::transformNodeMatrix( static_cast<TransformNode *>( node ) );
			}

			for( Node *child = node->getChildNodes(); child; child = child->next() )
			{
				readVRMLNode( child, transformation );
			}
		}
		else if( node->isShapeNode() )
		{
			readVRMLShape( static_cast<ShapeNode *>( node ), transformation );
		}
	}

	void SIG_Body::load()
	{
		// A geometry given by setGeometry takes precedence over the file.
		if( geometry )
		{
			return;
		}

		// Before the geometry is created: a body that throws here stays without one.
		SceneGraph bodyScene;
		SIG_VRMLUtils::loadScene( bodyScene, getGeometryFile() );

		geometry = new SIG_Geometry();

		NEWMAT::Matrix identity( 4, 4 );
		identity = 0;
		for( int i = 1; i <= 4; i++ )
		{
			identity( i, i ) = 1;
		}

		for( Node *currentNode = bodyScene.getNodes(); currentNode; currentNode = currentNode->next() )
		{
			readVRMLNode( currentNode, identity );
		}
	}

	void SIG_Body::setGeometry( SIG_Geometry *geo )
	{
		// The same geometry again must not be freed while the body keeps it.
		if( geo == geometry )
		{
			return;
		}
		delete geometry;
		geometry = geo;
	}

	SIG_Geometry const *SIG_Body::getGeometry() const
	{
		return geometry;
	}

	QString SIG_Body::getGeometryFile() const
	{
		// geometryFile is empty when the robot file ends early (see the SIG_Body
		// QTextStream constructor). QString::at() must not be called on an empty string.
		if( !geometryFile.isEmpty() && geometryFile.at( 0 ) == '/' )
		{
			return geometryFile;
		}
		else
		{
			QString retval;
			retval = directory;
			retval.append( geometryFile );
			return retval;
		}
	}

	QString SIG_Body::getName() const
	{
		return geometryFile;
	}

	void SIG_Body::writeToFileTransfer( QTextStream &tx )
	{
		tx << "Body " << SIGEL_Tools::SIG_FileUtils::quotedIfContainsWhitespace( geometryFile ) << ' ' << SIGEL_Tools::SIG_FileUtils::quotedIfContainsWhitespace( directory ) << ' ';
		if( geometry )
		{
			tx << "y\n";
			geometry->writeToFileTransfer( tx );
		}
		else
		{
			tx << "n\n";
		}
	}

}
