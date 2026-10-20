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
#include <QImage>
#include <QVector3D>
#include <QtMath>
#include "SIGEL_Visualisation/SIG_EnvironmentRenderer.h"
#include "SIGEL_Visualisation/SIG_Visualisation.h"

#include "qmessagebox.h"
#include "SIGEL_Tools/SIG_DialogParent.h"

#include <stdlib.h>

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{

  SIG_EnvironmentRenderer::SIG_EnvironmentRenderer(SIGEL_Environment::SIG_Environment const &environment)
    : SIG_Renderer( 2, 0),
      environment(environment),
      terrain( environment.getTerrain() ),
      gridSpacing( 1 ),
      lookPoint(0, 0, 0),
      eyePoint(0, 0, 0),
      reach( SIG_Visualisation::farPlane ),
      showRobotPath( false )
  {
    SIG_Matrix idRotation;
    idRotation.makeone();

    SIG_VisualSceneObject *plane = new SIG_VisualSceneObject( 0, "Plane" );

    plane->setPosition( SIG_Vector(0, 0, 0) );
    plane->setRotation( idRotation );

    delete sceneObjects[ 0 ];
    sceneObjects[ 0 ] = plane;

    SIG_VisualSceneObject *grid = new SIG_VisualSceneObject( 1, "Grid" );

    grid->setColor( SIG_Vector( 80.0 / 255, 80.0 / 255, 80.0 / 255 ) );

    grid->setPosition( SIG_Vector(0, 0, 0) );
    grid->setRotation( idRotation );
    grid->setVisible( false );

    delete sceneObjects[ 1 ];
    sceneObjects[ 1 ] = grid;

    buildDisplayLists();
  }

  SIG_EnvironmentRenderer::~SIG_EnvironmentRenderer()
  {
    // Nothing else frees the path points.
    qDeleteAll( robotPathPoints );
    robotPathPoints.clear();
  }

  void SIG_EnvironmentRenderer::setPlaneColor( double red, double green, double blue )
  {
    sceneObjects[ 0 ]->setColor( SIG_Vector( red, green, blue ) );
  }

  void SIG_EnvironmentRenderer::setGridColor( double red, double green, double blue )
  {
    sceneObjects[ 1 ]->setColor( SIG_Vector( red, green, blue ) );
  }

  double SIG_EnvironmentRenderer::groundDepth( const SIGEL_Environment::SIG_Terrain &terrain, int x, int z )
  {
    return terrain.getHeight( qBound( 0, x, terrain.getDimensionX() - 1 ), qBound( 0, z, terrain.getDimensionZ() - 1 ) );
  }

  void SIG_EnvironmentRenderer::setTerrainVertex( GLfloat v[3], int x, int z,
						  const SIGEL_Environment::SIG_Terrain &terrain )
  {
    v[0] = x;
    v[1] = -groundDepth( terrain, x, z );
    v[2] = z;
  }

  void SIG_EnvironmentRenderer::terrainVertex( GLfloat const v[3], int x, GLfloat t, bool withTexture )
  {
    if (withTexture)
      glTexCoord2f( (x % 2 == 0) ? 0.0 : 1.0, t );
    glVertex3fv( v );
  }

  void SIG_EnvironmentRenderer::buildDisplayLists()
  {
    // The plane and the grid have no display lists: drawInit and drawGrid draw them each
    // frame, around the eye. Only the texture is made once.
    withTexture = environment.getWithTexture();
    if (withTexture == true)
      withTexture = initTexture();
  }

  void SIG_EnvironmentRenderer::visibleCells( int &minX, int &maxX, int &minZ, int &maxZ ) const
  {
    // Row z lies at SIGEL z = -z.
    minX = qFloor( eyePoint.x - reach );
    maxX = qCeil( eyePoint.x + reach );
    minZ = qFloor( -eyePoint.z - reach );
    maxZ = qCeil( -eyePoint.z + reach );
  }

  void SIG_EnvironmentRenderer::drawGrid() const
  {
    int minX, maxX, minZ, maxZ;
    visibleCells( minX, maxX, minZ, maxZ );

    GLfloat vertex[2][3];

    glTranslatef(0,0.01,0);
    glRotated(180,1,0,0);

    for (int z=minZ; z<=maxZ; ++z) {
      glBegin(GL_LINES);
      for (int x=minX; x<maxX; ++x) {
        setTerrainVertex( vertex[0], x,   z, terrain );
        setTerrainVertex( vertex[1], x+1, z, terrain );

        glVertex3fv(vertex[0]);
        glVertex3fv(vertex[1]);
      }
      glEnd();
    }

    for (int x=minX; x<=maxX; ++x) {
      glBegin(GL_LINES);
      for (int z=minZ; z<maxZ; ++z) {
        setTerrainVertex( vertex[0], x, z,   terrain );
        setTerrainVertex( vertex[1], x, z+1, terrain );

        glVertex3fv(vertex[0]);
        glVertex3fv(vertex[1]);
      }
      glEnd();
    }
  }

  void SIG_EnvironmentRenderer::renderRobotPath()
  {
    if (robotPathPoints.count() >= 2)
      {
	glLineWidth( 2 );
	glColor3d( 1, 1, 0 );

	glBegin( GL_LINE_STRIP );
	for ( qsizetype i = 0; i < robotPathPoints.size(); i++ )
	  {
	    SIG_Vector *actPoint = robotPathPoints.at( i );
	    glVertex3d( GLdouble( actPoint->x ),
			GLdouble( actPoint->y ),
			GLdouble( actPoint->z ) );
	  };
	glEnd();

	glLineWidth( 1 );
      };
  }

  void SIG_EnvironmentRenderer::render()
  {
    renderPlane();
    renderGridAndPath();
  }

  void SIG_EnvironmentRenderer::renderPlane()
  {
    if (!sceneObjects[ 0 ]->getVisible())
      return;

    sceneObjects[ 0 ]->applyColor();
    glPushMatrix();
    sceneObjects[ 0 ]->applyTransformation();
    drawInit();
    glPopMatrix();
  }

  void SIG_EnvironmentRenderer::renderGridAndPath()
  {
    // Lines have no normals; lit, they take whatever normal came last.
    glPushAttrib( GL_ENABLE_BIT );
    glDisable( GL_LIGHTING );
    glDisable( GL_TEXTURE_2D );

    if (sceneObjects[ 1 ]->getVisible())
      {
	sceneObjects[ 1 ]->applyColor();
	glPushMatrix();
	sceneObjects[ 1 ]->applyTransformation();
	drawGrid();
	glPopMatrix();
      }

    if (showRobotPath)
      renderRobotPath();

    glPopAttrib();
  }

  void SIG_EnvironmentRenderer::setLookPoint( SIG_Vector newPosition )
  {
    lookPoint = newPosition;
  }

  void SIG_EnvironmentRenderer::setEyePoint( SIG_Vector newPosition, double newReach )
  {
    eyePoint = newPosition;
    reach = newReach;
  }

  void SIG_EnvironmentRenderer::setShowPlane( bool newShowPlane )
  {
    sceneObjects[ 0 ]->setVisible( newShowPlane );
  }

  void SIG_EnvironmentRenderer::setShowGrid( bool newShowGrid )
  {
    sceneObjects[ 1 ]->setVisible( newShowGrid );
  }

  void SIG_EnvironmentRenderer::setShowRobotPath( bool newShowRobotPath )
  {
    showRobotPath = newShowRobotPath;
  }

  void SIG_EnvironmentRenderer::addRobotPathPoint( SIG_Vector newPoint )
  {
    SIG_Vector *newPointObject = new SIG_Vector( newPoint );

    robotPathPoints.append( newPointObject );
  }

  bool SIG_EnvironmentRenderer::loadTexture( QString fileName )
  {
    QImage image( fileName );

    if (image.isNull())
      {
	QMessageBox warn(QMessageBox::Warning, "Warning", "Could not open the texture file.\nThe floor is shown without a texture.", QMessageBox::Retry, SIGEL_Tools::dialogParent());
	warn.exec();

	return false;
      }

    image.convertTo( QImage::Format_RGBA8888 );

    texture.width  = image.width();
    texture.height = image.height();

    texture.imageData.assign( image.constBits(), image.constBits() + image.sizeInBytes() );

    GLubyte const alpha = environment.getTexAlpha();

    for (GLuint i = 3; i < texture.imageData.size(); i += 4)
      texture.imageData[ i ] = alpha;

    return true;
  }

  bool SIG_EnvironmentRenderer::initTexture() {
    // Load up our textures.
    QString texFile = environment.getTextureFile();

    if ( texFile.length() == 0 ) {
  		QMessageBox warn(QMessageBox::Warning, "Warning", "The specified texture file does not exist.\nThe floor is shown without a texture.", QMessageBox::Retry, SIGEL_Tools::dialogParent());
  		warn.exec();
    	return false;
    }

    if ( !loadTexture( texFile ) )
    	return false;

    // Set up texture environment.
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

    // Generate OpenGL texture IDs.
    glGenTextures(1, &texture.texID);

    // Give our data to OpenGL.
    glBindTexture(GL_TEXTURE_2D, texture.texID);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 								// RGBA textures.
                 texture.width, texture.height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, 													// imageData is a GLubyte pointer.
                 texture.imageData.data());

    return true;
  }

  void SIG_EnvironmentRenderer::drawInit() const {
   int minX, maxX, minZ, maxZ;
   visibleCells( minX, maxX, minZ, maxZ );

   glFrontFace( GL_CCW );

	 if (withTexture) {
   	 glEnable(GL_TEXTURE_2D);
	 	 glBindTexture(GL_TEXTURE_2D, texture.texID);
	 }

   glRotated(180,1,0,0);

   // Each cell is two triangles, split along the same diagonal as in DynaMechs.
   GLfloat a0[3], b0[3], a1[3], b1[3];
   for (int z=minZ; z<maxZ; ++z) {
       glBegin(GL_TRIANGLES);
       for (int x=minX+1; x<=maxX; ++x) {
            // a is row z+1, b is row z; 0 is column x-1, 1 is column x.
            setTerrainVertex( a0, x-1, z+1, terrain );
            setTerrainVertex( b0, x-1, z,   terrain );
            setTerrainVertex( a1, x,   z+1, terrain );
            setTerrainVertex( b1, x,   z,   terrain );

            QVector3D normal = QVector3D::normal( QVector3D( a0[0], a0[1], a0[2] ),
						  QVector3D( b0[0], b0[1], b0[2] ),
						  QVector3D( a1[0], a1[1], a1[2] ) );
            glNormal3f( normal.x(), normal.y(), normal.z() );
            terrainVertex( a0, x-1, 1.0, withTexture );
            terrainVertex( b0, x-1, 0.0, withTexture );
            terrainVertex( a1, x,   1.0, withTexture );

            normal = QVector3D::normal( QVector3D( b1[0], b1[1], b1[2] ),
					QVector3D( a1[0], a1[1], a1[2] ),
					QVector3D( b0[0], b0[1], b0[2] ) );
            glNormal3f( normal.x(), normal.y(), normal.z() );
            terrainVertex( a1, x,   1.0, withTexture );
            terrainVertex( b0, x-1, 0.0, withTexture );
            terrainVertex( b1, x,   0.0, withTexture );
       } // for-loop (x)
       glEnd();
   } // for-loop (z)

	 if (withTexture) glDisable(GL_TEXTURE_2D);
  }

}
