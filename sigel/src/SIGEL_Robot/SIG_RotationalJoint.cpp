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
#include "SIGEL_Robot/SIG_RotationalJoint.h"

#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/IFunctions.h"

namespace SIGEL_Robot {
        SIG_RotationalJoint::SIG_RotationalJoint (SIG_Robot *par, QString n, int nr)
                : SIG_Joint (par, n, nr)
        { }

        SIG_RotationalJoint::SIG_RotationalJoint (SIG_Robot *par, QTextStream & tx)
                : SIG_Joint (par, tx)
        {
                leftBase = SIG_Robot::streamToVector (tx);
                leftDir = SIG_Robot::streamToVector (tx);
                leftHand = SIG_Robot::streamToVector (tx);
                rightBase = SIG_Robot::streamToVector (tx);
                rightDir = SIG_Robot::streamToVector (tx);
                rightHand = SIG_Robot::streamToVector (tx);
                tx >> minimum >> maximum >> initial;
        }
        
        SIG_RotationalJoint::~SIG_RotationalJoint (void)
        { }

        SIG_Joint::JointType SIG_RotationalJoint::getJointType (void) const
        {
                return tRotationalJoint;
        }
        
        void SIG_RotationalJoint::setLeftPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportH)
        {
                leftBase = _winportB;
                leftDir = _winportD;
                leftHand = _winportH;
        }
        
        void SIG_RotationalJoint::setRightPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportH)
        {
                rightBase = _winportB;
                rightDir = _winportD;
                rightHand = _winportH;
        }
        
        void SIG_RotationalJoint::setRange (DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                minimum = mn;
                maximum = mx;
                initial = ii;
        }
        
        DL_vector SIG_RotationalJoint::getLeftBase (void) const
        {
                return leftBase;
        }
        
        DL_vector SIG_RotationalJoint::getLeftDir (void) const
        {
                return leftDir;
        }
        
        DL_vector SIG_RotationalJoint::getLeftHand (void) const
        {
                return leftHand;
        }
        
        DL_vector SIG_RotationalJoint::getRightBase (void) const
        {
                return rightBase;
        }
        
        DL_vector SIG_RotationalJoint::getRightDir (void) const
        {
                return rightDir;
        }
        
        DL_vector SIG_RotationalJoint::getRightHand (void) const
        {
                return rightHand;
        }
        
        DL_Scalar SIG_RotationalJoint::getMin (void) const
        {
                return minimum;
        }
        
        DL_Scalar SIG_RotationalJoint::getMax (void) const
        {
                return maximum;
        }
        
        DL_Scalar SIG_RotationalJoint::getIni (void) const
        {
                return initial;
        }

        void SIG_RotationalJoint::transformPoints
        (SIG_Link *side, DL_vector mov, DL_matrix rot)
        {
                if (side == leftLink) {
                        tfap (mov, rot, &leftBase);
                        tfap (mov, rot, &leftDir);
                        tfap (mov, rot, &leftHand);
                } else if (side == rightLink) {
                        tfap (mov, rot, &rightBase);
                        tfap (mov, rot, &rightDir);
                        tfap (mov, rot, &rightHand);
                }
        }
        
        void SIG_RotationalJoint::getGeomRelation (DL_vector &t, DL_matrix &o, SIG_Link *origin)
        {
                if (leftLink == origin) {
                        calculateAnyJoint
                                (leftBase, leftDir, leftHand,
                                 rightBase, rightDir, rightHand,
                                 initial, 0.0,
                                 o, t,
                                 name);
                } else {
                        calculateAnyJoint
                                (rightBase, rightDir, rightHand,
                                 leftBase, leftDir, leftHand,
                                 360-initial, 0.0,
                                 o, t,
                                 name);
                }
        }

	void SIG_RotationalJoint::getMDH (SIG_Link * & predecessor,
					  double & a, double & alpha,
					  double & d, double & theta,
					  double & screwD, double & screwTheta)
	{
		switch (mdh_predecessor_is_left) {
		case 0:  predecessor = rightLink; break;
		case 1:  predecessor = leftLink; break;
		default: predecessor = 0;
		}
		a = mdh_a;
		alpha = mdh_alpha;
		d = mdh_d;
		theta = mdh_theta;
		screwD = mdh_screw_d;
		screwTheta = mdh_screw_theta;
	}

        void SIG_RotationalJoint::writeToFileTransfer (QTextStream & tx)
        {
                tx << "RotationalJoint ";
                SIG_Joint::writeToFileTransfer (tx);
                SIG_Robot::vectorToStream (tx, leftBase);
                SIG_Robot::vectorToStream (tx, leftDir);
                SIG_Robot::vectorToStream (tx, leftHand);
                SIG_Robot::vectorToStream (tx, rightBase);
                SIG_Robot::vectorToStream (tx, rightDir);
                SIG_Robot::vectorToStream (tx, rightHand);
                tx << minimum << ' ' << maximum << ' ' << initial << '\n';
        }
}
