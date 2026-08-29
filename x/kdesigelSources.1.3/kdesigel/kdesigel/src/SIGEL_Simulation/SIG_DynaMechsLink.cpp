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
#include "compat/q2compat.h"
#include "SIGEL_Simulation/SIG_DynaMechsLink.h"

#include "SIGEL_Tools/SIG_TypeConverter.h"
#include "SIGEL_Tools/SIG_IO.h"

#include <dm.h>
#include <dmContactModel.hpp>
#include <dmMDHLink.hpp>

#include <cmath>

using namespace SIGEL_Tools;

namespace SIGEL_Simulation
{

  SIG_DynaMechsLink::SIG_DynaMechsLink( int dynaMechsLinkNumber,
					SIGEL_Robot::SIG_Link const *link,
					dmRigidBody *dynaMechsLink,
					double screwD,
					double screwTheta )
    : dynaMechsLinkNumber( dynaMechsLinkNumber ),
      link( link ),
      dynaMechsLink( dynaMechsLink ),
      transformation( 4, 4 ),
      screwD( screwD ),
      screwTheta( screwTheta ),
      screwLink( 0 )
  {
    for (int j=1; j<=3; j++)
      transformation( 4, j ) = 0;

    transformation( 4, 4 ) = 1;

    if ((screwD != 0 ) || (screwTheta != 0))
      screwLink = new dmZScrewTxLink( screwD, screwTheta );

    SIGEL_Robot::SIG_Geometry *geometry = const_cast< SIGEL_Robot::SIG_Geometry* >(link->getGeometry());

    SIGEL_Robot::SIG_Material const *material = link->getMaterial();

    double density = material->getDensity();

    SIGEL_Robot::SIG_Mirtich inertiaCalculator( geometry, link->getName() );

    DL_Scalar mass;
    DL_vector centerOfMass;
    DL_matrix inertiaTensor;

    inertiaCalculator.computePhysics( density,
				      mass,
				      centerOfMass,
				      inertiaTensor );

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "Density: " << density << "\n";
    SIGEL_Tools::SIG_IO::cerr << "Mass: " << mass << "\n";
    SIGEL_Tools::SIG_IO::cerr << "Center of mass:";
    for (int i=0; i<3; i++)
      SIGEL_Tools::SIG_IO::cerr << " " << centerOfMass.get( i );
    SIGEL_Tools::SIG_IO::cerr << "\n";
    SIGEL_Tools::SIG_IO::cerr << "Inertia tensor:\n";
    for (int i=0; i<3; i++)
      {
	for (int j=0; j<3; j++)
	  SIGEL_Tools::SIG_IO::cerr << inertiaTensor.get( i, j ) << " ";
	    SIGEL_Tools::SIG_IO::cerr << "\n";
      };
#endif

    CartesianTensor dynaMechsInertiaTensor;
    CartesianVector dynaMechsCenterOfMass;

    SIG_TypeConverter::toRotationMatrix( inertiaTensor, dynaMechsInertiaTensor );
    SIG_TypeConverter::toCartesianVector( centerOfMass, dynaMechsCenterOfMass );

    dynaMechsLink->setInertiaParameters( mass,
					 dynaMechsInertiaTensor,
					 dynaMechsCenterOfMass );

    dmContactModel *contactModel = new dmContactModel();

    // A const reference, not a copy: this took a whole Q2PtrVector by value
    // on every link construction for no reason.
    const QList< DL_vector * > &vertices = geometry->getVertices();

    int noOfContactPoints = vertices.size();

#ifdef _WINDOWS
	CartesianVector *contactPoints = new CartesianVector[ noOfContactPoints ];
#else
    CartesianVector contactPoints[ noOfContactPoints ];
#endif

    for (int i=0; i<vertices.size(); i++)
      {
	DL_vector &vertex = *(vertices[i]);

	SIG_TypeConverter::toCartesianVector( vertex, contactPoints[i] );
      };

    contactModel->setContactPoints( noOfContactPoints,
				    contactPoints );

    dynaMechsLink->addForce( contactModel );

#ifdef _WINDOWS
	if( contactPoints ) delete[] contactPoints;
#endif	
  };

