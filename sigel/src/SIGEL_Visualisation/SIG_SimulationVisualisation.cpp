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
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include "SIGEL_Visualisation/SIG_SimulationVisualisation.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

#include <QtMath>
#include <cmath>

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{
  // The lighting of GL_LIGHT0 and GL_LIGHT1 with colour material is
  // computed per vertex. Both lights are positional without attenuation,
  // have no ambient part, and the material has no specular part, so each
  // gives only its diffuse term. The sun's diffuse light goes out
  // separately in the secondary colour, so the fragment shader can shadow
  // it. Both are colour varyings, so GL_FLAT shading keeps flat mode flat.
  char const *const SIG_SimulationVisualisation::shadowVertexShaderSource =
    "#version 120\n"
    "uniform mat4 shadowMatrix;\n"
    "uniform vec3 sunDirection;\n"
    "uniform float sunIntensity;\n"
    "varying vec4 shadowCoordinate;\n"
    "vec3 cameraLight( int index, vec3 normal, vec3 eyePosition )\n"
    "{\n"
    "  vec3 towardsLight = normalize( gl_LightSource[ index ].position.xyz - eyePosition );\n"
    "  return max( dot( normal, towardsLight ), 0.0 ) * gl_LightSource[ index ].diffuse.rgb;\n"
    "}\n"
    "void main()\n"
    "{\n"
    "  vec4 eyePosition = gl_ModelViewMatrix * gl_Vertex;\n"
    "  vec3 normal = normalize( gl_NormalMatrix * gl_Normal );\n"
    "  vec3 light = gl_LightModel.ambient.rgb\n"
    "             + cameraLight( 0, normal, eyePosition.xyz )\n"
    "             + cameraLight( 1, normal, eyePosition.xyz );\n"
    "  gl_FrontColor = vec4( light * gl_Color.rgb, gl_Color.a );\n"
    "  vec3 sun = max( dot( normal, sunDirection ), 0.0 ) * sunIntensity * gl_Color.rgb;\n"
    "  gl_FrontSecondaryColor = vec4( sun, 1.0 );\n"
    "  gl_TexCoord[ 0 ] = gl_MultiTexCoord0;\n"
    "  shadowCoordinate = shadowMatrix * eyePosition;\n"
    "  gl_Position = ftransform();\n"
    "}\n";

  // Each of the 16 Poisson disk taps is a hardware 2x2 comparison, as the
  // shadow map filters linearly. The terrain texture is applied as
  // GL_DECAL does it.
  char const *const SIG_SimulationVisualisation::shadowFragmentShaderSource =
    "#version 120\n"
    "uniform sampler2D terrainTexture;\n"
    "uniform sampler2DShadow shadowMap;\n"
    "uniform bool terrainTextured;\n"
    "uniform float shadowFilterRadius;\n"
    "varying vec4 shadowCoordinate;\n"
    "const int poissonTapCount = 16;\n"
    "const vec2 poissonDisk[ poissonTapCount ] = vec2[ poissonTapCount ](\n"
    "  vec2( -0.94201624, -0.39906216 ), vec2(  0.94558609, -0.76890725 ),\n"
    "  vec2( -0.09418410, -0.92938870 ), vec2(  0.34495938,  0.29387760 ),\n"
    "  vec2( -0.91588581,  0.45771432 ), vec2( -0.81544232, -0.87912464 ),\n"
    "  vec2( -0.38277543,  0.27676845 ), vec2(  0.97484398,  0.75648379 ),\n"
    "  vec2(  0.44323325, -0.97511554 ), vec2(  0.53742981, -0.47373420 ),\n"
    "  vec2( -0.26496911, -0.41893023 ), vec2(  0.79197514,  0.19090188 ),\n"
    "  vec2( -0.24188840,  0.99706507 ), vec2( -0.81409955,  0.91437590 ),\n"
    "  vec2(  0.19984126,  0.78641367 ), vec2(  0.14383161, -0.14100790 ) );\n"
    "void main()\n"
    "{\n"
    "  vec3 coordinate = shadowCoordinate.xyz / shadowCoordinate.w;\n"
    "  float sunlight = 0.0;\n"
    "  for (int i = 0; i < poissonTapCount; i++)\n"
    "    sunlight += shadow2D( shadowMap, vec3( coordinate.xy + poissonDisk[ i ] * shadowFilterRadius,\n"
    "                                           coordinate.z ) ).r;\n"
    "  sunlight /= float( poissonTapCount );\n"
    "  vec4 color = vec4( min( gl_Color.rgb + gl_SecondaryColor.rgb * sunlight, 1.0 ), gl_Color.a );\n"
    "  if (terrainTextured)\n"
    "  {\n"
    "    vec4 texel = texture2D( terrainTexture, gl_TexCoord[ 0 ].st );\n"
    "    color.rgb = mix( color.rgb, texel.rgb, texel.a );\n"
    "  }\n"
    "  gl_FragColor = color;\n"
    "}\n";

   SIG_SimulationVisualisation::SIG_SimulationVisualisation(SIGEL_Robot::SIG_Robot const &robot,
							    SIGEL_Environment::SIG_Environment const &environment,
							    SIGEL_Simulation::SIG_SimulationParameters const &simulationParameter,
							    SIGEL_Program::SIG_Program const &program)
     : SIG_Visualisation(),
       shadowContext( nullptr ),
       glFunctions( nullptr ),
       shadowProgram( nullptr ),
       shadowFramebuffer( 0 ),
       shadowTexture( 0 ),
       showShadows( true ),
       environmentRenderer(environment),
       robotRenderer(robot),
       robot(robot),
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

     // The slot is overwritten deliberately; QList::insert would shift.
     floatingTexts.resize( robot.getNrOfPoints() );
     for (int i=0; i<robot.getNrOfPoints(); i++)
       floatingTexts[ i ] = robotRenderer.floatingTexts[ i ];

     // The box around every vertex at time 0 gives the robot's centre and size.
     // A robot without vertices keeps a box of size 0 at the origin.
     DL_vector lowest( 0, 0, 0 ), highest( 0, 0, 0 );
     bool anyVertex = false;
     for ( SIGEL_Robot::SIG_Link *link : robot.getLinks() )
       {
	 SIG_SceneObject *recordedLink = renderRecorder->robotLinks[ link->getNumber() ];
	 for ( DL_vector *vertex : link->getGeometry()->getVertices() )
	   {
	     DL_vector worldVertex;
	     recordedLink->rotation.times( vertex, &worldVertex );
	     worldVertex.plusis( &recordedLink->position );
	     if (!anyVertex)
	       {
		 lowest.assign( &worldVertex );
		 highest.assign( &worldVertex );
		 anyVertex = true;
	       };
	     for (int k=0; k<3; k++)
	       {
		 if (worldVertex.get(k) < lowest.get(k))
		   lowest.set( k, worldVertex.get(k) );
		 if (worldVertex.get(k) > highest.get(k))
		   highest.set( k, worldVertex.get(k) );
	       };
	   };
       };

     DL_vector centre;
     centre.assign( &lowest );
     centre.plusis( &highest );
     centre.timesis( 0.5 );

     DL_vector diagonal;
     diagonal.assign( &highest );
     diagonal.minusis( &lowest );
     robotRadius = diagonal.norm() / 2;

     SIG_SceneObject *recordedRoot = renderRecorder->robotLinks[ robot.getRootLink()->getNumber() ];
     centre.minusis( &recordedRoot->position );
     recordedRoot->rotation.transposetimes( &centre, &centreInRootLink );

     updateRobotLinks();
     updateRobotPoints();

     initShadowMapping();
   };

   SIG_SimulationVisualisation::~SIG_SimulationVisualisation()
   {
     delete simulation;
     delete renderRecorder;

     // shadowProgram exists only when the framebuffer and texture do.
     // Without its context current the objects go with the context.
     if (shadowProgram && (QOpenGLContext::currentContext() == shadowContext))
       {
	 glFunctions->glDeleteFramebuffers( 1, &shadowFramebuffer );
	 glDeleteTextures( 1, &shadowTexture );
       };
     delete shadowProgram;
   };

  void SIG_SimulationVisualisation::initShadowMapping()
  {
    shadowContext = QOpenGLContext::currentContext();
    if (!shadowContext)
      {
	SIGEL_Tools::SIG_IO::cerr << "Error: shadow mapping failed: there is no current OpenGL context."
				  << Qt::endl;
	return;
      };

    // A robot whose geometry files hold no vertices has radius 0 and casts no shadow.
    if (robotRadius <= 0)
      return;

    glFunctions = shadowContext->extraFunctions();

    glGenTextures( 1, &shadowTexture );
    glBindTexture( GL_TEXTURE_2D, shadowTexture );
    glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, shadowMapSize, shadowMapSize, 0,
		  GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    // Outside the shadow map everything is in sunlight.
    GLfloat const borderDepth[4] = { 1, 1, 1, 1 };
    glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderDepth );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL );
    glBindTexture( GL_TEXTURE_2D, 0 );

    GLint previousFramebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &previousFramebuffer );
    glFunctions->glGenFramebuffers( 1, &shadowFramebuffer );
    glFunctions->glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
    glFunctions->glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
					 GL_TEXTURE_2D, shadowTexture, 0 );
    glDrawBuffer( GL_NONE );
    glReadBuffer( GL_NONE );
    GLenum const framebufferStatus = glFunctions->glCheckFramebufferStatus( GL_FRAMEBUFFER );
    glFunctions->glBindFramebuffer( GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer) );

    QOpenGLShaderProgram *program = new QOpenGLShaderProgram();
    QString failure;
    if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
      failure = QString( "the shadow map framebuffer is incomplete, status 0x%1." )
	.arg( framebufferStatus, 0, 16 );
    else if (!program->addShaderFromSourceCode( QOpenGLShader::Vertex, shadowVertexShaderSource ))
      failure = "the vertex shader does not compile:\n" + program->log();
    else if (!program->addShaderFromSourceCode( QOpenGLShader::Fragment, shadowFragmentShaderSource ))
      failure = "the fragment shader does not compile:\n" + program->log();
    else if (!program->link())
      failure = "the shader program does not link:\n" + program->log();

    if (!failure.isEmpty())
      {
	SIGEL_Tools::SIG_IO::cerr << "Error: shadow mapping failed: " << failure << Qt::endl;
	delete program;
	glFunctions->glDeleteFramebuffers( 1, &shadowFramebuffer );
	glDeleteTextures( 1, &shadowTexture );
	shadowFramebuffer = 0;
	shadowTexture = 0;
	return;
      };

    program->bind();
    program->setUniformValue( "terrainTexture", static_cast<GLint>(0) );
    program->setUniformValue( "shadowMap", static_cast<GLint>(1) );
    program->setUniformValue( "shadowFilterRadius", shadowFilterRadius / shadowMapSize );
    program->setUniformValue( "sunIntensity", sunIntensity );
    program->release();

    shadowProgram = program;
  };

  QMatrix4x4 SIG_SimulationVisualisation::renderShadowMap()
  {
    // The map covers the robot as the sun sees it. Its centre moves in
    // whole texels, so the shadow's edge does not shimmer as the robot moves.
    float const halfSize = robotRadius;
    float const texelSize = 2 * halfSize / shadowMapSize;

    QMatrix4x4 lightView;
    lightView.lookAt( sunDirection, QVector3D( 0, 0, 0 ), QVector3D( 0, 1, 0 ) );

    DL_vector robotCentre = getRobotCentre();
    QVector3D const centre = lightView.map( QVector3D( robotCentre.get(0),
							robotCentre.get(1),
							robotCentre.get(2) ) );
    float const centreX = std::floor( centre.x() / texelSize ) * texelSize;
    float const centreY = std::floor( centre.y() / texelSize ) * texelSize;

    QMatrix4x4 lightProjection;
    lightProjection.ortho( centreX - halfSize, centreX + halfSize,
			   centreY - halfSize, centreY + halfSize,
			   -centre.z() - halfSize, -centre.z() + halfSize );

    GLint previousFramebuffer = 0;
    glGetIntegerv( GL_FRAMEBUFFER_BINDING, &previousFramebuffer );

    glPushAttrib( GL_ENABLE_BIT | GL_POLYGON_BIT | GL_VIEWPORT_BIT );
    glFunctions->glBindFramebuffer( GL_FRAMEBUFFER, shadowFramebuffer );
    glViewport( 0, 0, shadowMapSize, shadowMapSize );
    glClear( GL_DEPTH_BUFFER_BIT );

    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadMatrixf( lightProjection.constData() );
    glMatrixMode( GL_MODELVIEW );
    glLoadMatrixf( lightView.constData() );

    glDisable( GL_CULL_FACE );
    glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    glEnable( GL_POLYGON_OFFSET_FILL );
    glPolygonOffset( 2, 4 );

    robotRenderer.renderLinks();

    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    glFunctions->glBindFramebuffer( GL_FRAMEBUFFER, static_cast<GLuint>(previousFramebuffer) );
    glPopAttrib();

    return lightProjection * lightView;
  };

  void SIG_SimulationVisualisation::renderShadowedScene( QMatrix4x4 const &lightMatrix )
  {
    GLfloat cameraViewValues[16];
    glGetFloatv( GL_MODELVIEW_MATRIX, cameraViewValues );
    // QMatrix4x4 reads its values row by row; OpenGL gives them column by column.
    QMatrix4x4 const cameraView = QMatrix4x4( cameraViewValues ).transposed();

    QMatrix4x4 textureFromClip;
    textureFromClip.translate( 0.5f, 0.5f, 0.5f );
    textureFromClip.scale( 0.5f );
    QMatrix4x4 const shadowMatrix = textureFromClip * lightMatrix * cameraView.inverted();

    glFunctions->glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, shadowTexture );
    glFunctions->glActiveTexture( GL_TEXTURE0 );

    shadowProgram->bind();
    shadowProgram->setUniformValue( "shadowMatrix", shadowMatrix );
    shadowProgram->setUniformValue( "sunDirection", cameraView.mapVector( sunDirection ).normalized() );

    shadowProgram->setUniformValue( "terrainTextured", static_cast<GLint>(environmentRenderer.getWithTexture()) );
    environmentRenderer.renderPlane();
    shadowProgram->setUniformValue( "terrainTextured", static_cast<GLint>(false) );
    robotRenderer.renderLinks();

    shadowProgram->release();

    glFunctions->glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    glFunctions->glActiveTexture( GL_TEXTURE0 );

    environmentRenderer.renderGridAndPath();
    robotRenderer.renderPoints();
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

  void SIG_SimulationVisualisation::setShowShadows( bool newShowShadows )
  {
    showShadows = newShowShadows;
  };

   void SIG_SimulationVisualisation::visualize()
   {
     bool const shadowed = showShadows
                        && (shadowProgram != nullptr)
                        && (   (viewSettings.renderMode == SIG_ViewSettings::flatShaded)
                            || (viewSettings.renderMode == SIG_ViewSettings::garoudShaded) );
     QMatrix4x4 lightMatrix;
     if (shadowed)
       lightMatrix = renderShadowMap();

     SIG_Visualisation::visualize();

     environmentRenderer.setLookPoint( viewSettings.lookPoint );

     bool const pointMode = (viewSettings.renderMode == SIG_ViewSettings::points);
     bool const hidden = (viewSettings.renderMode == SIG_ViewSettings::hiddenLine)
                      || (viewSettings.renderMode == SIG_ViewSettings::points);
     if (hidden)
       {
	 // The filled polygons go into the depth buffer only, pushed back a
	 // little, so that the edges or points drawn next are hidden behind them.
	 glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	 glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
	 glEnable( GL_POLYGON_OFFSET_FILL );
	 glPolygonOffset( 1, 1 );

	 environmentRenderer.render();
	 robotRenderer.render();

	 glDisable( GL_POLYGON_OFFSET_FILL );
	 glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	 glPolygonMode( GL_FRONT_AND_BACK, pointMode ? GL_POINT : GL_LINE );
	 // Lines and points were drawn at the same depth in the first pass.
	 glDepthFunc( GL_LEQUAL );
       };

     // The robot's anchor points set their own point size, so it is set
     // again here, before the visible pass.
     if (pointMode)
       glPointSize( 2 );

     if (shadowed)
       renderShadowedScene( lightMatrix );
     else
       {
	 environmentRenderer.render();
	 robotRenderer.render();
       };

     if (hidden)
       glDepthFunc( GL_LESS );
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

  DL_vector SIG_SimulationVisualisation::getRobotCentre() const
  {
    DL_vector offset = centreInRootLink;
    DL_vector centre;
    getRobotRotation().times( &offset, &centre );

    DL_vector position = getRobotPosition();
    centre.plusis( &position );

    return centre;
  };

  double SIG_SimulationVisualisation::getRobotRadius() const
  {
    return robotRadius;
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

    static double const xAngle = 100;

    static double const xAngleRad = qDegreesToRadians( xAngle );

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
