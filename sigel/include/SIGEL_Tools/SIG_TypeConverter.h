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
#ifndef SIGEL_TOOLS_SIG_TYPECONVERTER_H
#define SIGEL_TOOLS_SIG_TYPECONVERTER_H

#include <pointvector.h>
#include <matrix.h>
#include <newmat.h>
#include <dm.h>

namespace SIGEL_Tools
{

class SIG_TypeConverter
  {

  public:

    static DL_vector toDL_vector( NEWMAT::ColumnVector input ); /*tested*/

    static DL_vector toDL_vector( CartesianVector const input );

    static NEWMAT::ColumnVector toColumnVector( DL_vector input );

    static NEWMAT::ColumnVector toColumnVector( CartesianVector const input );

    static void toCartesianVector( DL_vector input, CartesianVector result );

    static void toCartesianVector( NEWMAT::ColumnVector input, CartesianVector result );

    static DL_matrix toDL_matrix( NEWMAT::Matrix input );

    static DL_matrix toDL_matrix( RotationMatrix const input );

    static NEWMAT::Matrix toMatrix( DL_matrix input );

    static NEWMAT::Matrix toMatrix( RotationMatrix const input );

    static void toRotationMatrix( NEWMAT::Matrix input, RotationMatrix result );

    static void toRotationMatrix( DL_matrix input, RotationMatrix result );

    static NEWMAT::Matrix sigelToDynaMechs();

    static NEWMAT::Matrix dynaMechsToSigel();

    static NEWMAT::Matrix sigelToPovray();
  };

};

#endif // SIGEL_TOOLS_SIG_TYPECONVERTER_H
