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
#include "matrix.h"
#include "pointvector.h"
#include <cmath>
#include <cfloat>

#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Simulation/SIG_SimulationCannotSolveException.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

#include <dm.h>
#include "dmContactModel.hpp"
#include "dmArticulation.hpp"


using namespace SIGEL_Tools;

SIGEL_Simulation::SIG_DynaMechsSimulationQueries::SIG_DynaMechsSimulationQueries(SIG_DynaMechsSimulationData& theSimulationData)
  : simulationData(theSimulationData)
{ };

void SIGEL_Simulation::SIG_DynaMechsSimulationQueries::sense(int sensorNo,
							     QList<SIG_Register> & registers) const
{
	SIGEL_Robot::SIG_Sensor		*sensor = 0;
	SIGEL_Robot::SIG_JointSensor	*jointSensor;
	SIGEL_Robot::SIG_Joint const	*joint;
	SIGEL_Robot::SIG_PitchRollSensor *prSensor;
	SIGEL_Robot::SIG_ContactSensor	 *ctSensor;
	SIGEL_Robot::SIG_Link const	*link;
	SIG_DynaMechsLink	*sigDMLink;
	dmABForKinStruct *forKinStruct;
	NEWMAT::Matrix dynaMechsOrientation;
	long double minRegisterValue,registerValueRange;
	DL_matrix	myMat;
	dmLink			*internalLink;
	dmContactModel	*contactModel;
	dmRigidBody		*rBody;
	int		linkNumber,registerValue, i;
	double 	q, qd,minPos, maxPos,
			posRange, scaledState,
			val1, val2,
			roll, pitch, angle;
	bool 	hasContact;


#ifdef SIG_DEBUG
	for (int iInt=0; iInt<simulationData.sensors.size(); iInt++)
	{	SIGEL_Tools::SIG_IO::cerr << "Sensor #" << iInt << ": \"" << simulationData.sensors[iInt]->getName() << "\"" << Qt::endl;
	}
#endif

  if (simulationData.sensors.size() > 0)
    {
#ifdef _WINDOWS
      long double minRegisterValue = ::pow( static_cast<long double>(2),
					       static_cast<long double>(registers[0].getSize() - 1) );
#else
      long double minRegisterValue = std::pow( static_cast<long double>(2),
					       static_cast<long double>(registers[0].getSize() - 1) );
#endif					

      int absoluteSensorNo = sensorNo + static_cast< int >(minRegisterValue);

      // Unsigned on purpose, as in SIG_DynaMechsCommandInterface::moveDrive.
      int sensorIndex = absoluteSensorNo % static_cast< uint >(simulationData.sensors.size());

      sensor = simulationData.sensors[ sensorIndex ];
    }


  if (!sensor)
	{	SIGEL_Tools::SIG_IO::cerr << "attempt to read invalid sensor (sensorNo=" << sensorNo << ") in 'SIG_DynaMechsSimulationQueries::sense()'" << Qt::endl;
		return;
	}

	//	SIGEL 1.0 always assumed a joint-sensor type -- ouch !
	//	Changed to handle different types of sensors	(jb, 12/2001)
	switch (sensor->getSensorType())
	{
    	// joint-Sensor type
		case SIGEL_Robot::SIG_Sensor::tJointSensor:		jointSensor = static_cast< SIGEL_Robot::SIG_JointSensor* >(sensor);
														joint = jointSensor->getJoint();
														linkNumber = simulationData.jointIndices[ joint->getNumber() ];

														sigDMLink = simulationData.dynaMechsLinks[ linkNumber ];
														internalLink = sigDMLink->dynaMechsLink;

														internalLink->getState( &q, &qd );

														minPos = joint->getMechsMinPos();
														maxPos = joint->getMechsMaxPos();

														if ( (minPos == (- DBL_MAX)) && (maxPos == DBL_MAX) )
														posRange = 1;
														else
														{	posRange = maxPos - minPos;

															if (q > maxPos)
																q = maxPos;
															else if (q < minPos)
																q = minPos;
														};

														scaledState = (q - minPos) / posRange;

#ifdef _WINDOWS
														minRegisterValue = - ::pow( static_cast<long double>(2),
																			static_cast<long double>(registers[0].getSize() - 1) );
#else
														minRegisterValue = - std::pow( static_cast<long double>(2),
																			static_cast<long double>(registers[0].getSize() - 1) );
#endif																			

														registerValueRange = - minRegisterValue * 2;

														registerValue = static_cast< int >( scaledState * registerValueRange + minRegisterValue );
														break;

		// pitch/roll type
		case SIGEL_Robot::SIG_Sensor::tPitchRollSensor:

														prSensor = static_cast< SIGEL_Robot::SIG_PitchRollSensor* >(sensor);
														link = prSensor->getLink();
														sigDMLink = simulationData.dynaMechsLinks[ link->getNumber() ];

														myMat = getLinkOrientation(link->getNumber());

#ifdef _WINDOWS
														pitch = ::acos( myMat.get(1,0) );
														roll = -::asin( myMat.get(0,0) );
#else
														pitch = std::acos( myMat.get(1,0) );
														roll = -std::asin( myMat.get(0,0) );
#endif														

														// what type (0..360 Grad) ?
														if (prSensor->IsPitchType())
														{	angle = pitch * (360.0 / (2.0*3.14159265));
														}
														else
														{	angle = roll * (360.0 / (2.0*3.14159265));
														}

														// limit to 0..360 degrees
														angle = fmod(angle, 360.0);

														// limit to -90/90 degrees -- required for scaling value to some register value
														if (angle > 90)
														{	angle = 90;
														}
														else if (angle < -90)
														{	angle = -90;
														}

														// min. pos is -90, see above..
														scaledState = (angle + 90) / 180;

#ifdef _WINDOWS
														minRegisterValue = - ::pow( static_cast<long double>(2),
																			static_cast<long double>(registers[0].getSize() - 1) );
#else
														minRegisterValue = - std::pow( static_cast<long double>(2),
																			static_cast<long double>(registers[0].getSize() - 1) );
#endif																			

														registerValueRange = - minRegisterValue * 2;

														registerValue = static_cast< int >( scaledState * registerValueRange + minRegisterValue );
														break;

		// contact type
		case SIGEL_Robot::SIG_Sensor::tContactSensor:

														ctSensor = static_cast< SIGEL_Robot::SIG_ContactSensor* >(sensor);
														link = ctSensor->getLink();

														sigDMLink = simulationData.dynaMechsLinks[ link->getNumber() ];
														rBody = sigDMLink->dynaMechsLink;

														// always assume our contact model is at index 0 in dmRigidBody !
														// this seems to work since we add only one force to this bloody object..
														contactModel = static_cast<dmContactModel *>(rBody->getForce(0));
														if (contactModel == NULL)
														{	SIGEL_Tools::SIG_IO::cerr << "failed to get the dmContactModel" << Qt::endl;
															break;
														}

														//	check all points/vertices for ground contact
														i = 0;
														hasContact = false;

														while (i<contactModel->getNumContactPoints())
														{
															if (contactModel->getContactState(i))
															{ hasContact = true;
															  break;
															}

															i++;
														}

														// contact means 1, no contact zero
														if (hasContact)
														{	scaledState = 1;
														}
														else
														{	scaledState = 0;
														}

#ifdef _WINDOWS
														minRegisterValue = - ::pow( static_cast<long double>(2),
																			static_cast<long double>(registers[0].getSize() - 1) );
#else
														minRegisterValue = - std::pow( static_cast<long double>(2),
																			static_cast<long double>(registers[0].getSize() - 1) );
#endif																			

														registerValueRange = - minRegisterValue * 2;

														registerValue = static_cast< int >( scaledState * registerValueRange + minRegisterValue );
														break;

		// dunno this type !
		default:	SIGEL_Tools::SIG_IO::cerr << "\tPANIC !!  UNKNOWN SENSOR TYPE !!\n" << Qt::endl;	//	somebody want to change this to this funky throw/catch thingy ?
	}

      // A scaledState of exactly 1 maps one past the register's top, and
      // SIG_Register::makeValid would wrap it to the bottom.
      if ( registerValue > registers[0].getMaxValue() )
        registerValue = registers[0].getMaxValue();

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "Reading sensor value from sensor "
				<< sensor->getName()
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
				<< "minRegisterValue: "
				<< static_cast< double >(minRegisterValue)
				<< "\n"
				<< "registerValueRange: "
				<< static_cast< double >(registerValueRange)
				<< "\n"
				<< "state: "
				<< q
				<< ", scaledState: "
				<< scaledState
				<< ", registerValue: "
				<< registerValue
				<< Qt::endl;
#endif

      registers[0].loadValue( registerValue );
};

