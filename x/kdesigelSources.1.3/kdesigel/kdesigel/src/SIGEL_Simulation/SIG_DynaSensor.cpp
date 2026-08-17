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
#include "SIGEL_Simulation/SIG_DynaSensor.h"
#include <cmath>

DL_Scalar SIGEL_Simulation::SIG_DynaSensor::senseJoint1()
{
 switch (joint->joint->getJointType())
 {
  case SIGEL_Robot::SIG_Joint::tTranslationalJoint :
  {
   DL_point lFix,rFix,lFixB;
   joint->leftDyna->dyna->to_world(&joint->leftFix,&lFix);  
   joint->rightDyna->dyna->to_world(&joint->rightFix,&rFix);  
   DL_vector diff;
   lFix.minus(&rFix,&diff);
   return diff.norm();
   break;
  }; 
  case SIGEL_Robot::SIG_Joint::tRotationalJoint :
  {
   DL_point lFix,lFixB;
   joint->leftDyna->dyna->to_world(&joint->leftFix,&lFix);  
   joint->leftDyna->dyna->to_world(&joint->leftFixB,&lFixB);  
   DL_vector lUp,rUp,dir;
   joint->leftDyna->dyna->to_world(&joint->leftUp,&lUp);
   joint->rightDyna->dyna->to_world(&joint->rightUp,&rUp);
   lFix.minus(&lFixB,&dir);
   dir.normalize();
   lUp.normalize();
   rUp.normalize();
   DL_Scalar prod=lUp.inprod(&rUp);
#ifdef _WINDOWS
   double pi=4 * ::atan(1);
#else
   double pi=4*std::atan(1);
#endif
   DL_Scalar angle=acos(prod)*180/pi;
   DL_vector x;
   dir.crossprod(&lUp,&x);
   DL_Scalar sp=x.inprod(&rUp);
   if (sp>0)
    angle=360-angle;
   return angle;
   break;
  }; 
  case SIGEL_Robot::SIG_Joint::tCylindricalJoint :
  {
   DL_point lFix,rFix;
   joint->leftDyna->dyna->to_world(&joint->leftFix,&lFix);  
   joint->rightDyna->dyna->to_world(&joint->rightFix,&rFix);  
   DL_vector diff;
   lFix.minus(&rFix,&diff);
   return diff.norm();
   break;
  }; 
 };
 return 0;
};

DL_Scalar SIGEL_Simulation::SIG_DynaSensor::senseJoint2()
{
 switch (joint->joint->getJointType())
 {
  case SIGEL_Robot::SIG_Joint::tTranslationalJoint :
  {
   break;
  }; 
  case SIGEL_Robot::SIG_Joint::tRotationalJoint :
  {
   break;
  }; 
  case SIGEL_Robot::SIG_Joint::tCylindricalJoint :
  {
   DL_point lFix,lFixB;
   joint->leftDyna->dyna->to_world(&joint->leftFix,&lFix);  
   joint->leftDyna->dyna->to_world(&joint->leftFixB,&lFixB);  
   DL_vector lUp,rUp,dir;
   joint->leftDyna->dyna->to_world(&joint->leftUp,&lUp);
   joint->rightDyna->dyna->to_world(&joint->rightUp,&rUp);
   lFix.minus(&lFixB,&dir);
   dir.normalize();
   lUp.normalize();
   rUp.normalize();
   DL_Scalar prod=lUp.inprod(&rUp);
#ifdef _WINDOWS
   double pi=4 * ::atan(1);
#else
   double pi=4*std::atan(1);
#endif
   DL_Scalar angle=acos(prod)*180/pi;
   DL_vector x;
   dir.crossprod(&lUp,&x);
   DL_Scalar sp=x.inprod(&rUp);
   if (sp>0)
    angle=360-angle;
   return angle;
   break;
  }; 
 };
 return 0;
};

