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
#include "SIGEL_Simulation/SIG_DynaMoSimulationQueries.h"
#include <qdatetime.h>
#include "matrix.h"
#include "pointvector.h"
#include "SIGEL_Simulation/SIG_DynaLink.h"
#include "SIGEL_Simulation/SIG_Dyna.h"
#include "SIGEL_Simulation/SIG_DynaSensor.h"
#include <cmath>
#include "NaN.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Simulation/SIG_SimulationCannotSolveException.h"

SIGEL_Simulation::SIG_DynaMoSimulationQueries::SIG_DynaMoSimulationQueries(SIG_DynaMoSimulationData& theSimulationData)
 : simulationData(theSimulationData)
{ };  

void SIGEL_Simulation::SIG_DynaMoSimulationQueries::sense(int sensorNo,QVector<SIG_Register> & registers) const
{
 if (simulationData.dynaSystem.dynaSensors.size()==0) return;
 int modSensorNo=sensorNo % simulationData.dynaSystem.dynaSensors.size();
 SIG_DynaSensor sensor=simulationData.dynaSystem.getSensor(modSensorNo);
 registers[0]->loadValue(sensor.senseJoint1());
 if (sensor.joint->joint->getJointType()==SIGEL_Robot::SIG_Joint::tCylindricalJoint)
  registers[1]->loadValue(sensor.senseJoint2());
};


QTime SIGEL_Simulation::SIG_DynaMoSimulationQueries::getActualSimulationTime() const
{
  double d=simulationData.simulationParameter.getStepSize()*simulationData.actualFrame*1000;
  QTime time(0,0,0,0);
  return time.addMSecs(static_cast<int>(floor(d)));
};

DL_vector SIGEL_Simulation::SIG_DynaMoSimulationQueries::getLinkPosition(int linkNo) const
{
  SIG_DynaLink &theLink=simulationData.dynaSystem.getLink(linkNo);
  DL_vector pos;
  theLink.position.tovector(&pos);
  simulationData.dynaSystem.foundNaN=false;
  DL_vector pos2=simulationData.dynaSystem.unNaN(pos);
  if (!simulationData.dynaSystem.foundNaN)
   return pos2;
  else
   return simulationData.environment.getStartPosition();
};

DL_matrix SIGEL_Simulation::SIG_DynaMoSimulationQueries::getLinkOrientation(int linkNo) const
{
  SIG_DynaLink &theLink=simulationData.dynaSystem.getLink(linkNo);
  DL_matrix orientation=theLink.orientation;
  return simulationData.dynaSystem.unNaN(orientation);
};

int SIGEL_Simulation::SIG_DynaMoSimulationQueries::getRootNumber() const
{
  return simulationData.dynaSystem.rootLinkNo;
};

vector<double*>* SIGEL_Simulation::SIG_DynaMoSimulationQueries::getUsedForces() const
{
  // has to be implemented properly!
  return 0;
};

int SIGEL_Simulation::SIG_DynaMoSimulationQueries::getLinkCount() const
{
  return simulationData.dynaSystem.dynaLinks.size();  
};

void SIGEL_Simulation::SIG_DynaMoSimulationQueries::checkDynas() const
{
  for(int i=0;i<getLinkCount();i++)
  {
   SIG_DynaLink &theLink=simulationData.dynaSystem.getLink(i);
   DL_vector pos;
   theLink.position.tovector(&pos);
   simulationData.dynaSystem.foundNaN=false;
   DL_vector pos2=simulationData.dynaSystem.unNaN(pos);
   if (simulationData.dynaSystem.foundNaN)
   { 
    QString msg("NaN Position found! ");
    msg+=theLink.link->getName();
    // throw SIG_SimulationCannotSolveException(__FILE__,__LINE__,msg);
    
   }; 
   DL_matrix orientation=theLink.orientation;
   simulationData.dynaSystem.foundNaN=false;
   DL_matrix orientation2=simulationData.dynaSystem.unNaN(orientation);
   if (simulationData.dynaSystem.foundNaN)
   { 
    QString msg("NaN Orientation found! ");
    msg+=theLink.link->getName();
    // throw SIG_SimulationCannotSolveException(__FILE__,__LINE__,msg);

   }; 
  };  
};

#ifdef SIG_DEBUG
void SIGEL_Simulation::SIG_DynaMoSimulationQueries::printDynaDatas() const
{
 int sz=getLinkCount();
 for (int i=0; i<sz; i++)
 {
  SIG_DynaLink &theLink=simulationData.dynaSystem.getLink(i);
  SIGEL_Tools::SIG_IO::cerr << "   --- Link " << i << " ------------------------\n";
  SIGEL_Tools::SIG_IO::cerr << theLink.link->getName() << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Dyna: " << theLink.dyna << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Masse: " << theLink.dyna->get_mass() << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Inertia X:" << static_cast<SIG_Dyna*>(theLink.dyna)->get_inertiatensor()->x
                               << " Y:" << static_cast<SIG_Dyna*>(theLink.dyna)->get_inertiatensor()->y
                               << " Z:" << static_cast<SIG_Dyna*>(theLink.dyna)->get_inertiatensor()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Vel X:" << theLink.dyna->get_velocity()->x
                               << " Y:" << theLink.dyna->get_velocity()->y
                               << " Z:" << theLink.dyna->get_velocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "AVel X:" << theLink.dyna->get_angvelocity()->x
                                << " Y:" << theLink.dyna->get_angvelocity()->y
                                << " Z:" << theLink.dyna->get_angvelocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Pos X:" << theLink.dyna->get_position()->x
                               << " Y:" << theLink.dyna->get_position()->y
                               << " Z:" << theLink.dyna->get_position()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Orient " << theLink.dyna->get_orientation()->c0.x
                               << " " << theLink.dyna->get_orientation()->c1.x
                               << " " << theLink.dyna->get_orientation()->c2.x << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << theLink.dyna->get_orientation()->c0.y
                               << " " << theLink.dyna->get_orientation()->c1.y
                               << " " << theLink.dyna->get_orientation()->c2.y << "\n";
  SIGEL_Tools::SIG_IO::cerr << " " << theLink.dyna->get_orientation()->c0.z
                               << " " << theLink.dyna->get_orientation()->c1.z
                               << " " << theLink.dyna->get_orientation()->c2.z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Next Vel X:" << theLink.dyna->get_next_velocity()->x
                               << " Y:" << theLink.dyna->get_next_velocity()->y
                               << " Z:" << theLink.dyna->get_next_velocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Next AVel X:" << theLink.dyna->get_next_angvelocity()->x
                                << " Y:" << theLink.dyna->get_next_angvelocity()->y
                                << " Z:" << theLink.dyna->get_next_angvelocity()->z << "\n";
  SIGEL_Tools::SIG_IO::cerr << "Next Pos X:" << theLink.dyna->get_next_position()->x
                               << " Y:" << theLink.dyna->get_next_position()->y
                               << " Z:" << theLink.dyna->get_next_position()->z << "\n";
 };
};

int SIGEL_Simulation::SIG_DynaMoSimulationQueries::getGPTestValue() const
{
 return simulationData.gptestvalue;
};
#endif
