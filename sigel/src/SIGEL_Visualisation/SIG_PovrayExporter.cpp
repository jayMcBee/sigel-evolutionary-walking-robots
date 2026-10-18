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
	   << visualisation.getRobotRenderer().createPovrayDeclarations()
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
	   << visualisation.getEnvironmentRenderer().exportToPovray()
	   << visualisation.getRobotRenderer().exportToPovray()
	   << "object {\n"
	   << "  headLight\n"
	   << "  translate "
	   << SIG_Renderer::vectorToPovray( SIG_TypeConverter::sigelToPovray() * finalEyePointColumnVector )
	   << "\n"
	   << "}\n"
	   << "\n"
	   << "camera {\n"
	   << "  right cameraRight\n"
	   << "  up y\n"
	   << "  direction cameraDirection\n"
	   << "  location "
	   << SIG_Renderer::vectorToPovray( SIG_TypeConverter::sigelToPovray() * finalEyePointColumnVector )
	   << "\n"
	   << "  look_at "
	   << SIG_Renderer::vectorToPovray( SIG_TypeConverter::sigelToPovray() * lookPointColumnVector )
	   << "\n"
	   << "}\n"
	   << "\n"
	   << "global_settings { ambient_light rgb "
	   << SIG_Renderer::vectorToPovray( visualisation.getAmbientSceneColor() )
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
	   << SIG_Renderer::vectorToPovray( planeColorVector )
	   << " }\n"
	   << "                     finish { ambient rgb "
	   << SIG_Renderer::vectorToPovray( planeColorVector )
	   << "\n"
	   << "                              diffuse 1 }\n"
	   << "                   }\n"
	   << "                 }\n"
	   << "\n";

    int const fieldEdgeLength = environmentRenderer.getFieldEdgeLength();

    int const noOfFields = 70;

    int const minX = - fieldEdgeLength * noOfFields;
    int const maxX =   fieldEdgeLength * noOfFields;
    int const minZ = - fieldEdgeLength * noOfFields;
    int const maxZ =   fieldEdgeLength * noOfFields;

    NEWMAT::ColumnVector gridColorVector = SIG_TypeConverter::toColumnVector( environmentRenderer.sceneObjects[ 1 ]->getColor() );

    stream << "#declare Grid = union {\n";

    for (int x=minX; x<=maxX; x+=fieldEdgeLength)
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

    for (int z=minZ; z<=maxZ; z+=fieldEdgeLength)
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
	   << "  pigment { rgb " << SIG_Renderer::vectorToPovray( gridColorVector ) << " }\n"
	   << "  finish { ambient rgb " << SIG_Renderer::vectorToPovray( gridColorVector ) << "\n"
	   << "           diffuse 1 }\n"
	   << "}\n";

    return declarationsString;
  }

}
