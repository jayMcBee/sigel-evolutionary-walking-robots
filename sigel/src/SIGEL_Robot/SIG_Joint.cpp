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
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"

#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Robot/IFunctions.h"

#include <cmath>
#include <numbers>
#include "SIGEL_Tools/SIG_Logging.h"

using namespace SIGEL_Tools;

namespace SIGEL_Robot {

	SIG_Joint::SIG_Joint (SIG_Robot *par, QString n, int nr)
	        : parent (par),
	          name (n),
	          number (nr),
	          leftLink (nullptr),
	          rightLink (nullptr),
	          mdh_a (0),
	          mdh_alpha (0),
	          mdh_d (0),
	          mdh_theta (0),
	          mdh_screw_d(0),
	          mdh_screw_theta(0),
	          mechsMinPos(0),
	          mechsMaxPos(0),
	          mdh_predecessor_is_left (-1)
	{ }

	SIG_Joint::SIG_Joint (SIG_Robot *par, QTextStream & tx)
	        : parent (par),
	          number (-1),
	          leftLink (nullptr),
	          rightLink (nullptr)
	{
		QString n1, n2;

		tx >> n1  // Read away the "Joint" keyword.
		   >> name
		   >> number;
		tx >> n1 >> n2; // read da names o' da links.
		SIG_Link *l1 = parent->lookupLink (n1);
		SIG_Link *l2 = parent->lookupLink (n2);
		if (!l1 || !l2)
			throw SIG_UnstreamingError (__FILE__, __LINE__, "joint '" + name + "' names unknown link '" + (l1 ? n2 : n1) + "'");
		setLeftLink (l1);
		setRightLink (l2);
		tx >> mdh_a >> mdh_alpha >> mdh_d >> mdh_theta
		   >> mdh_screw_d >> mdh_screw_theta
		   >> mechsMinPos >> mechsMaxPos
		   >> mdh_predecessor_is_left;
	}

	QString SIG_Joint::getName () const
	{
		return name;
	}

	int SIG_Joint::getNumber () const
	{
		return number;
	}

	void SIG_Joint::setLeftLink (SIG_Link *theLink)
	{
		leftLink = theLink;
		leftLink->addJoint (this);
	}

	void SIG_Joint::setRightLink (SIG_Link *theLink)
	{
		rightLink = theLink;
		rightLink->addJoint (this);
	}

	SIG_Link const *SIG_Joint::getLeftLink () const
	{
		return leftLink;
	}

	SIG_Link const *SIG_Joint::getRightLink () const
	{
		return rightLink;
	}

	SIG_Link *SIG_Joint::otherSide (SIG_Link *myself) const
	{
		return myself == leftLink ? rightLink : leftLink;
	}

	// getGeomRelation is ABSTRACT too!
	// Thank God nobody has asked for it yet.

	void SIG_Joint::getMDH (SIG_Link * & predecessor,
	                        double & a, double & alpha,
	                        double & d, double & theta,
	                        double & screwD, double & screwTheta)
	{
		throw SIG_Exception (__FILE__, __LINE__,
		        QString ("Joint ") + name +
		        " has no MDH parameters.");
	}

	void SIG_Joint::writeToFileTransfer (QTextStream & tx)
	{
		tx << "Joint "
		   << getName () << ' '
		   << getNumber () << ' '
		   << getLeftLink ()->getName () << ' '
		   << getRightLink ()->getName () << ' '
		   << mdh_a << ' ' << mdh_alpha << ' '
		   << mdh_d << ' ' << mdh_theta << ' '
		   << mdh_screw_d << ' ' << mdh_screw_theta << ' '
		   << mechsMinPos << ' ' << mechsMaxPos << ' '
		   << mdh_predecessor_is_left << '\n';
	}

	void SIG_Joint::tfap (SIG_Vector mov, SIG_Matrix rot, SIG_Vector *p)
	{
		p->plusis (&mov);
		SIG_Vector v (p);
		rot.times (&v, p);
	}

	double SIG_Joint::getMechsMinPos() const
	{
		return mechsMinPos;
	}

	double SIG_Joint::getMechsMaxPos() const
	{
		return mechsMaxPos;
	}

