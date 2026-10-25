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
#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include <QFile>
#include "SIGEL_Tools/SIG_FileUtils.h"

namespace {

	/*
	 * CyberVRML97 runs a USE while its parser still holds the node that a
	 * "}" just before it closes, so the USE lands inside that node. Where
	 * the node is the one the USE names, it contains itself and
	 * Node::createInstanceNode aborts on an assertion; elsewhere the copy
	 * sits in the wrong place and the geometry is wrong. A comma after the
	 * "}" lets the parser close the node first. Tokens as in vrml.l.
	 * Returns the line of the first such USE, or 0.
	 */
	int lineOfUseAfterNodeEnd (QString const & vrml)
	{
		int line = 1;
		bool afterNodeEnd = false;
		qsizetype i = 0;
		while (i < vrml.size ()) {
			QChar const c = vrml.at (i);
			if (c == '\n') {
				line++;
				i++;
			} else if (c.isSpace ())
				i++;
			else if (c == '#') {
				while (i < vrml.size () && vrml.at (i) != '\n')
					i++;
			} else if (c == '"') {
				for (i++; i < vrml.size () && vrml.at (i) != '"'; i++)
					if (vrml.at (i) == '\n')
						line++;
				i++;
				afterNodeEnd = false;
			} else if (c.isLetterOrNumber () || c == '_' || c == '-' || c == '+' || c == '.') {
				qsizetype const start = i;
				while (i < vrml.size ()
				       && (vrml.at (i).isLetterOrNumber () || vrml.at (i) == '_'
					   || vrml.at (i) == '-' || vrml.at (i) == '+' || vrml.at (i) == '.'))
					i++;
				if (afterNodeEnd && QStringView (vrml).mid (start, i - start) == u"USE")
					return line;
				afterNodeEnd = false;
			} else {
				afterNodeEnd = (c == '}');
				i++;
			}
		}
		return 0;
	}

}

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

	NEWMAT::Matrix SIG_Body::createRotationMatrix( QList< float > rotation )
	{
		NEWMAT::Matrix rotationMatrix(4,4);
		double const x = rotation[0];
		double const y = rotation[1];
		double const z = rotation[2];
		double const a = rotation[3];

		double const sinA = std::sin(a);
		double const cosA = std::cos(a);
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
	}

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

		int const useLine = lineOfUseAfterNodeEnd (QString::fromUtf8 (vrmlFile.readAll ()));
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
