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

#include <QFile>
#include <QTextStream>
#include <QtMath>

#include <cmath>

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{

  SIG_PovrayExporter::SIG_PovrayExporter( SIG_SimulationVisualisation const &visualisation )
    : visualisation( visualisation )
  { }

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
    // from the opposite side, lower down. The fill's colour is linear: a quarter of
    // srgb <0.75, 0.82, 1>, since POV-Ray scales an srgb colour before it decodes it.
    QVector3D const sun = SIG_SimulationVisualisation::sunDirection.normalized() * lightDistance;

    QVector3D const fill = QVector3D( -sun.x(), sun.y() / 2, -sun.z() ).normalized() * lightDistance;

    NEWMAT::ColumnVector keyPosition( 3 );
    keyPosition << sun.x() << sun.y() << sun.z();

    NEWMAT::ColumnVector fillPosition( 3 );
    fillPosition << fill.x() << fill.y() << fill.z();

    // The sky has the screen's colours: skyBottomColor at the horizon, skyTopColor overhead.
    // The ground fog is dense at the floor and thin above it, so that the floor fades into
    // the horizon while the robot, a few units away, stays clear.
    NEWMAT::ColumnVector skyTopColor( 3 );
    skyTopColor << SIG_Visualisation::skyTopColor.x() << SIG_Visualisation::skyTopColor.y() << SIG_Visualisation::skyTopColor.z();

    NEWMAT::ColumnVector horizonColor( 3 );
    horizonColor << SIG_Visualisation::skyBottomColor.x() << SIG_Visualisation::skyBottomColor.y() << SIG_Visualisation::skyBottomColor.z();

    stream << environmentDeclarations()
	   << "\n";

    if (!writeResource( stream, "materials.inc" ))
      return false;

    stream << robotDeclarations()
	   << "#declare cameraRight = " << aspectRatio << "*x;\n"
	   << "#declare cameraDirection = <0,0," << directionLength << ">;\n"
	   << "\n"
	   << "light_source {\n"
	   << "  " << vectorToPovray( SIG_TypeConverter::sigelToPovray() * keyPosition ) << "\n"
	   << "  srgb <1, 0.96, 0.9>\n"
	   << "  area_light 100*x, 100*z, 9, 9\n"
	   << "  adaptive 1\n"
	   << "  circular\n"
	   << "  orient\n"
	   << "}\n"
	   << "\n"
	   << "light_source {\n"
	   << "  " << vectorToPovray( SIG_TypeConverter::sigelToPovray() * fillPosition ) << "\n"
	   << "  rgb <0.131, 0.16, 0.25>\n"
	   << "  area_light 150*x, 150*z, 5, 5\n"
	   << "  adaptive 1\n"
	   << "  circular\n"
	   << "  orient\n"
	   << "}\n"
	   << "\n"
	   << "sky_sphere {\n"
	   << "  pigment {\n"
	   << "    gradient y\n"
	   << "    color_map {\n"
	   << "      [0.5 srgb " << vectorToPovray( horizonColor ) << "]\n"
	   << "      [1.0 srgb " << vectorToPovray( skyTopColor ) << "]\n"
	   << "    }\n"
	   << "    scale 2\n"
	   << "    translate -y\n"
	   << "  }\n"
	   << "}\n"
	   << "\n"
	   << "fog {\n"
	   << "  fog_type 2\n"
	   << "  fog_offset 1\n"
	   << "  fog_alt 3\n"
	   << "  distance 60\n"
	   << "  srgb " << vectorToPovray( horizonColor ) << "\n"
	   << "}\n"
	   << "\n";

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
    // the indirect light changes little from frame to frame.
    if (!writeResource( stream, "globalSettings.inc" ))
      return false;

    stream << "#include "
	   << "\""
	   << includeFileName
	   << "\"\n"
	   << "\n"
	   << environmentObjects()
	   << sceneObjects( visualisation.getRobotRenderer() )
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

  QString SIG_PovrayExporter::environmentDeclarations() const
  {
    SIG_EnvironmentRenderer const &environmentRenderer = visualisation.getEnvironmentRenderer();

    QString declarationsString;
    QTextStream stream( &declarationsString, QIODevice::WriteOnly );

    NEWMAT::ColumnVector planeColorVector = SIG_TypeConverter::toColumnVector( environmentRenderer.sceneObjects[ 0 ]->getColor() );

    stream << "#declare Plane = plane {\n"
	   << "                   <0,1,0>, 0\n"
	   << "                   texture {\n"
	   << "                     pigment { srgb "
	   << vectorToPovray( planeColorVector )
	   << " }\n"
	   << "                     finish { ambient 0 diffuse 1 }\n"
	   << "                   }\n"
	   << "                 }\n"
	   << "\n";

    int const gridSpacing = environmentRenderer.getGridSpacing();

    int const noOfFields = 70;

    int const minX = - gridSpacing * noOfFields;
    int const maxX =   gridSpacing * noOfFields;
    int const minZ = - gridSpacing * noOfFields;
    int const maxZ =   gridSpacing * noOfFields;

    NEWMAT::ColumnVector gridColorVector = SIG_TypeConverter::toColumnVector( environmentRenderer.sceneObjects[ 1 ]->getColor() );

    stream << "#declare Grid = union {\n";

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
	       << ">, 0.02\n"
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
	       << ">, 0.02\n"
	       << "  }\n";
      };

    stream << "\n"
	   << "  pigment { srgb " << vectorToPovray( gridColorVector ) << " }\n"
	   << "  finish { ambient 0 diffuse 1 }\n"
	   << "}\n";

    return declarationsString;
  }

  QString SIG_PovrayExporter::robotDeclarations() const
  {
    SIGEL_Robot::SIG_Robot const &robot = visualisation.getRobotRenderer().getRobot();

    QString declarationsString;

    QTextStream stream( &declarationsString, QIODevice::WriteOnly );

    // The patterns scale with the robot, so that a small and a large robot look alike. A
    // robot without vertices has radius 0 and gets the patterns of a robot of radius 1.
    double const robotRadius = visualisation.getRobotRadius();

    double const patternSize = (robotRadius > 0) ? robotRadius : 1;

    // A robot file gives a material only a colour. It becomes paint, a little greyer and
    // darker than the colour, so that pure colours look like paint and not like plastic.
    // Where the noise pattern is high, the paint is worn off and the steel shows. The
    // map's textures take on the pattern's scale, so the paint's bumps, at 14/30 of it,
    // are robotRadius / 30 across.
    for ( SIGEL_Robot::SIG_Material *materialPtr : robot.getMaterials() )
      {
	SIGEL_Robot::SIG_Material &actMaterial = *materialPtr;

	NEWMAT::ColumnVector paintColor = SIG_TypeConverter::toColumnVector( actMaterial.getColour() );

	double const grey = 0.2126 * paintColor( 1 ) + 0.7152 * paintColor( 2 ) + 0.0722 * paintColor( 3 );

	for (int i=1; i<=3; i++)
	  paintColor( i ) = (paintColor( i ) + (grey - paintColor( i )) * 0.45) * 0.8;

	QString paint;
	QTextStream paintStream( &paint, QIODevice::WriteOnly );

	paintStream << "pigment { srgb " << vectorToPovray( paintColor ) << " }\n"
		    << "      normal { bumps 0.015 scale 0.4667 }\n"
		    << "      finish { sigelPaintFinish }";

	stream << "#declare "
	       << actMaterial.getName()
	       << " = texture {\n"
	       << "  bozo\n"
	       << "  turbulence 1\n"
	       << "  scale " << patternSize / 14 << "\n"
	       << "  texture_map {\n"
	       << "    [0.00 " << paint << "]\n"
	       << "    [0.70 " << paint << "]\n"
	       << "    [0.73 sigelSteel]\n"
	       << "    [1.00 sigelSteel]\n"
	       << "  }\n"
	       << "}\n"
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
			    << ", 0.05\n"
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

    int xPos = static_cast< int >( std::floor( lookPoint.x / gridSpacing ) * gridSpacing );
    int zPos = - static_cast< int >( std::floor( lookPoint.z / gridSpacing ) * gridSpacing );

    stream << "union {\n"
	   << sceneObjects( environmentRenderer )
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
		   << ", 0.02\n"
		   << "  open\n"
		   << "  pigment { srgb <1,1,0> }\n"
		   << "  finish { ambient 0 diffuse 1 }\n"
		   << "}\n";
	  };

	stream << "\n";
      };

    return resultString;
  }

  QString SIG_PovrayExporter::sceneObjects( SIG_Renderer const &renderer )
  {
    GLuint const noOfObjects = renderer.getNoOfObjects();

    QString resultString;
    QTextStream resultStream( &resultString, QIODevice::WriteOnly );

    for (int i=0; i < noOfObjects; i++)
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
