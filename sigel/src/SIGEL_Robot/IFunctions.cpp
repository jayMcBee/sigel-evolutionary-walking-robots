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
#include "SIGEL_Robot/IFunctions.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

#include <dm.h>
#include <cmath>
#include <newmat.h>

using namespace SIGEL_Tools;

namespace SIGEL_Robot {

        double tolerantACos( double cosInput )
	{
	  if (cosInput > 1)
	    cosInput= 1;
	  else if (cosInput < -1)
	    cosInput = -1;

#ifdef _WINDOWS
	  return ::acos( cosInput );
#else
	  return std::acos( cosInput );
#endif	
	};

        DL_vector orthogonalVector( DL_vector input )
	{
	  int firstIndex;

	  for (int i=0; i<3; i++)
	    if (input.get( i )!=0)
	      {
		firstIndex = i;
		break;
	      };

	  int secondIndex = (firstIndex + 1) % 3;

	  int thirdIndex = (secondIndex + 1) % 3;

	  DL_vector result;
	  result.set( firstIndex, - input.get( secondIndex ) );
	  result.set( secondIndex, input.get( firstIndex ) );
	  result.set( thirdIndex, 0 );

	  result.normalize();

	  return result;
	};

        DL_matrix rotationMatrix(DL_vector v, DL_Scalar phi)
	{
#ifdef _WINDOWS	
	  double sinPhi = ::sin( phi / 2 );
	  double cosPhi = ::cos( phi / 2 );
#else
	  double sinPhi = std::sin( phi / 2 );
	  double cosPhi = std::cos( phi / 2 );
#endif

	  Quaternion q;
	  q[0] = v.x * sinPhi;
	  q[1] = v.y * sinPhi;
	  q[2] = v.z * sinPhi;
	  q[3] = cosPhi;

	  normalizeQuat( q );

	  RotationMatrix result;

	  buildRotMat( q, result );

	  return SIG_TypeConverter::toDL_matrix( result );
	}

        NEWMAT::Matrix phatRockingUpStylinVectorBendingAngleSwingingMasterFunction( DL_vector _winportA,
										    DL_vector _winportB,
										    DL_vector _winportC,
										    DL_vector otherA,
										    DL_vector otherB,
										    DL_vector otherC )
	{
	  DL_vector firstTranslationVector = _winportA;
	  firstTranslationVector.minusis( &otherA );

	  otherB.plusis( &firstTranslationVector );
	  otherC.plusis( &firstTranslationVector );

	  _winportB.minusis( &_winportA );
	  _winportC.minusis( &_winportA );
	  otherB.minusis( &_winportA );
	  otherC.minusis( &_winportA );

	  _winportB.normalize();
	  _winportC.normalize();
	  otherB.normalize();
	  otherC.normalize();

#ifdef SIG_DEBUG
	  SIGEL_Tools::SIG_IO::cerr << "B:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "C:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "otherB:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "otherC:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

	  DL_matrix firstRotation;

	  double const minimalAngleMeasure = 0.00001;

#ifdef _WINDOWS
	  double const pi = ::atan( 1 ) * 4;
#else
	  double const pi = std::atan( 1 ) * 4;
#endif	

	  double bsAngle = tolerantACos( _winportB.inprod( &otherB ) );

	  if (bsAngle > minimalAngleMeasure)
	    {
#ifdef _WINDOWS	
	      if (::abs( pi - bsAngle ) > minimalAngleMeasure)
#else
	      if (std::abs( pi - bsAngle ) > minimalAngleMeasure)
#endif	
		{
		  DL_vector bsNormal;

		  otherB.crossprod( &_winportB, &bsNormal );
		  bsNormal.normalize();

		  firstRotation = rotationMatrix( bsNormal, bsAngle );
		}
	      else
		{
		  DL_vector rotationAxis = orthogonalVector( _winportB );

		  firstRotation = rotationMatrix( rotationAxis, bsAngle );
		};
	    }
	  else
	    firstRotation.makeone();

	  DL_vector bufferVector = otherB;
	  firstRotation.times( &bufferVector, &otherB );
	  bufferVector = otherC;
	  firstRotation.times( &bufferVector, &otherC );

#ifdef SIG_DEBUG
	  SIGEL_Tools::SIG_IO::cerr << "otherB and otherC have been rotated.\n";
	  SIGEL_Tools::SIG_IO::cerr << "B:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "C:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "otherB:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "otherC:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

	  DL_matrix secondRotation;

	  DL_vector u = orthogonalVector( _winportB );

	  DL_vector v;
	  _winportB.crossprod( &u, &v );
	  v.normalize();

	  DL_vector cProjected;

	  SIG_Joint::calculateCut( DL_vector(0, 0, 0),
				   u,
				   v,
				   _winportC,
				   _winportB,
				   cProjected );
	  cProjected.normalize();

	  DL_vector otherCProjected;

	  SIG_Joint::calculateCut( DL_vector(0, 0, 0),
				   u,
				   v,
				   otherC,
				   _winportB,
				   otherCProjected );
	  otherCProjected.normalize();

	  double csAngle = tolerantACos( otherCProjected.inprod( &cProjected ) );

          if (csAngle > minimalAngleMeasure)
	    {
#ifdef _WINDOWS
	      if (::abs( pi - csAngle ) > minimalAngleMeasure)
#else
	      if (std::abs( pi - csAngle ) > minimalAngleMeasure)
#endif	
		{
		  DL_vector csNormal;

		  otherCProjected.crossprod( &cProjected, &csNormal );
		  csNormal.normalize();

		  if (_winportB.inprod( &csNormal ) < 0)
		    csAngle = 2 * pi - csAngle;
		};

	      secondRotation = rotationMatrix( _winportB, csAngle );
	    }
	  else
	    secondRotation.makeone();

#ifdef SIG_DEBUG
	  bufferVector = otherB;
	  secondRotation.times( &bufferVector, &otherB );
	  bufferVector = otherC;
	  secondRotation.times( &bufferVector, &otherC );

	  SIGEL_Tools::SIG_IO::cerr << "otherB and otherC have been rotated.\n";
	  SIGEL_Tools::SIG_IO::cerr << "B:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "C:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "otherB:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";

	  SIGEL_Tools::SIG_IO::cerr << "otherC:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

	  NEWMAT::Matrix transformation( 4, 4 );

	  transformation << 1 << 0 << 0 << 0
			 << 0 << 1 << 0 << 0
			 << 0 << 0 << 1 << 0
			 << 0 << 0 << 0 << 1;

	  NEWMAT::Matrix firstTransformation = transformation;
	  NEWMAT::Matrix secondTransformation = transformation;
	  NEWMAT::Matrix thirdTransformation = transformation;
	  NEWMAT::Matrix fourthTransformation = transformation;

	  firstTransformation.SubMatrix( 1, 3, 4, 4 ) = - SIG_TypeConverter::toColumnVector( otherA );

	  secondTransformation.SubMatrix( 1, 3, 1, 3 ) = SIG_TypeConverter::toMatrix( firstRotation );

	  thirdTransformation.SubMatrix( 1, 3, 1, 3 ) = SIG_TypeConverter::toMatrix( secondRotation );

	  fourthTransformation.SubMatrix( 1, 3, 4, 4 ) = SIG_TypeConverter::toColumnVector( _winportA );

	  transformation =   fourthTransformation
                           * thirdTransformation
                           * secondTransformation
                           * firstTransformation;

	  return transformation;
	};

