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
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"

#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"
#include "SIGEL_Tools/SIG_IO.h"

#include <dm.h>
#include <dmContactModel.hpp>
#include <dmIntegEuler.hpp>
#include <dmIntegRK4.hpp>
#include <dmIntegRK45.hpp>
#include <dmMobileBaseLink.hpp>
#include <dmMDHLink.hpp>
#include <dmPrismaticLink.hpp>
#include <dmRevoluteLink.hpp>

#include <cstdlib>
#include <cmath>
#include <cfloat>

using namespace SIGEL_Tools;

SIGEL_Simulation::SIG_DynaMechsSimulationData::SIG_DynaMechsSimulationData( SIGEL_Robot::SIG_Robot const& robot,
							  SIGEL_Environment::SIG_Environment const& environment,
							  SIGEL_Simulation::SIG_SimulationParameters const& simulationParameter)
  : SIG_SimulationData( robot, environment, simulationParameter ),
    dynaMechsLinks( robot.getLinkIter().count() ),
    jointIndices( robot.getJointIter().count() ),
    drives( robot.getDriveIter().count() ),
    driveForcesTimeAccounts( robot.getDriveIter().count() ),
    sensors( robot.getSensorIter().count() ),
#ifdef _WINDOWS
    pi( ::atan( 1 ) * 4 )
#else
    pi( std::atan( 1 ) * 4 )
#endif
{
  for (int i=0; i<dynaMechsLinks.size(); i++)
    dynaMechsLinks.insert( i, 0 );

  dynaMechsLinks.setAutoDelete( true );

  jointIndices.fill( 0 );

  driveForcesTimeAccounts.fill( 0 );

  for (int j=0; j<drives.size(); j++)
    drives.insert( j, 0 );

  for (int k=0; k<sensors.size(); k++)
    sensors.insert( k, 0 );

  initializeEnvironment();

  switch (simulationParameter.getDynaMechsIntegrator())
    {
    case SIG_SimulationParameters::Euler:
      dynaMechsIntegrator = new dmIntegEuler();
      break;
    case SIG_SimulationParameters::RungeKutta4:
      dynaMechsIntegrator = new dmIntegRK4();
      break;
    case SIG_SimulationParameters::RungeKutta45:
      dynaMechsIntegrator = new dmIntegRK45();
      break;
    };

  initializeArticulation();

  QDictIterator< SIGEL_Robot::SIG_Drive > driveIt = robot.getDriveIter();

  driveIt.toFirst();

  while (driveIt.current())
    {
      SIGEL_Robot::SIG_Drive *actDrive = driveIt.current();

      switch (actDrive->getMode())
	{
		//	should work for both, simple-servo and force drives..
	  case SIGEL_Robot::SIG_Drive::tForceMode:
	  case SIGEL_Robot::SIG_Drive::tServoSimpleMode:
	  {
	    int jointNumber = actDrive->getJoint()->getNumber();

	    int linkNumber = jointIndices[ jointNumber ];

	    if (dynaMechsLinks[ linkNumber ])
	      {

#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Inserting drive "
					  << actDrive->getName()
					  << " at joint "
					  << jointNumber
					  << " corresponding to link "
					  << linkNumber
					  << ": "
					  << dynaMechsLinks[ linkNumber ]->link->getName()
					  << ".\n";
#endif

		drives.insert( actDrive->getNumber(), actDrive );
	      };
	  };
	  break;
	};
      ++driveIt;
    };

  QDictIterator< SIGEL_Robot::SIG_Sensor > sensorIt = robot.getSensorIter();

  sensorIt.toFirst();

  while (sensorIt.current())
  {
	switch (sensorIt.current()->getSensorType())
	{

	 // tJointSensor:
	  case SIGEL_Robot::SIG_Sensor::tJointSensor:
	  {
	    SIGEL_Robot::SIG_JointSensor *actSensor = static_cast< SIGEL_Robot::SIG_JointSensor* >(sensorIt.current());

	    SIGEL_Robot::SIG_Joint const *joint = actSensor->getJoint();

	    int linkNumber = jointIndices[ joint->getNumber() ];

	    if (dynaMechsLinks[ linkNumber ])
		{	sensors.insert( actSensor->getNumber(), actSensor );
		}
		else SIGEL_Tools::SIG_IO::cerr << "Houston, we've got a problem here !  No link, no fun in  SIG_DynaMechsSimulationData (1)\n";
	  }
	  break;

	 // tPitchRollSensor
	  case SIGEL_Robot::SIG_Sensor::tPitchRollSensor:
	  {
	    SIGEL_Robot::SIG_PitchRollSensor *actSensor = static_cast< SIGEL_Robot::SIG_PitchRollSensor* >(sensorIt.current());
	    SIGEL_Robot::SIG_Link const *link = actSensor->getLink();

	    if (dynaMechsLinks[ link->getNumber() ])
		{	sensors.insert( actSensor->getNumber(), actSensor );
		}
		else SIGEL_Tools::SIG_IO::cerr << "Houston, we've got a problem here !  No link, no fun in  SIG_DynaMechsSimulationData (2)\n";

	  }
	  break;

	 // tContactSensor
	  case SIGEL_Robot::SIG_Sensor::tContactSensor:
	  {
	    SIGEL_Robot::SIG_ContactSensor *actSensor = static_cast< SIGEL_Robot::SIG_ContactSensor* >(sensorIt.current());
	    SIGEL_Robot::SIG_Link const *link = actSensor->getLink();

	    if (dynaMechsLinks[ link->getNumber() ])
		{	sensors.insert( actSensor->getNumber(), actSensor );
		}
		else SIGEL_Tools::SIG_IO::cerr << "Houston, we've got a problem here !  No link, no fun in  SIG_DynaMechsSimulationData (3)\n";

	  }
	  break;
}

	++sensorIt;
  }

  dynaMechsIntegrator->setSystem( &dynaMechsSystem );
};