QTime SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getActualSimulationTime() const
{
  int secs = static_cast< int >(simulationData.actualFrame * simulationData.simulationParameter.getStepSize());

  QTime time = QTime( 0, 0 ).addSecs( secs );

  return time;
};

DL_vector SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getLinkPosition(int linkNo) const
{
  SIG_DynaMechsLink *dynaMechsLink = simulationData.dynaMechsLinks[ linkNo ];

  if (dynaMechsLink)
    {
      NEWMAT::ColumnVector position =   SIG_TypeConverter::dynaMechsToSigel()
	                              * dynaMechsLink->transformation.SubMatrix( 1, 3, 4, 4 );

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "Position of link "
				<< dynaMechsLink->link->getName()
				<< ":";
      for (int i=1; i<=3; i++)
	SIGEL_Tools::SIG_IO::cerr << " "
				  << dynaMechsLink->transformation( i, 4 );
      SIGEL_Tools::SIG_IO::cerr << Qt::endl;

      SIGEL_Tools::SIG_IO::cerr << "Position of link (DynaMechs forward kinematics) "
				<< dynaMechsLink->link->getName()
				<< ", internal number "
				<< dynaMechsLink->dynaMechsLinkNumber
				<< ":";
      dmABForKinStruct const *forKinStruct = simulationData.dynaMechsSystem.getForKinStruct( dynaMechsLink->dynaMechsLinkNumber );
      NEWMAT::ColumnVector dynaMechsPosition = SIG_TypeConverter::toColumnVector( forKinStruct->p_ICS );
      for (int i=1; i<=3; i++)
	SIGEL_Tools::SIG_IO::cerr << " "
				  << dynaMechsPosition( i );
      SIGEL_Tools::SIG_IO::cerr << Qt::endl;
#endif

      return SIG_TypeConverter::toDL_vector( position );
    }
  else
    {
      DL_vector position(0, 0, 0);

      return position;
    };
};

