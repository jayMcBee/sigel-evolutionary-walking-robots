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
#ifndef SIGEL_ROBOT_SIG_JOINT_H
#define SIGEL_ROBOT_SIG_JOINT_H

namespace SIGEL_Robot { class SIG_Joint; }

#include <qstring.h>
#include <pointvector.h>
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Robot.h"

namespace SIGEL_Robot
{
  /**
   * SIG_Joint is an abstract class modelling a joint.
   *
   * The two adjacent links are named left and right. Despite
   * this naming scheme, both links are treated equally.
   */
        class SIG_Joint {
        public:
                /**
                 * Because typeid
                 * does not work with all compilers, this
                 * enum indicates the type of link the objects
                 * stands for.
                 */
                enum JointType { tTranslationalJoint,
                                 tRotationalJoint,
                                 tCylindricalJoint,
                                 tGlueJoint };
        protected:
                SIG_Robot *parent;
                QString name;
                int number;
                SIG_Link *leftLink, *rightLink;
		double mdh_a, mdh_alpha, mdh_d, mdh_theta;
		double mdh_screw_d, mdh_screw_theta, mechsMinPos, mechsMaxPos;
		int mdh_predecessor_is_left; // Only 0 or 1
  
        public:
                /**
                 * Default constructor. This will be used when
                 * the robot is parsed.
                 */
                SIG_Joint(SIG_Robot *par, QString n, int nr);
                /**
                 * Stream constructor. This will be used when the
                 * robot has been serialised for saving or transmitting.
                 *
                 * This is intended to be called by the constructor
                 * of a derived class, and therefore designed that way.
                 * Expect ugly things to happen otherwise.
                 */
                SIG_Joint (SIG_Robot *par, QTextStream & tx);
                /**
                 * Destructor.
                 */
                virtual ~SIG_Joint (void);
                /**
                 * Returns the name of the joint.
                 */
                QString getName (void) const;
                /**
                 * Returns the number of the joint.
                 */
                int getNumber (void) const;
                /**
                 * Returns the type of the joint the object
                 * represents.
                 */
                virtual JointType getJointType (void) const = 0;
                /**
                 * Sets the left_side link. Links on both side are
                 * treated equally. Left and right are only
                 * meant to identify them.
                 */
                void setLeftLink (SIG_Link *thelink);
                /**
                 * Sets the right-side link.
                 */
                void setRightLink (SIG_Link *thelink);
                /**
                 * Returns the left-side link.
                 */
                SIG_Link const *getLeftLink() const;
                /**
                 * Returns the right-side link.
                 */
                SIG_Link const *getRightLink() const;
                /**
                 * Returns the link on the opposite side of
                 * the joint.
                 */
                SIG_Link *otherSide (SIG_Link *myself) const;
                /**
                 * Translates and rotates the points associated to
                 * the given link.
                 */
                virtual void transformPoints (SIG_Link *side,
                                              DL_vector mov, DL_matrix rot) =0;
                /**
                 * Returns the initial geometric relation of the
                 * link on the opposite side. t will be filled with
                 * a translation vector, o with a rotation
                 * matrix.
                 */
                virtual void getGeomRelation
                        (DL_vector &t, DL_matrix &o, SIG_Link *origin) = 0;
		/**
		 * If this is a joint with exactly one Degree of Freedom,
		 * getMDH will return a set of modified Denavit-Hartenberg
		 * parameters.
 		 */
		virtual void getMDH (SIG_Link * & predecessor,
				     double & a, double & alpha,
				     double & d, double & theta,
				     double & screwD, double & screwTheta);

		double getMechsMinPos() const;

		double getMechsMaxPos() const;

                /**
                 * Writes the general joint data into a stream.
                 */
                virtual void writeToFileTransfer (QTextStream &tx);

		void transformToDynaMechs( SIG_Link *predecessor,
					   double screwD,
					   double screwTheta );

		bool continuable( SIG_Link *predecessor ) const;

		void calculateMDH( SIG_Link *caller );

		static void calculateCut( DL_vector a,
			           	  DL_vector u,
				   	  DL_vector v,
				   	  DL_vector b,
				   	  DL_vector w,
				  	  DL_vector &cut );

		static double normalizeRadAngle( double input );

        protected:
                /**
                 * some method to make my living easier.
                 */
                void tfap (DL_vector mov, DL_matrix rot, DL_vector *p);
        };
}

#endif // SIGEL_ROBOT_SIG_JOINT_H
