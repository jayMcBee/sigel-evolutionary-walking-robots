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
#include "SIGEL_Simulation/SIG_DynaDrive.h"
#include "SIGEL_Simulation/SIG_Dyna.h"
#include "SIGEL_Tools/SIG_IO.h"

void SIGEL_Simulation::SIG_DynaDrive::applyForce(DL_Scalar f1, DL_Scalar f2)
{
 SIGEL_Tools::SIG_IO::cerr << "Move " << number << " (" << f1 << "," << f2 << ")\n";
 
 if (joint==0) return;
 DL_dyna * ldyna=joint->leftDyna->dyna;
 DL_dyna * rdyna=joint->rightDyna->dyna;
 switch(drive->getJoint()->getJointType())
 {
  case SIGEL_Robot::SIG_Joint::tTranslationalJoint :
  {
   if (f1==0) return;
   DL_point lpoint(&joint->leftFix);
   DL_vector ldir(&joint->leftDir);
   ldir.normalize();
   ldir.timesis(f1);

   DL_point rpoint(&joint->rightFix);
   DL_vector rdir(&joint->rightDir);
   rdir.normalize();
   rdir.timesis(-f1);
   
   ldyna->applyforce(&lpoint,ldyna,&ldir);
   rdyna->applyforce(&rpoint,rdyna,&rdir);
   break;
  };
  case SIGEL_Robot::SIG_Joint::tRotationalJoint :
  {
   if (f1==0) return;
   
   DL_point lpoint(&joint->leftFix);
   DL_vector lup(&joint->leftUp);
   lup.normalize();
   lpoint.plusis(&lup);
   DL_point ldirp(&joint->leftFixB);
   DL_vector ldir;
   ldirp.minus(&lpoint,&ldir);
   ldir.normalize();
   DL_vector lfdir;
   ldir.crossprod(&lup,&lfdir);
   lfdir.timesis(f1);
   
   DL_point rpoint(&joint->rightFix);
   DL_vector rup(&joint->rightUp);
   rup.normalize();
   rpoint.plusis(&rup);
   DL_point rdirp(&joint->rightFixB);
   DL_vector rdir;
   rdirp.minus(&rpoint,&rdir);
   rdir.normalize();
   DL_vector rfdir;
   rdir.crossprod(&rup,&rfdir);
   rfdir.timesis(-f1);
   
   ldyna->applyforce(&lpoint,ldyna,&lfdir);
   rdyna->applyforce(&rpoint,rdyna,&rfdir);
   break;
  };
  case SIGEL_Robot::SIG_Joint::tCylindricalJoint :
  {
   if ((f1==0)&&(f2==0)) return;
   {
    DL_point lpoint(&joint->leftFix);
    DL_vector ldir(&joint->leftDir);
    ldir.normalize();
    ldir.timesis(f1); 
 
    DL_point rpoint(&joint->rightFix);
    DL_vector rdir(&joint->rightDir);
    rdir.normalize();
    rdir.timesis(-f1);
    
    ldyna->applyforce(&lpoint,ldyna,&ldir);
    rdyna->applyforce(&rpoint,rdyna,&rdir);
   };
   {
    DL_point lpoint(&joint->leftFix);
    DL_vector lup(&joint->leftUp);
    lup.normalize();
    lpoint.plusis(&lup);
    DL_point ldirp(&joint->leftFixB);
    DL_vector ldir;
    ldirp.minus(&lpoint,&ldir);
    ldir.normalize();
    DL_vector lfdir;
    ldir.crossprod(&lup,&lfdir);
    lfdir.timesis(f2);
   
    DL_point rpoint(&joint->rightFix);
    DL_vector rup(&joint->rightUp);
    rup.normalize();
    rpoint.plusis(&rup);
    DL_point rdirp(&joint->rightFixB);
    DL_vector rdir;
    rdirp.minus(&rpoint,&rdir);
    rdir.normalize();
    DL_vector rfdir;
    rdir.crossprod(&rup,&rfdir);
    rfdir.timesis(-f2);
   
    ldyna->applyforce(&lpoint,ldyna,&lfdir);
    rdyna->applyforce(&rpoint,rdyna,&rfdir);
   };
   break;
  };
 }; 
};

