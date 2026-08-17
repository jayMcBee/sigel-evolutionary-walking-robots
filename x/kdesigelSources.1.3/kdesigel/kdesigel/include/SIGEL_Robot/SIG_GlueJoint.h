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
#ifndef SIGEL_ROBOT_SIG_GLUEJOINT_H
#define SIGEL_ROBOT_SIG_GLUEJOINT_H

namespace SIGEL_Robot { class SIG_GlueJoint; }

#include "SIGEL_Robot/SIG_Joint.h"

namespace SIGEL_Robot
{
  /**
   * The class SIG_GlueJoint models a static connection
   * between two links. To be precise: There will be
   * zero degrees of freedom with this joint type.
   */
  class SIG_GlueJoint : public SIG_Joint {
  private:
    DL_vector ptA1, ptA2, ptA3;
    DL_vector ptB1, ptB2, ptB3;

  public:
    SIG_GlueJoint (SIG_Robot *par, QString n, int nr = -1);
    SIG_GlueJoint (SIG_Robot *par, QTextStream & tx);
    virtual ~SIG_GlueJoint (void);

    virtual JointType getJointType (void) const;

    void setPlaneA (DL_vector p1, DL_vector p2, DL_vector p3);
    void setPlaneB (DL_vector p1, DL_vector p2, DL_vector p3);
    void getPlaneA (DL_vector &p1, DL_vector &p2, DL_vector &p3);
    void getPlaneB (DL_vector &p1, DL_vector &p2, DL_vector &p3);

    virtual void transformPoints (SIG_Link *side,
                                  DL_vector mov, DL_matrix rot);
    virtual void getGeomRelation
      (DL_vector &t, DL_matrix &o, SIG_Link *origin);

    virtual void writeToFileTransfer (QTextStream & tx);
  };
}

#endif // SIGEL_ROBOT_SIG_GLUEJOINT_H