	void SIG_Joint::transformToDynaMechs( SIG_Link *predecessor,
	                                      double screwD,
	                                      double screwTheta )
	{
		mdh_screw_d = screwD;
		mdh_screw_theta = screwTheta;

		SIG_Link *successor;

		bool successorIsLeftLink;

		if (predecessor == leftLink)
		{
			successor = rightLink;
			mdh_predecessor_is_left = 1;
			successorIsLeftLink = false;
		}
		else
		{
			successor = leftLink;
			mdh_predecessor_is_left = 0;
			successorIsLeftLink = true;
		};

		SIG_Vector base;
		SIG_Vector dir;
		SIG_Vector hand;

		switch (getJointType())
		{
		case tTranslationalJoint:
		{
			SIG_TranslationalJoint *translationalJoint = static_cast< SIG_TranslationalJoint* >(this);

			base = ( successorIsLeftLink ) ? translationalJoint->getLeftBase() : translationalJoint->getRightBase();
			dir = ( successorIsLeftLink ) ? translationalJoint->getLeftDir() : translationalJoint->getRightDir();
			hand = ( successorIsLeftLink ) ? translationalJoint->getLeftFix() : translationalJoint->getRightFix();
		};
		break;
		case tRotationalJoint:
		{
			SIG_RotationalJoint *rotationalJoint = static_cast< SIG_RotationalJoint* >(this);

			base = ( successorIsLeftLink ) ? rotationalJoint->getLeftBase() : rotationalJoint->getRightBase();
			dir = ( successorIsLeftLink ) ? rotationalJoint->getLeftDir() : rotationalJoint->getRightDir();
			hand = ( successorIsLeftLink ) ? rotationalJoint->getLeftHand() : rotationalJoint->getRightHand();
		};
		break;
		};

		successor->transformToDynaMechs( this,
		                                 base,
		                                 dir,
		                                 hand );
	}

	bool SIG_Joint::continuable( SIG_Link *predecessor ) const
	{
		SIG_Link *successor = ( predecessor == leftLink ) ? rightLink : leftLink;

		return ( !((mdh_predecessor_is_left==0) || (mdh_predecessor_is_left==1)) && !successor->isMDHVisited());
	}

