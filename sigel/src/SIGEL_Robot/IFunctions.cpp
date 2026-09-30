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

	  return std::acos( cosInput );
	};

        SIG_Vector orthogonalVector( SIG_Vector input )
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

	  SIG_Vector result;
	  result.set( firstIndex, - input.get( secondIndex ) );
	  result.set( secondIndex, input.get( firstIndex ) );
	  result.set( thirdIndex, 0 );

	  result.normalize();

	  return result;
	};

        SIG_Matrix rotationMatrix(SIG_Vector v, double phi)
	{
	  double sinPhi = std::sin( phi / 2 );
	  double cosPhi = std::cos( phi / 2 );

	  Quaternion q;
	  q[0] = v.x * sinPhi;
	  q[1] = v.y * sinPhi;
	  q[2] = v.z * sinPhi;
	  q[3] = cosPhi;

	  normalizeQuat( q );

	  RotationMatrix result;

	  buildRotMat( q, result );

	  return SIG_TypeConverter::toSIG_Matrix( result );
	}

        NEWMAT::Matrix phatRockingUpStylinVectorBendingAngleSwingingMasterFunction( SIG_Vector _winportA,
										    SIG_Vector _winportB,
										    SIG_Vector _winportC,
										    SIG_Vector otherA,
										    SIG_Vector otherB,
										    SIG_Vector otherC )
	{
	  SIG_Vector firstTranslationVector = _winportA;
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
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "C:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "otherB:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "otherC:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;
#endif

	  SIG_Matrix firstRotation;

	  double const minimalAngleMeasure = 0.00001;

	  double const pi = std::atan( 1 ) * 4;

	  double bsAngle = tolerantACos( _winportB.inprod( &otherB ) );

	  if (bsAngle > minimalAngleMeasure)
	    {
	      if (std::abs( pi - bsAngle ) > minimalAngleMeasure)
		{
		  SIG_Vector bsNormal;

		  otherB.crossprod( &_winportB, &bsNormal );
		  bsNormal.normalize();

		  firstRotation = rotationMatrix( bsNormal, bsAngle );
		}
	      else
		{
		  SIG_Vector rotationAxis = orthogonalVector( _winportB );

		  firstRotation = rotationMatrix( rotationAxis, bsAngle );
		};
	    }
	  else
	    firstRotation.makeone();

	  SIG_Vector bufferVector = otherB;
	  firstRotation.times( &bufferVector, &otherB );
	  bufferVector = otherC;
	  firstRotation.times( &bufferVector, &otherC );

#ifdef SIG_DEBUG
	  SIGEL_Tools::SIG_IO::cerr << "otherB and otherC have been rotated." << Qt::endl;
	  SIGEL_Tools::SIG_IO::cerr << "B:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "C:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "otherB:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "otherC:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;
#endif

	  SIG_Matrix secondRotation;

	  SIG_Vector u = orthogonalVector( _winportB );

	  SIG_Vector v;
	  _winportB.crossprod( &u, &v );
	  v.normalize();

	  SIG_Vector cProjected;

	  SIG_Joint::calculateCut( SIG_Vector(0, 0, 0),
				   u,
				   v,
				   _winportC,
				   _winportB,
				   cProjected );
	  cProjected.normalize();

	  SIG_Vector otherCProjected;

	  SIG_Joint::calculateCut( SIG_Vector(0, 0, 0),
				   u,
				   v,
				   otherC,
				   _winportB,
				   otherCProjected );
	  otherCProjected.normalize();

	  double csAngle = tolerantACos( otherCProjected.inprod( &cProjected ) );

          if (csAngle > minimalAngleMeasure)
	    {
	      if (std::abs( pi - csAngle ) > minimalAngleMeasure)
		{
		  SIG_Vector csNormal;

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

	  SIGEL_Tools::SIG_IO::cerr << "otherB and otherC have been rotated." << Qt::endl;
	  SIGEL_Tools::SIG_IO::cerr << "B:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "C:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << _winportC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "otherB:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherB.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;

	  SIGEL_Tools::SIG_IO::cerr << "otherC:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << otherC.get( i );
	  SIGEL_Tools::SIG_IO::cerr << Qt::endl;
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

        void calculateAnyJoint (SIG_Vector _winportA, SIG_Vector _winportB, SIG_Vector _winportC,
                                SIG_Vector _winportD, SIG_Vector _winportE, SIG_Vector _winportF,
                                double winkel, double verschiebung,
                                SIG_Matrix & _winport_o, SIG_Vector & _winport_t,
                                QString someIdentifier)
        {
                // Make the points coverable.
                SIG_Vector r, zw;
                double h1;
                
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
		SIGEL_Tools::SIG_IO::cerr << "Computing the transformation of "
					  << someIdentifier
					  << Qt::endl;
#endif


		NEWMAT::Matrix transformation = phatRockingUpStylinVectorBendingAngleSwingingMasterFunction( _winportA,
													     _winportB,
													     _winportC,
													     _winportD,
													     _winportE,
													     _winportF );

		_winport_o = SIG_TypeConverter::toSIG_Matrix( transformation.SubMatrix( 1, 3, 1, 3 ) );
		_winport_t = SIG_TypeConverter::toSIG_Vector( transformation.SubMatrix( 1, 3, 4, 4 ) );

                SIG_Vector v (&_winportB);
                v.minusis (&_winportA);

                SIG_Vector schiebung (&v);
                schiebung.timesis (verschiebung);
                _winport_t.plusis (&schiebung);

                double phi = (winkel / 180.0) * M_PI;

                SIG_Matrix drehmatrix;

		drehmatrix=rotationMatrix(v,-phi);
                _winport_t.minusis (&_winportA);
                SIG_Vector stflorianhilf;
                drehmatrix.times (&_winport_t, &stflorianhilf);
                _winport_t.assign (&stflorianhilf);
                _winport_t.plusis (&_winportA);

                SIG_Matrix hilf;
                drehmatrix.times (&_winport_o, &hilf);
                _winport_o.assign (&hilf);
        }
} // namespace
