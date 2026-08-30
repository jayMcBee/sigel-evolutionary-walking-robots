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
#include <QIODevice>
#include <QFile>
#include "SIGEL_Visualisation/SIG_SimulationVisualisation.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

#include <cmath>

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{
   SIG_SimulationVisualisation::SIG_SimulationVisualisation(SIGEL_Robot::SIG_Robot const &robot,
							    SIGEL_Environment::SIG_Environment const &environment,
							    SIGEL_Simulation::SIG_SimulationParameters const &simulationParameter,
							    SIGEL_Program::SIG_Program const &program)
     : SIG_Visualisation(),
       environmentRenderer(environment),
       robotRenderer(robot),
       robot(robot),
       environment(environment),
       simulationParameter(simulationParameter),
       program(program),
       frameCounter( 0 )
   {
     robotPathPointAddingRate = static_cast< int >( 0.5 / simulationParameter.getStepSize() );

     if (robotPathPointAddingRate == 0)
       robotPathPointAddingRate = 1;

     int noOfLinks = robot.getLinks().size();
     renderRecorder = new SIG_RenderRecorder(noOfLinks);

     simulation = new SIGEL_Simulation::SIG_Simulation(robot,
						       environment,
						       program,
						       simulationParameter,
						       *renderRecorder);

     floatingTexts.resize( robot.getNrOfPoints() );
     for (int i=0; i<robot.getNrOfPoints(); i++)
       floatingTexts.insert( i, robotRenderer.floatingTexts[ i ] );

     updateRobotLinks();
     updateRobotPoints();
   };

   SIG_SimulationVisualisation::~SIG_SimulationVisualisation()
   {
     delete simulation;
     delete renderRecorder;
   };

  void SIG_SimulationVisualisation::setPointsVisible( bool visible )
  {
    robotRenderer.setPointsVisible( visible );
  };

  void SIG_SimulationVisualisation::setPlaneColor( QColor newColor )
  {
    double red = double( newColor.red() ) / 255;
    double green = double( newColor.green() ) / 255;
    double blue = double( newColor.blue() ) / 255;

    environmentRenderer.setPlaneColor( red,
				       green,
				       blue );
  };

  void SIG_SimulationVisualisation::setShowPlane( bool newShowPlane )
  {
    environmentRenderer.setShowPlane( newShowPlane );
  };

  void SIG_SimulationVisualisation::setShowGrid( bool newShowGrid )
  {
    environmentRenderer.setShowGrid( newShowGrid );
  };

  void SIG_SimulationVisualisation::setShowRobotPath( bool newShowRobotPath )
  {
    environmentRenderer.setShowRobotPath( newShowRobotPath );
  };

   void SIG_SimulationVisualisation::visualize()
   {
     SIG_Visualisation::visualize();

     environmentRenderer.setLookPoint( viewSettings.lookPoint );
     environmentRenderer.setRenderMode( viewSettings.renderMode );
     environmentRenderer.render();
     robotRenderer.render();
   };

   void SIG_SimulationVisualisation::makeTimeSteps(int noOfTimeSteps)
   {
     for (int i=1; i <= noOfTimeSteps; i++)
       {
	 simulation->makeTimeSteps( 1 );

	 if ( frameCounter == 0 )
	   {
	     DL_vector realRobotPosition;

	     DL_vector robotsRealOrigin = robot.initialLocation;
	     robotsRealOrigin.timesis( -1 );

	     this->getRobotRotation().times( &robotsRealOrigin,
					     &realRobotPosition );

	     DL_vector robotPosition = this->getRobotPosition();

	     realRobotPosition.plusis( &robotPosition );

	     environmentRenderer.addRobotPathPoint( realRobotPosition );
	   };

	 frameCounter++;
	 frameCounter %= robotPathPointAddingRate;
       };

     updateRobotLinks();
     updateRobotPoints();
   };

  void SIG_SimulationVisualisation::updateRobotLinks()
  {
    int noOfLinks = robot.getLinks().size();
    for (int i=0; i<noOfLinks; i++)
      {
	robotRenderer.sceneObjects[i]->setPosition( renderRecorder->robotLinks[i]->position );
	robotRenderer.sceneObjects[i]->setRotation( renderRecorder->robotLinks[i]->rotation );
      };
  };

  void SIG_SimulationVisualisation::updateRobotPoints()
  {
    int noOfPoints = robotRenderer.sceneObjects.size() - robot.getLinks().size();

    for (int i=robot.getLinks().size(); i<robotRenderer.sceneObjects.size(); i++)
      {
	int linkNumber = robotRenderer.sceneObjects[i]->getNumber();

	robotRenderer.sceneObjects[i]->setPosition( renderRecorder->robotLinks[ linkNumber ]->position );
	robotRenderer.sceneObjects[i]->setRotation( renderRecorder->robotLinks[ linkNumber ]->rotation );
      };
  };

  QTime SIG_SimulationVisualisation::getSimulationTime() const
  {
    return renderRecorder->simulationTime;
  };

  DL_vector SIG_SimulationVisualisation::getRobotPosition() const
  {
    int rootLinkNumber = robot.getRootLink()->getNumber();

    return renderRecorder->robotLinks[ rootLinkNumber ]->position;
  };

  DL_matrix SIG_SimulationVisualisation::getRobotRotation() const
  {
    int rootLinkNumber = robot.getRootLink()->getNumber();

    return renderRecorder->robotLinks[ rootLinkNumber ]->rotation;
  };

  bool SIG_SimulationVisualisation::exportToPovray( QString includeFilename,
						    QString fileName )
  {
    QFile file( fileName );

    if (!file.open( QIODevice::WriteOnly ))
      return false;

    QTextStream stream( &file );

    DL_vector finalEyePoint;
    if (viewSettings.relativeEyePoint)
      {
	finalEyePoint.assign( &viewSettings.lookPoint );
	finalEyePoint.plusis( &viewSettings.eyePoint );
      }
    else
      {
	finalEyePoint.assign( &viewSettings.eyePoint );
      };

    NEWMAT::ColumnVector finalEyePointColumnVector = SIG_TypeConverter::toColumnVector( finalEyePoint );

    NEWMAT::ColumnVector lookPointColumnVector = SIG_TypeConverter::toColumnVector( viewSettings.lookPoint );

    stream << "#include "
	   << "\""
	   << includeFilename
	   << "\"\n"
	   << "\n"
	   << environmentRenderer.exportToPovray()
	   << robotRenderer.exportToPovray()
	   << "object {\n"
	   << "  headLight\n"
	   << "  translate "
	   << SIG_Renderer::vectorToPovray( SIG_TypeConverter::sigelToPovray() * finalEyePointColumnVector )
	   << "\n"
	   << "}\n"
	   << "\n"
	   << "camera {\n"
	   << "  cameraSettings\n"
	   << "  location "
	   << SIG_Renderer::vectorToPovray( SIG_TypeConverter::sigelToPovray() * finalEyePointColumnVector )
	   << "\n"
	   << "  look_at "
	   << SIG_Renderer::vectorToPovray( SIG_TypeConverter::sigelToPovray() * lookPointColumnVector )
	   << "\n"
	   << "}\n"
	   << "\n"
	   << "global_settings { ambient_light rgb "
	   << SIG_Renderer::vectorToPovray( ambientSceneColor )
	   << " }\n";

    file.close();

    return true;
  };

  bool SIG_SimulationVisualisation::createPovrayIncludeFile( QString fileName,
							     double aspectRatio )
  {
    QFile file( fileName );

    if (!file.open( QIODevice::WriteOnly ))
      return false;

    QTextStream stream( &file );

#ifdef _WINDOWS
    static double const pi = 4 * ::atan( 1 );
#else
    static double const pi = 4 * std::atan( 1 );
#endif

    static double const xAngle = 100;

    static double const xAngleRad = (xAngle / 360) * 2 * pi;

#ifdef _WINDOWS
    static double const directionLength = 0.5 * 1 / ::tan( xAngleRad / 2 );
#else
    static double const directionLength = 0.5 * 1 / std::tan( xAngleRad / 2 );
#endif

    stream << environmentRenderer.createPovrayDeclarations()
	   << "\n"
	   << robotRenderer.createPovrayDeclarations()
	   << "#declare cameraSettings = camera {\n"
	   << "                            right " << aspectRatio << "*x\n"
	   << "                            up y\n"
	   << "                            direction <0,0,"
	   << directionLength
	   << ">\n"
	   << "                          }\n"
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
  };
}
