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
#ifndef SIGEL_ROBOT_SIG_CYLINDRICALJOINT_H
#define SIGEL_ROBOT_SIG_CYLINDRICALJOINT_H

namespace SIGEL_Robot { class SIG_CylindricalJoint; }

#include "qstring.h"
#include "pointvector.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Joint.h"

namespace SIGEL_Robot
{

/**
 * SIG_CylindricalJoint models a joint, at which the
 * adjacent links can rotate around and move along
 * exactly one common axis.
 *
 * This class specializes SIG_Joint.
 */
  class SIG_CylindricalJoint : public SIG_Joint {
  private:
    DL_vector leftBase, rightBase;
    DL_vector leftDir, rightDir;
    DL_vector leftHand, rightHand;
    DL_Scalar rotMin, rotMax, rotIni;
    DL_Scalar traMin, traMax, traIni;
  public:
    SIG_CylindricalJoint (SIG_Robot *par, QString n, int nr = -1);
    SIG_CylindricalJoint (SIG_Robot *par, QTextStream & tx);
    virtual ~SIG_CylindricalJoint (void);
    virtual JointType getJointType (void) const;

    void setLeftPoints (DL_vector VB, DL_vector VD, DL_vector VH);
    void setRightPoints (DL_vector VB, DL_vector VD, DL_vector VH);
    void setRotationalRange (DL_Scalar mn, DL_Scalar mx, DL_Scalar ii);
    void setTranslationalRange (DL_Scalar mn, DL_Scalar mx, DL_Scalar ii);
    
    DL_vector getLeftBase (void) const;
    DL_vector getLeftDir (void) const;
    DL_vector getLeftHand (void) const;
    DL_vector getRightBase (void) const;
    DL_vector getRightDir (void) const;
    DL_vector getRightHand (void) const;
    DL_Scalar getMinRot (void) const;
    DL_Scalar getMaxRot (void) const;
    DL_Scalar getIniRot (void) const;
    DL_Scalar getMinTrans (void) const;
    DL_Scalar getMaxTrans (void) const;
    DL_Scalar getIniTrans (void) const;

    virtual void transformPoints (SIG_Link *side,
                                  DL_vector mov, DL_matrix rot);
    virtual void getGeomRelation
      (DL_vector &t, DL_matrix &o, SIG_Link *origin);
    virtual void writeToFileTransfer (QTextStream & tx);
  };
}

#endif // SIGEL_ROBOT_SIG_CYLINDRICALJOINT_H
