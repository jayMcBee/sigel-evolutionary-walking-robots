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
#ifndef SIGEL_ROBOT_SIG_CONTACTSENSOR_H
#define SIGEL_ROBOT_SIG_CONTACTSENSOR_H

namespace SIGEL_Robot { class SIG_ContactSensor; }

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include "SIGEL_Robot/SIG_Joint.h"

namespace SIGEL_Robot {
  /**
   * SIG_ContactSensor models a sensor attached to a link.
   * When the link has contact to the ground the sensor write
   * 1 to the register, else 0.
   * This can be interpreted somehow by the CMP command comparing
   * two register values and eventually enabling the JMP command.
   */
  class SIG_ContactSensor : public SIG_Sensor {
  private:
    SIG_Link *theLink;

  public:
    SIG_ContactSensor (SIG_Robot *par, QString n, int nr = -1);
    SIG_ContactSensor (SIG_Robot *par, QTextStream & tx);
    virtual ~SIG_ContactSensor (void);

    virtual SensorType getSensorType (void) const;

	/**
	* Set link associated with this sensor.
	*/
    void setLink (SIG_Link *j);

	/**
	* Get the link this sensor is attached to.
	*/
	SIG_Link const *getLink (void) const;

    virtual void writeToFileTransfer (QTextStream & tx);
  };
}

#endif
