/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#include "SIGEL_Visualisation/SIG_PovrayExporter.h"

#include "SIGEL_Visualisation/SIG_SimulationVisualisation.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QPainter>
#include <QTextStream>
#include <QTransform>
#include <QtMath>

#include <cmath>

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{

	SIG_PovrayExporter::SIG_PovrayExporter( SIG_SimulationVisualisation const &visualisation )
		: visualisation( visualisation )
	{
	}

	bool SIG_PovrayExporter::writeIncludeFile( QString fileName, SIG_PovrayFloor const &floor ) const
	{
		QFile file( fileName );

		if (!file.open( QIODevice::WriteOnly ))
			return false;

		QTextStream stream( &file );

		// The OpenGL renderer's vertical field of view: at this distance the frame is 1 high.
		static double const directionLength = 0.5 / std::tan( qDegreesToRadians( SIG_Visualisation::fieldOfView ) / 2 );

		// Far enough that the light falls in parallel across the robot's whole walk.
		static double const lightDistance = 1000;

		// The key light is the OpenGL renderer's sun, so shadows fall the same way. The fill
		// light comes from the opposite side, lower down.
		QVector3D const sun = SIG_SimulationVisualisation::sunDirection.normalized() * lightDistance;

		QVector3D const fill = QVector3D( -sun.x(), sun.y() / 2, -sun.z() ).normalized() * lightDistance;

		NEWMAT::ColumnVector keyPosition( 3 );
		keyPosition << sun.x() << sun.y() << sun.z();

		NEWMAT::ColumnVector fillPosition( 3 );
		fillPosition << fill.x() << fill.y() << fill.z();

		// The sky has the OpenGL renderer's colours: skyBottomColor at the horizon, skyTopColor
		// overhead.
		NEWMAT::ColumnVector skyTopColor( 3 );
		skyTopColor << SIG_Visualisation::skyTopColor.x() << SIG_Visualisation::skyTopColor.y() << SIG_Visualisation::skyTopColor.z();

		NEWMAT::ColumnVector horizonColor( 3 );
		horizonColor << SIG_Visualisation::skyBottomColor.x() << SIG_Visualisation::skyBottomColor.y() << SIG_Visualisation::skyBottomColor.z();

		QString floorImageName;

		if (!floor.textureFile.isEmpty())
		{
			QFileInfo const includeFile( fileName );
			floorImageName = includeFile.completeBaseName() + "_floor.png";

			if (!writeFloorImage( includeFile.dir().filePath( floorImageName ), floor ))
				return false;
		}

		stream << environmentDeclarations( floorImageName, floor )
		       << "\n";

		if (!writeResource( stream, "materials.inc" ))
			return false;

		stream << robotDeclarations()
		       << "#declare cameraDirection = <0,0," << directionLength << ">;\n";

		// scene.inc reads these names. POV-Ray parses in one pass, so they come before it.
		stream << "#declare sigelSunPosition = " << vectorToPovray( SIG_TypeConverter::sigelToPovray() * keyPosition ) << ";\n"
		       << "#declare sigelFillPosition = " << vectorToPovray( SIG_TypeConverter::sigelToPovray() * fillPosition ) << ";\n"
		       << "#declare sigelHorizonColor = " << vectorToPovray( horizonColor ) << ";\n"
		       << "#declare sigelSkyTopColor = " << vectorToPovray( skyTopColor ) << ";\n"
		       << "\n";

		if (!writeResource( stream, "scene.inc" ))
			return false;

		file.close();

		return true;
	}

	bool SIG_PovrayExporter::writeFrame( QString includeFileName, QString fileName ) const
	{
		QFile file( fileName );

		if (!file.open( QIODevice::WriteOnly ))
			return false;

		QTextStream stream( &file );

		SIG_ViewSettings const &viewSettings = visualisation.viewSettings;

		SIG_Vector finalEyePoint = viewSettings.getAbsoluteEyePoint();

		NEWMAT::ColumnVector finalEyePointColumnVector = SIG_TypeConverter::toColumnVector( finalEyePoint );

		NEWMAT::ColumnVector lookPointColumnVector = SIG_TypeConverter::toColumnVector( viewSettings.lookPoint );

		stream << "#version 3.7;\n"
		       << "\n";

		// globalSettings.inc: dense radiosity with a low error bound, so the indirect light
		// changes little between frames. Its brightness 0.6 puts less sky light into the
		// shadows, for more contrast.
		if (!writeResource( stream, "globalSettings.inc" ))
			return false;

		stream << "#include "
		       << "\""
		       << includeFileName
		       << "\"\n"
		       << "\n"
		       << environmentObjects()
		       << sceneObjects( visualisation.getRobotRenderer(), 0, visualisation.getRobotRenderer().getNoOfObjects() )
		       << "camera {\n"
		       << "  right x*image_width/image_height\n"
		       << "  up y\n"
		       << "  direction cameraDirection\n"
		       << "  location "
		       << vectorToPovray( SIG_TypeConverter::sigelToPovray() * finalEyePointColumnVector )
		       << "\n"
		       << "  look_at "
		       << vectorToPovray( SIG_TypeConverter::sigelToPovray() * lookPointColumnVector )
		       << "\n"
		       << "}\n";

		file.close();

		return true;
	}

	QString SIG_PovrayExporter::environmentDeclarations( QString floorImageName, SIG_PovrayFloor const &floor ) const
	{
		SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

		QString declarationsString;
		QTextStream stream( &declarationsString, QIODevice::WriteOnly );

		NEWMAT::ColumnVector planeColorVector = SIG_TypeConverter::toColumnVector( environmentRenderer.sceneObjects[ 0 ]->getColor() );

		SIGEL_Environment::SIG_Terrain const &terrain = environmentRenderer.getTerrain();

		if (terrain.isFlat())
			stream << "#declare Ground = plane { <0,1,0>, 0 }\n";
		else
			stream << "#declare Ground = mesh2 {\n"
			       << terrainMesh()
			       << "}\n";

		// scene.inc reads these names to build the floor.
		stream << "\n"
		       << "#declare sigelFloorColor = " << vectorToPovray( planeColorVector ) << ";\n";

		// A mirrored floor's image holds the texture twice, so it is two tiles wide.
		if (!floorImageName.isEmpty())
		{
			double const tileSize = floor.tileSize * 2 * patternSize();
			double const imageWidth = floor.mirrored ? 2 * tileSize : tileSize;

			stream << "#declare sigelFloorImage = \"" << floorImageName << "\";\n"
			       << "#declare sigelFloorImageWidth = " << imageWidth << ";\n"
			       << "#declare sigelFloorTileSize = " << tileSize << ";\n";
		}

		stream << "\n";

		// The lines lie on a copy of the ground, raised as the OpenGL renderer raises them;
		// the copy is clear between the lines.
		stream << "#declare Grid = object {\n"
		       << "  Ground\n"
		       << "  translate <0,0.01,0>\n"
		       << gridLines( "x" )
		       << gridLines( "z" )
		       << "  no_shadow\n"
		       << "}\n";

		return declarationsString;
	}

	QString SIG_PovrayExporter::robotDeclarations() const
	{
		SIGEL_Robot::SIG_Robot const &robot = visualisation.getRobotRenderer().getRobot();

		QString declarationsString;

		QTextStream stream( &declarationsString, QIODevice::WriteOnly );

		stream << "#declare sigelPatternSize = " << patternSize() << ";\n"
		       << "\n";

		// A robot file gives a material only a colour. It becomes paint, a little greyer and
		// darker than the colour, so that pure colours look like paint and not like plastic.
		for ( SIGEL_Robot::SIG_Material *materialPtr : robot.getMaterials() )
		{
			SIGEL_Robot::SIG_Material &actMaterial = *materialPtr;

			NEWMAT::ColumnVector paintColor = SIG_TypeConverter::toColumnVector( actMaterial.getColour() );

			double const grey = 0.2126 * paintColor( 1 ) + 0.7152 * paintColor( 2 ) + 0.0722 * paintColor( 3 );

			for (int i=1; i<=3; i++)
				paintColor( i ) = (paintColor( i ) + (grey - paintColor( i )) * 0.45) * 0.8;

			stream << "#declare "
			       << actMaterial.getName()
			       << " = sigelPaint( "
			       << vectorToPovray( paintColor )
			       << " )\n"
			       << "\n";
		};

		QString pointDataString;
		QTextStream pointDataStream( &pointDataString, QIODevice::WriteOnly );

		for ( SIGEL_Robot::SIG_Link *linkPtr : robot.getLinks() )
		{
			SIGEL_Robot::SIG_Link &actLink = *linkPtr;

			int linkNumber = actLink.getNumber();

			QString polygonDataString;
			QTextStream polygonDataStream( &polygonDataString, QIODevice::WriteOnly );

			stream << "#declare "
			       << actLink.getName()
			       << " = mesh {\n";

			SIGEL_Robot::SIG_Geometry const *actGeometry = actLink.getGeometry();
			SIGEL_Robot::SIG_GeometryIterator actGeoIter(actGeometry);

			while (actGeoIter.valid())
			{
				SIGEL_Robot::SIG_Polygon const &actPolygon = actGeoIter.current();

				NEWMAT::ColumnVector firstVertex = SIG_TypeConverter::toColumnVector( actPolygon.getVertex( 0 ) );

				NEWMAT::ColumnVector secondVertex = SIG_TypeConverter::toColumnVector( actPolygon.getVertex( 1 ) );

				for ( int i=2; i < actPolygon.getNumVertices(); i++ )
				{
					NEWMAT::ColumnVector thirdVertex = SIG_TypeConverter::toColumnVector( actPolygon.getVertex( i ) );

					polygonDataStream << "  triangle {\n";

					polygonDataStream << "    "
					                  << vectorToPovray( firstVertex )
					                  << ", "
					                  << vectorToPovray( secondVertex )
					                  << ", "
					                  << vectorToPovray( thirdVertex )
					                  << "\n"
					                  << "  }\n";

					secondVertex = thirdVertex;
				};

				actGeoIter.next();
			};

			for ( const SIGEL_Robot::SIG_Link::NamedPoint &pointEntry : actLink.getPoints() )
			{
				NEWMAT::ColumnVector pointPosition = SIG_TypeConverter::toColumnVector( *pointEntry.value );

				NEWMAT::ColumnVector linkColor = SIG_TypeConverter::toColumnVector( actLink.getMaterial()->getColour() );

				for (int i=1; i<=3; i++)
				{
					double newValue = linkColor( i ) - 0.3;
					newValue = ( newValue < 0 ) ? 0 : newValue;
					linkColor( i ) = newValue;
				};

				pointDataStream << "#declare "
				                << pointEntry.name
				                << " = sphere {\n"
				                << "  "
				                << vectorToPovray( pointPosition )
				                << ", " << anchorPointRadius() << "\n"
				                << "  texture {\n"
				                << "    pigment { srgb "
				                << vectorToPovray( linkColor )
				                << " }\n"
				                << "    finish { sigelPaintFinish }\n"
				                << "  }\n"
				                << "}\n"
				                << "\n";
			};

			stream << polygonDataString
			       << "\n"
			       << "  texture { "
			       << actLink.getMaterial()->getName()
			       << "  }\n"
			       << "}\n"
			       << "\n";
		};

		stream << pointDataString;

		return declarationsString;
	}

	QString SIG_PovrayExporter::environmentObjects() const
	{
		SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

		bool const showRobotPath = environmentRenderer.getShowRobotPath();

		QList< SIG_Vector * > const &robotPathPoints = environmentRenderer.getRobotPathPoints();

		QString resultString;
		QTextStream stream( &resultString, QIODevice::WriteOnly );

		stream << sceneObjects( environmentRenderer, 0, 2 );

		if (showRobotPath && robotPathPoints.count() >= 2)
		{
			// One cylinder from each point to the next.
			for ( qsizetype i = 1; i < robotPathPoints.size(); i++ )
			{
				SIG_Vector *prevPoint = robotPathPoints.at( i - 1 );
				SIG_Vector *actPoint  = robotPathPoints.at( i );
				NEWMAT::ColumnVector base =   SIG_TypeConverter::sigelToPovray()
				                            * SIG_TypeConverter::toColumnVector( *prevPoint );
				NEWMAT::ColumnVector cap =   SIG_TypeConverter::sigelToPovray()
				                           * SIG_TypeConverter::toColumnVector( *actPoint );

				stream << "cylinder {\n"
				       << "  "
				       << vectorToPovray( base )
				       << ", "
				       << vectorToPovray( cap )
				       << ", " << lineRadius() << "\n"
				       << "  open\n"
				       << "  pigment { srgb <1,1,0> }\n"
				       << "  finish { ambient 0 diffuse 1 }\n"
				       << "}\n";
			};

			stream << "\n";
		};

		return resultString;
	}

	QString SIG_PovrayExporter::gridLines( QString axis ) const
	{
		SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

		NEWMAT::ColumnVector const color = SIG_TypeConverter::toColumnVector( environmentRenderer.sceneObjects[ 1 ]->getColor() );

		QString lineColor;
		QTextStream( &lineColor ) << "srgbt <" << color( 1 ) << "," << color( 2 ) << "," << color( 3 ) << ",0>";
		QString clear = "srgbt <0,0,0,1>";

		// One line every grid spacing, as wide as the robot's path.
		double const halfWidth = lineRadius() / environmentRenderer.getGridSpacing();

		QString linesString;
		QTextStream stream( &linesString, QIODevice::WriteOnly );

		stream << "  texture {\n"
		       << "    pigment {\n"
		       << "      gradient " << axis << "\n"
		       << "      color_map {\n"
		       << "        [0 " << lineColor << "]\n"
		       << "        [" << halfWidth << " " << lineColor << "]\n"
		       << "        [" << halfWidth << " " << clear << "]\n"
		       << "        [" << 1 - halfWidth << " " << clear << "]\n"
		       << "        [" << 1 - halfWidth << " " << lineColor << "]\n"
		       << "        [1 " << lineColor << "]\n"
		       << "      }\n"
		       << "      scale " << environmentRenderer.getGridSpacing() << "\n"
		       << "    }\n"
		       << "    finish { ambient 0 diffuse 1 }\n"
		       << "  }\n";

		return linesString;
	}

	QString SIG_PovrayExporter::terrainMesh() const
	{
		SIGEL_Environment::SIG_Terrain const &terrain = visualisation.getEnvironmentRenderer().getTerrain();

		SIG_PovrayMesh mesh;

		// Each cell is split along the same diagonal as in SIG_EnvironmentRenderer::drawInit
		// and in DynaMechs.
		for (int z=0; z<terrain.getDimensionZ()-1; z++)
			for (int x=0; x<terrain.getDimensionX()-1; x++)
			{
				triangle( mesh, terrainVector( x, z + 1 ), terrainVector( x, z ), terrainVector( x + 1, z + 1 ) );
				triangle( mesh, terrainVector( x + 1, z + 1 ), terrainVector( x, z ), terrainVector( x + 1, z ) );
			}

		terrainBeyondEdges( mesh );

		return "  vertex_vectors {\n"
		       "    " + QString::number( mesh.vertices.size() ) + ",\n"
		       "    " + mesh.vertices.join( ",\n    " ) + "\n"
		       "  }\n"
		       "  face_indices {\n"
		       "    " + QString::number( mesh.faces.size() ) + ",\n"
		       "    " + mesh.faces.join( ",\n    " ) + "\n"
		       "  }\n";
	}

	void SIG_PovrayExporter::terrainBeyondEdges( SIG_PovrayMesh &mesh ) const
	{
		SIGEL_Environment::SIG_Terrain const &terrain = visualisation.getEnvironmentRenderer().getTerrain();

		int const minX = 0;
		int const maxX = terrain.getDimensionX() - 1;
		int const minZ = 0;
		int const maxZ = terrain.getDimensionZ() - 1;

		// Row minZ lies at the largest SIGEL z, so its edge faces north.
		QVector3D const west( -1, 0, 0 );
		QVector3D const east( 1, 0, 0 );
		QVector3D const north( 0, 0, 1 );
		QVector3D const south( 0, 0, -1 );

		for (int z=minZ; z<maxZ; z++)
		{
			edgeStrip( mesh, terrainVector( minX, z ), terrainVector( minX, z + 1 ), west );
			edgeStrip( mesh, terrainVector( maxX, z ), terrainVector( maxX, z + 1 ), east );
		}

		for (int x=minX; x<maxX; x++)
		{
			edgeStrip( mesh, terrainVector( x, minZ ), terrainVector( x + 1, minZ ), north );
			edgeStrip( mesh, terrainVector( x, maxZ ), terrainVector( x + 1, maxZ ), south );
		}

		edgeCorner( mesh, terrainVector( minX, minZ ), west, north );
		edgeCorner( mesh, terrainVector( maxX, minZ ), east, north );
		edgeCorner( mesh, terrainVector( minX, maxZ ), west, south );
		edgeCorner( mesh, terrainVector( maxX, maxZ ), east, south );
	}

	QVector3D SIG_PovrayExporter::terrainVector( int x, int z ) const
	{
		SIGEL_Environment::SIG_Terrain const &terrain = visualisation.getEnvironmentRenderer().getTerrain();

		// As in SIG_EnvironmentRenderer::drawInit, row z lies at SIGEL z = -z. The mesh is in
		// SIGEL coordinates; sceneObjects turns it into POV-Ray's.
		return QVector3D( x, SIG_EnvironmentRenderer::groundDepth( terrain, x, z ), -z );
	}

	QList< float > SIG_PovrayExporter::outwardSteps()
	{
		// Small cells near the terrain, where large ones show their seams; each one twice the last.
		QList< float > steps = { 0 };

		for (float step=1; step<=groundReach; step*=2)
			steps.append( step );

		return steps;
	}

	void SIG_PovrayExporter::edgeStrip( SIG_PovrayMesh &mesh, QVector3D from, QVector3D to, QVector3D outward )
	{
		QList< float > const steps = outwardSteps();

		// Outward, the ground keeps the edge's height.
		for (qsizetype i = 1; i < steps.size(); i++)
			quadrangle( mesh, from + outward * steps[ i - 1 ], to + outward * steps[ i - 1 ],
			            to + outward * steps[ i ], from + outward * steps[ i ] );
	}

	void SIG_PovrayExporter::edgeCorner( SIG_PovrayMesh &mesh, QVector3D corner, QVector3D outward1, QVector3D outward2 )
	{
		QList< float > const steps = outwardSteps();

		for (qsizetype i = 1; i < steps.size(); i++)
			for (qsizetype j = 1; j < steps.size(); j++)
				quadrangle( mesh, corner + outward1 * steps[ i - 1 ] + outward2 * steps[ j - 1 ],
				            corner + outward1 * steps[ i ] + outward2 * steps[ j - 1 ],
				            corner + outward1 * steps[ i ] + outward2 * steps[ j ],
				            corner + outward1 * steps[ i - 1 ] + outward2 * steps[ j ] );
	}

	void SIG_PovrayExporter::quadrangle( SIG_PovrayMesh &mesh, QVector3D a, QVector3D b, QVector3D c, QVector3D d )
	{
		triangle( mesh, a, b, c );
		triangle( mesh, a, c, d );
	}

	void SIG_PovrayExporter::triangle( SIG_PovrayMesh &mesh, QVector3D a, QVector3D b, QVector3D c )
	{
		mesh.faces.append( QString( "<%1,%2,%3>" ).arg( vertexIndex( mesh, a ) ).arg( vertexIndex( mesh, b ) ).arg( vertexIndex( mesh, c ) ) );
	}

	int SIG_PovrayExporter::vertexIndex( SIG_PovrayMesh &mesh, QVector3D vertex )
	{
		// Nine digits keep every float exactly; vectorToPovray's five move the far vertices.
		QString const text = QString( "<%1, %2, %3>" ).arg( vertex.x(), 0, 'g', 9 ).arg( vertex.y(), 0, 'g', 9 ).arg( vertex.z(), 0, 'g', 9 );

		if (mesh.vertexIndices.contains( text ))
			return mesh.vertexIndices.value( text );

		mesh.vertexIndices.insert( text, mesh.vertices.size() );
		mesh.vertices.append( text );

		return mesh.vertices.size() - 1;
	}

	double SIG_PovrayExporter::patternSize() const
	{
		double const robotRadius = visualisation.getRobotRadius();

		return (robotRadius > 0) ? robotRadius : 1;
	}

	double SIG_PovrayExporter::lineRadius() const
	{
		// A tuning constant: half a percent, from sample renders of robots with radius 0.37 to 3.32.
		return patternSize() / 200;
	}

	double SIG_PovrayExporter::anchorPointRadius() const
	{
		// A tuning constant from the same renders: five times the line radius, so that a point
		// is larger than the path it sits on.
		return patternSize() / 40;
	}

	bool SIG_PovrayExporter::writeFloorImage( QString fileName, SIG_PovrayFloor const &floor ) const
	{
		SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

		// The OpenGL renderer ignores the image's own alpha.
		QImage const texture = QImage( floor.textureFile ).convertToFormat( QImage::Format_RGB32 );

		if (texture.isNull())
		{
			SIG_IO::cerr << "Error: cannot read the floor texture " << floor.textureFile << "." << Qt::endl;
			return false;
		}

		SIG_Vector const floorColor = environmentRenderer.sceneObjects[ 0 ]->getColor();

		int const copies = floor.mirrored ? 2 : 1;

		QImage image( copies * texture.width(), texture.height(), QImage::Format_RGB32 );
		image.fill( QColor::fromRgbF( floorColor.x, floorColor.y, floorColor.z ) );

		QPainter painter( &image );
		painter.setOpacity( floor.textureOpacity );

		if (floor.mirrored)
			painter.drawImage( 0, 0, texture );

		// Laid flat, the image's columns run from right to left, so the mirrored copy is the
		// one that reads as the texture does.
		painter.setTransform( QTransform( -1, 0, 0, 1, image.width(), 0 ) );
		painter.drawImage( 0, 0, texture );
		painter.end();

		if (!image.save( fileName, "PNG" ))
		{
			SIG_IO::cerr << "Error: cannot write the floor image " << fileName << "." << Qt::endl;
			return false;
		}

		return true;
	}

	QString SIG_PovrayExporter::sceneObjects( SIG_Renderer const &renderer, GLuint first, GLuint end )
	{
		QString resultString;
		QTextStream resultStream( &resultString, QIODevice::WriteOnly );

		for (GLuint i=first; i < end; i++)
			if (renderer.sceneObjects[ i ]->getVisible())
			{
				NEWMAT::Matrix rotation = (   SIG_TypeConverter::sigelToPovray()
				                            * SIG_TypeConverter::toMatrix( renderer.sceneObjects[ i ]->getRotation() ) ).t();
				NEWMAT::ColumnVector position =   SIG_TypeConverter::sigelToPovray()
				                                * SIG_TypeConverter::toColumnVector( renderer.sceneObjects[ i ]->getPosition() );

				resultStream << "object {\n"
				             << "  "
				             << renderer.sceneObjects[ i ]->name
				             << "\n"
				             << "  matrix <";
				for (int i=1; i<=3; i++)
					resultStream << " " << rotation( 1, i ) << ",";
				resultStream << "\n"
				             << "          ";
				for (int j=1; j<=3; j++)
					resultStream << " " << rotation( 2, j ) << ",";
				resultStream << "\n"
				             << "          ";
				for (int k=1; k<=3; k++)
					resultStream << " " << rotation( 3, k ) << ",";
				resultStream << "\n"
				             << "           0 0 0 >\n"
				             << "  translate "
				             << vectorToPovray( position )
				             << "\n"
				             << "}\n"
				             << "\n";
			};

		return resultString;
	}

	QString SIG_PovrayExporter::vectorToPovray( NEWMAT::ColumnVector input )
	{
		QString resultString;

		QTextStream resultStream( &resultString, QIODevice::WriteOnly );
		resultStream.setRealNumberPrecision( 5 );

		resultStream << "<" << input( 1 ) << ", " << input( 2 ) << ", " << input( 3 ) << ">";

		return resultString;
	}

	QString SIG_PovrayExporter::vectorToPovray( QVector3D input )
	{
		NEWMAT::ColumnVector vector( 3 );
		vector << input.x() << input.y() << input.z();

		return vectorToPovray( vector );
	}

	bool SIG_PovrayExporter::writeResource( QTextStream &stream, QString fileName )
	{
		QFile file( ":/SIG_PovrayExporter/povray/" + fileName );

		if (!file.open( QIODevice::ReadOnly ))
		{
			SIG_IO::cerr << "Error: the POV-Ray text " << fileName << " is missing from the program." << Qt::endl;
			return false;
		}

		stream << file.readAll();

		return true;
	}

}
