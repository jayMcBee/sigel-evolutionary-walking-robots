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
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Tools/SIG_IO.h"


namespace SIGEL_Robot {
        SIG_Drive::SIG_Drive (SIG_Robot *par, QString n, int nr)
                : parent (par), number (nr), name (n), theJoint (0), minforce (0.0), maxforce (1.0)
        { }

        SIG_Drive::SIG_Drive (SIG_Robot *par, QTextStream & tx)
                : parent (par), number (-1), name (), theJoint (0), minforce (0.0), maxforce (1.0)
        {
			QString buf;

			tx >> buf;

			if (buf == "force")
				mode = tForceMode;
			else if (buf == "relative")
				mode = tRelativeMode;
			else if (buf == "absolute")
				mode = tAbsoluteMode;
			else if (buf == "simpleservo")
				mode = tServoSimpleMode;
			else
			{	SIGEL_Tools::SIG_IO::cerr << "Unsupported Drive Mode <" << buf << "> !!\n\n";
			}

			tx >> name >> number >> minforce >> maxforce >> buf;
			theJoint = parent->lookupJoint (buf);
        }

        SIG_Drive::~SIG_Drive (void)
        { }

        QString SIG_Drive::getName (void) const
        {
                return name;
        }

        int SIG_Drive::getNumber (void) const
        {
                return number;
        }

        void SIG_Drive::setJoint (SIG_Joint *j)
        {
                theJoint = j;
        }

        void SIG_Drive::setMode (SIG_Drive::DriveMode dm)
        {
                mode = dm;
        }

        SIG_Joint const *SIG_Drive::getJoint (void) const
        {
                return theJoint;
        }

        SIG_Drive::DriveMode SIG_Drive::getMode (void) const
        {
                return mode;
        }

        void SIG_Drive::setForces(DL_Scalar mn, DL_Scalar mx)
        {
                minforce = mn;
                maxforce = mx;
        }

        DL_Scalar SIG_Drive::getMinForce (void) const
        {
                return minforce;
        }

        DL_Scalar SIG_Drive::getMaxForce (void) const
        {
                return maxforce;
        }

        void SIG_Drive::writeToFileTransfer (QTextStream & tx)
        {
		tx << "Drive ";
		if (mode == tForceMode)
			tx << "force ";
		else if (mode == tRelativeMode)
			tx << "relative ";
		else if (mode == tAbsoluteMode)
			tx << "absolute ";
		else if (mode == tServoSimpleMode)
			tx << "simpleservo ";
		else
			tx << "huh_grumpf ";
                tx << name << ' ' << number << ' '
                   << minforce << ' ' << maxforce << ' '
                   << theJoint->getName () << '\n';
        }
}
