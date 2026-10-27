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
#include "SIGEL_Robot/SIG_GeometryIterator.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include <QFile>
#include <QList>
#include "SIGEL_Tools/SIG_FileUtils.h"
#include "SIGEL_Robot/SIG_VRMLUtils.h"

namespace SIGEL_Robot
{

	SIG_Body::SIG_Body(SIG_Robot *par, QString f, QString d)
		: parent(par),
		  geometryFile(f),
		  directory (d),
		  geometry(nullptr)
	{ }

	SIG_Body::SIG_Body(SIG_Robot *par, QTextStream & tx)
		: parent (par)
	{
		QString tmpstr;
		// "Body" has already been read in SIG_Robot.
		geometryFile = SIGEL_Tools::SIG_FileUtils::readWordOrQuoted (tx);
		directory = SIGEL_Tools::SIG_FileUtils::readWordOrQuoted (tx);
		tx >> tmpstr;
		if (tmpstr == "y") {
			geometry = new SIG_Geometry (tx);
		} else {
			geometry = nullptr;
		}
	}

	SIG_Body::~SIG_Body()
	{
		delete geometry;
	}

	void SIG_Body::readVRMLNode(Node *node, NEWMAT::Matrix transformation)
	{
		if ( node->isGroupNode() || node->isTransformNode() ) {
			if (node->isTransformNode())
				transformation = transformation * SIG_VRMLUtils::transformNodeMatrix( static_cast<TransformNode*> (node) );

			for (Node *child = node->getChildNodes(); child; child = child->next())
				readVRMLNode(child, transformation);
		} else if ( node->isShapeNode() ) {
			ShapeNode *shapeNode = static_cast<ShapeNode*> (node);

			GeometryNode *geometryNode = shapeNode->getGeometry();

			if (geometryNode && geometryNode->isIndexedFaceSetNode()) {
				IndexedFaceSetNode *indexedFaceSetNode
					= static_cast<IndexedFaceSetNode*> (geometryNode);

				CoordinateNode *coordinateNode = indexedFaceSetNode->getCoordinateNodes();
				if (coordinateNode) {
					int noOfVertices = coordinateNode->getNPoints();
					int noOfIndices = indexedFaceSetNode->getNCoordIndexes();
					// Held by value, not as owned SIG_Vector pointers: the NEWMAT multiply and
					// the SIG_Polygon allocations below can throw, and values need no free on the
					// unwinding path.
					QList< SIG_Vector > vertices( noOfVertices );

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

					SIG_Polygon *actPolygon = nullptr;

					for (int j=0; j < noOfIndices; j++) {
						int actIndex = indexedFaceSetNode->getCoordIndex(j);

						// Before the polygon is created: SIG_Polygon self-registers in its
						// constructor, so skipping every vertex would leave a 0-vertex face that
						// SIG_Mirtich::compFaceNormal reads verts[0..2] from.
						if (actIndex < 0)
							actPolygon = nullptr;
						else if ( actIndex < vertices.size() ) {
							if (!actPolygon)
								actPolygon = new SIG_Polygon( geometry );
							actPolygon->appendVertex( vertices[ actIndex ] );
						};
					};
				};
			};
		};
	}

	void SIG_Body::load()
	{
		// A geometry given by setGeometry takes precedence over the file.
		if (geometry)
			return;

		// Before the geometry is created: a body that throws here stays without one.
		QFile vrmlFile (getGeometryFile ());
		if (!vrmlFile.open (QIODeviceBase::ReadOnly))
		{
			QString const message = "cannot be read.";
			throw SIG_GeometryFileError (__FILE__, __LINE__, getGeometryFile (), message);
		}

		int const useLine = SIG_VRMLUtils::lineOfUseAfterNodeEnd (QString::fromUtf8 (vrmlFile.readAll ()));
		if (useLine)
		{
			QString const message = "USE directly after \"}\" is not supported. Put a comma after the \"}\".";
			throw SIG_GeometryFileError (__FILE__, __LINE__, getGeometryFile (), useLine, message);
		}

		SceneGraph bodyScene;
		QByteArray geometryFileBytes = getGeometryFile ().toUtf8();
		bodyScene.load( geometryFileBytes.data() );

		// CyberVRML97 keeps the nodes it parsed before an error, which would give a partial geometry.
		if (!bodyScene.isOK ())
		{
			QString message = "syntax error.";
			QString const errorToken = QString::fromUtf8 (bodyScene.getErrorToken ());
			if (!errorToken.isEmpty ())
				message = "syntax error at \"" + errorToken + "\".";

			int const errorLine = bodyScene.getErrorLineNumber ();
			throw SIG_GeometryFileError (__FILE__, __LINE__, getGeometryFile (), errorLine, message);
		}

		geometry = new SIG_Geometry ();

		NEWMAT::Matrix identity(4,4);
		identity = 0;
		for (int i=1; i<=4; i++)
			identity(i,i) = 1;

		for (Node *actNode = bodyScene.getNodes(); actNode; actNode = actNode->next())
			readVRMLNode( actNode,  identity);
	}

	void SIG_Body::setGeometry (SIG_Geometry *geo)
	{
		// The same geometry again must not be freed while the body keeps it.
		if (geo == geometry)
			return;
		delete geometry;
		geometry = geo;
	}

	SIG_Geometry const *SIG_Body::getGeometry () const
	{
		return geometry;
	}

	QString SIG_Body::getGeometryFile () const
	{
		// geometryFile is empty when the robot file ends early (see the SIG_Body
		// QTextStream constructor). QString::at() must not be called on an empty string.
		if (!geometryFile.isEmpty() && geometryFile.at (0) == '/')
			return geometryFile;
		else {
			QString retval;
			retval = directory;
			retval.append (geometryFile);
			return retval;
		}
	}

	QString SIG_Body::getName () const 
	{ 
		return geometryFile;
	}

	void SIG_Body::writeToFileTransfer (QTextStream & tx)
	{
		tx << "Body " << SIGEL_Tools::SIG_FileUtils::quotedIfContainsWhitespace (geometryFile) << ' ' << SIGEL_Tools::SIG_FileUtils::quotedIfContainsWhitespace (directory) << ' ';
		if (geometry) {
			tx << "y\n";
			geometry->writeToFileTransfer (tx);
		} else
			tx << "n\n";
	}

}