void SIGEL_Simulation::SIG_DynaMechsSimulationData::setNewFrame( bool newValue )
{ };

void SIGEL_Simulation::SIG_DynaMechsSimulationData::simulationProgress()
{
  Float stepSize = static_cast< Float >(simulationParameter.getStepSize());

  // hier findet der eigentliche Simulationsprozeß statt!
  // ebenfalls wird hier der ABDynamics-Algorithmus aufgerufen, der in dmSystem steht.
  dynaMechsIntegrator->simulate( stepSize );

  for (int i = 0; i < drives.size(); i++) {
      SIGEL_Robot::SIG_Drive *actDrive = drives[ i ];

      if (actDrive)
	{
	  driveForcesTimeAccounts[ i ] -= simulationParameter.getStepSize();

#ifdef SIG_DEBUG
	  SIGEL_Tools::SIG_IO::cerr << "driveForcesTimeAccount[ "
				    << i
				    << " ], drive "
				    << actDrive->getName()
				    << ": "
				    << driveForcesTimeAccounts[ i ]
				    << "\n";
#endif

	  if (driveForcesTimeAccounts[ i ] <= 0)
	    {
#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "Resetting drive "
					<< actDrive->getName()
					<< ".\n";
#endif
	      driveForcesTimeAccounts[ i ] = 0;

	      int jointNumber = actDrive->getJoint()->getNumber();

	      int linkNumber = jointIndices[ jointNumber ];

	      SIG_DynaMechsLink *dynaMechsLink = dynaMechsLinks[ linkNumber ];

	      double resetForce = 0;

	      dynaMechsLink->dynaMechsLink->setJointInput( &resetForce );
	    };
	};
    };

  dynaMechsLinks[ robot.getRootLink()->getNumber() ]->forwardKinematics( 0 );
};

void SIGEL_Simulation::SIG_DynaMechsSimulationData::initializeEnvironment()
{
  NEWMAT::ColumnVector gravity( 3 );
  gravity = SIG_TypeConverter::toColumnVector( environment.getGravity() );

  gravity = SIG_TypeConverter::sigelToDynaMechs() * gravity;

  CartesianVector dynaMechsGravity;
  SIG_TypeConverter::toCartesianVector( gravity, dynaMechsGravity );
  dynaMechsEnvironment.setGravity( dynaMechsGravity );

  dynaMechsEnvironment.setGroundPlanarSpringConstant( environment.getGroundPlanarSpringConstant() );
  dynaMechsEnvironment.setGroundNormalSpringConstant( environment.getGroundNormalSpringConstant() );
  dynaMechsEnvironment.setGroundPlanarDamperConstant( environment.getGroundPlanarDamperConstant() );
  dynaMechsEnvironment.setGroundNormalDamperConstant( environment.getGroundNormalDamperConstant() );
  dynaMechsEnvironment.setFrictionCoeffs( environment.getFrictionCoeff_u_s(),
					  environment.getFrictionCoeff_u_k() );

#ifdef _WINDOWS
	char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else
	char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif	

  QString sigelRootString( sigelRootCString );

  QString terrainDataFileName = sigelRootString + "/Terrain.ter";

  QCString terrainDataFileNameQCString = terrainDataFileName.utf8();
  char const *terrainDataFileNameCString = terrainDataFileNameQCString;

  dynaMechsEnvironment.loadTerrainData( terrainDataFileNameCString );

  dmEnvironment::setEnvironment( &dynaMechsEnvironment );
};

