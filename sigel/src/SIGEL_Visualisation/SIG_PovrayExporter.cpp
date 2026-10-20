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

	bool SIG_PovrayExporter::writeIncludeFile( QString fileName, double aspectRatio ) const
	{
		QFile file( fileName );

		if (!file.open( QIODevice::WriteOnly ))
			return false;

		QTextStream stream( &file );

		// The screen's vertical field of view: the frame's height is 1 at this distance.
		static double const directionLength = 0.5 / std::tan( qDegreesToRadians( SIG_Visualisation::fieldOfView ) / 2 );

		// Far enough that the light falls in parallel across the robot's whole walk.
		static double const lightDistance = 1000;

		// The key light is the screen's sun, so the shadows fall the same way; the fill comes
		// from the opposite side, lower down.
		QVector3D const sun = SIG_SimulationVisualisation::sunDirection.normalized() * lightDistance;

		QVector3D const fill = QVector3D( -sun.x(), sun.y() / 2, -sun.z() ).normalized() * lightDistance;

		NEWMAT::ColumnVector keyPosition( 3 );
		keyPosition << sun.x() << sun.y() << sun.z();

		NEWMAT::ColumnVector fillPosition( 3 );
		fillPosition << fill.x() << fill.y() << fill.z();

		// The sky has the screen's colours: skyBottomColor at the horizon, skyTopColor overhead.
		NEWMAT::ColumnVector skyTopColor( 3 );
		skyTopColor << SIG_Visualisation::skyTopColor.x() << SIG_Visualisation::skyTopColor.y() << SIG_Visualisation::skyTopColor.z();

		NEWMAT::ColumnVector horizonColor( 3 );
		horizonColor << SIG_Visualisation::skyBottomColor.x() << SIG_Visualisation::skyBottomColor.y() << SIG_Visualisation::skyBottomColor.z();

		QString floorImageName;

		if (visualisation.getEnvironmentRenderer().getWithTexture())
		{
			QFileInfo const includeFile( fileName );
			floorImageName = includeFile.completeBaseName() + "_floor.png";

			if (!writeFloorImage( includeFile.dir().filePath( floorImageName ) ))
				return false;
		}

		stream << environmentDeclarations( floorImageName )
		       << "\n";

		if (!writeResource( stream, "materials.inc" ))
			return false;

		stream << robotDeclarations()
		       << "#declare cameraRight = " << aspectRatio << "*x;\n"
		       << "#declare cameraDirection = <0,0," << directionLength << ">;\n";

		// Inputs to scene.inc: the light positions and sky colours, declared ahead of it for
		// POV-Ray's single-pass parser.
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

		// globalSettings.inc samples the radiosity densely with a low error bound, so that
		// the indirect light changes little from frame to frame. Its brightness 0.6 puts less
		// sky light into the shadows, for more contrast.
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
		       << "  right cameraRight\n"
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

	QString SIG_PovrayExporter::environmentDeclarations( QString floorImageName ) const
	{
		SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

		QString declarationsString;
		QTextStream stream( &declarationsString, QIODevice::WriteOnly );

		NEWMAT::ColumnVector planeColorVector = SIG_TypeConverter::toColumnVector( environmentRenderer.sceneObjects[ 0 ]->getColor() );

		SIGEL_Environment::SIG_Terrain const &terrain = environmentRenderer.getTerrain();

		if (terrain.isFlat())
			stream << "#declare Plane = plane {\n"
			       << "                   <0,1,0>, 0\n";
		else
			stream << "#declare Plane = mesh2 {\n"
			       << terrainMesh();

		stream << "                   texture {\n";

		// The image lies in the square from 0 to 1 in x and y; laid flat, its top row is
		// towards +z, as on screen. It holds the texture twice, so it is two tiles wide.
		if (floorImageName.isEmpty())
			stream << "                     pigment { srgb " << vectorToPovray( planeColorVector ) << " }\n";
		else
			stream << "                     pigment { image_map { png \"" << floorImageName << "\" interpolate 2 }"
			       << " scale <" << 2 * floorTileSize() << "," << floorTileSize() << ",1> rotate x*90 }\n";

		// A tuning constant from sample renders: at diffuse 1, the sun washes the floor out.
		stream << "                     finish { ambient 0 diffuse 0.8 }\n"
		       << "                   }\n"
		       << "                 }\n"
		       << "\n";

		NEWMAT::ColumnVector gridColorVector = SIG_TypeConverter::toColumnVector( environmentRenderer.sceneObjects[ 1 ]->getColor() );

		stream << "#declare Grid = union {\n"
		       << (terrain.isFlat() ? flatGrid() : terrainGrid());

		stream << "\n"
		       << "  pigment { srgb " << vectorToPovray( gridColorVector ) << " }\n"
		       << "  finish { ambient 0 diffuse 1 }\n"
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

		SIG_Vector const &lookPoint = environmentRenderer.getLookPoint();

		int const gridSpacing = environmentRenderer.getGridSpacing();

		bool const showRobotPath = environmentRenderer.getShowRobotPath();

		QList< SIG_Vector * > const &robotPathPoints = environmentRenderer.getRobotPathPoints();

		QString resultString;
		QTextStream stream( &resultString, QIODevice::WriteOnly );

		int xPos = 0;
		int zPos = 0;

		// On a flat floor the grid moves with the look point, by whole grid cells; the plane
		// stays, so that a texture on it does not jump. On a terrain both stay, as drawn.
		if (environmentRenderer.getTerrain().isFlat())
		{
			xPos = static_cast< int >( std::floor( lookPoint.x / gridSpacing ) * gridSpacing );
			zPos = - static_cast< int >( std::floor( lookPoint.z / gridSpacing ) * gridSpacing );
		}

		stream << sceneObjects( environmentRenderer, 0, 1 );

		// An object, not a union: POV-Ray warns about a union of fewer than two objects.
		if (environmentRenderer.sceneObjects[ 1 ]->getVisible())
			stream << "object {\n"
			       << sceneObjects( environmentRenderer, 1, 2 )
			       << "translate <"
			       << xPos
			       << ",0,"
			       << zPos
			       << ">\n"
			       << "}\n"
			       << "\n";

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

	QString SIG_PovrayExporter::flatGrid() const
	{
		SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

		int const gridSpacing = environmentRenderer.getGridSpacing();

		int const noOfFields = 70;

		int const minX = - gridSpacing * noOfFields;
		int const maxX =   gridSpacing * noOfFields;
		int const minZ = - gridSpacing * noOfFields;
		int const maxZ =   gridSpacing * noOfFields;

		QString gridString;
		QTextStream stream( &gridString, QIODevice::WriteOnly );

		for (int x=minX; x<=maxX; x+=gridSpacing)
		{
			stream << "  cylinder {\n"
			       << "    <"
			       << x
			       << ",0.01,"
			       << minZ
			       << ">, <"
			       << x
			       << ",0.01,"
			       << maxZ
			       << ">, " << lineRadius() << "\n"
			       << "  }\n";
		};

		for (int z=minZ; z<=maxZ; z+=gridSpacing)
		{
			stream << "  cylinder {\n"
			       << "    <"
			       << minX
			       << ",0.01,"
			       << z
			       << ">, <"
			       << maxX
			       << ",0.01,"
			       << z
			       << ">, " << lineRadius() << "\n"
			       << "  }\n";
		};

		return gridString;
	}

	QString SIG_PovrayExporter::terrainMesh() const
	{
		SIGEL_Environment::SIG_Terrain const &terrain = visualisation.getEnvironmentRenderer().getTerrain();

		// The vertices SIG_EnvironmentRenderer::drawInit draws.
		int const minX = 1 - terrain.getDimensionX();
		int const maxX = terrain.getDimensionX() - 1;
		int const minZ = 1 - terrain.getDimensionZ();
		int const maxZ = terrain.getDimensionZ() - 1;

		int const rowLength = maxX - minX + 1;

		QString meshString;
		QTextStream stream( &meshString, QIODevice::WriteOnly );

		stream << "  vertex_vectors {\n"
		       << "    " << rowLength * (maxZ - minZ + 1);

		for (int z=minZ; z<=maxZ; z++)
			for (int x=minX; x<=maxX; x++)
				stream << ",\n    " << terrainPoint( x, z, 0 );

		stream << "\n"
		       << "  }\n"
		       << "  face_indices {\n"
		       << "    " << 2 * (rowLength - 1) * (maxZ - minZ);

		// Each cell is split along the same diagonal as in drawInit and in DynaMechs.
		for (int z=minZ; z<maxZ; z++)
			for (int x=minX; x<maxX; x++)
			{
				int const lower = (z - minZ) * rowLength + (x - minX);
				int const upper = lower + rowLength;

				stream << ",\n    <" << upper << "," << lower << "," << upper + 1 << ">"
				       << ", <" << upper + 1 << "," << lower << "," << lower + 1 << ">";
			}

		stream << "\n"
		       << "  }\n";

		return meshString;
	}

	QString SIG_PovrayExporter::terrainGrid() const
	{
		SIGEL_Environment::SIG_Terrain const &terrain = visualisation.getEnvironmentRenderer().getTerrain();

		// The lines SIG_EnvironmentRenderer::buildGrid draws, raised by the same 0.01.
		int const minX = 1 - terrain.getDimensionX();
		int const maxX = terrain.getDimensionX() - 1;
		int const minZ = 1 - terrain.getDimensionZ();
		int const maxZ = terrain.getDimensionZ() - 1;

		QString gridString;
		QTextStream stream( &gridString, QIODevice::WriteOnly );

		for (int z=minZ; z<maxZ; z++)
			for (int x=minX; x<maxX; x++)
				stream << "  cylinder {\n"
				       << "    " << terrainPoint( x, z, 0.01 ) << ", " << terrainPoint( x + 1, z, 0.01 )
				       << ", " << lineRadius() << "\n"
				       << "  }\n";

		for (int x=minX; x<maxX; x++)
			for (int z=minZ; z<maxZ; z++)
				stream << "  cylinder {\n"
				       << "    " << terrainPoint( x, z, 0.01 ) << ", " << terrainPoint( x, z + 1, 0.01 )
				       << ", " << lineRadius() << "\n"
				       << "  }\n";

		return gridString;
	}

	QString SIG_PovrayExporter::terrainPoint( int x, int z, double lift ) const
	{
		SIGEL_Environment::SIG_Terrain const &terrain = visualisation.getEnvironmentRenderer().getTerrain();

		// drawInit turns the terrain over the x axis: row z lies at SIGEL z = -z.
		NEWMAT::ColumnVector point( 3 );
		point << x << SIG_EnvironmentRenderer::groundDepth( terrain, x, z ) + lift << -z;

		// In SIGEL coordinates, as the plane and the grid are declared: sceneObjects turns them into POV-Ray's.
		return vectorToPovray( point );
	}

	double SIG_PovrayExporter::patternSize() const
	{
		double const robotRadius = visualisation.getRobotRadius();

		return (robotRadius > 0) ? robotRadius : 1;
	}

	double SIG_PovrayExporter::lineRadius() const
	{
		// A tuning constant with no physical meaning: half a percent, tuned on sample renders
		// of robots with radius 0.37 to 3.32.
		return patternSize() / 200;
	}

	double SIG_PovrayExporter::anchorPointRadius() const
	{
		// A tuning constant from the same renders: five times the line radius, so that a point
		// is larger than the path it sits on.
		return patternSize() / 40;
	}

	double SIG_PovrayExporter::floorTileSize() const
	{
		// A tuning constant from sample renders of the Flagstones texture.
		return patternSize() * 2;
	}

	bool SIG_PovrayExporter::writeFloorImage( QString fileName ) const
	{
		SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

		SIGEL_Environment::SIG_Environment const &environment = environmentRenderer.getEnvironment();

		// The screen ignores the image's own alpha.
		QImage const texture = QImage( environment.getTextureFile() ).convertToFormat( QImage::Format_RGB32 );

		if (texture.isNull())
		{
			SIG_IO::cerr << "Error: cannot read the floor texture " << environment.getTextureFile() << "." << Qt::endl;
			return false;
		}

		SIG_Vector const floorColor = environmentRenderer.sceneObjects[ 0 ]->getColor();

		QImage image( 2 * texture.width(), texture.height(), QImage::Format_RGB32 );
		image.fill( QColor::fromRgbF( floorColor.x, floorColor.y, floorColor.z ) );

		QPainter painter( &image );
		painter.setOpacity( environment.getTexAlpha() / 255.0 );
		painter.drawImage( 0, 0, texture );
		// Mirrors the second copy into the right half.
		painter.setTransform( QTransform( -1, 0, 0, 1, 2 * texture.width(), 0 ) );
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
