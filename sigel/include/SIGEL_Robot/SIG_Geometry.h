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
#ifndef SIGEL_ROBOT_SIG_GEOMETRY_H
#define SIGEL_ROBOT_SIG_GEOMETRY_H

namespace SIGEL_Robot { class SIG_Geometry; }

#include <QList>
#include <qstring.h>
#include "SIGEL_Tools/SIG_Vector.h"
#include "SIGEL_Tools/SIG_Matrix.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SIGEL_Robot/SIG_Body.h"

namespace SIGEL_Robot {

  /**
   * This is a representation of the surface of a link body.
   *
   * Currently, only polygonal surfaces are supported.
   *
   * In order to build a geometry, make a new SIG_Geometry object dynamically,
   * and then create multiple SIG_Polygon objects dynamically with your
   * new geometry object as the constructor argument. Then add vertices
   * to the polygon using addVertex.
   */
  class SIG_Geometry {
    friend class SIG_Polygon;
  private:
    // Neither list may hold a null: the SIG_DynaMechsLink constructor
    // dereferences every vertex, and SIG_Geometry(const SIG_Geometry *)
    // and writeToFileTransfer dereference every polygon.
    QList<SIG_Polygon *> polygons;
    QList<SIG_Vector *> vertices;

public:
    // Both lists own raw pointers that ~SIG_Geometry qDeleteAll's, so a
    // compiler-generated copy would shallow-copy them and free twice. Use
    // SIG_Geometry(const SIG_Geometry *) for a deep copy.
    SIG_Geometry (const SIG_Geometry &) = delete;
    SIG_Geometry &operator= (const SIG_Geometry &) = delete;

  protected:
    /**
     * Adds a polygon.
     *
     * This is used by SIG_Polygon to add itself to the geometry.
     */
    void addPolygon (SIG_Polygon *p);

  public:
    /**
     * Standard constructor.
     *
     * This makes the world entirely black.
     */
    SIG_Geometry () = default;
    /**
     * Unreal copy constructor.
     *
     * This constructor makes this geometry object a deep
     * copy of the argument. Remember: Never use SIG_Geometry
     * statically or via reference. Use pointers instead!
     */
    SIG_Geometry (const SIG_Geometry *geom);
    /**
     * Unstreaming constructor.
     *
     * Reads a geometry object and all its polygons from
     * a stream. All stream issues described elsewhere
     * apply also here.
     */
    SIG_Geometry (QTextStream & tx);
    /**
     * Destructor.
     */
    ~SIG_Geometry ();

    /**
     * getOrAddVertex searches for the vertex and hands out
     * its index. If the vertex cannot be found, it will be
     * added and the new index will be returned.
     * A vertex matches only if all three coordinates are
     * exactly equal.
     *
     * Since the array of vertices has to be searched,
     * this is a potentially expensive function. But once
     * an index is handed out, the vertex may not be given
     * another one. This would happen in a reorder operation.
     *
     * TODO: There has to be yet another level of indirection that
     * enables us to order the vertices in some manner without
     * having to change the indices.
     */
    int getOrAddVertex (SIG_Vector vertex);
    /**
     * Returns the vector of vertices.
     */
    QList<SIG_Vector *> const & getVertices () const;
    /**
     * Returns the number of vertices.
     */
    int getNumVertices () const;
    /**
     * Returns the ith vertex of the geometry.
     */
    SIG_Vector getVertex (int i) const;
    /**
     * Returns the list of polygons.
     */
    QList<SIG_Polygon *> const & getPolygons () const;
    /**
     * getNumPolygons reports the number of polygons in
     * the geometry object.
     */
    int getNumPolygons () const;
    /**
     * Move the geometry.
     *
     * By means of moving all vertices.
     */
    void translate (SIG_Vector dir);
    /**
     * Rotate the geometry.
     *
     * You have to provide a rotation matrix to do this.
     * You may also provide any other matrix. So you can
     * perform all transformation that can be done with a
     * matrix multiplication.
     */
    void rotate (SIG_Matrix mat);

    /**
     * Write the geometry to a stream.
     */
    void writeToFileTransfer (QTextStream & tx) const;
  };

}

#endif
