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
#ifndef SIGEL_ROBOT_SIG_TRANSLATIONALJOINT_H
#define SIGEL_ROBOT_SIG_TRANSLATIONALJOINT_H

namespace SIGEL_Robot { class SIG_TranslationalJoint; }

#include <qstring.h>
#include <pointvector.h>

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Joint.h"

namespace SIGEL_Robot
{
  /**
   * SIG_TranslationalJoint models a joint, at which the
   * adjacent links can translate along a common axis.
   *
   * This class specializes SIG_Joint.
   */
  class SIG_TranslationalJoint : public SIG_Joint {
  private:
    DL_vector rightBase, leftBase, rightFix;
    DL_vector leftDir, rightDir, leftFix;
    DL_Scalar minimum, maximum, initial;
  public:
    SIG_TranslationalJoint (SIG_Robot *par, QString n, int nr = -1);
    SIG_TranslationalJoint (SIG_Robot *par, QTextStream & tx);
    ~SIG_TranslationalJoint ();
    virtual JointType getJointType (void) const;
    void setLeftPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportF);
    void setRightPoints (DL_vector _winportB, DL_vector _winportD, DL_vector _winportF);
    void setRange (DL_Scalar mn, DL_Scalar mx, DL_Scalar ii);
    DL_vector getLeftBase () const;
    DL_vector getLeftDir () const;
    DL_vector getLeftFix () const;
    DL_vector getRightBase () const;
    DL_vector getRightDir () const;
    DL_vector getRightFix () const;
    DL_Scalar getMin () const;
    DL_Scalar getMax () const;
    DL_Scalar getIni () const;

    virtual void transformPoints (SIG_Link *side,
                                  DL_vector mov, DL_matrix rot);
    virtual void getGeomRelation
      (DL_vector &t, DL_matrix &o, SIG_Link *origin);
    virtual void getMDH (SIG_Link * & predecessor,
			 double & a, double & alpha,
			 double & d, double & theta,
			 double & screwD, double & screwTheta);
    virtual void writeToFileTransfer (QTextStream & tx);
  };
}

#endif // SIGEL_ROBOT_SIG_TRANSLATIONALJOINT_H
