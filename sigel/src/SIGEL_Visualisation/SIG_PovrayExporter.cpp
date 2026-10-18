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

#include <QFile>
#include <QTextStream>
#include <QtMath>

#include <cmath>

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

    stream << visualisation.getEnvironmentRenderer().createPovrayDeclarations()
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

}
