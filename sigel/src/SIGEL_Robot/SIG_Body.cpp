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
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Robot/SIG_VRMLUtils.h"
#include "SIGEL_Robot/SIG_VRMLFileError.h"

#include <cmath>
#include <numbers>

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
		int corners = 0;
		int faces = 0;

		// The end of the list ends the last face, as -1 does.
		for( int i = 0; i <= noOfIndices; i++ )
		{
			int currentIndex = i < noOfIndices ? indexedFaceSetNode->getCoordIndex( i ) : -1;
			if( currentIndex < -1 )
			{
				throw SIG_VRMLFileError( __FILE__, __LINE__, getGeometryFile(), "coordIndex " + QString::number( currentIndex ) + " is neither -1 nor the number of a point." );
			}
			else if( currentIndex == -1 )
			{
				if( currentPolygon && corners < 3 )
				{
					throw SIG_VRMLFileError( __FILE__, __LINE__, getGeometryFile(), "face " + QString::number( faces ) + " of an IndexedFaceSet has fewer than 3 points." );
				}
				currentPolygon = nullptr;
				corners = 0;
			}
			else if( currentIndex >= vertices.size() )
			{
				throw SIG_VRMLFileError( __FILE__, __LINE__, getGeometryFile(), "coordIndex " + QString::number( currentIndex ) + " is beyond the " + QString::number( vertices.size() ) + " points of the coord field." );
			}
			else
			{
				// Making a SIG_Polygon adds it to the geometry. Wait for the first usable vertex,
				// so no empty face is added.
				if( !currentPolygon )
				{
					currentPolygon = new SIG_Polygon( geometry );
					faces++;
				}
				currentPolygon->appendVertex( vertices[currentIndex] );
				corners++;
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

	void SIG_Body::appendCylinderFaces( CylinderNode *cylinderNode, NEWMAT::Matrix const &transformation )
	{
		float const radius = cylinderNode->getRadius();
		float const y = cylinderNode->getHeight() / 2;
		int const steps = 16;

		// Corner i of the top ring is corner i, the one below it is corner steps + i.
		CoordinateNode corners;
		for( int i = 0; i < 2 * steps; i++ )
		{
			double const angle = 2 * std::numbers::pi * ( i % steps ) / steps;
			corners.addPoint( radius * std::cos( angle ), i < steps ? y : -y, radius * std::sin( angle ) );
		}
		QList< SIG_Vector > vertices = transformedVertices( &corners, transformation );

		// Counterclockwise seen from outside, as an IndexedFaceSet with ccw TRUE.
		SIG_Polygon *top = new SIG_Polygon( geometry );
		SIG_Polygon *bottom = new SIG_Polygon( geometry );
		for( int i = 0; i < steps; i++ )
		{
			top->appendVertex( vertices[ steps - 1 - i ] );
			bottom->appendVertex( vertices[ steps + i ] );

			int const next = ( i + 1 ) % steps;
			SIG_Polygon *side = new SIG_Polygon( geometry );
			side->appendVertex( vertices[ i ] );
			side->appendVertex( vertices[ next ] );
			side->appendVertex( vertices[ steps + next ] );
			side->appendVertex( vertices[ steps + i ] );
		}
	}

	void SIG_Body::readVRMLShape( ShapeNode *shapeNode, NEWMAT::Matrix const &transformation )
	{
		GeometryNode *geometryNode = shapeNode->getGeometry();

		if( geometryNode && geometryNode->isIndexedFaceSetNode() )
		{
			IndexedFaceSetNode *indexedFaceSetNode = static_cast<IndexedFaceSetNode *>( geometryNode );
			if( !indexedFaceSetNode->getCCW() )
			{
				throw SIG_VRMLFileError( __FILE__, __LINE__, getGeometryFile(), "ccw FALSE is not supported; write the faces counterclockwise seen from outside." );
			}

			// Without a coord field there are no points, so appendFaces refuses any index.
			QList< SIG_Vector > vertices;
			CoordinateNode *coordinateNode = indexedFaceSetNode->getCoordinateNodes();
			if( coordinateNode )
			{
				vertices = transformedVertices( coordinateNode, transformation );
			}
			appendFaces( indexedFaceSetNode, vertices );
		}
		else if( geometryNode && geometryNode->isBoxNode() )
		{
			appendBoxFaces( static_cast<BoxNode *>( geometryNode ), transformation );
		}
		else if( geometryNode && geometryNode->isCylinderNode() )
		{
			appendCylinderFaces( static_cast<CylinderNode *>( geometryNode ), transformation );
		}
		else if( geometryNode && ( geometryNode->isSphereNode() || geometryNode->isConeNode() ) )
		{
			QString const shape = geometryNode->isSphereNode() ? "Sphere" : "Cone";
			SIGEL_Tools::SIG_IO::cerr << "VRML file \"" << getGeometryFile() << "\": a " << shape << " is not read, the link gets no faces from it." << Qt::endl;
		}
	}

	void SIG_Body::readVRMLNode( Node *node, NEWMAT::Matrix transformation )
	{
		if( node->isGroupingNode() || node->isLodNode() || node->isSwitchNode() )
		{
			if( node->isTransformNode() )
			{
				transformation = transformation * SIG_VRMLUtils::transformNodeMatrix( static_cast<TransformNode *>( node ) );

				// A mirror turns the corner order round, and the faces would point inwards.
				NEWMAT::Matrix const linearPart = transformation.SubMatrix( 1, 3, 1, 3 );
				if( linearPart.LogDeterminant().Sign() < 0 )
				{
					throw SIG_VRMLFileError( __FILE__, __LINE__, getGeometryFile(), "a Transform with a negative scale mirrors the faces; this is not supported." );
				}
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

		try
		{
			for( Node *currentNode = bodyScene.getNodes(); currentNode; currentNode = currentNode->next() )
			{
				readVRMLNode( currentNode, identity );
			}

			// SIG_Mirtich takes the mass of the link from these faces.
			if( geometry->getNumPolygons() == 0 )
			{
				throw SIG_VRMLFileError( __FILE__, __LINE__, getGeometryFile(), "the file has no faces." );
			}
		}
		catch( ... )
		{
			// A refused file leaves the body without a geometry, as a file that cannot be parsed does.
			delete geometry;
			geometry = nullptr;
			throw;
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
