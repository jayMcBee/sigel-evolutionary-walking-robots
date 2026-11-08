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
#include <QList>
#include "SIGEL_Simulation/SIG_DynaMechsSimulationQueries.h"

#include <qdatetime.h>
#include "SIGEL_Tools/SIG_Matrix.h"
#include "SIGEL_Tools/SIG_Vector.h"
#include <cmath>
#include <numbers>
#include <optional>

#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

#include <dm.h>
#include "dmContactModel.hpp"
#include "dmArticulation.hpp"

using namespace SIGEL_Tools;

SIGEL_Simulation::SIG_DynaMechsSimulationQueries::SIG_DynaMechsSimulationQueries( SIG_DynaMechsSimulationData &theSimulationData )
	: simulationData( theSimulationData )
{
}

void SIGEL_Simulation::SIG_DynaMechsSimulationQueries::sense( int sensorNo,
                                                              QList<SIG_Register> &registers ) const
{
	SIGEL_Robot::SIG_Sensor *sensor = nullptr;

#ifdef SIG_DEBUG
	for( int iInt = 0; iInt < simulationData.sensors.size(); iInt++ )
	{
		SIGEL_Tools::SIG_IO::cerr << "Sensor #" << iInt << ": \"" << simulationData.sensors[iInt]->getName() << "\"" << Qt::endl;
	}
#endif

	if( simulationData.sensors.size() > 0 )
	{
		long double minRegisterValue = std::pow( static_cast<long double>( 2 ),
		                                         static_cast<long double>( registers[0].getSize() - 1 ) );

		int absoluteSensorNo = sensorNo + static_cast<int>( minRegisterValue );

		// Unsigned on purpose, as in SIG_DynaMechsCommandInterface::moveDrive.
		int sensorIndex = absoluteSensorNo % static_cast<uint>( simulationData.sensors.size() );

		sensor = simulationData.sensors[sensorIndex];
	}

	if( !sensor )
	{
		SIGEL_Tools::SIG_IO::cerr << "attempt to read invalid sensor (sensorNo=" << sensorNo << ") in 'SIG_DynaMechsSimulationQueries::sense()'" << Qt::endl;
		return;
	}

	std::optional<double> sensorReading;

	switch( sensor->getSensorType() )
	{
		// joint-Sensor type
		case SIGEL_Robot::SIG_Sensor::tJointSensor:
			sensorReading = readJointSensor( static_cast<SIGEL_Robot::SIG_JointSensor const *>( sensor ) );
			break;

		// pitch/roll type
		case SIGEL_Robot::SIG_Sensor::tPitchRollSensor:
			sensorReading = readPitchRollSensor( static_cast<SIGEL_Robot::SIG_PitchRollSensor *>( sensor ) );
			break;

		// contact type
		case SIGEL_Robot::SIG_Sensor::tContactSensor:
			sensorReading = readContactSensor( static_cast<SIGEL_Robot::SIG_ContactSensor const *>( sensor ) );
			break;

		// dunno this type !
		default:
			SIGEL_Tools::SIG_IO::cerr << "\tPANIC !!  UNKNOWN SENSOR TYPE !!\n" << Qt::endl; //	somebody want to change this to this funky throw/catch thingy ?
	}

	if( !sensorReading )
	{
		return;
	}

	long double minRegisterValue = -std::pow( static_cast<long double>( 2 ),
	                                          static_cast<long double>( registers[0].getSize() - 1 ) );

	long double registerValueRange = -minRegisterValue * 2;

	int registerValue = static_cast<int>( *sensorReading * registerValueRange + minRegisterValue );

	// Keeps a sensorReading of 1 at the register's top value: the mapping gives
	// one more, and SIG_Register::makeValid would wrap it to the bottom.
	if( registerValue > registers[0].getMaxValue() )
	{
		registerValue = registers[0].getMaxValue();
	}

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "minRegisterValue: "
	                          << static_cast<double>( minRegisterValue )
	                          << "\n"
	                          << "registerValueRange: "
	                          << static_cast<double>( registerValueRange )
	                          << "\n"
	                          << "registerValue: "
	                          << registerValue
	                          << Qt::endl;
#endif

	registers[0].loadValue( registerValue );
}

