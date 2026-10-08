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

        SIG_Joint::JointType SIG_CylindricalJoint::getJointType () const
        { return tCylindricalJoint; }
        
        void SIG_CylindricalJoint::setLeftPoints (SIG_Vector base, SIG_Vector dir, SIG_Vector hand)
        {
                leftBase = base;
                leftDir = dir;
                leftHand = hand;
        }

        void SIG_CylindricalJoint::setRightPoints (SIG_Vector base, SIG_Vector dir, SIG_Vector hand)
        {
                rightBase = base;
                rightDir = dir;
                rightHand = hand;
        }

        void SIG_CylindricalJoint::setRotationalRange (double mn, double mx, double ii)
        {
                rotMin = mn;
                rotMax = mx;
                rotIni = ii;
        }

        void SIG_CylindricalJoint::setTranslationalRange (double mn, double mx, double ii)
        {
                traMin = mn;
                traMax = mx;
                traIni = ii;
        }
    
        SIG_Vector SIG_CylindricalJoint::getLeftBase () const { return leftBase; }

        SIG_Vector SIG_CylindricalJoint::getLeftDir () const { return leftDir; }

        SIG_Vector SIG_CylindricalJoint::getLeftHand () const { return leftHand; }

        SIG_Vector SIG_CylindricalJoint::getRightBase () const { return rightBase; }

        SIG_Vector SIG_CylindricalJoint::getRightDir () const { return rightDir; }

        SIG_Vector SIG_CylindricalJoint::getRightHand () const { return rightHand; }

        double SIG_CylindricalJoint::getMinRot () const { return rotMin; }

        double SIG_CylindricalJoint::getMaxRot () const { return rotMax; }

        double SIG_CylindricalJoint::getIniRot () const { return rotIni;}

        double SIG_CylindricalJoint::getMinTrans () const { return traMin; }

        double SIG_CylindricalJoint::getMaxTrans () const { return traMax; }

        double SIG_CylindricalJoint::getIniTrans () const { return traIni; }

        void SIG_CylindricalJoint::transformPoints
        (SIG_Link *side, SIG_Vector mov, SIG_Matrix rot)
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
        
        void SIG_CylindricalJoint::getGeomRelation (SIG_Vector &t, SIG_Matrix &o, SIG_Link *origin)
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
