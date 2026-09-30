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
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Robot/IFunctions.h"

namespace SIGEL_Robot {
        SIG_GlueJoint::SIG_GlueJoint (SIG_Robot *par, QString n, int nr)
                : SIG_Joint (par, n, nr)
        { }

        SIG_GlueJoint::SIG_GlueJoint (SIG_Robot *par, QTextStream & tx)
                : SIG_Joint (par, tx)
        {
                ptA1 = SIG_Robot::streamToVector (tx);
                ptA2 = SIG_Robot::streamToVector (tx);
                ptA3 = SIG_Robot::streamToVector (tx);
                ptB1 = SIG_Robot::streamToVector (tx);
                ptB2 = SIG_Robot::streamToVector (tx);
                ptB3 = SIG_Robot::streamToVector (tx);
        }

        SIG_Joint::JointType SIG_GlueJoint::getJointType () const
        {
                return tGlueJoint;
        }
        
        void SIG_GlueJoint::setPlaneA (SIG_Vector p1, SIG_Vector p2, SIG_Vector p3)
        {
                ptA1 = p1;
                ptA2 = p2;
                ptA3 = p3;
        }
        
        void SIG_GlueJoint::setPlaneB (SIG_Vector p1, SIG_Vector p2, SIG_Vector p3)
        {
                ptB1 = p1;
                ptB2 = p2;
                ptB3 = p3;
        }
        
        void SIG_GlueJoint::getPlaneA (SIG_Vector &p1, SIG_Vector &p2, SIG_Vector &p3)
        {
                p1 = ptA1;
                p2 = ptA2;
                p3 = ptA3;
        }
        
        void SIG_GlueJoint::getPlaneB (SIG_Vector &p1, SIG_Vector &p2, SIG_Vector &p3)
        {
                p1 = ptB1;
                p2 = ptB2;
                p3 = ptB3;
        }
        
        void SIG_GlueJoint::transformPoints
        (SIG_Link *side, SIG_Vector mov, SIG_Matrix rot)
        {
                if (side == leftLink) {
                        tfap (mov, rot, &ptA1);
                        tfap (mov, rot, &ptA2);
                        tfap (mov, rot, &ptA3);
                } else if (side == rightLink) {
                        tfap (mov, rot, &ptB1);
                        tfap (mov, rot, &ptB2);
                        tfap (mov, rot, &ptB3);
                }
        }
        
        void SIG_GlueJoint::getGeomRelation (SIG_Vector &t, SIG_Matrix &o, SIG_Link *origin)
        {
                if (leftLink == origin) {
                        calculateAnyJoint
                                (ptA1, ptA2, ptA3,
                                 ptB1, ptB2, ptB3,
                                 0.0, 0.0,
                                 o, t,
                                 name);
                } else {
                        calculateAnyJoint
                                (ptA1, ptA2, ptA3,
                                 ptB1, ptB2, ptB3,
                                 0.0, 0.0,
                                 o, t,
                                 name);
                }
        }

        void SIG_GlueJoint::writeToFileTransfer (QTextStream & tx)
        {
                tx << "GlueJoint ";
                SIG_Joint::writeToFileTransfer (tx);
                SIG_Robot::vectorToStream (tx, ptA1);
                SIG_Robot::vectorToStream (tx, ptA2);
                SIG_Robot::vectorToStream (tx, ptA3);
                SIG_Robot::vectorToStream (tx, ptB1);
                SIG_Robot::vectorToStream (tx, ptB2);
                SIG_Robot::vectorToStream (tx, ptB3);
                tx << '\n';
        }
}
