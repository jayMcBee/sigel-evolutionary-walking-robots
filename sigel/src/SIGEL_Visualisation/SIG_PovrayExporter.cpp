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

    static double const xAngle = 100;

    static double const xAngleRad = qDegreesToRadians( xAngle );

    static double const directionLength = 0.5 * 1 / std::tan( xAngleRad / 2 );

    stream << environmentDeclarations()
	   << "\n"
	   << robotDeclarations()
	   << "#declare cameraRight = " << aspectRatio << "*x;\n"
	   << "#declare cameraDirection = <0,0," << directionLength << ">;\n"
	   << "\n"
	   << "#declare headLight = light_source {\n"
	   << "                       <0,0,0>,\n"
	   << "                       rgb 1\n"
	   << "                       fade_distance 3\n"
	   << "                       fade_power 1\n"
	   << "                     }\n"
	   << "\n"
	   << "background { rgb <0.258824,0.258824,0.435294> }\n"
	   << "\n"
	   << "fog {\n"
	   << "  fog_type 1\n"
	   << "  distance 80\n"
	   << "  rgb <0.258824,0.258824,0.435294>\n"
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

    stream << "#include "
	   << "\""
	   << includeFileName
	   << "\"\n"
	   << "\n"
	   << environmentObjects()
	   << sceneObjects( visualisation.getRobotRenderer() )
	   << "object {\n"
	   << "  headLight\n"
	   << "  translate "
	   << vectorToPovray( SIG_TypeConverter::sigelToPovray() * finalEyePointColumnVector )
	   << "\n"
	   << "}\n"
	   << "\n"
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
	   << "}\n"
	   << "\n"
	   << "global_settings { ambient_light rgb "
	   << vectorToPovray( visualisation.getAmbientSceneColor() )
	   << " }\n";

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
	   << "                     pigment { rgb "
	   << vectorToPovray( planeColorVector )
	   << " }\n"
	   << "                     finish { ambient rgb "
	   << vectorToPovray( planeColorVector )
	   << "\n"
	   << "                              diffuse 1 }\n"
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
	   << "  pigment { rgb " << vectorToPovray( gridColorVector ) << " }\n"
	   << "  finish { ambient rgb " << vectorToPovray( gridColorVector ) << "\n"
	   << "           diffuse 1 }\n"
	   << "}\n";

    return declarationsString;
  }

  QString SIG_PovrayExporter::robotDeclarations() const
  {
    SIGEL_Robot::SIG_Robot const &robot = visualisation.getRobotRenderer().getRobot();

    QString declarationsString;

    QTextStream stream( &declarationsString, QIODevice::WriteOnly );

    for ( SIGEL_Robot::SIG_Material *materialPtr : robot.getMaterials() )
      {
	SIGEL_Robot::SIG_Material &actMaterial = *materialPtr;

	stream << "#declare "
	       << actMaterial.getName()
	       << " = texture {\n"
	       << "  pigment { rgb "
	       << vectorToPovray( SIG_TypeConverter::toColumnVector( actMaterial.getColour() ) )
	       << " }\n"
	       << "  finish { ambient rgb "
	       << vectorToPovray( SIG_TypeConverter::toColumnVector( actMaterial.getColour() ) )
	       << "\n"
	       << "           diffuse 1 }\n"
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
			    << "    "
			    << actLink.getMaterial()->getName()
			    << "\n"
			    << "    pigment { rgb "
			    << vectorToPovray( linkColor )
			    << " }\n"
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
		   << "  pigment { rgb <1,1,0> }\n"
		   << "  finish { ambient rgb <1,1,0>\n"
		   << "           diffuse 1 }\n"
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

}
