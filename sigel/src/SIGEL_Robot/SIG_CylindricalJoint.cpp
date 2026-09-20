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
#include "SIGEL_Robot/SIG_CylindricalJoint.h"
#include "SIGEL_Robot/IFunctions.h"

namespace SIGEL_Robot {
        SIG_CylindricalJoint::SIG_CylindricalJoint (SIG_Robot *par, QString n, int nr)
                : SIG_Joint (par, n, nr)
        { }

        SIG_CylindricalJoint::SIG_CylindricalJoint (SIG_Robot *par, QTextStream & tx)
                : SIG_Joint (par, tx)
        {
                leftBase = SIG_Robot::streamToVector (tx);
                leftDir = SIG_Robot::streamToVector (tx);
                leftHand = SIG_Robot::streamToVector (tx);
                rightBase = SIG_Robot::streamToVector (tx);
                rightDir = SIG_Robot::streamToVector (tx);
                rightHand = SIG_Robot::streamToVector (tx);
                tx >> rotMin >> rotMax >> rotIni
                   >> traMin >> traMax >> traIni;
        }

        SIG_CylindricalJoint::~SIG_CylindricalJoint (void)
        { }

        SIG_Joint::JointType SIG_CylindricalJoint::getJointType (void) const
        { return tCylindricalJoint; }
        
        void SIG_CylindricalJoint::setLeftPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportH)
        {
                leftBase = _winportB;
                leftDir = _winportD;
                leftHand = _winportH;
        }

        void SIG_CylindricalJoint::setRightPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportH)
        {
                rightBase = _winportB;
                rightDir = _winportD;
                rightHand = _winportH;
        }

        void SIG_CylindricalJoint::setRotationalRange (DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                rotMin = mn;
                rotMax = mx;
                rotIni = ii;
        }

        void SIG_CylindricalJoint::setTranslationalRange (DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                traMin = mn;
                traMax = mx;
                traIni = ii;
        }
    
        DL_vector SIG_CylindricalJoint::getLeftBase (void) const { return leftBase; }
        DL_vector SIG_CylindricalJoint::getLeftDir (void) const { return leftDir; }
        DL_vector SIG_CylindricalJoint::getLeftHand (void) const { return leftHand; }
        DL_vector SIG_CylindricalJoint::getRightBase (void) const { return rightBase; }
        DL_vector SIG_CylindricalJoint::getRightDir (void) const { return rightDir; }
        DL_vector SIG_CylindricalJoint::getRightHand (void) const { return rightHand; }
        DL_Scalar SIG_CylindricalJoint::getMinRot (void) const { return rotMin; }
        DL_Scalar SIG_CylindricalJoint::getMaxRot (void) const { return rotMax; }
        DL_Scalar SIG_CylindricalJoint::getIniRot (void) const { return rotIni;}
        DL_Scalar SIG_CylindricalJoint::getMinTrans (void) const { return traMin; }
        DL_Scalar SIG_CylindricalJoint::getMaxTrans (void) const { return traMax; }
        DL_Scalar SIG_CylindricalJoint::getIniTrans (void) const { return traIni; }

        void SIG_CylindricalJoint::transformPoints
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
        
        void SIG_CylindricalJoint::getGeomRelation (DL_vector &t, DL_matrix &o, SIG_Link *origin)
        {
                if (leftLink == origin) {
                        calculateAnyJoint
                                (leftBase, leftDir, leftHand,
                                 rightBase, rightDir, rightHand,
                                 rotIni, traIni,
                                 o, t,
                                 name);
                } else {
                        calculateAnyJoint
                                (rightBase, rightDir, rightHand,
                                 leftBase, leftDir, leftHand,
                                 360-rotIni, -traIni,
                                 o, t,
                                 name);
                }
        }

        void SIG_CylindricalJoint::writeToFileTransfer (QTextStream & tx)
        {
                tx << "CylindricalJoint ";
                SIG_Joint::writeToFileTransfer (tx);
                SIG_Robot::vectorToStream (tx, leftBase);
                SIG_Robot::vectorToStream (tx, leftDir);
                SIG_Robot::vectorToStream (tx, leftHand);
                SIG_Robot::vectorToStream (tx, rightBase);
                SIG_Robot::vectorToStream (tx, rightDir);
                SIG_Robot::vectorToStream (tx, rightHand);
                tx << rotMin << ' ' << rotMax << ' ' << rotIni << ' '
                   << traMin << ' ' << traMax << ' ' << traIni << '\n';
        }
}