  void SIG_DynaMechsLink::forwardKinematics( SIG_DynaMechsLink *caller )
  {
    if (!caller)
      {
	QList< double > q( 7 );

	QList< double > qd( 6 );

	dynaMechsLink->getState( q.data(), qd.data() );

	transformation.SubMatrix( 1, 3, 4, 4 ) = SIG_TypeConverter::toColumnVector( q.data() + 4 );

	normalizeQuat( q.data() );

	RotationMatrix _winport_R;

	buildRotMat( q.data(), _winport_R );

	transformation.SubMatrix( 1, 3, 1, 3 ) = SIG_TypeConverter::toMatrix( _winport_R );
      }
    else
      {
	dmMDHLink *dynaMechsMDHLink = static_cast< dmMDHLink* >(dynaMechsLink);

	double a;
	double alpha;
	double d;
	double theta;

	dynaMechsMDHLink->getMDHParameters( &a,
					    &alpha,
					    &d,
					    &theta );

	transformation = caller->transformation;

	if (screwLink)
	  transformation *=   buildTranslationMatrix( 0, 0, screwD )
                            * buildZRotationMatrix( screwTheta );

	transformation *=   buildXRotationMatrix( alpha )
	                  * buildTranslationMatrix( a, 0, d )
	                  * buildZRotationMatrix( theta );

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "theta: "
				  << theta
				  << ", d: "
				  << d
				  << "\n";

	SIGEL_Tools::SIG_IO::cerr << "New Transformation:\n";
	for (int i=1; i<=4; i++)
	  {
	    for (int j=1; j<=4; j++)
	      SIGEL_Tools::SIG_IO::cerr << transformation( i, j ) << " ";
	    SIGEL_Tools::SIG_IO::cerr << "\n";
	  };
#endif

      };

    for (SIG_DynaMechsLink *actSuccessor : successors)
      actSuccessor->forwardKinematics( this );
  };

  NEWMAT::Matrix SIG_DynaMechsLink::buildXRotationMatrix( double angle )
  {
    NEWMAT::Matrix transformation(4, 4);

#ifdef _WINDOWS
    double sinAngle = ::sin( angle );
    double cosAngle = ::cos( angle );
#else
    double sinAngle = std::sin( angle );
    double cosAngle = std::cos( angle );
#endif

    transformation << 1 << 0 << 0 << 0
		   << 0 << cosAngle << - sinAngle << 0
		   << 0 << sinAngle << cosAngle << 0
		   << 0 << 0 << 0 << 1;

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "X-Angle: " << angle << "\n"
			      << "Matrix:\n";
    for (int i=1; i<=4; i++)
      {
	for (int j=1; j<=4; j++)
	  SIGEL_Tools::SIG_IO::cerr << transformation( i, j ) << " ";
	SIGEL_Tools::SIG_IO::cerr << "\n";
      };
#endif

    return transformation;
  };

  NEWMAT::Matrix SIG_DynaMechsLink::buildZRotationMatrix( double angle )
  {
    NEWMAT::Matrix transformation(4, 4);

#ifdef _WINDOWS
    double sinAngle = ::sin( angle );
    double cosAngle = ::cos( angle );
#else
    double sinAngle = std::sin( angle );
    double cosAngle = std::cos( angle );
#endif

    transformation << cosAngle << - sinAngle << 0 << 0
		   << sinAngle << cosAngle << 0 << 0
		   << 0 << 0 << 1 << 0
		   << 0 << 0 << 0 << 1;

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "Z-Angle: " << angle << "\n"
			      << "Matrix:\n";
    for (int i=1; i<=4; i++)
      {
	for (int j=1; j<=4; j++)
	  SIGEL_Tools::SIG_IO::cerr << transformation( i, j ) << " ";
	SIGEL_Tools::SIG_IO::cerr << "\n";
      };
#endif

    return transformation;
  };

  NEWMAT::Matrix SIG_DynaMechsLink::buildTranslationMatrix( double x,
							    double y,
							    double z )
  {
    NEWMAT::Matrix transformation(4, 4);

    transformation << 1 << 0 << 0 << x
		   << 0 << 1 << 0 << y
		   << 0 << 0 << 1 << z
		   << 0 << 0 << 0 << 1;

#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "Move: " << x << " " << y << " " << z << "\n"
			      << "Matrix:\n";
    for (int i=1; i<=4; i++)
      {
	for (int j=1; j<=4; j++)
	  SIGEL_Tools::SIG_IO::cerr << transformation( i, j ) << " ";
	SIGEL_Tools::SIG_IO::cerr << "\n";
      };
#endif

    return transformation;
  };
}
