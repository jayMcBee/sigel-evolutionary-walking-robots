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
#include "SIGEL_Simulation/SIG_TranslationalController.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Simulation/SIG_Dyna.h"
#include <cmath>

SIGEL_Simulation::SIG_TranslationalController::SIG_TranslationalController()
{
 minimum=0;
 maximum=0;
 joint=0;
};

SIGEL_Simulation::SIG_TranslationalController::~SIG_TranslationalController()
{
};

void SIGEL_Simulation::SIG_TranslationalController::init(SIG_DynaJoint * theJoint, DL_Scalar theMinimum, DL_Scalar theMaximum)
{
 minimum=theMinimum;
 maximum=theMaximum;
 joint=theJoint;
 activate();
};

void SIGEL_Simulation::SIG_TranslationalController::calculate_and_apply()
{
  /* The distance between the two fix-vectors must now be measured in world coordinates,
   * after which it should be checked whether one of the limits has been exceeded.
   * Finally a force should act on the two Dynas 
   * This force is computed somehow from the velocity acting at the two points
   */
 DL_point lFix,rFix,nextlFix,nextrFix;
 joint->leftDyna->dyna->to_world(&joint->leftFix,&lFix);  
 joint->rightDyna->dyna->to_world(&joint->rightFix,&rFix);  
 joint->leftDyna->dyna->new_toworld(&joint->leftFix,&nextlFix);  
 joint->rightDyna->dyna->new_toworld(&joint->rightFix,&nextrFix);  
 DL_vector diff,nextdiff;
 lFix.minus(&rFix,&diff);
 nextlFix.minus(&nextrFix,&nextdiff);
 double deflect=diff.norm();
 double nextdeflect=nextdiff.norm();

#ifdef SIG_DEBUG 
 SIGEL_Tools::SIG_IO::cerr << "Abstand " << deflect << " Einheiten.\n";
#endif 

 DL_point llFix,lrFix;
 joint->leftDyna->dyna->to_local(&lFix,0,&llFix);
 joint->rightDyna->dyna->to_local(&rFix,0,&lrFix);
 DL_vector forcel=(dynamic_cast<SIG_Dyna*>(joint->leftDyna->dyna))->getPointForce(&llFix);
 DL_vector forcer=(dynamic_cast<SIG_Dyna*>(joint->rightDyna->dyna))->getPointForce(&lrFix);
 DL_vector wforcel,wforcer;
 joint->leftDyna->dyna->to_world(&forcel,&wforcel);
 joint->rightDyna->dyna->to_world(&forcer,&wforcer);
 wforcel.inprod(&diff);
 wforcer.inprod(&diff);
 DL_Scalar nforcel=wforcel.norm();
 DL_Scalar nforcer=wforcer.norm();
 DL_Scalar reagent=(nforcel+nforcer);
 DL_Scalar border=(maximum-minimum)*0.0;
 
 if (deflect<(minimum+border))
 {
  DL_Scalar faktor=reagent*((minimum+border)-deflect);
  if (nextdeflect>deflect)
   faktor=faktor/2;
#ifdef SIG_DEBUG 
 SIGEL_Tools::SIG_IO::cerr << "#### ZU KLEIN ###################################################\n";
#endif 
  diff.normalize();
  diff.timesis(faktor);
  joint->leftDyna->dyna->applyforce(&llFix,joint->leftDyna->dyna,&diff);
  diff.timesis(-1);
  joint->rightDyna->dyna->applyforce(&lrFix,joint->rightDyna->dyna,&diff);
 }; 
 if (deflect>(maximum-border))
 {
  DL_Scalar faktor=reagent*(deflect-(maximum-border));
  if (nextdeflect<deflect)
   faktor=faktor/2;
#ifdef SIG_DEBUG 
 SIGEL_Tools::SIG_IO::cerr << "#### ZU GROSS ###################################################\n";
#endif 
  diff.normalize();
  diff.timesis(faktor);
  joint->rightDyna->dyna->applyforce(&lrFix,joint->rightDyna->dyna,&diff);
  diff.timesis(-1);
  joint->leftDyna->dyna->applyforce(&llFix,joint->leftDyna->dyna,&diff);
 };
};
