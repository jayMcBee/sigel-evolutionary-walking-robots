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
#include <vector>
#include "SIGEL_Tools/SIG_Vector.h"

namespace SIGEL_Visualisation
{

  /**
   * The floor texture: RGBA pixels, read from an image file.
   */
  typedef struct {
		std::vector< GLubyte > imageData;

		GLuint   width;
		GLuint   height;

		GLuint   texID;     // The OpenGL texture name.
	} Texture;
	
  /**
   * Draws an environment: the plane, the grid and the robot path.
   */
  class SIG_EnvironmentRenderer : public SIG_Renderer
  {
  public:

    /**
     * Creates the plane and the grid, which is hidden, and loads the texture.
     *
     * @pre An OpenGL context is current.
     */
    SIG_EnvironmentRenderer(SIGEL_Environment::SIG_Environment const &environment);

    /**
     * Deletes the robot path points.
     */
    ~SIG_EnvironmentRenderer();
    SIG_EnvironmentRenderer( const SIG_EnvironmentRenderer & ) = delete;
    SIG_EnvironmentRenderer &operator=( const SIG_EnvironmentRenderer & ) = delete;

    /**
     * Draws the plane, then the grid and the robot path, each if it is shown.
     */
    void render() override;

    /**
     * Draws the plane, if it is shown.
     */
    void renderPlane();

    /**
     * Draws the grid and the robot path, each if it is shown.
     */
    void renderGridAndPath();

    /**
     * Returns true if the plane is drawn with the texture.
     */
    bool getWithTexture() const { return withTexture; }

    SIGEL_Environment::SIG_Environment const &getEnvironment() const { return environment; }

    SIGEL_Environment::SIG_Terrain const &getTerrain() const { return terrain; }

    int getGridSpacing() const { return gridSpacing; }

    bool getShowRobotPath() const { return showRobotPath; }

    QList< SIG_Vector * > const &getRobotPathPoints() const { return robotPathPoints; }

    /** The plane and the grid are drawn as far as reach around the eye point. */
    void setEyePoint( SIG_Vector newPosition, double newReach );

    void setPlaneColor( double red, double green, double blue );

    void setGridColor( double red, double green, double blue );

    void setShowPlane( bool newShowPlane );

    void setShowGrid( bool newShowGrid );

    void setShowRobotPath( bool newShowRobotPath );

    void addRobotPathPoint( SIG_Vector newPoint );

    /**
     * Reads an image file of any size and format Qt reads (PNM, PNG, JPEG) into texture,
     * and gives each pixel the environment's texture alpha. Shows a warning and returns
     * false if the file cannot be read.
     */
    bool loadTexture( QString fileName );

    /**
     * Loads the environment's texture file and gives it to OpenGL. Shows a warning and
     * returns false if no file name is set or the file cannot be read.
     */
    bool initTexture();
		
    /**
     * Draws the ground around the eye, textured if withTexture is set.
     */
    void drawInit() const;

    /**
     * The depth at column x and row z. Past the terrain, DynaMechs gives each point the
     * height of the nearest edge, so the ground continues each edge outward without end;
     * this lookup does the same.
     */
    static double groundDepth( const SIGEL_Environment::SIG_Terrain &terrain, int x, int z );

  private:

    /**
     * Loads the texture, if the environment has one.
     */
    void buildDisplayLists() override;

    /** The columns and rows of the cells within reach of the eye. */
    void visibleCells( int &minX, int &maxX, int &minZ, int &maxZ ) const;

    /** Draws the grid lines around the eye. */
    void drawGrid() const;

    /**
     * Sets v to the terrain vertex at column x and row z.
     */
    static void setTerrainVertex( GLfloat v[3], int x, int z,
				  const SIGEL_Environment::SIG_Terrain &terrain );

    /**
     * Draws the vertex v in column x. The texture repeats every two columns;
     * t is 1 on the cell's upper row and 0 on its lower row.
     */
    static void terrainVertex( GLfloat const v[3], int x, GLfloat t, bool withTexture );

    void renderRobotPath();

    SIGEL_Environment::SIG_Environment const &environment;

    /** Made once: getTerrain prints its warning on each call. */
    SIGEL_Environment::SIG_Terrain const terrain;

    GLint const gridSpacing;

    SIG_Vector eyePoint;

    double reach;

    bool showRobotPath;

    bool withTexture;

    QList< SIG_Vector * > robotPathPoints;

    Texture texture;
  };

}

#endif // SIGEL_VISUALISATION_SIG_ENVIRONMENTRENDERER_H
