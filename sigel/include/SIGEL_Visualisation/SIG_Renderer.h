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
#ifndef SIGEL_VISUALISATION_SIG_RENDERER_H
#define SIGEL_VISUALISATION_SIG_RENDERER_H

#include <QList>

#include <GL/gl.h>

#include "SIGEL_Visualisation/SIG_VisualSceneObject.h"

namespace SIGEL_Visualisation
{

  /**
   * This is the abstract superclass of all concrete Renderer classes.
   *
   * Its task is to execute the appropriate OpenGL-commands to render
   * something. What is rendered is specified in the subclasses.
   */
  class SIG_Renderer
  {
  public:
    /**
     * Reserves one OpenGL display list per scene object.
     *
     * @pre An OpenGL context is current.
     * @param noOfObjects The size of sceneObjects.
     * @param noOfFloatingTexts The size of floatingTexts.
     */
    SIG_Renderer(int noOfObjects, int noOfFloatingTexts);

    /**
     * Deletes the scene objects and the floating texts, and frees the display lists.
     */
    virtual ~SIG_Renderer();
    SIG_Renderer( const SIG_Renderer & ) = delete;
    SIG_Renderer &operator=( const SIG_Renderer & ) = delete;

    /**
     * Draws this renderer's objects into the current OpenGL context.
     *
     * @pre buildDisplayLists has run.
     */
    virtual void render() = 0;

    GLuint getNoOfObjects() const { return noOfObjects; }

    /**
     * Scene object i uses display list displayListsOffset + i, unless the subclass draws it
     * itself. Public, so that the owner can move the objects as the simulation runs.
     */
    QList<SIG_VisualSceneObject *> sceneObjects;

    QList<SIG_FloatingText *> floatingTexts;

  protected:

    /**
     * The number of the first display list
     * of this SIG_Renderer.
     */
    GLuint displayListsOffset;

    /**
     * The number of objects in sceneObjects, which
     * should be rendered by the method renderSceneObjects.
     */
    GLuint noOfObjects;

    /**
     * Prepares the drawing, such as recording the OpenGL commands that draw each scene
     * object in its display list.
     */
    virtual void buildDisplayLists() = 0;

    /**
     * Calls the display lists of the scene objects from
     * index first up to, but not including, index end.
     *
     * @pre buildDisplayLists has run.
     */
    void renderSceneObjects( GLuint first, GLuint end );
  };

}

#endif // SIGEL_VISUALISATION_SIG_RENDERER_H
