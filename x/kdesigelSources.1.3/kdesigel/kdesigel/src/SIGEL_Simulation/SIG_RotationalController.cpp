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
#include "SIGEL_Simulation/SIG_RotationalController.h"
#include "SIGEL_Simulation/SIG_Dyna.h"
#include "pointvector.h"
#include <cmath>
#include "SIGEL_Tools/SIG_IO.h"

SIGEL_Simulation::SIG_RotationalController::SIG_RotationalController()
{
 minimum=0;
 maximum=0;
 joint=0;
};

SIGEL_Simulation::SIG_RotationalController::~SIG_RotationalController()
{
};

void SIGEL_Simulation::SIG_RotationalController::init(SIG_DynaJoint * theJoint, DL_Scalar theMinimum, DL_Scalar theMaximum)
{
 minimum=theMinimum;
 maximum=theMaximum;
 joint=theJoint;
 activate();
};

void SIGEL_Simulation::SIG_RotationalController::calculate_and_apply()
{
  /* The angle between the two up-vectors must now be measured in world coordinates,
   * after which it should be checked whether one of the limits has been exceeded.
   * Finally a force should act on the two Dynas 
   * This force is computed somehow from the velocity acting at the two points
   */

 /* Daten auslesen */
 DL_vector lUp,rUp,dir,nextlUp,nextrUp,nextdir;
 DL_point fix,fixB,nextfix,nextfixB;
 joint->leftDyna->dyna->to_world(&joint->leftUp,&lUp);
 joint->rightDyna->dyna->to_world(&joint->rightUp,&rUp);
 joint->rightDyna->dyna->to_world(&joint->rightFix,&fix);
 joint->rightDyna->dyna->to_world(&joint->rightFixB,&fixB);
 fixB.minus(&fix,&dir);
 lUp.normalize();
 rUp.normalize();
 dir.normalize();
 joint->leftDyna->dyna->new_toworld(&joint->leftUp,&nextlUp);
 joint->rightDyna->dyna->new_toworld(&joint->rightUp,&nextrUp);
 joint->rightDyna->dyna->new_toworld(&joint->rightFix,&nextfix);
 joint->rightDyna->dyna->new_toworld(&joint->rightFixB,&nextfixB);
 nextfixB.minus(&nextfix,&nextdir);
 nextlUp.normalize();
 nextrUp.normalize();
 nextdir.normalize();
 
 /* Compute the angle */
 DL_Scalar prod=lUp.inprod(&rUp);
 DL_Scalar nextprod=nextlUp.inprod(&nextrUp);
#ifdef _WINDOWS
 double pi=4 * ::atan(1);
#else
 double pi=4*std::atan(1);
#endif
 double angle=acos(prod)*180/pi;
 double nextangle=acos(nextprod)*180/pi;
 
 /* Determine whether the angle is greater than 180 degrees */
 DL_vector x,nextx;
 dir.crossprod(&lUp,&x);
 DL_Scalar sp=x.inprod(&rUp);
 if (sp>0)
  angle=360-angle;
 nextdir.crossprod(&nextlUp,&nextx);
 DL_Scalar nextsp=nextx.inprod(&nextrUp);
 if (nextsp>0)
  nextangle=360-nextangle;
 
 /* Compute the point of application and direction of the counter-force */
 DL_point lpoint(&joint->leftFix);
 DL_vector lup(&joint->leftUp);
 lup.normalize();
 lpoint.plusis(&lup);
 
 DL_point lFix(&joint->leftFix);
 DL_point lFixB(&joint->leftFixB);
 DL_vector ldir;
 lFixB.minus(&lFix,&ldir);
 ldir.normalize();
 
 DL_vector lfdir;
 ldir.crossprod(&lup,&lfdir);
 lfdir.normalize();
 
 DL_point rpoint(&joint->rightFix);
 DL_vector rup(&joint->rightUp);
 rup.normalize();
 rpoint.plusis(&rup);
 
 DL_point rFix(&joint->rightFix);
 DL_point rFixB(&joint->rightFixB);
 DL_vector rdir;
 rFixB.minus(&rFix,&rdir);
 rdir.normalize();
 
 DL_vector rfdir;
 rdir.crossprod(&rup,&rfdir);
 rfdir.normalize();

 DL_Scalar newMax=maximum;
 if (minimum>maximum)
 {
  newMax=newMax+360;
  if (angle<180)
   angle=angle+360;
  if (nextangle<180)
   nextangle=nextangle+360;
 };
 
#ifdef SIG_DEBUG 
 SIGEL_Tools::SIG_IO::cerr << "Auslenkung Min:"<< minimum <<" Akt:" << angle << " Max:" << newMax <<"\n";
#endif 

 DL_point llpoint,lrpoint;
 joint->leftDyna->dyna->to_local(&lpoint,0,&llpoint);
 joint->rightDyna->dyna->to_local(&rpoint,0,&lrpoint);
 DL_vector llfdir,lrfdir;
 joint->leftDyna->dyna->to_local(&lfdir,0,&llfdir);
 joint->rightDyna->dyna->to_local(&rfdir,0,&lrfdir);
 
 DL_vector forcel=(dynamic_cast<SIG_Dyna*>(joint->leftDyna->dyna))->getPointForce(&llpoint);
 DL_vector forcer=(dynamic_cast<SIG_Dyna*>(joint->rightDyna->dyna))->getPointForce(&lrpoint);
 forcel.inprod(&llfdir);
 forcer.inprod(&lrfdir);
 DL_Scalar nforcel=forcel.norm();
 DL_Scalar nforcer=forcer.norm();
 DL_Scalar reagent=(nforcel+nforcer);
 DL_Scalar border=(newMax-minimum)*0.0;
 
 if (angle<(minimum+border))
 {
  DL_Scalar faktor=((minimum+border)-angle)*reagent;
  if (nextangle>angle)
   faktor=faktor/2;
  lfdir.timesis(faktor);
  rfdir.timesis(-faktor);
#ifdef SIG_DEBUG 
 SIGEL_Tools::SIG_IO::cerr << "#### ZU KLEIN ###################################################\n";
 SIGEL_Tools::SIG_IO::cerr << "Gegenkraft mit:" << lfdir.norm() <<"\n";
#endif 
  joint->leftDyna->dyna->applyforce(&llpoint,joint->leftDyna->dyna,&lfdir); 
  joint->rightDyna->dyna->applyforce(&lrpoint,joint->rightDyna->dyna,&rfdir);
 };
 if (angle>(newMax-border))
 {
  DL_Scalar faktor=(angle-(newMax-border))*reagent;
  if (nextangle<angle)
   faktor=faktor/2;
  lfdir.timesis(-faktor);
  rfdir.timesis(faktor);
#ifdef SIG_DEBUG 
 SIGEL_Tools::SIG_IO::cerr << "#### ZU GROSS ###################################################\n";
 SIGEL_Tools::SIG_IO::cerr << "Gegenkraft mit:" << lfdir.norm() <<"\n";
#endif 
  joint->leftDyna->dyna->applyforce(&llpoint,joint->leftDyna->dyna,&lfdir);
  joint->rightDyna->dyna->applyforce(&lrpoint,joint->rightDyna->dyna,&rfdir);
 };
};