        void calculateAnyJoint (DL_vector _winportA, DL_vector _winportB, DL_vector _winportC,
                                DL_vector _winportD, DL_vector _winportE, DL_vector _winportF,
                                double winkel, double verschiebung,
                                DL_matrix & _winport_o, DL_vector & _winport_t,
                                QString someIdentifier)
        {
                // Make the points coverable.
                DL_vector r, zw;
                DL_Scalar h1;
                
                r.assign (&_winportB);
                r.minusis (&_winportA);
                r.normalize ();
                _winportB.assign (&_winportA);
                _winportB.plusis (&r);

                zw.assign (&_winportC);
                zw.minusis (&_winportA);
                h1 = zw.inprod (&r);
                r.timesis (h1);
                zw.minusis (&r);
                zw.normalize ();
                _winportC.assign (&_winportA);
                _winportC.plusis (&zw);
                
                r.assign (&_winportE);
                r.minusis (&_winportD);
                r.normalize ();
                _winportE.assign (&_winportD);
                _winportE.plusis (&r);

                zw.assign (&_winportF);
                zw.minusis (&_winportD);
                h1 = zw.inprod (&r);
                r.timesis (h1);
                zw.minusis (&r);
                zw.normalize ();
                _winportF.assign (&_winportD);
                _winportF.plusis (&zw);
                // End of the covering construction

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Phat processing of "
					  << someIdentifier
					  << "\n";
#endif


		NEWMAT::Matrix transformation = phatRockingUpStylinVectorBendingAngleSwingingMasterFunction( _winportA,
													     _winportB,
													     _winportC,
													     _winportD,
													     _winportE,
													     _winportF );

		_winport_o = SIG_TypeConverter::toDL_matrix( transformation.SubMatrix( 1, 3, 1, 3 ) );
		_winport_t = SIG_TypeConverter::toDL_vector( transformation.SubMatrix( 1, 3, 4, 4 ) );

                DL_vector v (&_winportB);
                v.minusis (&_winportA);

                DL_vector schiebung (&v);
                schiebung.timesis (verschiebung);
                _winport_t.plusis (&schiebung);

                DL_Scalar phi = (winkel / 180.0) * M_PI;

                DL_matrix drehmatrix;

		drehmatrix=rotationMatrix(v,-phi);
                _winport_t.minusis (&_winportA);
                DL_vector stflorianhilf;
                drehmatrix.times (&_winport_t, &stflorianhilf);
                _winport_t.assign (&stflorianhilf);
                _winport_t.plusis (&_winportA);

                DL_matrix hilf;
                drehmatrix.times (&_winport_o, &hilf);
                _winport_o.assign (&hilf);
        }
} // namespace
