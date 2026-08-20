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
#include "SIGEL_Simulation/SIG_DynaMoCommandInterface.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "pointvector.h"
#include <cmath>

SIGEL_Simulation::SIG_DynaMoCommandInterface::SIG_DynaMoCommandInterface(SIG_DynaMoSimulationData& theSimulationData)
  : simulationData(theSimulationData)
{ };

void SIGEL_Simulation::SIG_DynaMoCommandInterface::moveDrive
(int driveNo,
 Q2PtrVector<SIG_Register> const& registers)
{
#ifdef SIG_DEBUG
  simulationData.gptestvalue=registers[0]->getValue();
#endif

 if (simulationData.dynaSystem.dynaDrives.size()==0) return;
 int modDriveNo=driveNo % simulationData.dynaSystem.dynaDrives.size();
 SIG_DynaDrive drive=simulationData.dynaSystem.getDrive(modDriveNo);
  
 DL_Scalar maxforce=drive.drive->getMaxForce();
 DL_Scalar minforce=drive.drive->getMinForce();
 DL_Scalar regsize1=registers[0]->getMaxValue();
 DL_Scalar regsize2=registers[1]->getMaxValue();
 DL_Scalar force1=registers[0]->getValue()*maxforce/regsize1;
 DL_Scalar force2=registers[1]->getValue()*maxforce/regsize2;
 if (fabs(force1)<minforce) force1=minforce*force1/fabs(force1);
 if (fabs(force2)<minforce) force2=minforce*force2/fabs(force2);
 
 drive.applyForce(force1,force2);
 
};
