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
#ifndef SIGEL_ROBOT_SIG_GEOMETRYITERATOR_H
#define SIGEL_ROBOT_SIG_GEOMETRYITERATOR_H

namespace SIGEL_Robot { class SIG_GeometryIterator; }

#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_Polygon.h"

namespace SIGEL_Robot {
  /**
   * A geometry iterator can be used to easily iterate over
   * all polygons in a surface description.
   *
   * Simply create the object saying which geometry you want to
   * iterate over, then you can say "iterate()" while "valid()" returns
   * true. Each time you say "iterate()" you will get another polygon.
   *
   * You will find an example
   * of how to use the iterator in the header file.
   */
  class SIG_GeometryIterator {
  private:
    SIG_Geometry const *iterating;
    int iteration;

  public:
    /**
     * Constructor.
     *
     * @param over This is the geometry object that contains the
     *             polygons the iterator will return.
     */
    SIG_GeometryIterator (SIG_Geometry const *over);
    /**
     * Destructor.
     */
    ~SIG_GeometryIterator (void);

    /**
     * Is there another polygon?
     */
    bool valid (void) const;
    /**
     * Convenient version of the valid() function.
     */
    operator bool (void) const { return valid (); }
    /**
     * Returns the current polygon.
     */
    SIG_Polygon const & current (void) const;
    /**
     * This will make the iterator move over to the next polygon.
     */
    void next (void);
    /**
     * Returns the current polygon and moves over to the next.
     */
    inline SIG_Polygon const & iterate (void)
      { SIG_Polygon const & a = current (); next (); return a; }
  };
}

/* 
How to use the geometry iterator:

Say you have got a geometry g for which you want to know
the data of all polygons.

SIG_Geometry *g = ......;

SIG_GeometryIterator git (g);
while (git) {
    SIG_Polygon const & poly = git.iterate ();
    cout << "The polygon has "
         << poly.getNumVertices ()
         << " vertices.\n";
    cout << "They are:\n";
    for (int i = 0; i < poly.getNumVertices (); i++) {
        cout << "With index "
             << poly.getVertexIndex (i)
             << ": ";
        DL_vector v = poly.getVertex (i);
        cout << v.get (0) << ","
             << v.get (1) << ","
             << v.get (2) << "\n";
    }
}
*/

#endif
