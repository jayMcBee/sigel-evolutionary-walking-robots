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
#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"

namespace SIGEL_Robot {
        SIG_JointSensor::SIG_JointSensor (SIG_Robot *par, QString n, int nr)
                : SIG_Sensor (par, n, nr),
                  theJoint (0)
        { }

        SIG_JointSensor::SIG_JointSensor (SIG_Robot *par, QTextStream & tx)
                : SIG_Sensor (par, tx)
        {
                QString b;
                tx >> b;
                theJoint = par->lookupJoint (b);
                if (!theJoint)
                        throw SIG_UnstreamingError (__FILE__, __LINE__, "sensor '" + getName () + "' names unknown joint '" + b + "'");
        }
        
        SIG_JointSensor::~SIG_JointSensor (void)
        { }

        SIG_Sensor::SensorType SIG_JointSensor::getSensorType (void) const
        {
                return tJointSensor;
        }
        
        void SIG_JointSensor::setJoint (SIG_Joint *j)
        {
                theJoint = j;
        }

        SIG_Joint const *SIG_JointSensor::getJoint (void) const
        {
                return theJoint;
        }

        void SIG_JointSensor::writeToFileTransfer (QTextStream & tx)
        {
                tx << "JointSensor ";
                SIG_Sensor::writeToFileTransfer (tx);
                tx << theJoint->getName () << '\n';
        }
}
