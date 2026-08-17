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
#ifndef SIGEL_ROBOT_SIG_SENSOR_H
#define SIGEL_ROBOT_SIG_SENSOR_H

namespace SIGEL_Robot { class SIG_Sensor; }

#include <qstring.h>
#include "SIGEL_Robot/SIG_Robot.h"

namespace SIGEL_Robot {
  /**
   * This class is a framework for different types of sensors.
   * Any sensor type class should be derived from this class, the
   * 'SensorType' enumeration declares all known types of sensors.
   */
  class SIG_Sensor {
  public:
    enum SensorType {
		tJointSensor,		//	a sensor measuring the angle of a joint
		tPitchRollSensor,	//	a sensor measuring a links roll+pitch angle
		tContactSensor		//	a sensor measuring contact of link and ground
	};

  private:
    SIG_Robot *parent;
    QString name;
    int number;
  public:
    SIG_Sensor (SIG_Robot *par, QString n, int nr);
    SIG_Sensor (SIG_Robot *par, QTextStream & tx);
    virtual ~SIG_Sensor (void);

    virtual SensorType getSensorType (void) const = 0;
    QString getName (void) const;
    int getNumber (void) const;

    virtual void writeToFileTransfer (QTextStream & tx);
  }; 
}

#endif //  SIGEL_ROBOT_SIG_SENSOR_H
