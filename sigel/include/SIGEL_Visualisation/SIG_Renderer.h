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
#include <qstring.h>
#include <qtextstream.h>

#include <newmat.h>

#ifdef _WINDOWS
#include <GL/glaux.h>
#else
#include <GL/gl.h>
#endif

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
     * The constructor of the SIG_Renderer.
     *
     * May used to initialize some stuff concerning the
     * OpenGL context. This could be the allocating of
     * display lists indices and the call of the
     * buildDisplayLists method.
     *
     * @param noOfObjects Becomes the value of the attribute
     *                    noOfObjects and initializes the
     *                    sizes of sceneObjects.
     * @param noOfFloatingTexts Intializes the size of
     *                          floatingTexts.
     */
    SIG_Renderer(int noOfObjects,
		 int noOfFloatingTexts);

    /**
     * The destructor of the SIG_Renderer.
     *
     * May used to clean up some OpenGL stuff.
     * For example the display lists could be cleared.
     */
    ~SIG_Renderer();
    // This class owns raw pointers in a QList and frees them in its
    // destructor, so a copy would free them twice. Qt 2's QGVector copy
    // cleared del_item and freed nothing.
    SIG_Renderer( const SIG_Renderer & ) = delete;
    SIG_Renderer &operator=( const SIG_Renderer & ) = delete;


    /**
     * This method finally starts the rendering
     * into the current OpenGL context.
     *
     * After the display lists are builded properly
     * they are called in this method to do
     * the rendering of the particular object(s).
     * This is mainly done by calling the
     * method renderSceneObjects. But if there is
     * more to show than just what is defined in the
     * display lists, this method is the place
     * to do it.
     *
     * @pre The display lists are builded.
     */
    virtual void render() = 0;

    virtual QString exportToPovray() = 0;

    virtual QString createPovrayDeclarations() = 0;

    static QString vectorToPovray( NEWMAT::ColumnVector input );

    /**
     * The array of SIG_VisualSceneObjects.
     *
     * It contains the actual properties of
     * the objects that are used to render
     * them. Its public so the object
     * containing the SIG_Renderer can
     * change the object properties with
     * the progress of a simulation for example.
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
     * This method builds the OpenGL display lists.
     *
     * Dependend from the kind of thing that should
     * be rendered a static (geometric) data structure
     * supplied in the subclass could be read.
     * OpenGL commands to draw this geometry are then
     * recorded in display lists.
     */
    virtual void buildDisplayLists() = 0;

    /**
     * Calls the display lists of the scene objects from
     * index first up to, but not including, index end.
     *
     * @pre The display lists are builded.
     */
    void renderSceneObjects( GLuint first, GLuint end );

    QString exportSceneObjectsToPovray();
  };

}

#endif // SIGEL_VISUALISATION_SIG_RENDERER_H
