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
#include <QOpenGLContext>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include "SIGEL_Visualisation/SIG_SimulationVisualisation.h"
#include "SIGEL_Tools/SIG_IO.h"

#include <cmath>

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{

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
		  renderRecorder( robot.getLinks().size() ),
		  simulation( robot, environment, program, simulationParameter, renderRecorder ),
		  frameCounter( 0 )
	{
		robotPathPointAddingRate = static_cast< int >( 0.5 / simulationParameter.getStepSize() );

		if (robotPathPointAddingRate == 0)
			robotPathPointAddingRate = 1;

		// The slot is overwritten deliberately; QList::insert would shift.
		floatingTexts.resize( robot.getNrOfPoints() );
		for (int i=0; i<robot.getNrOfPoints(); i++)
			floatingTexts[ i ] = robotRenderer.floatingTexts[ i ];

		// The box around every vertex at time 0 gives the robot's centre and size.
		// A robot without vertices keeps a box of size 0 at the origin.
		SIG_Vector lowest( 0, 0, 0 ), highest( 0, 0, 0 );
		bool anyVertex = false;
		for ( SIGEL_Robot::SIG_Link *link : robot.getLinks() )
		{
			SIG_SceneObject *recordedLink = renderRecorder.robotLinks[ link->getNumber() ];
			for ( SIG_Vector *vertex : link->getGeometry()->getVertices() )
			{
				SIG_Vector worldVertex;
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

		SIG_Vector centre;
		centre.assign( &lowest );
		centre.plusis( &highest );
		centre.timesis( 0.5 );

		SIG_Vector diagonal;
		diagonal.assign( &highest );
		diagonal.minusis( &lowest );
		robotRadius = diagonal.norm() / 2;

		SIG_SceneObject *recordedRoot = renderRecorder.robotLinks[ robot.getRootLink()->getNumber() ];
		centre.minusis( &recordedRoot->position );
		recordedRoot->rotation.transposetimes( &centre, &centreInRootLink );

		updateRobotLinks();
		updateRobotPoints();

		initShadowMapping();
	}

	SIG_SimulationVisualisation::~SIG_SimulationVisualisation()
	{
		// shadowProgram exists only when the framebuffer and texture do.
		// Without its context current the objects go with the context.
		if (shadowProgram && (QOpenGLContext::currentContext() == shadowContext))
		{
			glFunctions->glDeleteFramebuffers( 1, &shadowFramebuffer );
			glDeleteTextures( 1, &shadowTexture );
		};
		delete shadowProgram;
	}

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
		else if (!program->addShaderFromSourceFile( QOpenGLShader::Vertex, ":/SIG_SimulationVisualisation/shaders/shadow.vert" ))
			failure = "the vertex shader cannot be loaded or does not compile:\n" + program->log();
		else if (!program->addShaderFromSourceFile( QOpenGLShader::Fragment, ":/SIG_SimulationVisualisation/shaders/shadow.frag" ))
			failure = "the fragment shader cannot be loaded or does not compile:\n" + program->log();
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
	}

	QMatrix4x4 SIG_SimulationVisualisation::renderShadowMap()
	{
		// The map covers the robot as the sun sees it, with room for limbs that
		// swing out. Its centre moves in whole texels, so the shadow's edge does
		// not shimmer as the robot moves.
		float const halfSize = robotRadius * shadowBoxMargin;
		float const texelSize = 2 * halfSize / shadowMapSize;

		QMatrix4x4 lightView;
		lightView.lookAt( sunDirection, QVector3D( 0, 0, 0 ), QVector3D( 0, 1, 0 ) );

		SIG_Vector robotCentre = getRobotCentre();
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
	}

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
	}

	void SIG_SimulationVisualisation::setPointsVisible( bool visible )
	{
		robotRenderer.setPointsVisible( visible );
	}

	void SIG_SimulationVisualisation::setPlaneColor( QColor newColor )
	{
		double red = double( newColor.red() ) / 255;
		double green = double( newColor.green() ) / 255;
		double blue = double( newColor.blue() ) / 255;

		environmentRenderer.setPlaneColor( red, green, blue );
	}

	void SIG_SimulationVisualisation::setGridColor( QColor newColor )
	{
		environmentRenderer.setGridColor( newColor.redF(), newColor.greenF(), newColor.blueF() );
	}

	void SIG_SimulationVisualisation::setShowPlane( bool newShowPlane )
	{
		environmentRenderer.setShowPlane( newShowPlane );
	}

	void SIG_SimulationVisualisation::setShowGrid( bool newShowGrid )
	{
		environmentRenderer.setShowGrid( newShowGrid );
	}

	void SIG_SimulationVisualisation::setShowRobotPath( bool newShowRobotPath )
	{
		environmentRenderer.setShowRobotPath( newShowRobotPath );
	}

	void SIG_SimulationVisualisation::setShowShadows( bool newShowShadows )
	{
		showShadows = newShowShadows;
	}

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

		environmentRenderer.setEyePoint( viewSettings.getAbsoluteEyePoint(), drawnDistance() );

		bool const hidden = (viewSettings.renderMode == SIG_ViewSettings::hiddenLine);
		if (hidden)
		{
			// The filled polygons go into the depth buffer only, pushed back a
			// little, so that the edges drawn next are hidden behind them.
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
			glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );
			glEnable( GL_POLYGON_OFFSET_FILL );
			glPolygonOffset( 1, 1 );

			environmentRenderer.render();
			robotRenderer.render();

			glDisable( GL_POLYGON_OFFSET_FILL );
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
			// The edges were drawn at the same depth in the first pass.
			glDepthFunc( GL_LEQUAL );
		};

		if (shadowed)
			renderShadowedScene( lightMatrix );
		else if (hidden || (viewSettings.renderMode == SIG_ViewSettings::wireFrame))
		{
			// The plane's many edges are blended in, so the robot stands out.
			glPushAttrib( GL_COLOR_BUFFER_BIT );
			glEnable( GL_BLEND );
			glBlendColor( 0, 0, 0, unlitPlaneAlpha );
			glBlendFunc( GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA );
			environmentRenderer.renderPlane();
			glPopAttrib();

			environmentRenderer.renderGridAndPath();
			robotRenderer.render();
		}
		else
		{
			environmentRenderer.render();
			robotRenderer.render();
		};

		if (hidden)
			glDepthFunc( GL_LESS );
	}

	void SIG_SimulationVisualisation::makeTimeSteps(int noOfTimeSteps)
	{
		for (int i=1; i <= noOfTimeSteps; i++)
		{
			simulation.makeTimeSteps( 1 );

			if ( frameCounter == 0 )
			{
				SIG_Vector realRobotPosition;

				SIG_Vector robotsRealOrigin = robot.initialLocation;
				robotsRealOrigin.timesis( -1 );

				this->getRobotRotation().times( &robotsRealOrigin,
				                                &realRobotPosition );

				SIG_Vector robotPosition = this->getRobotPosition();

				realRobotPosition.plusis( &robotPosition );

				environmentRenderer.addRobotPathPoint( realRobotPosition );
			};

			frameCounter++;
			frameCounter %= robotPathPointAddingRate;
		};

		updateRobotLinks();
		updateRobotPoints();
	}

	void SIG_SimulationVisualisation::updateRobotLinks()
	{
		int noOfLinks = robot.getLinks().size();
		for (int i=0; i<noOfLinks; i++)
		{
			robotRenderer.sceneObjects[i]->setPosition( renderRecorder.robotLinks[i]->position );
			robotRenderer.sceneObjects[i]->setRotation( renderRecorder.robotLinks[i]->rotation );
		};
	}

	void SIG_SimulationVisualisation::updateRobotPoints()
	{
		for (int i=robot.getLinks().size(); i<robotRenderer.sceneObjects.size(); i++)
		{
			int linkNumber = robotRenderer.sceneObjects[i]->getNumber();

			robotRenderer.sceneObjects[i]->setPosition( renderRecorder.robotLinks[ linkNumber ]->position );
			robotRenderer.sceneObjects[i]->setRotation( renderRecorder.robotLinks[ linkNumber ]->rotation );
		};
	}

	SIG_Vector SIG_SimulationVisualisation::getRobotPosition() const
	{
		int rootLinkNumber = robot.getRootLink()->getNumber();

		return renderRecorder.robotLinks[ rootLinkNumber ]->position;
	}

	SIG_Matrix SIG_SimulationVisualisation::getRobotRotation() const
	{
		int rootLinkNumber = robot.getRootLink()->getNumber();

		return renderRecorder.robotLinks[ rootLinkNumber ]->rotation;
	}

	SIG_Vector SIG_SimulationVisualisation::getRobotCentre() const
	{
		SIG_Vector offset = centreInRootLink;
		SIG_Vector centre;
		getRobotRotation().times( &offset, &centre );

		SIG_Vector position = getRobotPosition();
		centre.plusis( &position );

		return centre;
	}

}
