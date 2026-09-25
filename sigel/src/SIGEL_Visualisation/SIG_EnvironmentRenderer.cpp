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
#include "SIGEL_Visualisation/SIG_EnvironmentRenderer.h"

#include "SIGEL_Tools/SIG_TypeConverter.h"

#include "dm.h"
#include "qmessagebox.h"
#include "SIGEL_Tools/SIG_DialogParent.h"

#include <cmath>
#include <stdlib.h>

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{

  SIG_EnvironmentRenderer::SIG_EnvironmentRenderer(SIGEL_Environment::SIG_Environment const &environment)
    : SIG_Renderer( 2, 0),
#ifdef _WINDOWS
		environment( const_cast< SIGEL_Environment::SIG_Environment &>(environment) ),
#else		
      environment(environment),
#endif
      lookPoint(0, 0, 0),
      fieldEdgeLength( 1 ),
      showRobotPath( false )
  {

    DL_matrix idRotation;
    idRotation.makeone();

    SIG_VisualSceneObject *plane = new SIG_VisualSceneObject( 0, "Plane" );

    double planeLevel = environment.getYPlaneLevel();

    plane->setPosition( DL_vector(0, planeLevel, 0) );
    plane->setRotation( idRotation );

    delete sceneObjects[ 0 ];
    sceneObjects[ 0 ] = plane;

    SIG_VisualSceneObject *grid = new SIG_VisualSceneObject( 1, "Grid" );

    grid->setColor( DL_vector( 1, 0, 0 ) );

    grid->setPosition( DL_vector(0, planeLevel, 0) );
    grid->setRotation( idRotation );
    grid->setVisible( false );

    delete sceneObjects[ 1 ];
    sceneObjects[ 1 ] = grid;

    buildDisplayLists();
  };

  SIG_EnvironmentRenderer::~SIG_EnvironmentRenderer()
  {
    // robotPathPoints had setAutoDelete(true) and nothing else freed it.
    qDeleteAll( robotPathPoints );
    robotPathPoints.clear();
  };

  void SIG_EnvironmentRenderer::setPlaneColor( double red,
					       double green,
					       double blue )
  {
    sceneObjects[ 0 ]->setColor( DL_vector( red,
					    green,
					    blue ) );
  };

  // The robot starts at the terrain's corner, so the ground is drawn as far
  // again on the negative side of each axis, with the start in the middle.
  // Past the terrain, DynaMechs gives each point the height of the nearest
  // edge, so the ground continues each edge outward; this lookup does the same.
  static double groundDepth( double **depth, int x_dim, int z_dim, int x, int z )
  {
    return depth[ qBound( 0, x, x_dim - 1 ) ][ qBound( 0, z, z_dim - 1 ) ];
  }

  // A terrain vertex at column x and row z.
  static void setTerrainVertex( GLfloat v[3], int x, int z, double grid_resolution,
				double **depth, int x_dim, int z_dim )
  {
    v[0] = ((GLfloat) x)*grid_resolution;
    v[1] = -groundDepth( depth, x_dim, z_dim, x, z );
    v[2] = ((GLfloat) z)*grid_resolution;
  }

  // The texture repeats every two columns; t is 1 on row z+1 and 0 on row z.
  static void terrainVertex( GLfloat const v[3], int x, GLfloat t, bool withTexture )
  {
    if (withTexture)
      glTexCoord2f( (x % 2 == 0) ? 0.0 : 1.0, t );
    glVertex3fv( v );
  }

  void SIG_EnvironmentRenderer::buildDisplayLists()
  {
    GLuint planeListIndex = displayListsOffset;
    buildPlane( planeListIndex );

    GLuint gridListIndex = displayListsOffset + 1;

    buildGrid( gridListIndex );
  };

  void SIG_EnvironmentRenderer::buildPlane(GLuint number)
  {

    glNewList(number, GL_COMPILE);

    glFrontFace( GL_CCW );

    withTexture = environment.getWithTexture();
		if (withTexture == true)
    	withTexture = initTexture();

    drawInit();

    glEndList();


  };

	void SIG_EnvironmentRenderer::buildGrid(GLuint number)
  {
   	int x,z;
   	GLfloat vertex[2][3];
	 	int x_dim, z_dim;
	 	double grid_resolution;
	 	double **depth;
	 	
  	glNewList(number, GL_COMPILE);
    glFrontFace( GL_CCW );

	 	dmEnvironment *dynaMechsEnvironment = environment.getDMEnvironment();
		depth = dynaMechsEnvironment->getTerrainData(x_dim, z_dim, grid_resolution);
	 	// TODO: the method getTerrainData(...) doesn't correctly sets the grid_resolution
	 	//	so it must be done manually
	 	grid_resolution =1; 	 	

	 	glTranslatef(0,0.01,0);
	 	glRotated(180,1,0,0);

   	for (z=1-z_dim; z<z_dim-1; ++z) {

      glBegin(GL_LINES);
    	for (x=1-x_dim; x<x_dim-1; ++x) {
          	
      	vertex[0][0] = ((GLfloat) x)*grid_resolution;
      	vertex[0][1] = -groundDepth(depth, x_dim, z_dim, x, z);
      	vertex[0][2] = ((GLfloat) z)*grid_resolution;

      	vertex[1][0] = ((GLfloat) x+1)*grid_resolution;
      	vertex[1][1] = -groundDepth(depth, x_dim, z_dim, x+1, z);
      	vertex[1][2] = ((GLfloat) z)*grid_resolution;
	
      	glVertex3fv(vertex[0]);
      	glVertex3fv(vertex[1]);
      }
      glEnd();
    }
    	
    for (x=1-x_dim; x<x_dim-1; ++x) {

      glBegin(GL_LINES);
      for (z=1-z_dim; z<z_dim-1; ++z) {
          	
      	vertex[0][0] = ((GLfloat) x)*grid_resolution;
      	vertex[0][1] = -groundDepth(depth, x_dim, z_dim, x, z);
      	vertex[0][2] = ((GLfloat) z)*grid_resolution;

      	vertex[1][0] = ((GLfloat) x)*grid_resolution;
      	vertex[1][1] = -groundDepth(depth, x_dim, z_dim, x, z+1);
      	vertex[1][2] = ((GLfloat) z+1)*grid_resolution;
        	
      	glVertex3fv(vertex[0]);
      	glVertex3fv(vertex[1]);
      }
      glEnd();
    }	

    glEndList();
  };

  void SIG_EnvironmentRenderer::renderRobotPath()
  {
    if (robotPathPoints.count() >= 2)
      {
	glLineWidth( 2 );
	glColor3d( 1, 1, 0 );

	glBegin( GL_LINE_STRIP );
	// Qt 2 cursor walk. The count() >= 2 guard above is what kept first()
	// off an empty list, which is UB in Qt 6.
	for ( qsizetype i = 0; i < robotPathPoints.size(); i++ )
	  {
	    DL_vector *actPoint = robotPathPoints.at( i );
	    glVertex3d( GLdouble( actPoint->x ),
			GLdouble( actPoint->y ),
			GLdouble( actPoint->z ) );
	  };
	glEnd();

	glLineWidth( 1 );
      };
  };

  void SIG_EnvironmentRenderer::render()
  {
    renderPlane();
    renderGridAndPath();
  };

  void SIG_EnvironmentRenderer::renderPlane()
  {
    renderSceneObjects( 0, 1 );
  };

  void SIG_EnvironmentRenderer::renderGridAndPath()
  {
    renderSceneObjects( 1, noOfObjects );

    if (showRobotPath)
      renderRobotPath();
  };

  bool SIG_EnvironmentRenderer::getWithTexture() const
  {
    return withTexture;
  };

  QString SIG_EnvironmentRenderer::exportToPovray()
  {
    QString resultString;
    QTextStream stream( &resultString, QIODevice::WriteOnly );

#ifdef _WINDOWS
    int xPos = static_cast< int >( ::floor( lookPoint.get( 0 ) / fieldEdgeLength ) * fieldEdgeLength );
    int zPos = - static_cast< int >( ::floor( lookPoint.get( 2 ) / fieldEdgeLength ) * fieldEdgeLength );
#else
    int xPos = static_cast< int >( std::floor( lookPoint.get( 0 ) / fieldEdgeLength ) * fieldEdgeLength );
    int zPos = - static_cast< int >( std::floor( lookPoint.get( 2 ) / fieldEdgeLength ) * fieldEdgeLength );
#endif

    stream << "union {\n"
	   << exportSceneObjectsToPovray()
	   << "translate <"
	   << xPos
	   << ",0,"
	   << zPos
	   << ">\n"
	   << "}\n"
	   << "\n";

    if (showRobotPath)
      if (robotPathPoints.count() >= 2)
	{
	  // pairs walk: (0,1), (1,2), ... -- the Qt 2 cursor version stepped
	  // prevPoint up behind actPoint
	  for ( qsizetype i = 1; i < robotPathPoints.size(); i++ )
	    {
	      DL_vector *prevPoint = robotPathPoints.at( i - 1 );
	      DL_vector *actPoint  = robotPathPoints.at( i );
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
  };

  QString SIG_EnvironmentRenderer::createPovrayDeclarations()
  {
    QString declarationsString;
    QTextStream stream( &declarationsString, QIODevice::WriteOnly );

    NEWMAT::ColumnVector planeColorVector = SIG_TypeConverter::toColumnVector( sceneObjects[ 0 ]->getColor() );

    stream << "#declare Plane = plane {\n"
	   << " less                  <0,1,0>, 0\n"
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

    int const noOfFields = 70;

    int const minX = - fieldEdgeLength * noOfFields;
    int const maxX =   fieldEdgeLength * noOfFields;
    int const minZ = - fieldEdgeLength * noOfFields;
    int const maxZ =   fieldEdgeLength * noOfFields;

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
	   << "  pigment { rgb <1,0,0> }\n"
	   << "  finish { ambient rgb <1,0,0>\n"
	   << "           diffuse 1 }\n"
	   << "}\n";

    return declarationsString;
  };

  void SIG_EnvironmentRenderer::setLookPoint( DL_vector newPosition )
  {
    lookPoint = newPosition;
  };

  void SIG_EnvironmentRenderer::setShowPlane( bool newShowPlane )
  {
    sceneObjects[ 0 ]->setVisible( newShowPlane );
  };

  void SIG_EnvironmentRenderer::setShowGrid( bool newShowGrid )
  {
    sceneObjects[ 1 ]->setVisible( newShowGrid );
  };

  void SIG_EnvironmentRenderer::setShowRobotPath( bool newShowRobotPath )
  {
    showRobotPath = newShowRobotPath;
  };

  void SIG_EnvironmentRenderer::addRobotPathPoint( DL_vector newPoint )
  {
    DL_vector *newPointObject = new DL_vector( newPoint );

    robotPathPoints.append( newPointObject );
  };

#ifdef _WINDOWS
  bool SIG_EnvironmentRenderer::loadPNMTexture(const char *filename) {
#else
  bool SIG_EnvironmentRenderer::loadPNMTexture(char *filename) {
#endif
		// the pnmMagic identifies a pnm file with pixmap (RGB) data
		// and it is stored in raw data format
		GLubyte		 pnmMagic[2] = { 'P', '6' };
  	GLubyte    fileMagic[2];

  	// Header data.
  	GLubyte    header[58];
  	GLuint     bytesPerPixel;

  	GLuint     imageSize;

  	FILE *file = fopen(filename, "rb");

  	// Start reading in the header, make sure we
  	// have the right PNM data type.
  	
  	if(file == NULL                                                           	||
  			fread(fileMagic, 1, sizeof(fileMagic), file)  != sizeof(fileMagic) 			||
  			memcmp(pnmMagic, fileMagic, sizeof(pnmMagic)) != 0                			||
  			fread(header, 1, sizeof(header), file)        != sizeof(header))
  	{
  		QMessageBox warn("Warning", "Could not open the texture-file!\nDisplay it without a texture!",QMessageBox::Warning, QMessageBox::Retry, QMessageBox::NoButton,QMessageBox::NoButton, SIGEL_Tools::dialogParent());
  		warn.exec();  	

  		return false;
  	}
  	
  	// The size and colour depth are fixed, not read from the header.
  	
  	texture.width  = 256;
  	texture.height = 256;
  	texture.bpp  = 24;
  	bytesPerPixel = 4;
  	imageSize     = texture.width * texture.height * bytesPerPixel;

  	texture.imageData = (GLubyte *)malloc(imageSize);

  	if(texture.imageData == NULL) {
  		QMessageBox warn("Warning", "The specified texture-file does not contain valid data!\nDisplay it without a texture!",QMessageBox::Warning, QMessageBox::Retry, QMessageBox::NoButton,QMessageBox::NoButton, SIGEL_Tools::dialogParent());
  		warn.exec();  		

			fclose(file);
  		return false;
  	}

  	// this for loop reads the the image data of the specified file and
  	// adds the missing (RGB)A information
  	char *currentPixel;
  	int alpha = environment.getTexAlpha();
  	currentPixel = (char*) texture.imageData;
  	
  	for(int i = 0; i < imageSize; i += bytesPerPixel) {
  		fread(currentPixel, 1, 3, file);
  		// this is the A information for RGBA
  		texture.imageData[i + 3] = alpha;
  		currentPixel += 4;
  	}
  	
  	fclose (file);
  	return true;
  };

  bool SIG_EnvironmentRenderer::initTexture() {

    // Load up our textures.
    QString texFile = environment.getTextureFile();

    if ( texFile.length() == 0 ) {
  		QMessageBox warn("Warning", "The specified texture-file does not exist!\n Display it without a texture!",QMessageBox::Warning, QMessageBox::Retry, QMessageBox::NoButton,QMessageBox::NoButton, SIGEL_Tools::dialogParent());
  		warn.exec();
    	return false;
    }

    if ( !loadPNMTexture(texFile.toUtf8().data()) )
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
                 texture.imageData);

    return true;
  };

  void SIG_EnvironmentRenderer::drawInit() {
   int x, z;

   GLfloat normal[3];
	 int x_dim, z_dim;
	 double grid_resolution;
	 double **depth;
	
	 dmEnvironment *dynaMechsEnvironment = environment.getDMEnvironment();
	
	 depth = dynaMechsEnvironment->getTerrainData(x_dim, z_dim, grid_resolution);
	 // TODO: the method getTerrainData(...) doesn't correctly sets the grid_resolution
	 //	so it must be done manually
	 grid_resolution =1;

	 if (withTexture) {
   	 glEnable(GL_TEXTURE_2D);
	 	 glBindTexture(GL_TEXTURE_2D, texture.texID);
	 }

   glRotated(180,1,0,0);

   // Each cell is drawn as two separate triangles. Strips in a display
   // list may be joined into one draw, and the joins show as lines in the
   // line modes.
   GLfloat a0[3], b0[3], a1[3], b1[3];
   for (z=1-z_dim; z<z_dim-1; ++z) {
       glBegin(GL_TRIANGLES);
       for (x=2-x_dim; x<x_dim; ++x) {
            // a is row z+1, b is row z; 0 is column x-1, 1 is column x.
            setTerrainVertex( a0, x-1, z+1, grid_resolution, depth, x_dim, z_dim );
            setTerrainVertex( b0, x-1, z,   grid_resolution, depth, x_dim, z_dim );
            setTerrainVertex( a1, x,   z+1, grid_resolution, depth, x_dim, z_dim );
            setTerrainVertex( b1, x,   z,   grid_resolution, depth, x_dim, z_dim );

            compute_face_normal(a0, b0, a1, normal);
            glNormal3fv(normal);
            terrainVertex( a0, x-1, 1.0, withTexture );
            terrainVertex( b0, x-1, 0.0, withTexture );
            terrainVertex( a1, x,   1.0, withTexture );

            compute_face_normal(b1, a1, b0, normal);
            glNormal3fv(normal);
            terrainVertex( a1, x,   1.0, withTexture );
            terrainVertex( b0, x-1, 0.0, withTexture );
            terrainVertex( b1, x,   0.0, withTexture );
       } // for-loop (x)
       glEnd();
   } // for-loop (z)

	 if (withTexture) glDisable(GL_TEXTURE_2D);
  }

  inline void SIG_EnvironmentRenderer::cross(float a[3], float b[3], float c[3]) {
     c[0] = a[1]*b[2] - a[2]*b[1];
     c[1] = a[2]*b[0] - a[0]*b[2];
     c[2] = a[0]*b[1] - a[1]*b[0];
  }

  inline float SIG_EnvironmentRenderer::normalize(float v[3]) {
      float norm = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

      if (norm > 0.0) {
         v[0] /= norm;
         v[1] /= norm;
         v[2] /= norm;
      }

      return norm;
  }


  inline void SIG_EnvironmentRenderer::compute_face_normal(float v0[3], float v1[3], float v2[3],float normal[3]) {
      float a[3], b[3];
      int i;

      for (i=0; i<3; ++i) {
          a[i] = v1[i] - v0[i];
          b[i] = v2[i] - v0[i];
      }

      cross(a, b, normal);
      normalize(normal);

  }

}