double SIGEL_Simulation::SIG_DynaMechsSimulationQueries::readJointSensor( SIGEL_Robot::SIG_JointSensor const *jointSensor ) const
{
	SIGEL_Robot::SIG_Joint const *joint = jointSensor->getJoint();
	int linkNumber = simulationData.jointIndices[joint->getNumber()];

	SIG_DynaMechsLink *sigDMLink = simulationData.dynaMechsLinks[linkNumber];
	dmLink *internalLink = sigDMLink->dynaMechsLink;

	double q, qd;
	internalLink->getState( &q, &qd );

	double minPos = joint->getMechsMinPos();
	double maxPos = joint->getMechsMaxPos();
	double posRange;

	if( ( minPos == maxPos ) && ( joint->getJointType() == SIGEL_Robot::SIG_Joint::tRotationalJoint ) )
	{
		// Equal limits are no limits, as in SIG_DynaMechsSimulationData: the angle is read over one turn from minPos.
		posRange = 2 * std::numbers::pi;
		q = minPos + std::fmod( q - minPos, posRange );

		if( q < minPos )
		{
			q += posRange;
		}
	}
	else
	{
		posRange = maxPos - minPos;

		if( q > maxPos )
		{
			q = maxPos;
		}
		else if( q < minPos )
		{
			q = minPos;
		}
	}

	double sensorReading = ( q - minPos ) / posRange;

#ifdef SIG_DEBUG
	SIGEL_Tools::SIG_IO::cerr << "Reading sensor value from sensor "
	                          << jointSensor->getName()
	                          << " at joint "
	                          << joint->getName()
	                          << " corresponding to link "
	                          << linkNumber
	                          << ": "
	                          << sigDMLink->link->getName()
	                          << "\n"
	                          << "minPos: "
	                          << minPos
	                          << ", maxPos: "
	                          << maxPos
	                          << ", posRange: "
	                          << posRange
	                          << "\n"
	                          << "state: "
	                          << q
	                          << ", sensorReading: "
	                          << sensorReading
	                          << Qt::endl;
#endif

	return sensorReading;
}

double SIGEL_Simulation::SIG_DynaMechsSimulationQueries::readPitchRollSensor( SIGEL_Robot::SIG_PitchRollSensor *prSensor ) const
{
	SIG_Matrix myMat = getLinkOrientation( prSensor->getLink()->getNumber() );

	double pitch = std::acos( myMat.get( 1, 0 ) );
	double roll = -std::asin( myMat.get( 0, 0 ) );
	double angle;

	// what type (0..360 degrees) ?
	if( prSensor->IsPitchType() )
	{
		angle = pitch * ( 360.0 / ( 2.0 * std::numbers::pi ) );
	}
	else
	{
		angle = roll * ( 360.0 / ( 2.0 * std::numbers::pi ) );
	}

	// limit to 0..360 degrees
	angle = fmod( angle, 360.0 );

	// limit to -90/90 degrees -- required for scaling value to some register value
	if( angle > 90 )
	{
		angle = 90;
	}
	else if( angle < -90 )
	{
		angle = -90;
	}

	// min. pos is -90, see above..
	return ( angle + 90 ) / 180;
}

std::optional<double> SIGEL_Simulation::SIG_DynaMechsSimulationQueries::readContactSensor( SIGEL_Robot::SIG_ContactSensor const *ctSensor ) const
{
	dmRigidBody *rBody = simulationData.dynaMechsLinks[ctSensor->getLink()->getNumber()]->dynaMechsLink;

	// always assume our contact model is at index 0 in dmRigidBody !
	// this seems to work since we add only one force to this bloody object..
	dmContactModel *contactModel = static_cast<dmContactModel *>( rBody->getForce( 0 ) );
	if( contactModel == nullptr )
	{
		SIGEL_Tools::SIG_IO::cerr << "failed to get the dmContactModel" << Qt::endl;
		return std::nullopt;
	}

	//	check all points/vertices for ground contact
	for( int i = 0; i < contactModel->getNumContactPoints(); i++ )
	{
		// contact means 1, no contact zero
		if( contactModel->getContactState( i ) )
		{
			return 1;
		}
	}

	return 0;
}

QTime SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getCurrentSimulationWholeSeconds() const
{
	int secs = static_cast<int>( simulationData.actualFrame * simulationData.simulationParameter.getStepSize() );

	QTime time = QTime( 0, 0 ).addSecs( secs );

	return time;
}

double SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getCurrentSimulationSeconds() const
{
	return simulationData.actualFrame * simulationData.simulationParameter.getStepSize();
}

SIG_Vector SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getLinkPosition( int linkNo ) const
{
	SIG_DynaMechsLink *dynaMechsLink = simulationData.dynaMechsLinks[linkNo];

	if( dynaMechsLink )
	{
		NEWMAT::ColumnVector position = SIG_TypeConverter::dynaMechsToSigel() * dynaMechsLink->transformation.SubMatrix( 1, 3, 4, 4 );

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Position of link "
		                          << dynaMechsLink->link->getName()
		                          << ":";
		for( int i = 1; i <= 3; i++ )
		{
			SIGEL_Tools::SIG_IO::cerr << " "
			                          << dynaMechsLink->transformation( i, 4 );
		}
		SIGEL_Tools::SIG_IO::cerr << Qt::endl;

		SIGEL_Tools::SIG_IO::cerr << "Position of link (DynaMechs forward kinematics) "
		                          << dynaMechsLink->link->getName()
		                          << ", internal number "
		                          << dynaMechsLink->dynaMechsLinkNumber
		                          << ":";
		dmABForKinStruct const *forKinStruct = simulationData.dynaMechsSystem.getForKinStruct( dynaMechsLink->dynaMechsLinkNumber );
		NEWMAT::ColumnVector dynaMechsPosition = SIG_TypeConverter::toColumnVector( forKinStruct->p_ICS );
		for( int i = 1; i <= 3; i++ )
		{
			SIGEL_Tools::SIG_IO::cerr << " "
			                          << dynaMechsPosition( i );
		}
		SIGEL_Tools::SIG_IO::cerr << Qt::endl;
#endif

		return SIG_TypeConverter::toSIG_Vector( position );
	}
	else
	{
		SIG_Vector position( 0, 0, 0 );

		return position;
	}
}

SIG_Matrix SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getLinkOrientation( int linkNo ) const
{
	SIG_DynaMechsLink *dynaMechsLink = simulationData.dynaMechsLinks[linkNo];

	if( dynaMechsLink )
	{
		NEWMAT::Matrix orientation = SIG_TypeConverter::dynaMechsToSigel() * dynaMechsLink->transformation.SubMatrix( 1, 3, 1, 3 );

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "\n-----\n\nOrientation of link "
		                          << dynaMechsLink->link->getName()
		                          << ":\n"
		                          << Qt::endl;
		for( int i = 1; i <= 3; i++ )
		{
			for( int j = 1; j <= 3; j++ )
			{
				SIGEL_Tools::SIG_IO::cerr << " " << dynaMechsLink->transformation( i, j );
			}
			SIGEL_Tools::SIG_IO::cerr << Qt::endl;
		}

		SIGEL_Tools::SIG_IO::cerr << "\nOrientation of link (DynaMechs forward kinematics) "
		                          << dynaMechsLink->link->getName()
		                          << ", internal number "
		                          << dynaMechsLink->dynaMechsLinkNumber
		                          << ":\n"
		                          << Qt::endl;
		dmABForKinStruct const *forKinStruct = simulationData.dynaMechsSystem.getForKinStruct( dynaMechsLink->dynaMechsLinkNumber );
		NEWMAT::Matrix dynaMechsOrientation = SIG_TypeConverter::toMatrix( forKinStruct->R_ICS );
		for( int i = 1; i <= 3; i++ )
		{
			for( int j = 1; j <= 3; j++ )
			{
				SIGEL_Tools::SIG_IO::cerr << " " << dynaMechsOrientation( i, j );
			}

			SIGEL_Tools::SIG_IO::cerr << Qt::endl;
		}
#endif

		return SIG_TypeConverter::toSIG_Matrix( orientation );
	}
	else
	{
		SIG_Matrix orientation;
		orientation.makeone();

		return orientation;
	}
}

int SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getRootNumber() const
{
	return simulationData.robot.getRootLink()->getNumber();
}

std::vector<double *> *SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getUsedForces() const
{
	std::vector<double *> *forceList = simulationData.dynaMechsSystem.getForces();

	return forceList;
}

int SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getLinkCount() const
{
	return simulationData.robot.getLinks().count();
}

void SIGEL_Simulation::SIG_DynaMechsSimulationQueries::checkDynas() const
{
}