void SIGEL_Simulation::SIG_DynaMechsSimulationData::initializeArticulation()
{
  SIGEL_Robot::SIG_Link const *rootLink = robot.getRootLink();

  dmMobileBaseLink *internalRootLink = new dmMobileBaseLink();


  SpatialVector velocity = { 0, 0, 0, 0, 0, 0 };

  Float initialState[7];

  NEWMAT::Matrix startRotation =   SIG_TypeConverter::sigelToDynaMechs()
                                 * SIG_TypeConverter::toMatrix( robot.initialOrientation );

  rotationMatrixToQuaternion( startRotation,
			      initialState[0],
			      initialState[1],
			      initialState[2],
			      initialState[3] );

  normalizeQuat( initialState );

  NEWMAT::ColumnVector startPosition =   SIG_TypeConverter::sigelToDynaMechs()
                                       * (   SIG_TypeConverter::toColumnVector( environment.getStartPosition() )
					   + SIG_TypeConverter::toColumnVector( robot.initialLocation ) );

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "Setting initial robot location: ";
  for (int i=1; i<=3; i++)
    SIGEL_Tools::SIG_IO::cerr << " " << startPosition( i );
  SIGEL_Tools::SIG_IO::cerr << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Setting initial robot orientation:\n";
  for (int i=1; i<=3; i++)
    {
      for (int j=1; j<=3; j++)
	SIGEL_Tools::SIG_IO::cerr << startRotation( i, j ) << " ";
      SIGEL_Tools::SIG_IO::cerr << "\n";
    };
#endif

  SIG_TypeConverter::toCartesianVector( startPosition, initialState + 4 );

  internalRootLink->setState( initialState, velocity );

  SIG_DynaMechsLink *dynaMechsRootLink = new SIG_DynaMechsLink( 0,
								rootLink,
								internalRootLink,
								0,
								0 );

  dynaMechsLinks.insert( rootLink->getNumber(), dynaMechsRootLink );

  dynaMechsSystem.addLink( internalRootLink, 0 );

  QList< SIGEL_Robot::SIG_Joint > rootJoints = rootLink->getJoints();

  SIGEL_Robot::SIG_Joint *actJoint = rootJoints.first();

  while (actJoint)
    {
      SIG_DynaMechsLink *newDynaMechsLink = initializeJoint( actJoint, rootLink );

      if (newDynaMechsLink)
	dynaMechsRootLink->successors.append( newDynaMechsLink );

      actJoint = rootJoints.next();
    };

  dynaMechsRootLink->forwardKinematics( 0 );
};

SIGEL_Simulation::SIG_DynaMechsLink *SIGEL_Simulation::SIG_DynaMechsSimulationData::initializeJoint( SIGEL_Robot::SIG_Joint *joint,
												     SIGEL_Robot::SIG_Link const *caller )
{
  double a;
  double alpha;
  double d;
  double theta;

  double screwD;
  double screwTheta;

  SIGEL_Robot::SIG_Link *predecessor;
  joint->getMDH( predecessor,
		 a,
		 alpha,
		 d,
		 theta,
		 screwD,
		 screwTheta );

  if (predecessor!=caller)
    return 0;

  SIGEL_Robot::SIG_Link const *link = ( joint->getLeftLink() == caller ) ? joint->getRightLink() : joint->getLeftLink();

  dmMDHLink *internalDynaMechsLink;

  switch (joint->getJointType())
    {
    case SIGEL_Robot::SIG_Joint::tTranslationalJoint:
      internalDynaMechsLink = new dmPrismaticLink();
      break;
    case SIGEL_Robot::SIG_Joint::tRotationalJoint:
      internalDynaMechsLink = new dmRevoluteLink();
      break;
    default:
      SIGEL_Tools::SIG_IO::cerr << "Cannot simulate robot with DynaMechs: Joint type not allowed!\n";
      exit( 1 );
    };

  double k_spring = simulationParameter.getJointLimitsK_spring();
  double b_damper = simulationParameter.getJointLimitsB_damper();

  double minLimit = joint->getMechsMinPos();
  double maxLimit = joint->getMechsMaxPos();

  if (minLimit==maxLimit)
    {
      k_spring = b_damper = 0;
      minLimit = - DBL_MAX;
      maxLimit = DBL_MAX;
    };

  double jointFriction = simulationParameter.getJointFrictionU_c();

  internalDynaMechsLink->setJointFriction( jointFriction );

  internalDynaMechsLink->setMDHParameters( a,
					   alpha,
					   d,
					   theta);

#ifdef SIG_DEBUG
  SIGEL_Tools::SIG_IO::cerr << "MDH Parameters of joint "
			    << joint->getName()
			    << ": a: "
			    << a
			    << " alpha: "
			    << alpha
			    << " d: "
			    << d
			    << " theta: "
			    << theta
			    << " screwD: "
			    << screwD
			    << " screwTheta: "
			    << screwTheta
			    << "\n"
			    << "Joint limits: Min: "
			    << minLimit
			    << " Max: "
			    << maxLimit
			    << "\n";
#endif

  internalDynaMechsLink->setJointLimits( minLimit,
					 maxLimit,
					 k_spring,
					 b_damper );

  int dynaMechsLinkIndex = dynaMechsSystem.getNumLinks();

  if ((screwD!=0) || (screwTheta!=0))
    dynaMechsLinkIndex++;

  SIG_DynaMechsLink *dynaMechsLink = new SIG_DynaMechsLink( dynaMechsLinkIndex,
							    link,
							    internalDynaMechsLink,
							    screwD,
							    screwTheta );

  dynaMechsLinks.insert( link->getNumber(), dynaMechsLink );

  jointIndices[ joint->getNumber() ] = link->getNumber();

  int predIndex = predecessor->getNumber();

  dmLink *internalPredecessor = dynaMechsLinks[ predIndex ]->dynaMechsLink;

  dmZScrewTxLink *screwPredecessor = dynaMechsLink->screwLink;

  if (screwPredecessor)
    {
      dynaMechsSystem.addLink( screwPredecessor, internalPredecessor );
      dynaMechsSystem.addLink( internalDynaMechsLink, screwPredecessor );
    }
  else
    dynaMechsSystem.addLink( internalDynaMechsLink, internalPredecessor );

  QList< SIGEL_Robot::SIG_Joint > joints = link->getJoints();

  SIGEL_Robot::SIG_Joint *actJoint = joints.first();

  while (actJoint)
    {
      SIG_DynaMechsLink *newSuccessor = initializeJoint( actJoint, link );

      if (newSuccessor)
	dynaMechsLink->successors.append( newSuccessor );

      actJoint = joints.next();
    };

  return dynaMechsLink;
};

