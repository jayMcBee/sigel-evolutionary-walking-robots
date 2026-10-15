/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef SIGEL_ENVIRONMENT_SIG_TERRAIN_H
#define SIGEL_ENVIRONMENT_SIG_TERRAIN_H

#include <QList>
#include <QString>

namespace SIGEL_Environment
{

  /**
   * The heights of a floor on a grid with a spacing of 1.
   */
  class SIG_Terrain
  {
  public:

    /** An empty grid of 0 by 0. */
    SIG_Terrain();

    /**
     * Makes the heights of function, which depends on x and z, on a grid of newDimensionX by newDimensionZ.
     * If the function cannot be parsed, a warning is printed, the terrain does not change and false is returned.
     */
    bool setFromFunction( const QString &function, int newDimensionX, int newDimensionZ );

    /**
     * Makes the heights of a PGM picture file in P2 format: a grey value of the maximum is a height of 5.
     * The picture's width is dimension X, its height is dimension Z.
     * If the file does not exist or is not in P2 format, a warning is printed, the terrain does not change and false is returned.
     */
    bool setFromPictureFile( const QString &pictureFile );

    /** Makes a grid of newDimensionX by newDimensionZ with height 0 at every point. */
    void setFlat( int newDimensionX, int newDimensionZ );

    int getDimensionX() const;

    int getDimensionZ() const;

    double getHeight( int x, int z ) const;

  private:

    int dimensionX;
    int dimensionZ;

    /** In the order they are made: z is the outer loop, x the inner one. */
    QList<double> heights;
  };

}

#endif // SIGEL_ENVIRONMENT_SIG_TERRAIN_H