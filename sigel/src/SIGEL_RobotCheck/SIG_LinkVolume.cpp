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
#include "SIGEL_RobotCheck/SIG_LinkVolume.h"

#include <algorithm>

SIGEL_RobotCheck::SIG_LinkVolume::SIG_LinkVolume()
  : lowCorner( 0, 0, 0 ),
    highCorner( 0, 0, 0 )
{
}

void SIGEL_RobotCheck::SIG_LinkVolume::addTriangle( SIG_Vector a, SIG_Vector b, SIG_Vector c )
{
  if ( triangles.isEmpty() )
    {
      lowCorner = a;
      highCorner = a;
    }

  widenBox( a );
  widenBox( b );
  widenBox( c );
  triangles.append( Triangle{ a, b, c } );
}

void SIGEL_RobotCheck::SIG_LinkVolume::widenBox( SIG_Vector corner )
{
  for ( int axis = 0; axis < 3; axis++ )
    {
      if ( corner.get( axis ) < lowCorner.get( axis ) )
        lowCorner.set( axis, corner.get( axis ) );
      if ( corner.get( axis ) > highCorner.get( axis ) )
        highCorner.set( axis, corner.get( axis ) );
    }
}

bool SIGEL_RobotCheck::SIG_LinkVolume::contains( const SIG_Vector &point ) const
{
  // A ray from a point inside a closed surface leaves through an odd number of triangles.
  // The ray's direction is slanted so that it does not run along the faces of a box.
  const double dx = 0.5377, dy = 0.2131, dz = 0.8157;

  int crossings = 0;
  for ( const Triangle &triangle : triangles )
    {
      double e1x = triangle.b.x - triangle.a.x, e1y = triangle.b.y - triangle.a.y, e1z = triangle.b.z - triangle.a.z;
      double e2x = triangle.c.x - triangle.a.x, e2y = triangle.c.y - triangle.a.y, e2z = triangle.c.z - triangle.a.z;

      double px = dy * e2z - dz * e2y, py = dz * e2x - dx * e2z, pz = dx * e2y - dy * e2x;
      double determinant = e1x * px + e1y * py + e1z * pz;
      if ( determinant == 0 )
        continue;

      double tx = point.x - triangle.a.x, ty = point.y - triangle.a.y, tz = point.z - triangle.a.z;
      double u = ( tx * px + ty * py + tz * pz ) / determinant;
      if ( u < 0 || u > 1 )
        continue;

      double qx = ty * e1z - tz * e1y, qy = tz * e1x - tx * e1z, qz = tx * e1y - ty * e1x;
      double v = ( dx * qx + dy * qy + dz * qz ) / determinant;
      if ( v < 0 || u + v > 1 )
        continue;

      double distance = ( e2x * qx + e2y * qy + e2z * qz ) / determinant;
      if ( distance > 0 )
        crossings++;
    }

  return crossings % 2 == 1;
}

double SIGEL_RobotCheck::SIG_LinkVolume::sharedWith( const SIG_LinkVolume &other, int samples, SIGEL_Tools::SIG_Randomizer &randomizer ) const
{
  // The box that both links' boxes share.
  SIG_Vector low( 0, 0, 0 );
  SIG_Vector high( 0, 0, 0 );
  double boxVolume = 1;
  for ( int axis = 0; axis < 3; axis++ )
    {
      SIG_Vector ownLow = lowCorner, ownHigh = highCorner, otherLow = other.lowCorner, otherHigh = other.highCorner;
      low.set( axis, std::max( ownLow.get( axis ), otherLow.get( axis ) ) );
      high.set( axis, std::min( ownHigh.get( axis ), otherHigh.get( axis ) ) );
      boxVolume *= std::max( 0.0, high.get( axis ) - low.get( axis ) );
    }
  if ( boxVolume == 0 )
    return 0;

  int insideBoth = 0;
  for ( int sample = 0; sample < samples; sample++ )
    {
      // SIG_Randomizer gives numbers below 32768.
      SIG_Vector point( low.x + ( high.x - low.x ) * randomizer.getRandomInt( 32768 ) / 32767.0,
                        low.y + ( high.y - low.y ) * randomizer.getRandomInt( 32768 ) / 32767.0,
                        low.z + ( high.z - low.z ) * randomizer.getRandomInt( 32768 ) / 32767.0 );
      if ( contains( point ) && other.contains( point ) )
        insideBoth++;
    }

  return boxVolume * insideBoth / samples;
}
