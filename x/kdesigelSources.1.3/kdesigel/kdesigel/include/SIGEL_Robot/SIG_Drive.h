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
#ifndef SIGEL_ROBOT_SIG_DRIVE_H
#define SIGEL_ROBOT_SIG_DRIVE_H

namespace SIGEL_Robot { class SIG_Drive; }

#include <qstring.h>
#include <pointvector.h>
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Joint.h"

namespace SIGEL_Robot
{

/**
 * SIG_Drive models an actuator.
 *
 * The actuator modelled by this class may apply forces
 * only between links which are already connected with a joint.
 */
  class SIG_Drive {
  public:
          enum DriveMode { tForceMode, tRelativeMode, tAbsoluteMode, tServoSimpleMode };
  private:
    SIG_Robot *parent;
    QString name;
    int number;
    SIG_Joint *theJoint;
    DriveMode mode;
    DL_Scalar minforce, maxforce ;
  public:
    SIG_Drive (SIG_Robot *par, QString n, int nr = -1);
    SIG_Drive (SIG_Robot *par, QTextStream & tx);
    ~SIG_Drive (void);
    QString getName (void) const;
    int getNumber (void) const;
    void setJoint (SIG_Joint *j);
    void setMode (DriveMode dm);
    SIG_Joint const *getJoint (void) const;
    DriveMode getMode (void) const;
    void setForces(DL_Scalar mn, DL_Scalar mx);
    DL_Scalar getMinForce (void) const;
    DL_Scalar getMaxForce (void) const;
    void writeToFileTransfer (QTextStream & tx);
  };
}

#endif // SIGEL_ROBOT_SIG_DRIVE_H