void SIGEL_Simulation::SIG_DynaMechsSimulationData::rotationMatrixToQuaternion( NEWMAT::Matrix rotationMatrix,
										double &x,
										double &y,
										double &z,
										double &w )
{
  rotationMatrix = rotationMatrix.t();

  double tr, s;

  tr = rotationMatrix(1,1) + rotationMatrix(2,2) + rotationMatrix(3,3) + 1;
  if (tr > 0.0625)
    {
#ifdef _WINDOWS
      s = ::sqrt(tr);
#else
      s = std::sqrt(tr);
#endif
      w = s*0.5;
      s = 0.5/s;

      x = (rotationMatrix(2,3) - rotationMatrix(3,2))*s;
      y = (rotationMatrix(3,1) - rotationMatrix(1,3))*s;
      z = (rotationMatrix(1,2) - rotationMatrix(2,1))*s;
      return;
    };

  tr = -rotationMatrix(1,1) - rotationMatrix(2,2) + rotationMatrix(3,3) + 1;
  if (tr > 0.0625)
    {
#ifdef _WINDOWS
      s = ::sqrt(tr);
#else
      s = std::sqrt(tr);
#endif
      z = s*0.5;
      s = 0.5/s;

      x = (rotationMatrix(3,1) - rotationMatrix(1,3))*s;
      y = (rotationMatrix(3,2) + rotationMatrix(2,3))*s;
      w = (rotationMatrix(1,2) - rotationMatrix(2,1))*s;
      return;
    };
 
  tr = -rotationMatrix(1,1) + rotationMatrix(2,2) - rotationMatrix(3,3) + 1;
  if (tr > 0.0625)
    {
#ifdef _WINDOWS
      s = ::sqrt(tr);
#else
      s = std::sqrt(tr);
#endif
      y = s*0.5;
      s = 0.5/s;

      x = (rotationMatrix(2,1) + rotationMatrix(1,2))*s;
      z = (rotationMatrix(3,2) + rotationMatrix(2,3))*s;
      w = (rotationMatrix(3,1) - rotationMatrix(1,3))*s;
      return;
    };

  tr = rotationMatrix(1,1) - rotationMatrix(2,2) - rotationMatrix(3,3) + 1;
  if (tr > 0.0625)
    {
#ifdef _WINDOWS
      s = ::sqrt(tr);
#else
      s = std::sqrt(tr);
#endif
      x = s*0.5;
      s = 0.5/s;

      y = (rotationMatrix(2,1) + rotationMatrix(1,2))*s;
      z = (rotationMatrix(3,1) - rotationMatrix(1,3))*s;
      w = (rotationMatrix(2,3) - rotationMatrix(3,2))*s;
      return;
    };
};


