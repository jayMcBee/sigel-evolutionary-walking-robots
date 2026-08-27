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

#ifdef _WINDOWS
#include <float.h>
#endif

#include "SIGEL_Simulation/SIG_DynaSystem.h"
#include "euler.h"
#include "doubleeuler.h"
#include "rungekutta2.h"
#include "rungekutta4.h"
#include "linehinge.h"
#include "connector.h"
#include "bar.h"
#include "pris.h"
#include "actuator_fv.h"
#include "ptp.h"
#include "math.h"
#include "SIGEL_Robot/SIG_GeometryIterator.h"
#include "SIGEL_Robot/SIG_Polygon.h"
#include "SOLID/solid.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Simulation/SIG_Dyna.h"
#include "SIGEL_Simulation/SIG_RotationalController.h"
#include "SIGEL_Simulation/SIG_TranslationalController.h"

bool SIGEL_Simulation::SIG_DynaSystem::foundNaN(false);

void SIGEL_Simulation::SIG_DynaSystem::doNewFrame()
{
 newFrame=true;
 dynaSystem->dynamics();
 checkCollisionPTPs(); 
};

void SIGEL_Simulation::SIG_DynaSystem::checkCollisionPTPs()
{
/* int sz=dynaLinks.size();
 for (int i=0; i<sz; i++)
  if (dynaLinks[i]!=0)
  {
   if (dynaLinks[i]->collptp!=0)
   {
    DL_vector rF,nV;
    DL_point dP;
    dynaLinks[i]->collptp->reactionforce(&rF);
    dynaLinks[i]->collptp->get_dyna_point(&dP);
    dynaLinks[i]->dyna->get_newvelocity(&dP,&nV); 
    if (nV.y>0)
    {
     rF.timesis(-1);
     dynaLinks[i]->dyna->applyforce(&dP,dynaLinks[i]->dyna,&rF);
     rF.assign(&environment.getGravity());
     rF.timesis(-2);
     dynaLinks[i]->dyna->applyforce(&dP,dynaLinks[i]->dyna,&rF);
#ifdef SIG_DEBUG
     SIGEL_Tools::SIG_IO::cerr << "----------------ptp deleted " << dynaLinks[i]->number << "\n";
#endif
     delete dynaLinks[i]->collptp;
     dynaLinks[i]->collptp=0;
    }; 
   }; 
  };  */
};


DL_matrix SIGEL_Simulation::SIG_DynaSystem::rotationMatrix(DL_vector axis, DL_Scalar angle)
{
#ifdef _WINDOWS
 double pi=4 * ::atan(1);
#else
 double pi=4*std::atan(1);
#endif
 DL_Scalar anglerad=angle*pi/180;
 DL_Scalar s=sin(angle);
 DL_Scalar c=cos(angle);
 DL_Scalar u=1-c;
 DL_matrix m;
 DL_vector v(&axis);
 
 m.c0.x=v.x*v.x*u+c;
 m.c0.y=v.x*v.y*u+v.z*s;
 m.c0.z=v.x*v.z*u-v.y*s;
 
 m.c1.x=v.y*v.x*u-v.z*s;
 m.c1.y=v.y*v.y*u+c;
 m.c1.z=v.y*v.z*u+v.x*s;
 
 m.c2.x=v.z*v.x*u+v.y*s;
 m.c2.y=v.z*v.y*u-v.x*s;
 m.c2.z=v.z*v.z*u+c;
 
 return m;
};      


