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
#include "SIGEL_Visualisation/SIG_Visualisation.h"

#ifdef _WINDOWS
#include <GL/glaux.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace SIGEL_Visualisation
{

  double const SIG_Visualisation::fieldOfView = 100;

  SIG_Visualisation::SIG_Visualisation()
    : viewSettings(),
      floatingTexts(),
      ambientSceneColor( 3 )
  {
    ambientSceneColor( 1 ) = ambientSceneColor( 2 ) = ambientSceneColor( 3 ) = 1;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);

    glClearColor(1, 1, 1, 1);
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );

    glEnable( GL_LIGHT0 );
    glEnable( GL_LIGHT1 );

    glCullFace( GL_BACK );

    GLfloat const diffuseLightIntensity = 1;

    QList< GLfloat > diffuseLightColor( 4 );
    for (int i=0; i<3; i++)
      diffuseLightColor[i] = diffuseLightIntensity;
    diffuseLightColor[3] = 1;

    glLightfv( GL_LIGHT0, GL_DIFFUSE, diffuseLightColor.data() );
    GLfloat diffuseLightColor1[4] = {0.6,0.6,0.6,0.0};
    glLightfv( GL_LIGHT1, GL_DIFFUSE, diffuseLightColor1 );
    GLfloat specularLightColor1[4] = {1.0,1.0,1.0,0.0};
    glLightfv( GL_LIGHT1, GL_SPECULAR, specularLightColor1 );
    GLfloat positionLight1[4] = {0.0,10.0,0.0,1.0};
    glLightfv( GL_LIGHT1, GL_POSITION, positionLight1 );

    QList< GLfloat > fogColor( 4 );
    fogColor.fill( 1 );

    updateAspectRatio();
  };

  void SIG_Visualisation::visualize()
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    DL_vector finalEyePoint;
    if (viewSettings.relativeEyePoint) {
     	finalEyePoint.assign( &viewSettings.lookPoint );
     	finalEyePoint.plusis( &viewSettings.eyePoint );
    }
    else {
    	finalEyePoint.assign( &viewSettings.eyePoint );
    };

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    gluLookAt( static_cast<GLdouble>(finalEyePoint.get(0)),
	       static_cast<GLdouble>(finalEyePoint.get(1)),
	       static_cast<GLdouble>(finalEyePoint.get(2)),
	       static_cast<GLdouble>(viewSettings.lookPoint.get(0)),
	       static_cast<GLdouble>(viewSettings.lookPoint.get(1)),
	       static_cast<GLdouble>(viewSettings.lookPoint.get(2)),
	       static_cast<GLdouble>(viewSettings.up.get(0)),
	       static_cast<GLdouble>(viewSettings.up.get(1)),
	       static_cast<GLdouble>(viewSettings.up.get(2)) );

    QList< GLfloat > lightPos( 4 );

    lightPos[0] = static_cast<GLfloat>(finalEyePoint.get(0));
    lightPos[1] = static_cast<GLfloat>(finalEyePoint.get(1));
    lightPos[2] = static_cast<GLfloat>(finalEyePoint.get(2));
    lightPos[3] = 1;
    glLightfv( GL_LIGHT0, GL_POSITION, lightPos.data() );

    switch (viewSettings.renderMode)
      {
      case SIG_ViewSettings::wireFrame:
       	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
       	glShadeModel( GL_FLAT );
       	glDisable( GL_LIGHTING );
       	glDisable( GL_COLOR_MATERIAL );
       	glDisable( GL_CULL_FACE );
       	break;
      case SIG_ViewSettings::flatShaded:
       	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
       	glShadeModel( GL_FLAT );
       	glEnable( GL_LIGHTING );
       	glEnable( GL_COLOR_MATERIAL );
       	glEnable( GL_CULL_FACE );
       	break;
      case SIG_ViewSettings::garoudShaded:
       	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
       	glShadeModel( GL_SMOOTH );
       	glEnable( GL_LIGHTING );
       	glEnable( GL_COLOR_MATERIAL );
       	glEnable( GL_CULL_FACE );
       	break;
      };
  };

  void SIG_Visualisation::setAmbientSceneColor( double red,
						double green,
						double blue )
  {
    ambientSceneColor( 1 ) = red;
    ambientSceneColor( 2 ) = green;
    ambientSceneColor( 3 ) = blue;

    QList< GLfloat > lightModelAmbientColor( 4 );

    lightModelAmbientColor[0] = red;
    lightModelAmbientColor[1] = green;
    lightModelAmbientColor[2] = blue;
    lightModelAmbientColor[3] = 1;

    glLightModelfv( GL_LIGHT_MODEL_AMBIENT, lightModelAmbientColor.data() );
  };

  void SIG_Visualisation::updateAspectRatio()
  {
    GLdouble const fovy = fieldOfView;
    GLdouble const _winport_near = 0.01;
    GLdouble const _winport_far = 50;
    GLdouble const aspectRatio = static_cast<GLdouble>(viewSettings.aspectRatio);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective( fovy, aspectRatio, _winport_near, _winport_far );
  };

  SIG_Visualisation::~SIG_Visualisation()
  { };

}