	void SIG_Joint::calculateMDH( SIG_Link *caller )
	{
		SIG_Link *predecessor;

		switch (mdh_predecessor_is_left)
		{
		case 0:
			predecessor = rightLink;
			break;
		case 1:
			predecessor = leftLink;
			break;
		default:
			predecessor = nullptr;
		};

		if (predecessor!=caller)
			return;

		SIG_Link *successor = ( leftLink == predecessor ) ? rightLink : leftLink;

		SIG_Vector predecessorsInitialPosition;
		SIG_Matrix predecessorsInitialRotation;

		predecessor->getInitialLocation( predecessorsInitialPosition,
		                                 predecessorsInitialRotation );

		SIG_Vector predOrigin = predecessorsInitialPosition;

		SIG_Vector successorsInitialPosition;
		SIG_Matrix successorsInitialRotation;

		successor->getInitialLocation( successorsInitialPosition,
		                               successorsInitialRotation );

		SIG_Vector succOrigin = successorsInitialPosition;

		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "Initial rotation of link " << successor->getName() << ".";

		SIG_Vector predXAxis = predecessorsInitialRotation.c0;
		SIG_Vector predZAxis = predecessorsInitialRotation.c2;

		SIG_Vector succXAxis = successorsInitialRotation.c0;
		SIG_Vector succYAxis = successorsInitialRotation.c1;
		SIG_Vector succZAxis = successorsInitialRotation.c2;

		SIG_Vector screwTranslationVector = predZAxis;
		screwTranslationVector.timesis( mdh_screw_d );

		predOrigin.plusis( &screwTranslationVector );

		SIG_Matrix screwRotationMatrix = rotationMatrix( predZAxis,
		                                                mdh_screw_theta );

		SIG_Vector helpPredXAxis = predXAxis;

		screwRotationMatrix.times( &helpPredXAxis, &predXAxis );

		// The predecessor's X-Axis and the successors Z-Axis must not
		// be parallel!

		SIG_Vector predXSuccZNormal;
		predXAxis.crossprod( &succZAxis, &predXSuccZNormal );
		predXSuccZNormal.normalize();

		SIG_Vector cut;

		calculateCut( succOrigin,
		              succZAxis,
		              predXSuccZNormal,
		              predOrigin,
		              predXAxis,
		              cut );

		SIG_Vector h = cut;
		h.minusis( &predOrigin );

		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "succOrigin: " << succOrigin.get( 0 ) << " " << succOrigin.get( 1 ) << " " << succOrigin.get( 2 );
		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "succZAxis: " << succZAxis.get( 0 ) << " " << succZAxis.get( 1 ) << " " << succZAxis.get( 2 );
		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "predXSuccZNormal: " << predXSuccZNormal.get( 0 ) << " " << predXSuccZNormal.get( 1 ) << " " << predXSuccZNormal.get( 2 );
		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "predOrigin: " << predOrigin.get( 0 ) << " " << predOrigin.get( 1 ) << " " << predOrigin.get( 2 );
		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "predXAxis: " << predXAxis.get( 0 ) << " " << predXAxis.get( 1 ) << " " << predXAxis.get( 2 );
		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "Cut: " << cut.get( 0 ) << " " << cut.get( 1 ) << " " << cut.get( 2 );

		mdh_a = h.norm();

		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "predZAxis: " << predZAxis.get( 0 ) << " " << predZAxis.get( 1 ) << " " << predZAxis.get( 2 );
		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "succZAxis: " << succZAxis.get( 0 ) << " " << succZAxis.get( 1 ) << " " << succZAxis.get( 2 );

		mdh_alpha = tolerantACos( predZAxis.inprod( &succZAxis ) );

		double const maximalParallelityMeasure = 0.00001;

		SIG_Vector rotationDir = h;
		rotationDir.normalize();

		SIG_Vector zzNormalVector;
		predZAxis.crossprod( &succZAxis, &zzNormalVector );

		if (zzNormalVector.norm() > maximalParallelityMeasure)
		{
			zzNormalVector.normalize();

			if (rotationDir.inprod( &zzNormalVector ) < 0)
				mdh_alpha = 2 * std::numbers::pi - mdh_alpha;
		};

		SIG_Vector distanceVector = succOrigin;

		distanceVector.minusis( &cut );

		mdh_d = distanceVector.norm();

		double const minimalDistanceMeasure = 0.00001;

		if (mdh_d > minimalDistanceMeasure)
		{
			distanceVector.normalize();

			if (distanceVector.inprod( &succZAxis ) < 0)
				mdh_d *= -1;
		};

		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "Predecessor's X-Axis: " << predXAxis.get( 0 ) << " " << predXAxis.get( 1 ) << " " << predXAxis.get( 2 );
		SIG_LOG_DEBUG( "SIGEL_Robot" ) << "Successor's X-Axis: " << succXAxis.get( 0 ) << " " << succXAxis.get( 1 ) << " " << succXAxis.get( 2 );

		mdh_theta = tolerantACos( predXAxis.inprod( &succXAxis ) );

		rotationDir = succZAxis;

		SIG_Vector xxNormalVector;
		predXAxis.crossprod( &succXAxis, &xxNormalVector );

		if (xxNormalVector.norm() > maximalParallelityMeasure)
		{
			xxNormalVector.normalize();

			if (rotationDir.inprod( &xxNormalVector) < 0)
				mdh_theta = 2 * std::numbers::pi - mdh_theta;
		};

		switch (getJointType())
		{
		case tTranslationalJoint:
		{
			SIG_TranslationalJoint *translationalJoint = static_cast< SIG_TranslationalJoint* >(this);

			if (translationalJoint->getMax()==translationalJoint->getMin())
			{
				mechsMinPos = mechsMaxPos = 0;
			}
			else
			{
				SIG_Vector localPredBase;
				SIG_Vector localSuccBase;

				double sigelMin;
				double sigelMax;

				if (mdh_predecessor_is_left==1)
				{
					localPredBase = translationalJoint->getLeftBase();
					localSuccBase = translationalJoint->getRightBase();

					sigelMin = translationalJoint->getMin();
					sigelMax = translationalJoint->getMax();
				}
				else
				{
					localPredBase = translationalJoint->getRightBase();
					localSuccBase = translationalJoint->getLeftBase();

					sigelMin = - translationalJoint->getMax();
					sigelMax = - translationalJoint->getMin();
				};

				SIG_Vector predBase;
				SIG_Vector succBase;

				predecessorsInitialRotation.times( &localPredBase, &predBase );
				predBase.plusis( &predecessorsInitialPosition );

				successorsInitialRotation.times( &localSuccBase, &succBase );
				succBase.plusis( &successorsInitialPosition );

				SIG_Vector predBaseOffset = cut;
				predBaseOffset.minusis( &predBase );

				SIG_Vector succBaseOffset = succOrigin;
				succBaseOffset.minusis( &succBase );

				double b = predBaseOffset.norm();

				if (b > minimalDistanceMeasure)
				{
					predBaseOffset.normalize();

					if (succZAxis.inprod( &predBaseOffset ) < 0)
						b *= -1;
				};

				double c = succBaseOffset.norm();

				if (c > minimalDistanceMeasure)
				{
					succBaseOffset.normalize();

					if (succZAxis.inprod( &succBaseOffset ) < 0)
						c *= -1;
				};

				mechsMinPos = sigelMin - b + c;
				mechsMaxPos = sigelMax - b + c;
			};
		};
		break;
		case tRotationalJoint:
		{
			SIG_RotationalJoint *rotationalJoint = static_cast< SIG_RotationalJoint* >(this);

			if (rotationalJoint->getMax()==rotationalJoint->getMin())
			{
				mechsMinPos = mechsMaxPos = 0;
			}
			else
			{
				SIG_Vector localPredHand;
				SIG_Vector localSuccHand;

				double sigelMax;
				double sigelMin;

				if (mdh_predecessor_is_left==1)
				{
					localPredHand = rotationalJoint->getLeftHand();
					localSuccHand = rotationalJoint->getRightHand();

					sigelMin = rotationalJoint->getMin();
					sigelMax = rotationalJoint->getMax();
				}
				else
				{
					localPredHand = rotationalJoint->getRightHand();
					localSuccHand = rotationalJoint->getLeftHand();

					sigelMin = - rotationalJoint->getMax();
					sigelMax = - rotationalJoint->getMin();
				};

				SIG_Vector predHand;
				SIG_Vector succHand;

				predecessorsInitialRotation.times( &localPredHand, &predHand );
				predHand.plusis( &predecessorsInitialPosition );

				successorsInitialRotation.times( &localSuccHand, &succHand );
				succHand.plusis( &successorsInitialPosition );

				SIG_Vector predHandProjected;

				calculateCut( succOrigin,
				              succXAxis,
				              succYAxis,
				              predHand,
				              succZAxis,
				              predHandProjected );

				SIG_Vector predHandProjectedDir = predHandProjected;
				predHandProjectedDir.minusis( &succOrigin );
				predHandProjectedDir.normalize();

				SIG_Vector succHandProjected;

				calculateCut( succOrigin,
				              succXAxis,
				              succYAxis,
				              succHand,
				              succZAxis,
				              succHandProjected );

				SIG_Vector succHandProjectedDir = succHandProjected;
				succHandProjectedDir.minusis( &succOrigin );
				succHandProjectedDir.normalize();

				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "predHand: " << predHand.get( 0 ) << " " << predHand.get( 1 ) << " " << predHand.get( 2 );
				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "predHandProjected: " << predHandProjected.get( 0 ) << " " << predHandProjected.get( 1 ) << " " << predHandProjected.get( 2 );
				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "predHandProjectedDir: " << predHandProjectedDir.get( 0 ) << " " << predHandProjectedDir.get( 1 ) << " " << predHandProjectedDir.get( 2 );
				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "Predecessors X-Axis: " << predXAxis.get( 0 ) << " " << predXAxis.get( 1 ) << " " << predXAxis.get( 2 );
				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "succHand: " << succHand.get( 0 ) << " " << succHand.get( 1 ) << " " << succHand.get( 2 );
				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "succHandProjected: " << succHandProjected.get( 0 ) << " " << succHandProjected.get( 1 ) << " " << succHandProjected.get( 2 );
				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "succHandProjectedDir: " << succHandProjectedDir.get( 0 ) << " " << succHandProjectedDir.get( 1 ) << " " << succHandProjectedDir.get( 2 );
				SIG_LOG_DEBUG( "SIGEL_Robot" ) << "Successor's X-Axis: " << succXAxis.get( 0 ) << " " << succXAxis.get( 1 ) << " " << succXAxis.get( 2 );

				double beta = tolerantACos( predHandProjectedDir.inprod( &predXAxis ) );

				rotationDir = succZAxis;

				SIG_Vector predHandProjectedDirPredXAxisNormalVector;
				predHandProjectedDir.crossprod( &predXAxis, &predHandProjectedDirPredXAxisNormalVector );

				if (predHandProjectedDirPredXAxisNormalVector.norm() > maximalParallelityMeasure)
				{
					predHandProjectedDirPredXAxisNormalVector.normalize();

					if (rotationDir.inprod( &predHandProjectedDirPredXAxisNormalVector ) < 0)
						beta = (2 * std::numbers::pi) - beta;
				};

				double gamma = tolerantACos( succHandProjectedDir.inprod( &succXAxis ) );

				SIG_Vector succHandProjectedDirSuccXAxisNormalVector;
				succHandProjectedDir.crossprod( &succXAxis, &succHandProjectedDirSuccXAxisNormalVector );

				if (succHandProjectedDirSuccXAxisNormalVector.norm() > maximalParallelityMeasure)
				{
					succHandProjectedDirSuccXAxisNormalVector.normalize();

					if (rotationDir.inprod( &succHandProjectedDirSuccXAxisNormalVector ) < 0)
						gamma = (2 * std::numbers::pi) - gamma;
				};

				if (rotationalJoint->getMax()==rotationalJoint->getMin())
				{
					mechsMinPos = mechsMaxPos = 0;
				}
				else
				{
					if (sigelMax < 0)
						sigelMax += 360;
					else if (sigelMax >= 360)
						sigelMax -= 360;

					if (sigelMin < 0)
						sigelMin += 360;
					else if (sigelMin >= 360)
						sigelMin -= 360;

					SIG_LOG_DEBUG( "SIGEL_Robot" ) << "sigelMax: " << sigelMax << ", sigelMin: " << sigelMin;

					mechsMinPos = (2 * std::numbers::pi) - ( (sigelMax / 360) * 2 * std::numbers::pi );
					mechsMaxPos = (2 * std::numbers::pi) - ( (sigelMin / 360) * 2 * std::numbers::pi );

					SIG_LOG_DEBUG( "SIGEL_Robot" ) << "mechsMinPos: " << mechsMinPos << ", mechsMaxPos: " << mechsMaxPos << ", beta: " << beta;

					mechsMinPos -= beta;
					mechsMaxPos -= beta;

					SIG_LOG_DEBUG( "SIGEL_Robot" ) << "New mechsMinPos: " << mechsMinPos << ", new mechsMaxPos: " << mechsMaxPos;

					mechsMinPos = normalizeRadAngle( mechsMinPos );
					mechsMaxPos = normalizeRadAngle( mechsMaxPos );

					SIG_LOG_DEBUG( "SIGEL_Robot" ) << "New mechsMinPos: " << mechsMinPos << ", new mechsMaxPos: " << mechsMaxPos << ", gamma: " << gamma;

					mechsMinPos += gamma;
					mechsMaxPos += gamma;

					SIG_LOG_DEBUG( "SIGEL_Robot" ) << "New mechsMinPos: " << mechsMinPos << ", new mechsMaxPos: " << mechsMaxPos;

					mechsMinPos = normalizeRadAngle( mechsMinPos );
					mechsMaxPos = normalizeRadAngle( mechsMaxPos );

					SIG_LOG_DEBUG( "SIGEL_Robot" ) << "New mechsMinPos: " << mechsMinPos << ", new mechsMaxPos: " << mechsMaxPos;

					double const minimalAngleDistance = 0.00001;

					if ( std::abs( mdh_theta - mechsMinPos ) <= minimalAngleDistance ){
						mdh_theta = mechsMinPos; }
					else if ( std::abs( mdh_theta - mechsMaxPos ) <= minimalAngleDistance ){
						mdh_theta = mechsMaxPos;}

					if ( mechsMaxPos < mechsMinPos ) {
						if ( mdh_theta <= mechsMaxPos )
							mechsMinPos -= 2 * std::numbers::pi;
						else
							mechsMaxPos += 2 * std::numbers::pi;
					}

					SIG_LOG_DEBUG( "SIGEL_Robot" ) << "New mechsMinPos: " << mechsMinPos << ", new mechsMaxPos: " << mechsMaxPos;
				};
			};
		};
		break;
		};

		const QList< SIG_Joint * > joints = successor->getJoints();

		for (SIG_Joint *currentJoint : joints)
			currentJoint->calculateMDH( successor );
	}

	void SIG_Joint::calculateCut( SIG_Vector a,
	                              SIG_Vector u,
	                              SIG_Vector v,
	                              SIG_Vector b,
	                              SIG_Vector w,
	                              SIG_Vector &cut )
	{
		NEWMAT::Matrix linEqSystem( 3, 3 );

		linEqSystem.Column( 1 ) = SIG_TypeConverter::toColumnVector( u );
		linEqSystem.Column( 2 ) = SIG_TypeConverter::toColumnVector( v );
		linEqSystem.Column( 3 ) = SIG_TypeConverter::toColumnVector( w ) * -1;

		NEWMAT::ColumnVector rightSide =   SIG_TypeConverter::toColumnVector( b )
		                                 - SIG_TypeConverter::toColumnVector( a );

		NEWMAT::ColumnVector solution = linEqSystem.i() * rightSide;

		cut = w;
		cut.timesis( solution( 3 ) );
		cut.plusis( &b );
	}

	double SIG_Joint::normalizeRadAngle( double input )
	{
		if (input < 0)
			return input + (2 * std::numbers::pi);
		else if (input >= (2*std::numbers::pi))
			return input - (2 * std::numbers::pi);

		return input;
	}

}
