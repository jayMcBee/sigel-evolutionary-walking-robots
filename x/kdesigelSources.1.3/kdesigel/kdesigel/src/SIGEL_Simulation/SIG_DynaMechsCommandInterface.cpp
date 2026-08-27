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
#include "compat/q2compat.h"
#include "SIGEL_Simulation/SIG_DynaMechsCommandInterface.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"

#include <dmRevoluteLink.hpp>

#include <pointvector.h>
#include <cmath>


SIGEL_Simulation::SIG_DynaMechsCommandInterface::SIG_DynaMechsCommandInterface(SIG_DynaMechsSimulationData& theSimulationData)
  : simulationData(theSimulationData)
{ };

void SIGEL_Simulation::SIG_DynaMechsCommandInterface::moveDrive(int driveNo,
								QList<SIG_Register> const& registers)
{
#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "entering method moveDrive\n";
#endif
  SIGEL_Robot::SIG_Drive *drive = 0;
  dmLink	*internalLink;
  dmRevoluteLink *intRevLink;
  double	  q, qd, intMin, intMax, intSpring, intDamper;
  long double     sigelDestAngle,
                  destAngle,
                  dmDestAngle;
  int 		  driveIndex = 0;


#ifdef SIG_DEBUG
	for (int iInt=0; iInt<simulationData.drives.size(); iInt++)
	{	SIGEL_Tools::SIG_IO::cerr << "Drive #" << iInt << ": \"" << simulationData.drives[iInt]->getName() << "\"\n";
	}
#endif

  // determine what drive to use (if any)
  if (simulationData.drives.size() > 0)
    {
#ifdef _WINDOWS
      long double minRegisterValue = - ::pow( static_cast<long double>(2),
						 static_cast<long double>(registers[0].getSize() - 1) );
#else
      long double minRegisterValue = - std::pow( static_cast<long double>(2),
						 static_cast<long double>(registers[0].getSize() - 1) );
#endif						

      int absoluteDriveNo = driveNo - static_cast< int >(minRegisterValue);

      driveIndex = absoluteDriveNo % simulationData.drives.size();


#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "Drive index: " << driveIndex << "\n";
#endif

     drive = simulationData.drives[ driveIndex ];
    };

  if (!drive)
  {	return;
  }

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "Drive exists!\n";
#endif

#ifdef _WINDOWS
      long double minRegisterValue = - ( ::pow( static_cast<long double>(2),
						   static_cast<long double>(registers[0].getSize() - 1) )
	                                 - 1 );
#else
      long double minRegisterValue = - ( std::pow( static_cast<long double>(2),
						   static_cast<long double>(registers[0].getSize() - 1) )
	                                 - 1 );
#endif	

      long double registerValueRange = - minRegisterValue * 2;

      int registerValue = registers[0].getValue();

      long double absoluteRegisterValue = registerValue - minRegisterValue;


	  // SERVO-TYPE:  Interprete register value as angle
	  // THIS IS HOW WE INTERPRETE THIS TYPE OF MOTOR:
	  // (switching to German from here on...)
	  // -> a 'push it on its way' motor. We may spend practically no time in the command,
	  //    so we do not wait until the final angle has been reached.
	  // The motor is sent on its way, always at maximum force.
	  // The end position is reached by manipulating the joint's min/max values, i.e.
	  // by a hard stop. In that position the motor is held partly by the constraint
	  // and partly by the active motor force ... hope that works..!
	 if (drive->getMode() == SIGEL_Robot::SIG_Drive::tServoSimpleMode)
	 {
		//printf("tServoSimpleMode Drive '%s'\n", (const char *)drive->getName());

		int linkNumber = simulationData.jointIndices[ drive->getJoint()->getNumber() ];
		SIG_DynaMechsLink *dynaMechsLink = simulationData.dynaMechsLinks[ linkNumber ];

		// we work on rotational joints only, wat anderes kucken wa garnich an !
		SIGEL_Robot::SIG_Joint *myJoint = const_cast<SIGEL_Robot::SIG_Joint *>( drive->getJoint() );

		if (myJoint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint)
		{	SIGEL_Tools::SIG_IO::cerr << "tServoSimpleMode type drives can work on rotational joints only !!\n";
			return;
		}
		// get the DynaMechs joint for current angle -- joint limits might be manipulated !
		intRevLink = (dmRevoluteLink *)dynaMechsLink->dynaMechsLink;
		intRevLink->getJointLimits(&intMin, &intMax, &intSpring, &intDamper);

		// get rel. position within the min-max range we want to turn
		// in SIGEL (0..360) notation
		SIGEL_Robot::SIG_RotationalJoint *myRotJ = dynamic_cast<SIGEL_Robot::SIG_RotationalJoint *>(myJoint);
		long double rjointrange = (long double)(myRotJ->getMax()) - (long double)(myRotJ->getMin());

		// but restore limits first; don't trust the manipulated values -> use original values stored in SIG_Joint::getMechsMinPos()
		intMin = myRotJ->getMechsMinPos();
		intMax = myRotJ->getMechsMaxPos();

		// where do we want the servo/joint to point to ?
		// RELATIVE VALUE ! Add '+ (long double)(myRotJ->getMin())' for a valid SIGEL-Joint position
		//sigelDestAngle = (absoluteRegisterValue/registerValueRange ) * rjointrange;

    // forget the line above; we want a MOVE -90 in our code turn the servo to
    // -90 angle (if it is allowed to turn there) for transparent programs !!
    if (registerValue < myRotJ->getMin())
    {  destAngle = myRotJ->getMin();
    }
    else if (registerValue > myRotJ->getMax())
    {  destAngle = myRotJ->getMax();
    }
    else destAngle = registerValue;

    // make it relative, i.e. zero means minimal allowed (defined) joint angle
    sigelDestAngle = destAngle - (long double)(myRotJ->getMin());

    //fprintf(stderr, "\tsigelDestAngle: %5.3f for registerValue: %d (min: %5.3f, max: %5.3f)\n", sigelDestAngle, registerValue, myRotJ->getMin(),myRotJ->getMax());

		// transform to a dynamechs position -> 0..2*pi, please ! Add dm-internal min. joint value here, therefore not above ...
		dmDestAngle = sigelDestAngle * (3.14159265 / 180.0) + intMin;

		//	q is our current angle
		intRevLink->getState( &q, &qd );
		//printf("; current dmAngle=%5.3f\tSIGEL Destination=%4.1f, dmDest=%5.3f\n", q, sigelDestAngle+ (long double)(myRotJ->getMin()), dmDestAngle);

		// power joint w. max. force (we only know [on/off]-type servos)
		double jointInput = drive->getMaxForce();

		// NOW FOR THE REALLY BAD PART:
		//	Manipulate DynaMechs min./max. limits :-)

		// 1 degrees as 0..2*pi
		const double onePiDegree = 0.0174533;

		// which direction to turn ?
		if (q > dmDestAngle)
		{	jointInput *= -1.0;

			// set new min. limit ! give a minimal freedom of 1 degree to prevent
			// something shitty happening within our simulation -- but only if the
			// dest. angle isn't already at our 'natural' joint limits !
			if (dmDestAngle - onePiDegree > intMin)
			{	intMin = dmDestAngle-onePiDegree;
			}
		}
		else
		{	// set new max. limit !  same stuff as above applies in this case..
			if (dmDestAngle + onePiDegree < intMax)
			{	intMax = dmDestAngle+onePiDegree;
			}
		}

		//printf("AFTER MANIP.:  dmMin: %5.3f, dmMax: %5.3f\n\n", intMin, intMax);

		// now fall down on your knees and pray to the lord this really works :-)
		intRevLink->setJointLimits(intMin, intMax, intSpring, intDamper);

		// power for some darn large amount of time; meaning our motors are always 'on';
		// somebody might want to add a 'STOP-MOVE/motor-off' terminal to SIGEL some time.. ?
		intRevLink->setJointInput( &jointInput );

		double forceDuration = simulationData.robot.getLangParam()->getCommand( "MOVE" )->getDuration();
		simulationData.driveForcesTimeAccounts[ driveIndex ] = forceDuration * 10000;
	 }

	 // FORCE-TYPE:  Interprete register value as force and
	 // apply (using minForce/maxForce boundaries) to joint
	 else
	 {
	 	if (absoluteRegisterValue<0)
			absoluteRegisterValue = 0;

		long double force = (   ( absoluteRegisterValue / registerValueRange )
			    * ( 2 * drive->getMaxForce() ) )
	                  - drive->getMaxForce();

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "minRegisterValue: " << static_cast< double >(minRegisterValue) << "\n"
					<< "registerValueRange: " << static_cast< double >(registerValueRange)	<< "\n"
					<< "registerValue: " << registerValue << "\n" << "maxForce: " << drive->getMaxForce() << "\n";
#endif

		// we act only if the force is >min. force applicable
#ifdef _WINDOWS		
		if (::abs(force) >= drive->getMinForce())	{
#else
		if (std::abs(force) >= drive->getMinForce())	{
#endif		
			double jointInput = static_cast< double >(force);

#ifdef SIG_DEBUG
			SIGEL_Tools::SIG_IO::cerr << "Applying force/torque " << jointInput << " at drive "	<< drive->getName()	<< ".\n";
#endif

			int linkNumber = simulationData.jointIndices[ drive->getJoint()->getNumber() ];

			SIG_DynaMechsLink *dynaMechsLink = simulationData.dynaMechsLinks[ linkNumber ];

			dynaMechsLink->dynaMechsLink->setJointInput( &jointInput );

			double forceDuration = simulationData.robot.getLangParam()->getCommand( "MOVE" )->getDuration();

			simulationData.driveForcesTimeAccounts[ driveIndex ] = forceDuration;
		}
    }
}
