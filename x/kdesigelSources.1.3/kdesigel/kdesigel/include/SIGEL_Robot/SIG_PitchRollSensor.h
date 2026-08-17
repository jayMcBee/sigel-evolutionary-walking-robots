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
#ifndef SIGEL_ROBOT_SIG_PITCHROLLSENSOR_H
#define SIGEL_ROBOT_SIG_PITCHROLLSENSOR_H

namespace SIGEL_Robot { class SIG_PitchRollSensor; }

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include "SIGEL_Robot/SIG_Joint.h"

namespace SIGEL_Robot {
  /**
   * SIG_PitchRollSensor models a sensor attached to a link.
   * It measures the pitch or roll angle of the joint.
   * joint has more than one degree of freedom, the sensor
   * will also measure more than one value.
   * Call either SetPitchType() or SetRollType() to determine if
   * the sensor should return pitch or roll information.
   */
  class SIG_PitchRollSensor : public SIG_Sensor {
  private:
    SIG_Link *theLink;
	bool	 isPitch,
			 isRoll;

  public:
    SIG_PitchRollSensor (SIG_Robot *par, QString n, int nr = -1);
    SIG_PitchRollSensor (SIG_Robot *par, QTextStream & tx);
    virtual ~SIG_PitchRollSensor (void);

    virtual SensorType getSensorType (void) const;
    void setLink (SIG_Link *j);
    SIG_Link const *getLink (void) const;
    virtual void writeToFileTransfer (QTextStream & tx);

	/**
	* This method makes the sensor measure the pitch angle (front/back).
	*/
	void SetPitchType( void )	{ isPitch = true; isRoll = false; }

	/**
	* This method makes the sensor measure the roll angle (left/right).
	*/
	void SetRollType( void )	{ isPitch = false; isRoll = true; }

	/**
	* Returns true if this SIG_PitchRollSensor instance is measuring pitch.
	*/
	bool  IsPitchType( void )	{ return isPitch; }
	bool  IsRollType( void )	{ return isRoll; }

  };
}

#endif
