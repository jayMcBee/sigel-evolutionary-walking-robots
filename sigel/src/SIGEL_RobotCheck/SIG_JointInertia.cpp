/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#include "SIGEL_RobotCheck/SIG_JointInertia.h"

#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Simulation/SIG_DynaMechsSimulationData.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <dmMDHLink.hpp>
#include <newmatap.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

SIGEL_RobotCheck::SIG_JointInertia::SIG_JointInertia( const SIGEL_Robot::SIG_Robot &preparedRobot,
                                                        const SIGEL_Environment::SIG_Environment &environment,
                                                        const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter )
  : smallest( std::numeric_limits<double>::infinity() )
{
  if ( preparedRobot.getJoints().isEmpty() )
    throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The robot has no joints." );

  for ( SIGEL_Robot::SIG_Joint *joint : preparedRobot.getJoints() )
    {
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "Joint " + joint->getName() + " does not turn." );
    }

  SIGEL_Simulation::SIG_DynaMechsSimulationData simulationData( preparedRobot, environment, simulationParameter );

  dmArticulation &system = simulationData.dynaMechsSystem;
  int stateSize = system.getNumDOFs();

  // Where each joint sits in the system's packed state.
  QList<dmLink *> jointLinks;
  QList<int> jointOffsets;
  int offset = 0;
  for ( unsigned int i = 0; i < system.getNumLinks(); i++ )
    {
      dmLink *link = system.getLink( i );
      if ( link->getNumDOFs() == 1 )
        {
          jointLinks.append( link );
          jointOffsets.append( offset );
        }
      offset += link->getNumDOFs();
    }

  int jointCount = jointLinks.size();

  std::vector<Float> position( stateSize );
  std::vector<Float> velocity( stateSize );
  system.getState( position.data(), velocity.data() );
  std::fill( velocity.begin(), velocity.end(), 0 );

  Float noTorque = 0;
  Float unitTorque = 1;
  for ( dmLink *link : jointLinks )
    link->setJointInput( &noTorque );

  // The same poses on every call, so that the issues do not change between calls.
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );

  for ( int pose = 0; pose <= randomPoses; pose++ )
    {
      if ( pose > 0 )
        {
          for ( int j = 0; j < jointCount; j++ )
            {
              Float minimum, maximum, spring, damper;
              static_cast<dmMDHLink *>( jointLinks[j] )->getJointLimits( &minimum, &maximum, &spring, &damper );

              // A joint without limits can stand at any angle.
              if ( maximum - minimum > 2 * M_PI )
                {
                  minimum = -M_PI;
                  maximum = M_PI;
                }

              // SIG_Randomizer gives numbers below 32768.
              position[ jointOffsets[j] ] = minimum + ( maximum - minimum ) * randomizer.getRandomInt( 32768 ) / 32767.0;
            }
        }

      system.setState( position.data(), velocity.data() );

      std::vector<Float> state( position );
      state.insert( state.end(), velocity.begin(), velocity.end() );
      std::vector<Float> atRest( 2 * stateSize );
      system.ABDynamics( state.data(), atRest.data() );

      // One unit of torque on a joint, less the robot at rest, gives one column of the inverse mass matrix.
      NEWMAT::Matrix inverseMass( jointCount, jointCount );
      for ( int j = 0; j < jointCount; j++ )
        {
          std::vector<Float> pushed( 2 * stateSize );
          jointLinks[j]->setJointInput( &unitTorque );
          system.ABDynamics( state.data(), pushed.data() );
          jointLinks[j]->setJointInput( &noTorque );

          for ( int i = 0; i < jointCount; i++ )
            inverseMass( i + 1, j + 1 ) = pushed[ stateSize + jointOffsets[i] ] - atRest[ stateSize + jointOffsets[i] ];
        }

      NEWMAT::SymmetricMatrix symmetric( jointCount );
      for ( int i = 1; i <= jointCount; i++ )
        {
          for ( int j = 1; j <= i; j++ )
            symmetric( i, j ) = ( inverseMass( i, j ) + inverseMass( j, i ) ) / 2;
        }

      NEWMAT::DiagonalMatrix eigenvalues( jointCount );
      NEWMAT::EigenValues( symmetric, eigenvalues );
      // The largest eigenvalue of the inverse mass matrix is the inverse of the smallest inertia.
      smallest = std::min( smallest, 1 / static_cast<double>( eigenvalues( jointCount ) ) );
    }

}

double SIGEL_RobotCheck::SIG_JointInertia::getSmallest() const
{
  return smallest;
}
