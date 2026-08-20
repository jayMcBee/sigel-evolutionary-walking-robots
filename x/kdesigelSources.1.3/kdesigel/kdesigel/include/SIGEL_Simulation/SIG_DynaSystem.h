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
#ifndef SIGEL_SIMULATION_SIG_DYNASYSTEM_H
#define SIGEL_SIMULATION_SIG_DYNASYSTEM_H

#ifdef _WINDOWS
#endif

namespace SIGEL_Simulation { class SIG_DynaCallbacks; };

#include "compat/q2compat.h"
#include "rungekutta4.h"
#include "constraint.h"
#include "constraint_manager.h"
#include <SOLID/solid.h>
#include <qstring.h>
#include <qobject.h>
#include "SIGEL_Simulation/SIG_DynaCallbacks.h"
#include "SIGEL_Simulation/SIG_DynaLink.h"
#include "SIGEL_Simulation/SIG_DynaJoint.h"
#include "SIGEL_Simulation/SIG_DynaSensor.h"
#include "SIGEL_Simulation/SIG_DynaDrive.h"
#include "SIGEL_Simulation/SIG_DynaSystemWrongNumberException.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"

namespace SIGEL_Simulation
{
/**
 * The class DynaSystem is the gate to the Dynamo and the Solid-Library
 * 
 * Here will be all the commands to create new dyna-objects, constraints and
 * for them their solid counterparts.
 */
  class SIG_DynaSystem : public QObject 
  {
    Q_OBJECT
    
  public:
  
  bool newFrame;
  static bool foundNaN;

  void doNewFrame();
  void checkCollisionPTPs();

  DL_matrix rotationMatrix(DL_vector axis, DL_Scalar angle);      
  static DL_Scalar unNaN(DL_Scalar val);
  static DL_point unNaN(DL_point val);
  static DL_vector unNaN(DL_vector val);
  static DL_matrix unNaN(DL_matrix val);


  /** The message callback for SIG_DynaCallbacks */
  void msgFunction(QString theMessage);

  /** The doCollision callback for SIG_DynaCallbacks */
  void doCollisionDetection();

  void clearAllDynamics();
  void initializeFloor();

  /** The Dynamo DynaSystem */
  DL_dyna_system* dynaSystem;

  /** The Dynamo Integrator */
  DL_m_integrator* mIntegrator;

  /** The Dynamo Constraint Manager */
  DL_constraint_manager* constraintManager;

  /** The Dynamo Callbacks */
  SIG_DynaCallbacks* dynaCallbacks;
  
  /** The Floor Material */
  SIGEL_Robot::SIG_Material * floorMaterial;

  /**
   * The constructor.
   *
   * Sets up Solid and Dynamo plus integrator and callbacks
   */
  SIG_DynaSystem(SIGEL_Environment::SIG_Environment const & theEnvironment,
                 SIG_SimulationParameters const & theSimulationParameters);

  /**
   * The destructor
   *
   * Finishes Solid and Dynamo and Destroys the objects
   */
  ~SIG_DynaSystem();
  
  /** Adds a link to the DynaSystem by converting it
   *
   * @param theLink
   * the Link to be added
   */
  void newLink(SIGEL_Robot::SIG_Link& theLink);

  /** Adds a joint to the DynaSystem by converting it
   *
   * @param theJoint
   * the joint to be added
   */
  void newJoint(SIGEL_Robot::SIG_Joint& theJoint);

  /** Adds a drive to the DynaSystem by converting it
   *
   * @param theDrive
   * the drive to be added
   */
  void newDrive(SIGEL_Robot::SIG_Drive& theDrive);

  /** Adds a sensor to the DynaSystem by converting it
   *
   * @param theSensor
   * the sensor to be added
   */
  void newSensor(SIGEL_Robot::SIG_Sensor& theSensor);

  /** Gets a link to examine its data
   *
   * @param number
   * the number of the link to return
   * @returns
   * a reference to the link
   */
  SIG_DynaLink& getLink(int number);

  /** Gets a joint to examine its data
   *
   * @param number
   * the number of the joint to return
   * @returns
   * a reference to the joint
   */
  SIG_DynaJoint& getJoint(int number);

  /** Gets a drive to examine its data
   *
   * @param number
   * the number of the drive to return
   * @returns
   * a reference to the drive
   */
  SIG_DynaDrive& getDrive(int number);

  /** Gets a sensor to examine its data
   *
   * @param number
   * the number of the sensor to return
   * @returns
   * a reference to the sensor
   */
  SIG_DynaSensor& getSensor(int number);

  /** here the dynasystem remembers the number of the root link in the static robot modell */
  int rootLinkNo;
  
  /** a reference to the environment */
  SIGEL_Environment::SIG_Environment const & environment;
  
  /** a reference to the simulation parameters */
  SIG_SimulationParameters const & simulationParameters;

  /** storage for the floor shape in SOLID */
  DtShapeRef floor;

  /** storage for the SIG_DynaLinks */
  Q2PtrVector<SIG_DynaLink> dynaLinks;

  /** storage for the SIG_DynaJoints */
  Q2PtrVector<SIG_DynaJoint> dynaJoints;

  /** storage for the SIG_DynaSensors */
  Q2PtrVector<SIG_DynaSensor> dynaSensors;

  /** storage for the SIG_DynaDrives */
  Q2PtrVector<SIG_DynaDrive> dynaDrives;

  /** The Collision callback of SOLID
   *
   * adds a collision constraint to Dynamo
   */
  static void collisionResponse(void * client_data,
				DtObjectRef obj1,
				DtObjectRef obj2,
				const DtCollData *coll_data);
				
 signals:
  /** if dynamo throws an error message, this signal will be thrown 
   *
   * (and will be catched by SIG_Simulation)
   */
  void signalDynamoMessage(QString theMessage);

};

}

#endif // SIGEL_SIMULATION_SIG_DYNASYSTEM_H

