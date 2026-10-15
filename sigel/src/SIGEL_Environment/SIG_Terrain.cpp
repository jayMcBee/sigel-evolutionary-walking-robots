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
#include "SIGEL_Environment/SIG_Terrain.h"

#include "SIGEL_Tools/SIG_IO.h"
#include "fparser.h"

#include <QByteArray>

#include <fstream>
#include <string>

SIGEL_Environment::SIG_Terrain::SIG_Terrain()
  : dimensionX( 0 ),
    dimensionZ( 0 )
{
}

bool SIGEL_Environment::SIG_Terrain::setFromFunction( const QString &function, int newDimensionX, int newDimensionZ )
{
  FunctionParser fp;
  if ( fp.Parse( function.toUtf8().constData(), "xz" ) != -1 )
    {
      SIGEL_Tools::SIG_IO::cerr << "Warning: the specified terrain function could not be parsed. It must depend on x and z, and contain only valid expressions -- see the documentation in supportingLibs/fparser." << Qt::endl;
      return false;
    }

  QList<double> newHeights;
  double vals[2];

  for ( int z = 0; z < newDimensionZ; ++z )
    for ( int x = 0; x < newDimensionX; ++x )
      {
        vals[0] = x;
        vals[1] = z;
        newHeights.append( fp.Eval( vals ) );
      }

  dimensionX = newDimensionX;
  dimensionZ = newDimensionZ;
  heights = newHeights;
  return true;
}

bool SIGEL_Environment::SIG_Terrain::setFromPictureFile( const QString &pictureFile )
{
  std::ifstream pgm( pictureFile.toUtf8().constData() );
  if ( !pgm )
    {
      SIGEL_Tools::SIG_IO::cerr << "Warning: the specified terrain file does not exist." << Qt::endl;
      return false;
    }

  int newDimensionX = 0;
  int newDimensionZ = 0;
  QList<double> newHeights;
  std::string s;
  double max = 0;
  int counter = 0;

  while ( !pgm.eof() )
    {
      pgm >> s; // read file content till white space

      if ( s.find( "#", 0 ) != std::string::npos )
        {
          pgm.ignore( 255, '\n' );
          continue;
        }

      switch ( counter )
        {
        case 0: // Magic Key
          if ( s.compare( "P2" ) != 0 )
            {
              SIGEL_Tools::SIG_IO::cerr << "Warning: the specified picture file is not in PGM format." << Qt::endl;
              return false;
            }
          ++counter;
          break;
        case 1: // x-DIM
          newDimensionX = QString( s.c_str() ).toInt();
          ++counter;
          break;
        case 2: // y-DIM
          newDimensionZ = QString( s.c_str() ).toInt();
          ++counter;
          break;
        case 3: // max. grey-value
          max = QString( s.c_str() ).toDouble();
          ++counter;
          break;
        default:
          newHeights.append( QString( s.c_str() ).toDouble() / max * 5 );
        }
    }

  dimensionX = newDimensionX;
  dimensionZ = newDimensionZ;
  heights = newHeights;
  return true;
}

void SIGEL_Environment::SIG_Terrain::setFlat( int newDimensionX, int newDimensionZ )
{
  dimensionX = newDimensionX;
  dimensionZ = newDimensionZ;
  heights = QList<double>( newDimensionX * newDimensionZ, 0.0 );
}

int SIGEL_Environment::SIG_Terrain::getDimensionX() const
{
  return dimensionX;
}

int SIGEL_Environment::SIG_Terrain::getDimensionZ() const
{
  return dimensionZ;
}

double SIGEL_Environment::SIG_Terrain::getHeight( int x, int z ) const
{
  // dmEnvironment::loadTerrainData reads the heights with x as the outer loop, but they are made with z as the outer loop.
  return heights[ x * dimensionZ + z ];
}