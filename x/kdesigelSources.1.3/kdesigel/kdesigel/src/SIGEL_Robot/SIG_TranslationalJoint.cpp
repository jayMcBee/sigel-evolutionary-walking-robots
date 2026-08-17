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
#include "SIGEL_Robot/SIG_TranslationalJoint.h"

#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/IFunctions.h"

namespace SIGEL_Robot {
        SIG_TranslationalJoint::SIG_TranslationalJoint (SIG_Robot *par, QString n, int nr)
                : SIG_Joint (par, n, nr)
        { }

        SIG_TranslationalJoint::SIG_TranslationalJoint (SIG_Robot *par, QTextStream &tx)
                : SIG_Joint (par, tx)
        {
                leftBase = SIG_Robot::streamToVector (tx);
                leftDir = SIG_Robot::streamToVector (tx);
                leftFix = SIG_Robot::streamToVector (tx);
                rightBase = SIG_Robot::streamToVector (tx);
                rightDir = SIG_Robot::streamToVector (tx);
                rightFix = SIG_Robot::streamToVector (tx);
                tx >> minimum >> maximum >> initial;
        }
        
        SIG_TranslationalJoint::~SIG_TranslationalJoint ()
        { }

        SIG_Joint::JointType SIG_TranslationalJoint::getJointType (void) const
        {
                return tTranslationalJoint;
        }
        
        void SIG_TranslationalJoint::setLeftPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportF)
        {
                leftBase = _winportB;
                leftDir = _winportD;
                leftFix = _winportF;
        }
        
        void SIG_TranslationalJoint::setRightPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportF)
        {
                rightBase = _winportB;
                rightDir = _winportD;
                rightFix = _winportF;
        }

        void SIG_TranslationalJoint::setRange (DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                minimum = mn;
                maximum = mx; 
                initial = ii;
        }
        
        DL_vector SIG_TranslationalJoint::getLeftBase () const
        {
                return leftBase;
        }
        
        DL_vector SIG_TranslationalJoint::getLeftDir () const
        {
                return leftDir;
        }
        
        DL_vector SIG_TranslationalJoint::getLeftFix () const
        {
                return leftFix;
        }
        
        DL_vector SIG_TranslationalJoint::getRightBase () const
        {
                return rightBase;
        }
        
        DL_vector SIG_TranslationalJoint::getRightDir () const
        {
                return rightDir;
        }
        
        DL_vector SIG_TranslationalJoint::getRightFix () const
        {
                return rightFix;
        }
        
        DL_Scalar SIG_TranslationalJoint::getMin () const
        {
                return minimum;
        }
        
        DL_Scalar SIG_TranslationalJoint::getMax () const
        {
                return maximum;
        }
        
        DL_Scalar SIG_TranslationalJoint::getIni () const
        {
                return initial;
        }

        void SIG_TranslationalJoint::transformPoints
        (SIG_Link *side, DL_vector mov, DL_matrix rot)
        {
                if (side == leftLink) {
                        tfap (mov, rot, &leftBase);
                        tfap (mov, rot, &leftDir);
                        tfap (mov, rot, &leftFix);
                } else if (side == rightLink) {
                        tfap (mov, rot, &rightBase);
                        tfap (mov, rot, &rightDir);
                        tfap (mov, rot, &rightFix);
                }
        }
        
        void SIG_TranslationalJoint::getGeomRelation (DL_vector &t, DL_matrix &o, SIG_Link *origin)
        {
                if (leftLink == origin) {
                        calculateAnyJoint
                                (leftBase, leftDir, leftFix,
                                 rightBase, rightDir, rightFix,
                                 0, initial,
                                 o, t,
                                 name);
                } else {
                        calculateAnyJoint
                                (rightBase, rightDir, rightFix,
                                 leftBase, leftDir, leftFix,
                                 0, -initial,
                                 o, t,
                                 name);
                }
        }

	void SIG_TranslationalJoint::getMDH (SIG_Link * & predecessor,
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

        void SIG_TranslationalJoint::writeToFileTransfer (QTextStream & tx)
        {
                tx << "TranslationalJoint ";
                SIG_Joint::writeToFileTransfer (tx);
                SIG_Robot::vectorToStream (tx, leftBase);
                SIG_Robot::vectorToStream (tx, leftDir);
                SIG_Robot::vectorToStream (tx, leftFix);
                SIG_Robot::vectorToStream (tx, rightBase);
                SIG_Robot::vectorToStream (tx, rightDir);
                SIG_Robot::vectorToStream (tx, rightFix);
                tx << minimum << ' ' << maximum << ' ' << initial << '\n';
        }
}