DL_matrix SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getLinkOrientation(int linkNo) const
{
  SIG_DynaMechsLink *dynaMechsLink = simulationData.dynaMechsLinks[ linkNo ];

  if (dynaMechsLink)
    {
      NEWMAT::Matrix orientation =   SIG_TypeConverter::dynaMechsToSigel()
	                           * dynaMechsLink->transformation.SubMatrix( 1, 3, 1, 3 );

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "\n-----\n\nOrientation of link "
				<< dynaMechsLink->link->getName()
				<< ":\n" << Qt::endl;
      for (int i=1; i<=3; i++)
	  {
		for (int j=1; j<=3; j++)
			SIGEL_Tools::SIG_IO::cerr << " " << dynaMechsLink->transformation( i, j );
		SIGEL_Tools::SIG_IO::cerr << Qt::endl;
      }

      SIGEL_Tools::SIG_IO::cerr << "\nOrientation of link (DynaMechs forward kinematics) "
				<< dynaMechsLink->link->getName()
				<< ", internal number "
				<< dynaMechsLink->dynaMechsLinkNumber
				<< ":\n" << Qt::endl;
      dmABForKinStruct const *forKinStruct = simulationData.dynaMechsSystem.getForKinStruct( dynaMechsLink->dynaMechsLinkNumber );
      NEWMAT::Matrix dynaMechsOrientation = SIG_TypeConverter::toMatrix( forKinStruct->R_ICS );
      for (int i=1; i<=3; i++)
	  {
		for (int j=1; j<=3; j++)
			SIGEL_Tools::SIG_IO::cerr << " " << dynaMechsOrientation( i, j );

		SIGEL_Tools::SIG_IO::cerr << Qt::endl;
	  }
#endif

      return SIG_TypeConverter::toDL_matrix( orientation );
    }
  else
    {
      DL_matrix orientation;
      orientation.makeone();

      return orientation;
    };
}


int SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getRootNumber() const
{
  return simulationData.robot.getRootLink()->getNumber();
}

std::vector<double*>* SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getUsedForces() const {

	std::vector<double*>* forceList = simulationData.dynaMechsSystem.getForces();

  return forceList;
}


int SIGEL_Simulation::SIG_DynaMechsSimulationQueries::getLinkCount() const
{
  //return simulationData.dynaMechsSystem.getNumLinks();
  return simulationData.robot.getLinks().count();
}


void SIGEL_Simulation::SIG_DynaMechsSimulationQueries::checkDynas() const
{
}
