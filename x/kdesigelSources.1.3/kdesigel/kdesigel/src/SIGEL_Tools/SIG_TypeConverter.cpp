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
#include "SIGEL_Tools/SIG_TypeConverter.h"

namespace SIGEL_Tools
{

  DL_vector SIG_TypeConverter::toDL_vector( NEWMAT::ColumnVector input )
  {
    DL_vector result;

    for (int i=0; i<3; i++)
      result.set( i, input( i+1 ) );

    return result;
  };

  DL_vector SIG_TypeConverter::toDL_vector( CartesianVector const input )
  {
    DL_vector result;

    for (int i=0; i<3; i++)
      result.set( i, input[i] );

    return result;
  };

  NEWMAT::ColumnVector SIG_TypeConverter::toColumnVector( DL_vector input )
  {
    NEWMAT::ColumnVector result( 3 );

    for (int i=0; i<3; i++)
      result( i+1 ) = input.get( i );

    return result;
  };

  NEWMAT::ColumnVector SIG_TypeConverter::toColumnVector( CartesianVector const input )
  {
    NEWMAT::ColumnVector result( 3 );

    for (int i=0; i<3; i++)
      result( i+1 ) = input[i];

    return result;
  };

  void SIG_TypeConverter::toCartesianVector( DL_vector input, CartesianVector result )
  {
    for (int i=0; i<3; i++)
      result[i] = input.get( i );
  };

  void SIG_TypeConverter::toCartesianVector( NEWMAT::ColumnVector input, CartesianVector result )
  {
    for (int i=0; i<3; i++)
      result[i] = input( i+1 );
  };

  DL_matrix SIG_TypeConverter::toDL_matrix( NEWMAT::Matrix input )
  {
    DL_matrix result;

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	result.set( i, j, input( i+1, j+1 ) );

    return result;
  };

  DL_matrix SIG_TypeConverter::toDL_matrix( RotationMatrix const input )
  {
    DL_matrix result;

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	result.set( i, j, input[j][i] );

    return result;
  };

  NEWMAT::Matrix SIG_TypeConverter::toMatrix( DL_matrix input )
  {
    NEWMAT::Matrix result( 3, 3 );

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	result( i+1, j+1 ) = input.get( i, j );

    return result;
  };

  NEWMAT::Matrix SIG_TypeConverter::toMatrix( RotationMatrix const input )
  {
    NEWMAT::Matrix result( 3, 3 );

    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	result( i+1, j+1 ) = input[j][i];

    return result;
  };

  void SIG_TypeConverter::toRotationMatrix( NEWMAT::Matrix input, RotationMatrix result )
  {
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	result[j][i] = input( i+1, j+1 );
  };

  void SIG_TypeConverter::toRotationMatrix( DL_matrix input, RotationMatrix result )
  {
    for (int i=0; i<3; i++)
      for (int j=0; j<3; j++)
	result[j][i] = input.get( i, j );
  };

  NEWMAT::Matrix SIG_TypeConverter::sigelToDynaMechs()
  {
    NEWMAT::Matrix transformation( 3, 3 );

    transformation << 1 << 0 << 0
		   << 0 << 0 << -1
		   << 0 << 1 << 0;

    return transformation;
  };

  NEWMAT::Matrix SIG_TypeConverter::dynaMechsToSigel()
  {
    NEWMAT::Matrix transformation( 3, 3 );

    transformation << 1 << 0 << 0
		   << 0 << 0 << 1
		   << 0 << -1 << 0;

    return transformation; 
  };

  NEWMAT::Matrix SIG_TypeConverter::sigelToPovray()
  {
    NEWMAT::Matrix transformation( 3, 3 );

    transformation << 1 << 0 << 0
		   << 0 << 1 << 0
		   << 0 << 0 << -1;

    return transformation; 
  };
}