DL_Scalar SIGEL_Simulation::SIG_DynaSystem::unNaN(DL_Scalar val)
{
 DL_Scalar v=0;
#ifdef _WINDOWS
 if (_isnan(val)) {
#else
 if (isnan(val)) {
#endif
  foundNaN=true;
  return v;
 }
 else
 {
  if (fabs(val)<0.0000001)
   return 0;
  else
   return val;
/*  if (fabs(val)>1000000)
   v=0;
  else
   v=val;
  return v; */
 };    
};

DL_vector SIGEL_Simulation::SIG_DynaSystem::unNaN(DL_vector val)
{
 DL_vector v(unNaN(val.x),unNaN(val.y),unNaN(val.z));
 return v;
};

DL_point SIGEL_Simulation::SIG_DynaSystem::unNaN(DL_point val)
{
 DL_point v(unNaN(val.x),unNaN(val.y),unNaN(val.z));
 return v;
};

DL_matrix SIGEL_Simulation::SIG_DynaSystem::unNaN(DL_matrix val)
{
 DL_vector vc0(unNaN(val.c0)),vc1(unNaN(val.c1)),vc2(unNaN(val.c2));
 DL_matrix v(&vc0,&vc1,&vc2);
 if(v.c0.norm()==0) v.c0.x=1;
 if(v.c1.norm()==0) v.c1.y=1;
 if(v.c2.norm()==0) v.c2.z=1;
 return v;
};

SIGEL_Simulation::SIG_DynaSystem::SIG_DynaSystem(SIGEL_Environment::SIG_Environment const & theEnvironment,
                                                 SIG_SimulationParameters const & theSimulationParameters)
 : environment(theEnvironment),
   simulationParameters(theSimulationParameters),
   QObject()
{
 rootLinkNo=-1;

 // Dynamo initialisieren
 dynaCallbacks=new SIG_DynaCallbacks();
 dynaCallbacks->dynaSystem=this;
  
 switch (simulationParameters.getIntegrator())
 {
  case SIG_SimulationParameters::itEuler       : mIntegrator=new DL_euler(); break;
  case SIG_SimulationParameters::itDoubleEuler : mIntegrator=new DL_double_euler(); break;
  case SIG_SimulationParameters::itRungeKutta2 : mIntegrator=new DL_rungekutta2(); break;
  default                                      : mIntegrator=new DL_rungekutta4();
 };

#ifdef SIG_DEBUG
 SIGEL_Tools::SIG_IO::cerr << "Gleich stuerzt er ab...\n";
#endif
 dynaSystem=new DL_dyna_system(dynaCallbacks,mIntegrator);
#ifdef SIG_DEBUG
 SIGEL_Tools::SIG_IO::cerr << "...doch nicht!\n";
#endif

 constraintManager=new DL_constraint_manager();

 DL_vector grav=environment.getGravity();
 dynaSystem->set_gravity(&grav);
 mIntegrator->set_stepsize(simulationParameters.getStepSize());
 constraintManager->max_error=simulationParameters.getStepSize();
 constraintManager->analytical=simulationParameters.getAnalytical();
 constraintManager->MaxIter=simulationParameters.getMaximalIterations();
 constraintManager->NrSkip=simulationParameters.getSkipFrames();
 constraintManager->max_collisionloops=simulationParameters.getMaximalCollisionLoops();
  
 switch (simulationParameters.getSolveMode())
 {
  case SIG_SimulationParameters::smtLUDecomposition   :constraintManager->solve_using_lud(); break;
  case SIG_SimulationParameters::smtConjugateGradient :constraintManager->solve_using_cg(); break;
  default                                             :constraintManager->solve_using_svd(); break;
 };
    
 //SOLID initialisieren
 dtEnableCaching();
 dtSetDefaultResponse(collisionResponse,DT_SMART_RESPONSE,stdout);

 initializeFloor();
};

SIGEL_Simulation::SIG_DynaSystem::~SIG_DynaSystem()
{
  //Shut down and destroy Dynamo
  clearAllDynamics();
  delete constraintManager;
  delete dynaSystem;
  delete mIntegrator;
  delete dynaCallbacks;
};

void SIGEL_Simulation::SIG_DynaSystem::initializeFloor()
{
 //A huge polygon is created as the base ground
 double y=environment.getYPlaneLevel();
  
 floor=dtNewComplexShape();
 dtBegin(DT_POLYGON);
 dtVertex(-1000,y,-1000);
 dtVertex(-1000,y, 1000);
 dtVertex( 1000,y, 1000);
 dtVertex( 1000,y,-1000);
 dtEnd();    
 dtEndComplexShape(); 
 dtCreateObject(0,floor);
};

void SIGEL_Simulation::SIG_DynaSystem::clearAllDynamics()
{
 rootLinkNo=-1;
 //Destroy all Dyna objects
 dtDeleteObject(0);
 int sz=dynaLinks.size();
 for (int i=0; i<sz; i++)
  if (dynaLinks[i]!=0)
  {
   dtDeleteObject(dynaLinks[i]);
   delete dynaLinks[i];
  };  
 sz=dynaJoints.size();
 for (int j=0; j<sz; j++)
  if (dynaJoints[j]!=0)
   delete dynaJoints[j];
 sz=dynaDrives.size();
 for (int k=0; k<sz; k++)
  if (dynaDrives[k]!=0)
   delete dynaDrives[k];   // 2003 deleted dynaJoints[k] here: every joint was
                           // freed twice and every drive leaked (D13)
 sz=dynaSensors.size();
 for (int l=0; l<sz; l++)
  if (dynaSensors[l]!=0)
   delete dynaSensors[l];
};

void SIGEL_Simulation::SIG_DynaSystem::collisionResponse(void * client_data,
							 DtObjectRef obj1,
							 DtObjectRef obj2,
							 const DtCollData *coll_data)
{
 //if (isnan(coll_data->normal[0]))
 // return;
    
 double min_geschwindigkeit=0;

 SIG_DynaLink* dlObj1=(SIG_DynaLink*)obj1;
 SIG_DynaLink* dlObj2=(SIG_DynaLink*)obj2;

 DL_point point1(coll_data->point1[0],
	         coll_data->point1[1],
      	         coll_data->point1[2]);
 DL_point point2(coll_data->point2[0],
                 coll_data->point2[1],
        	 coll_data->point2[2]);
 DL_vector normal(coll_data->normal[0],
     		  coll_data->normal[1],
     		  coll_data->normal[2]);
 normal.normalize();

 if ((dlObj1!=0)&&(dlObj2!=0))
 // Collision of two objects
 {
//  SIGEL_Tools::SIG_IO::cerr << "Kollision zweier Objekte\n";
  //If the velocity is large enough the collision is generated,
  // otherwise the velocity is cleared.
  if ((dlObj1->dyna->get_velocity()->norm()>min_geschwindigkeit)
    ||(dlObj2->dyna->get_velocity()->norm()>min_geschwindigkeit)
    ||(dlObj1->dyna->get_angvelocity()->norm()>min_geschwindigkeit)
    ||(dlObj2->dyna->get_angvelocity()->norm()>min_geschwindigkeit))
  //velocity large enough  
  {
//   SIGEL_Tools::SIG_IO::cerr << "-Geschwindigkeit gross genug\n";
   normal=unNaN(normal);
   if (normal.norm()==0)
    return;
//   SIGEL_Tools::SIG_IO::cerr << "N(w) x:" << normal.x << " y:" << normal.y << " z:" << normal.z << "\n";
       
    //Are the objects moving towards each other?
   DL_point p1,p2,p1n,p2n,p1w,p2w,p1nw,p2nw;
   DL_vector v1,v2,v1n,v2n,d,dn;
   p1=(dlObj1->dyna->get_position());
   p2=(dlObj2->dyna->get_position());
   p1n=(dlObj1->dyna->get_next_position());
   p2n=(dlObj2->dyna->get_next_position());
   p1.tovector(&v1);
   p2.tovector(&v2);
   p1n.tovector(&v1n);
   p2n.tovector(&v2n);
   d=v1;
   dn=v1n;
   d.minusis(&v2);
   dn.minusis(&v2n);
      
   if (d.norm()>dn.norm())
    //Duerfen sie ueberhaupt kollidieren?
    if (/*true*/(dlObj1->link->getNoCollides().find(dlObj2->link))!=-1)
    //collision permitted
    {
//     SIGEL_Tools::SIG_IO::cerr << "--Bewegen sich aufeinander zu und Kollision erlaubt\n";
     DL_vector F;

     SIG_Dyna* sd=dynamic_cast<SIG_Dyna*>(dlObj1->dyna); 
     F=sd->getPointForce(&point1);
        
     DL_Scalar Flength=F.inprod(&normal);
     DL_vector Fnormal(&normal);
 
     Fnormal.timesis(Flength);  //Normal force vector
     DL_vector Fortho(&F);
     Fortho.minusis(&Fnormal);  //Residual force vector
     DL_vector newFortho(&Fortho);
     // Force scaled by mu
     newFortho.normalize();
     DL_Scalar mue=dlObj1->link->getMaterial()->getFrictionValue(const_cast<SIGEL_Robot::SIG_Material*>(dlObj2->link->getMaterial()));
     DL_Scalar FRlength=Fnormal.norm()*mue;
     if (FRlength>Fortho.norm())
      FRlength=Fortho.norm();
     newFortho.timesis(-FRlength);
  
     dlObj1->dyna->applyforce(&point1,dlObj1->dyna,&newFortho);
     newFortho.timesis(-1);
     dlObj2->dyna->applyforce(&point2,dlObj2->dyna,&newFortho);
     DL_collision* collConstr=new DL_collision(dlObj1->dyna,&point1,dlObj2->dyna,&point2,&normal,2); 
    }; 
    //end: collision permitted
  }
  //end: velocity large enough 
  else
  //velocity too small
  { 
   DL_vector nullvektor(0,0,0);
   dlObj1->dyna->set_velocity(&nullvektor);
   dlObj2->dyna->set_velocity(&nullvektor);
   dlObj1->dyna->set_angvelocity(&nullvektor);
   dlObj2->dyna->set_angvelocity(&nullvektor);
  };
  //end: velocity too small
 } 
 //end: collision of two objects
 else
  if ((dlObj1==0)||(dlObj2==0))
 //Collision with the ground 
 {   
  //but if one of the pointers is 0 this was a collision with the ground.
  SIG_DynaLink* theDlObj;
  DL_point* thePoint;
  if (dlObj1==0)
  {
   theDlObj=dlObj2;
   thePoint=&point2;
  } 
  else
  {
   theDlObj=dlObj1;
   thePoint=&point1;
  }; 
    
  DL_point theWorldPoint,theWorldPointN;
  theDlObj->dyna->to_world(thePoint,&theWorldPoint);
  theDlObj->dyna->new_toworld(thePoint,&theWorldPointN);
  //specifically at the (local) point thePoint of the DynaLink theDlObj
  //The force still acting on the object must now be cleared, and
  //from the collision normal and gravity the new
  //counter-forces, less friction, must be computed.
  //For the interim goal a simple counter-force is sufficient.
  //If the velocity is large enough the collision is generated,
  // otherwise the velocity is cleared.
  if ((theDlObj->dyna->get_velocity()->norm()>min_geschwindigkeit)
    ||(theDlObj->dyna->get_angvelocity()->norm()>min_geschwindigkeit))
  //velocity large enough
  {
   //Is the object moving towards the ground?
   DL_point pw,pnw;
   theDlObj->dyna->to_world(thePoint,&pw);
   theDlObj->dyna->new_toworld(thePoint,&pnw);
   double y=pw.y;
   double yn=pnw.y;
   double yw=point2.y;
   normal.init(0,1,0);
   y=y-yw;
   yn=yn-yw;
   if ((y>yn))
   {
    DL_vector F;
    SIG_Dyna* sd=dynamic_cast<SIG_Dyna*>(theDlObj->dyna); 
    F=sd->getPointForce(thePoint);
 	  
    DL_Scalar Flength=F.inprod(&normal);
    DL_vector Fnormal(&normal);
 
    Fnormal.timesis(Flength);  //Normal force vector
    DL_vector Fortho(&F);
    Fortho.minusis(&Fnormal);  //Residual force vector
    DL_vector newFortho(&Fortho);
    // Counter-force scaled by mu
    newFortho.normalize();
    DL_Scalar mue=theDlObj->link->getMaterial()->getFrictionValue(theDlObj->floorMaterial);
    DL_Scalar FRlength=Fnormal.norm()*mue;
    if (FRlength>Fortho.norm())
     FRlength=Fortho.norm();
    newFortho.timesis(-FRlength);
	  
    theDlObj->dyna->applyforce(&theWorldPoint,0,&newFortho);
    DL_collision* collConstr=new DL_collision(theDlObj->dyna,thePoint,0,&theWorldPoint,&normal,1);
   };  
  }
  //end: velocity large enough
  else
  //velocity too small
  {
   DL_vector nullvektor(0,0,0);
   theDlObj->dyna->set_velocity(&nullvektor);
   theDlObj->dyna->set_angvelocity(&nullvektor);
  };
  //end: velocity too small
  /*
  if (theWorldPointN.y<0)
  {
   DL_vector F,M,Fp,V;
   theDlObj->dyna->get_velocity(thePoint,&V);
   F=dynamic_cast<SIG_Dyna*>(theDlObj->dyna)->getForce();
   Fp=dynamic_cast<SIG_Dyna*>(theDlObj->dyna)->getPointForce(thePoint);
   M=dynamic_cast<SIG_Dyna*>(theDlObj->dyna)->getMomentum();
/*#ifdef SIG_DEBUG
SIGEL_Tools::SIG_IO::cerr << "--------------coll data\n";
SIGEL_Tools::SIG_IO::cerr << "Point local X:" << thePoint->x << " Y:" << thePoint->y << " Z:" << thePoint->z << "\n";
SIGEL_Tools::SIG_IO::cerr << "Point world X:" << theWorldPoint.x << " Y:" << theWorldPoint.y << " Z:" << theWorldPoint.z << "\n";
SIGEL_Tools::SIG_IO::cerr << "Velocity X:" << V.x << " Y:" << V.y << " Z:" << V.z << "\n";
SIGEL_Tools::SIG_IO::cerr << "Momentum X:" << M.x << " Y:" << M.y << " Z:" << M.z << "\n";
SIGEL_Tools::SIG_IO::cerr << "Force X:" << F.x << " Y:" << F.y << " Z:" << F.z << "\n";
SIGEL_Tools::SIG_IO::cerr << "Force Point X:" << Fp.x << " Y:" << Fp.y << " Z:" << Fp.z << "\n";
#endif
   if ((theDlObj->collptp==0)&&(V.y<0))
   {
#ifdef SIG_DEBUG
    SIGEL_Tools::SIG_IO::cerr << "----------------ptp created " << theDlObj->number << "\n";
#endif
    theDlObj->collptp=new DL_ptp();
    theDlObj->collptp->init(theDlObj->dyna,thePoint,0,&theWorldPoint);
    theDlObj->collptp->soft();
    theDlObj->collptp->stiffness=0.01;
   }; 
  }; */
 };
 //End of ground collision handling
};

void SIGEL_Simulation::SIG_DynaSystem::msgFunction(QString theMessage)
{
  emit signalDynamoMessage(theMessage);
};

void SIGEL_Simulation::SIG_DynaSystem::doCollisionDetection()
{
 checkCollisionPTPs();
 //Solid aufrufen
  if (newFrame)
  {
   int j=dtTest();
   if (j==0)
    newFrame=false; 
  }
  else
   dtProceed();
};

void SIGEL_Simulation::SIG_DynaSystem::newLink(SIGEL_Robot::SIG_Link& theLink)
{
 SIGEL_Tools::SIG_IO::cerr << "Link " << theLink.getNumber() << " : " << theLink.getName() << "\n";
 const QList<SIGEL_Robot::SIG_Link::NamedPoint> &pts=theLink.getPoints();
 int n=theLink.getNrOfPoints();
 for (const SIGEL_Robot::SIG_Link::NamedPoint &np : pts) {
  DL_vector p=(np.value);
  SIGEL_Tools::SIG_IO::cerr << " " << np.name << " X:" << p.x << " Y:" << p.y << " Z:" << p.z << "\n";
 }


 //If it is the root link, register it
 if (theLink.isRootLink())
  rootLinkNo=theLink.getNumber();
 //Link in DynaLink umwandeln
 int sz=dynaLinks.size();
 DL_vector pos,startpos,inertia;
 DL_matrix orient2,orient; 
 

 // Holger's vars
 DL_Scalar m;   // (as for mass)
 DL_vector com; // (as for centre of mass)
 DL_matrix it,it2;  // (as for inertia tensor)
  
 theLink.getInitialLocation(pos,orient2);
 orient=unNaN(orient2);
 SIGEL_Tools::SIG_IO::cerr << " Pos X:" << pos.x << " Y:" << pos.y << " Z:" << pos.z << "\n";
 SIGEL_Tools::SIG_IO::cerr << " Orient: (" << orient.c0.x << "|" << orient.c1.x << "|" << orient.c2.x << ")\n";
 SIGEL_Tools::SIG_IO::cerr << "  (" << orient.c0.y << "|" << orient.c1.y << "|" << orient.c2.y << ")\n";
 SIGEL_Tools::SIG_IO::cerr << "  (" << orient.c0.z << "|" << orient.c1.z << "|" << orient.c2.z << ")\n";

 startpos=environment.getStartPosition();
 pos.plusis(&startpos);
  
 DL_point pos2;
 pos.topoint(&pos2);
 dynaLinks.resize(sz+1);
 dynaLinks.insert(sz,new SIG_DynaLink(unNaN(pos2),orient));
 dynaLinks[sz]->number=theLink.getNumber();
 dynaLinks[sz]->shape=dtNewComplexShape();
 dynaLinks[sz]->link=&theLink;
 dynaLinks[sz]->floorMaterial=floorMaterial;
 /* mal kurz weggemacht
   inertia=theLink.getInertiaVector();
   inertia=unNaN(inertia);
   if (inertia.x==0) inertia.x=1;
   if (inertia.y==0) inertia.y=1;
   if (inertia.z==0) inertia.z=1;
 */
 theLink.getPhysics (m, com, it2);
 it=unNaN(it2);
 if (it.c0.x==0) it.c0.x=1;
 if (it.c1.y==0) it.c1.y=1;
 if (it.c2.z==0) it.c2.z=1;
 m=unNaN(m);
 if (m==0) m=1;
 /*
 dynaLinks[sz]->dyna->set_inertiatensor(inertia.x,inertia.y,inertia.z);
 */
 dynaLinks[sz]->dyna->set_inertiatensor(it.get (0, 0),
                                        it.get (1, 1),
                                        it.get (2, 2));
 /*
 dynaLinks[sz]->dyna->set_mass(unNaN(theLink.getMass()));
 */
 dynaLinks[sz]->dyna->set_mass(m);
 dynaLinks[sz]->dyna->set_velodamping(unNaN(environment.getVeloDamping()));
 dynaLinks[sz]->dyna->set_elasticity(unNaN(theLink.getMaterial()->getElasticity()));
 /*
 SIGEL_Robot::SIG_Body const* body=theLink.getBody(); 
 SIGEL_Robot::SIG_Geometry const* geom=body->getGeometry();
 */
 SIGEL_Robot::SIG_Geometry const *geom=theLink.getGeometry();
 SIGEL_Robot::SIG_GeometryIterator iter(geom);
 while (iter.valid())
 {
  SIGEL_Robot::SIG_Polygon const & poly=iter.iterate();
     
  dtBegin(DT_POLYGON);
  int n=poly.getNumVertices();
      
  for (int i=0; i<n; i++)
  {
   DL_vector const & p=poly.getVertex(i);
   dtVertex(p.x,p.y,p.z);
  };  
  dtEnd();
 };
 
 dtEndComplexShape(); 
 dtCreateObject(dynaLinks[sz],(dynaLinks[sz]->shape));
};

void SIGEL_Simulation::SIG_DynaSystem::newJoint(SIGEL_Robot::SIG_Joint& theJoint)
{
 //Joint in dynaJoint eintragen
 int sz=dynaJoints.size();
 dynaJoints.resize(sz+1);
 dynaJoints.insert(sz,new SIG_DynaJoint());
  
 dynaJoints[sz]->number=theJoint.getNumber();

 SIGEL_Robot::SIG_Link const * lLink=theJoint.getLeftLink();
 SIGEL_Robot::SIG_Link const * rLink=theJoint.getRightLink();
 SIG_DynaLink & ldLink = getLink(lLink->getNumber());
 SIG_DynaLink & rdLink = getLink(rLink->getNumber());
 //Determine and create the matching DL_constraint
 switch (theJoint.getJointType()) {
 
  //------------- ROTATIONAL JOINT --------------------------------------------------  
  case SIGEL_Robot::SIG_Joint::tRotationalJoint :
  {
   //casten
   SIGEL_Robot::SIG_RotationalJoint & rotationalJoint = dynamic_cast<SIGEL_Robot::SIG_RotationalJoint&> (theJoint);
   //Evaluate the joint and create the constraint
   DL_linehinge * dynarot;
   dynarot=new DL_linehinge();
   //Read out the points
   DL_vector vlbase,vldir,vrbase,vrdir,vlhand,vrhand;
   vlbase=rotationalJoint.getLeftBase();
   vldir=rotationalJoint.getLeftDir();
   vlhand=rotationalJoint.getLeftHand();
   vrbase=rotationalJoint.getRightBase();
   vrdir=rotationalJoint.getRightDir();
   vrhand=rotationalJoint.getRightHand();
   //Adjust the distance between Dir1 and Dir2
   DL_vector nvldir(&vldir);
   nvldir.minusis(&vlbase);
   nvldir.normalize();
   nvldir.plusis(&vlbase);
   DL_vector nvrdir(&vrdir);
   nvrdir.minusis(&vrbase);
   nvrdir.normalize();
   nvrdir.plusis(&vrbase);
   //Create the constraint
   DL_point lbase,rbase,ldir,rdir;
   vlbase.topoint(&lbase);
   nvldir.topoint(&ldir);
   vrbase.topoint(&rbase);
   nvrdir.topoint(&rdir);
   dynarot->init(ldLink.dyna,&lbase,&ldir,rdLink.dyna,&rbase,&rdir);
   dynarot->soft();
   dynarot->stiffness=0.01;
   vlhand.minusis(&vlbase);
   vlhand.normalize();
   vrhand.minusis(&vrbase);
   vrhand.normalize();
   dynaJoints[sz]->joint=&theJoint;
   dynaJoints[sz]->constraint=dynarot;
   dynaJoints[sz]->leftDyna=&ldLink;
   dynaJoints[sz]->rightDyna=&rdLink;
   dynaJoints[sz]->leftFix.assign(&lbase);
   dynaJoints[sz]->leftFixB.assign(&ldir);
   dynaJoints[sz]->leftFixC.init(0,0,0);
   dynaJoints[sz]->leftDir.init(0,0,0);
   dynaJoints[sz]->leftUp.assign(&vlhand);
   dynaJoints[sz]->rightFix.assign(&rbase);
   dynaJoints[sz]->rightFixB.assign(&rdir);
   dynaJoints[sz]->rightFixC.init(0,0,0);
   dynaJoints[sz]->rightDir.init(0,0,0);
   dynaJoints[sz]->rightUp.assign(&vrhand);
   SIG_RotationalController * rc=new SIG_RotationalController();
   //rc->init(dynaJoints[sz],rotationalJoint.getMin(),rotationalJoint.getMax());
   dynaJoints[sz]->controller=rc;
   dynaJoints[sz]->deflection=0;
   break;
  };
  
  //------------- TRANSLATIONAL JOINT -----------------------------------------------  
  case SIGEL_Robot::SIG_Joint::tTranslationalJoint :
  {
   //casten
   SIGEL_Robot::SIG_TranslationalJoint & translationalJoint = dynamic_cast<SIGEL_Robot::SIG_TranslationalJoint&> (theJoint);
   //Evaluate the joint and create the constraint
   DL_pris * dynatrans;
   dynatrans=new DL_pris();
   //Read out the points
   DL_vector vlbase,vldir,vrbase,vrdir,vlfix,vrfix;
   vlbase=translationalJoint.getLeftBase();
   vldir=translationalJoint.getLeftDir();
   vlfix=translationalJoint.getLeftFix();
   vrbase=translationalJoint.getRightBase();
   vrdir=translationalJoint.getRightDir();
   vrfix=translationalJoint.getRightFix();
   //Convert the fix/dir points into vectors
   vldir.minusis(&vlbase);
   vlfix.minusis(&vlbase);
   vrdir.minusis(&vrbase);
   vrfix.minusis(&vrbase);
   //Create the constraint
   DL_point lbase,rbase,ldir,rdir,lfix,rfix;
   vlbase.topoint(&lbase);
   vrbase.topoint(&rbase);
   vldir.topoint(&ldir);
   vrdir.topoint(&rdir);
   vlfix.topoint(&lfix);
   vrfix.topoint(&rfix);
   dynatrans->init(ldLink.dyna,&lbase,&vldir,&vlfix,rdLink.dyna,&rbase,&vrdir,&vrfix);
   dynatrans->soft();
   dynatrans->stiffness=0.01;
   dynaJoints[sz]->joint=&theJoint;
   dynaJoints[sz]->constraint=dynatrans;
   dynaJoints[sz]->leftDyna=&ldLink;
   dynaJoints[sz]->rightDyna=&rdLink;
   dynaJoints[sz]->leftFix.assign(&lbase);
   dynaJoints[sz]->leftFixB.init(0,0,0);
   dynaJoints[sz]->leftFixC.init(0,0,0);
   dynaJoints[sz]->leftDir.assign(&vldir);
   dynaJoints[sz]->leftUp.assign(&vlfix);
   dynaJoints[sz]->rightFix.assign(&rbase);
   dynaJoints[sz]->rightFixB.init(0,0,0);
   dynaJoints[sz]->rightFixC.init(0,0,0);
   dynaJoints[sz]->rightDir.assign(&vrdir);
   dynaJoints[sz]->rightUp.assign(&vrfix);
   SIG_TranslationalController * tc=new SIG_TranslationalController();
   //tc->init(dynaJoints[sz],translationalJoint.getMin(),translationalJoint.getMax());
   dynaJoints[sz]->controller=tc;
   dynaJoints[sz]->deflection=0;
   break;
  };
  //------------- GLUE JOINT --------------------------------------------------------  
  case SIGEL_Robot::SIG_Joint::tGlueJoint :
  {
   //casten
   SIGEL_Robot::SIG_GlueJoint & glueJoint = dynamic_cast<SIGEL_Robot::SIG_GlueJoint&> (theJoint);
   //Evaluate the joint and create the constraint
   DL_connector * dynaglue;
   dynaglue=new DL_connector();
   //Read out the points
   DL_vector a1,a2,a3,b1,b2,b3;
   glueJoint.getPlaneA(a1,a2,a3);
   glueJoint.getPlaneB(b1,b2,b3);
   //Create the constraint
   DL_point pa1,pa2,pa3,pb1,pb2,pb3;
   a1.topoint(&pa1);
   a2.topoint(&pa2);
   a3.topoint(&pa3);
   b1.topoint(&pb1);
   b2.topoint(&pb2);
   b3.topoint(&pb3);
   dynaglue->init(ldLink.dyna,&pa1,&pa2,&pa3,rdLink.dyna,&pb1,&pb2,&pb3);
   dynaglue->soft();
   dynaglue->stiffness=0.01;
   dynaJoints[sz]->joint=&theJoint;
   dynaJoints[sz]->constraint=dynaglue;
   dynaJoints[sz]->leftDyna=&ldLink;
   dynaJoints[sz]->rightDyna=&rdLink;
   dynaJoints[sz]->leftFix.assign(&pa1);
   dynaJoints[sz]->leftFixB.assign(&pa2);
   dynaJoints[sz]->leftFixC.assign(&pa3);
   dynaJoints[sz]->leftDir.init(0,0,0);
   dynaJoints[sz]->leftUp.init(0,0,0);
   dynaJoints[sz]->rightFix.assign(&pb1);
   dynaJoints[sz]->rightFixB.assign(&pb2);
   dynaJoints[sz]->rightFixC.assign(&pb3);
   dynaJoints[sz]->rightDir.init(0,0,0);
   dynaJoints[sz]->rightUp.init(0,0,0);
   dynaJoints[sz]->controller=0;
   dynaJoints[sz]->deflection=0;
   break;
  };
  default :
  {
   dynaJoints[sz]->joint=0;  
   dynaJoints[sz]->constraint=0;
   dynaJoints[sz]->leftDyna=0;
   dynaJoints[sz]->rightDyna=0;
   dynaJoints[sz]->leftFix.init(0,0,0);
   dynaJoints[sz]->leftFixB.init(0,0,0);
   dynaJoints[sz]->leftFixC.init(0,0,0);
   dynaJoints[sz]->leftDir.init(0,0,0);
   dynaJoints[sz]->leftUp.init(0,0,0);
   dynaJoints[sz]->rightFix.init(0,0,0);
   dynaJoints[sz]->rightFixB.init(0,0,0);
   dynaJoints[sz]->rightFixC.init(0,0,0);
   dynaJoints[sz]->rightDir.init(0,0,0);
   dynaJoints[sz]->rightUp.init(0,0,0);
   dynaJoints[sz]->controller=0;
   dynaJoints[sz]->deflection=0;
  }; 
 };
};

void SIGEL_Simulation::SIG_DynaSystem::newSensor(SIGEL_Robot::SIG_Sensor& theSensor)
{
 //Sensor in dynaSensor eintragen
 int sz=dynaSensors.size();
 dynaSensors.resize(sz+1);
 dynaSensors.insert(sz,new SIG_DynaSensor());
  
 dynaSensors[sz]->number=theSensor.getNumber();
 dynaSensors[sz]->sensor=&theSensor;
 dynaSensors[sz]->joint=0;
};

void SIGEL_Simulation::SIG_DynaSystem::newDrive(SIGEL_Robot::SIG_Drive& theDrive)
{
 //Drive in dynaDrives eintragen
 int sz=dynaDrives.size();
 dynaDrives.resize(sz+1);
 dynaDrives.insert(sz,new SIG_DynaDrive());
  
 SIG_DynaJoint * djoint=&getJoint(theDrive.getJoint()->getNumber());
 dynaDrives[sz]->drive=&theDrive;
 dynaDrives[sz]->number=theDrive.getNumber();
 dynaDrives[sz]->joint=djoint;
};

SIGEL_Simulation::SIG_DynaLink& SIGEL_Simulation::SIG_DynaSystem::getLink(int number)
{
 //Fetch the DynaLink from the vector
 int sz=dynaLinks.size();
 for (int i=0; i<sz; i++)
  if (dynaLinks[i]->number==number)
  {
   return *(dynaLinks[i]);
  };
 throw SIG_DynaSystemWrongNumberException(__FILE__,__LINE__,"SIG_DynaLink with this number doesn't exist");
};

SIGEL_Simulation::SIG_DynaJoint& SIGEL_Simulation::SIG_DynaSystem::getJoint(int number)
{
 //Fetch the DynaJoint from the vector
 int sz=dynaJoints.size();
 for (int i=0; i<sz; i++)
  if (dynaJoints[i]->number==number)
   return *(dynaJoints[i]);
 throw SIG_DynaSystemWrongNumberException(__FILE__,__LINE__,"SIG_DynaJoint with this number doesn't exist");
};

SIGEL_Simulation::SIG_DynaDrive& SIGEL_Simulation::SIG_DynaSystem::getDrive(int number)
{
 //Fetch the DynaDrive from the vector
 int sz=dynaDrives.size();
 for (int i=0; i<sz; i++)
  if (dynaDrives[i]->number==number)
   return *(dynaDrives[i]);
 throw SIG_DynaSystemWrongNumberException(__FILE__,__LINE__,"SIG_DynaDrive with this number doesn't exist");
};

SIGEL_Simulation::SIG_DynaSensor& SIGEL_Simulation::SIG_DynaSystem::getSensor(int number)
{
 //Fetch the DynaSensor from the vector
 int sz=dynaSensors.size();
 for (int i=0; i<sz; i++)
  if (dynaSensors[i]->number==number)
   return *(dynaSensors[i]);
 throw SIG_DynaSystemWrongNumberException(__FILE__,__LINE__,"SIG_DynaSensor with this number doesn't exist");
};


