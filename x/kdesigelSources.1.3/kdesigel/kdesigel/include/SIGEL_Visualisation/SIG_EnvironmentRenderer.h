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
#ifndef SIGEL_VISUALISATION_SIG_ENVIRONMENTRENDERER_H
#define SIGEL_VISUALISATION_SIG_ENVIRONMENTRENDERER_H

#include "SIGEL_Visualisation/SIG_Renderer.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Visualisation/SIG_ViewSettings.h"

#include <QList>
#include <pointvector.h>

namespace SIGEL_Visualisation
{

  /**
  	*	This struct defines a TGA-file to support Textures
  	*/
  typedef struct {
		// GLubyte will work for images up to 32-bits.
		GLubyte *imageData;

		// Stores the color depth of the image in bits
		// per pixel.
		GLuint   bpp;

		GLuint   width;
		GLuint   height;

		GLuint   texID;     // For texture objects; more later.

	} Texture;
	
  /**
   * This classes task is the rendering of SIG_Environment objects.
   */
  class SIG_EnvironmentRenderer : public SIG_Renderer
  {
  public:

    /**
     * The constructor of the SIG_EnvironmentRenderer.
     *
     * It reads the supplied SIG_Environment, fills the inherited
     * QList sceneObjects and calls the method buildDisplayLists.
     *
     * @param environment The environment that should be rendered.
     */
    SIG_EnvironmentRenderer(SIGEL_Environment::SIG_Environment const &environment);

    /**
     * The destructor of the SIG_EnvironmentRenderer.
     *
     * It cleans up the OpenGL stuff like the display lists.
     */
    ~SIG_EnvironmentRenderer();
    // This class owns raw pointers in a QList and frees them in its
    // destructor, so a copy would free them twice. Qt 2's QGVector copy
    // cleared del_item and freed nothing.
    SIG_EnvironmentRenderer( const SIG_EnvironmentRenderer & ) = delete;
    SIG_EnvironmentRenderer &operator=( const SIG_EnvironmentRenderer & ) = delete;


    /**
     * Calls the inherited method renderSceneObjects and
     * does additional rendering like drawing the main plane
     * which is the main component of every SIG_Environment.
     *
     * @pre The display lists are builded.
     */
    void render();

    QString exportToPovray();

    QString createPovrayDeclarations();

    void setLookPoint( DL_vector newPosition );

    void setRenderMode( SIG_ViewSettings::renderModeType newRenderMode );

    void setPlaneColor( double red,
			double green,
			double blue );

    void setShowPlane( bool newShowPlane );

    void setShowGrid( bool newShowGrid );

    void setShowRobotPath( bool newShowRobotPath );

    void addRobotPathPoint( DL_vector newPoint );

    /**
    	*	This functions loads the specified file and writes all the relevant data
    	*	into the texture struct. For now the pnm format only supports RGB data. Thus
    	*	the RGBA information generates the function itself. The user specifies the magnitude
    	*	of the A information
    	*
    	*	@param *filename
    	*		This is the path of the texture file. The file format must be pnm.
    	*
    	*/
#ifdef _WINDOWS
    bool loadPNMTexture(const char *filename);
#else
    bool loadPNMTexture(char *filename);
#endif

    /**
    	* This function loads and initialize the texture. It does all the opengl stuff
    	*	to initialize texture mapping.
    	*
    	*/
    bool initTexture();
		
		/**
			*	This function generates the display list. It specifies the ground and
			*	the texture coordinates. The texture is only generated if the user turns
			* the option on.
			*
			*/
    void drawInit();

    /**	
			*	The next three functions are copied from the gldraw.cpp file in the dynamechs
			*	directory. For more information have a look at gldraw.cpp. These functions are
			* used by drawInit() and computes the normal vector. This function uses the next two
			*	functions to fullfill his task.
			*
			*
			*/
    inline void compute_face_normal(float v0[3], float v1[3], float v2[3],float normal[3]);

		/**
			*	This function computes the cross product. It is used by compute_face_normal.
			*
			*/
    inline void cross(float a[3], float b[3], float c[3]);

		/**
			* This function normalizes a vector. It is used by compute_face_normal.
			*
			*/
    inline float normalize(float v[3]);


  private:

    /**
     * Reads the environment attribute and uses
     * its properties like geometry of objects to
     * build display lists.
     */
    void buildDisplayLists();

    /**
     * Builds the display list to render the plane that
     * is part of every SIGEL_Environment::SIG_Environment.
     *
     * @param number The display list number to occupy.
     */
    void buildPlane(GLuint number);

    void buildGrid(GLuint number);

    void renderRobotPath();

    /**
     * The SIG_Environment object that should be rendered
     * by this SIG_EnvironmentRenderer.
     */
#ifdef _WINDOWS
    SIGEL_Environment::SIG_Environment &environment;
#else
    SIGEL_Environment::SIG_Environment const &environment;
#endif

    GLint const fieldEdgeLength;

    DL_vector lookPoint;

    SIG_ViewSettings::renderModeType renderMode;

    bool showPlane;

    bool showGrid;

    bool showRobotPath;

    bool withTexture;

    QList< DL_vector * > robotPathPoints;

    /**
    	*	This variable holds a texture
    	*/
    Texture texture;
  };

}

#endif // SIGEL_VISUALISATION_SIG_ENVIRONMENTRENDERER_H
