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
#include "SIGEL_Robot/SIG_PitchRollSensor.h"

namespace SIGEL_Robot {
        SIG_PitchRollSensor::SIG_PitchRollSensor (SIG_Robot *par, QString n, int nr)
                : SIG_Sensor (par, n, nr),
                  theLink (0)
        {
			// initially neither pitch nor roll type
			isPitch = false;
			isRoll = false;
		}

        SIG_PitchRollSensor::SIG_PitchRollSensor (SIG_Robot *par, QTextStream & tx)
                : SIG_Sensor (par, tx)
        {
			QString myLink, myMode;

			tx >> myMode >> myLink;

			theLink = par->lookupLink(myLink);

			// initially neither pitch nor roll type
			if  (myMode == "PitchType")
			{	isPitch = true;
				isRoll = false;
			}
			else
			{	isPitch = false;
				isRoll = true;
			}
		}

        SIG_PitchRollSensor::~SIG_PitchRollSensor (void)
        { }

        SIG_Sensor::SensorType SIG_PitchRollSensor::getSensorType (void) const
        {
                return tPitchRollSensor;
        }

        void SIG_PitchRollSensor::setLink(SIG_Link *j)
        {
                theLink = j;
        }

        SIG_Link const *SIG_PitchRollSensor::getLink(void) const
        {
                return theLink;
        }

        void SIG_PitchRollSensor::writeToFileTransfer (QTextStream & tx)
        {
			tx << "PitchRollSensor ";
			SIG_Sensor::writeToFileTransfer (tx);

			if (isPitch)
			{	tx << "PitchType ";
			}
			else
			{	tx << "RollType ";
			}

			tx << theLink->getName () << '\n';
        }
}
