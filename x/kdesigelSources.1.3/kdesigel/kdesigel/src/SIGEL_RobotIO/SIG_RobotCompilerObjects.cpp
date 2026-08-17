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
#include "SIGEL_RobotIO/SIG_RobotCompilerObjects.h"
#include "SIGEL_RobotIO/SIG_RobotIOExceptions.h"
#include "SIGEL_Robot/SIG_Body.h"

using SIGEL_Robot::SIG_Body;

namespace SIGEL_RobotIO {
        SIG_RobotCompilerObjects::SIG_RobotCompilerObjects
        (SIG_RobotScanner &sc, SIG_Robot *tg, QString homepath)
                : SIG_RobotCompiler (sc, tg, homepath),
                  linknumber (0),
                  jointnumber (0),
                  drivenumber (0),
                  sensornumber (0)
        { }
        
        SIG_RobotCompilerObjects::~SIG_RobotCompilerObjects (void)
        { }

        SIG_Material *SIG_RobotCompilerObjects::materialFind (QString name)
        {
                if (target->lookupMaterial (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate material name '"+name+"'");
                SIG_Material *m = new SIG_Material (target, name);
                target->addMaterial (m);
                return m;
        }

        void SIG_RobotCompilerObjects::materialDensity (SIG_Material *material,
                                                        DL_Scalar density)
        {
                material->setDensity (density);
        }

        void SIG_RobotCompilerObjects::materialFriction (SIG_Material *material,
                                                         QString otherSide,
                                                         DL_Scalar frictionconst)
        {
                // will be done in pass 2
        }

        void SIG_RobotCompilerObjects::materialElasticity (SIG_Material *material,
                                                           DL_Scalar elasconst)
        {
                material->setElasticity (elasconst);
        }

        void SIG_RobotCompilerObjects::materialColour (SIG_Material *material,
                                                       DL_Scalar red,
                                                       DL_Scalar green,
                                                       DL_Scalar blue)
        {
                material->setColour (DL_vector (red, green, blue));
        }

        void SIG_RobotCompilerObjects::materialFinish (SIG_Material *material)
        {
                // nuffing to be dunn
        }

        SIG_Link *SIG_RobotCompilerObjects::linkFind (QString name)
        {
                if (target->lookupLink (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate link name '"+name+"'");
                SIG_Link *l = new SIG_Link (target, name, linknumber++);
                target->addLink (l);
                return l;
        }

        void SIG_RobotCompilerObjects::linkIsTorso (SIG_Link *link)
        {
                if ((target->getRootLink () != 0) &&
                    (target->getRootLink () != link))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Two different links are marked as torsos");
                target->setRootLink (link);
        }

        void SIG_RobotCompilerObjects::linkGeometryFile (SIG_Link *link,
                                                         QString geometryfile)
        {
                SIG_Body *b;
                if ((b = target->lookupBody (geometryfile)) == 0) {
                        b = new SIG_Body (target, geometryfile, dirprefix);
                        target->addBody (b);
                }
                link->setBody (b);
                b->addUsingLink (link);
        }

        void SIG_RobotCompilerObjects::linkMaterial (SIG_Link *link,
                                                     QString material)
        {
                // will be done in ...Structure
        }

        void SIG_RobotCompilerObjects::linkPoint (SIG_Link *link,
                                                  QString pointname,
                                                  DL_Scalar x,
                                                  DL_Scalar y,
                                                  DL_Scalar z)
        {
                link->addPoint (pointname, DL_vector (x, y, z));
        }

        void SIG_RobotCompilerObjects::linkNoCollide (SIG_Link *link,
                                                      QString nocollide)
        {
                // will be done in ...Structure
        }

        void SIG_RobotCompilerObjects::linkFinish (SIG_Link *link)
        {
                // nothing to be done.
        }

        SIG_RotationalJoint *SIG_RobotCompilerObjects::rjointFind (QString name)
        {
                if (target->lookupJoint (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate joint name '"+name+"'");
                SIG_RotationalJoint *j = new SIG_RotationalJoint (target, name,
                                                                  jointnumber++);
                target->addJoint (j);
                return j;
        }

        void SIG_RobotCompilerObjects::rjointLinking (SIG_RotationalJoint *rj,
                                                      QString Alink, QString AB, QString AD, QString AH,
                                                      QString Blink, QString BB, QString BD, QString BH)
        {
                // This will be completely done in ...Structure.
        }

        void SIG_RobotCompilerObjects::rjointExtents (SIG_RotationalJoint *rj,
                                                      DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                rj->setRange (mn, mx, ii);
        }

        void SIG_RobotCompilerObjects::rjointFinish (SIG_RotationalJoint *rj)
        {
                // nothing to be done for finish.
        }
        
        SIG_TranslationalJoint *SIG_RobotCompilerObjects::tjointFind (QString name)
        {
                if (target->lookupJoint (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate joint name '"+name+"'");
                SIG_TranslationalJoint *j = new SIG_TranslationalJoint (target, name,
                                                                        jointnumber++);
                target->addJoint (j);
                return j;
        }
        
        void SIG_RobotCompilerObjects::tjointLinking (SIG_TranslationalJoint *tj,
                                                      QString Alink, QString AB, QString AD, QString AF,
                                                      QString Blink, QString BB, QString BD, QString BF)
        {
                // This will be done in pass 2.
        }
        
        void SIG_RobotCompilerObjects::tjointExtents (SIG_TranslationalJoint *tj,
                                                      DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                tj->setRange (mn, mx, ii);
        }
        
        void SIG_RobotCompilerObjects::tjointFinish (SIG_TranslationalJoint *tj)
        {
                // Nothing to do.
        }
        
        SIG_CylindricalJoint *SIG_RobotCompilerObjects::cjointFind (QString name)
        {
                if (target->lookupJoint (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate joint name '"+name+"'");
                SIG_CylindricalJoint *cj = new SIG_CylindricalJoint (target, name,
                                                                     jointnumber++);
                target->addJoint (cj);
                return cj;
        }
        
        void SIG_RobotCompilerObjects::cjointLinking (SIG_CylindricalJoint *cj,
                                                      QString Alink, QString AB, QString AD, QString AH,
                                                      QString Blink, QString BB, QString BD, QString BH)
        {
                // Interobject thingies will be done in pass 2.
        }
        
        void SIG_RobotCompilerObjects::cjointRotExtents (SIG_CylindricalJoint *cj,
                                                         DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                cj->setRotationalRange (mn, mx, ii);
        }
        
        void SIG_RobotCompilerObjects::cjointTraExtents (SIG_CylindricalJoint *cj,
                                                         DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                cj->setTranslationalRange (mn, mx, ii);
        }
        
        void SIG_RobotCompilerObjects::cjointFinish (SIG_CylindricalJoint *cj)
        {
                // nuffin too doo.
        }

        SIG_GlueJoint *SIG_RobotCompilerObjects::glueFind (QString name)
        {
                if (target->lookupJoint (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate joint name '"+name+"'");
                SIG_GlueJoint *gj = new SIG_GlueJoint (target, name, jointnumber++);
                target->addJoint (gj);
                return gj;
        }
        
        void SIG_RobotCompilerObjects::glueLinking (SIG_GlueJoint *glue,
                                                    QString Alink, QString AA, QString AB, QString AC,
                                                    QString Blink, QString BP, QString BQ, QString BR)
        {
                // There is no point in doing this in pass 1.
        }

        void SIG_RobotCompilerObjects::glueFinish (SIG_GlueJoint *glue)
        {
                // Nothing to do.
        }

        SIG_Drive *SIG_RobotCompilerObjects::driveFind (QString name)
        {
                if (target->lookupDrive (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate drive name '"+name+"'");
                SIG_Drive *d = new SIG_Drive (target, name, drivenumber++);
                target->addDrive (d);
                return d;
        }

        void SIG_RobotCompilerObjects::driveMode (SIG_Drive *d, QString mode)
        {
                if (mode == "force")
                        d->setMode (SIG_Drive::tForceMode);
                else if (mode == "relative")
                        d->setMode (SIG_Drive::tRelativeMode);
                else if (mode == "absolute")
                        d->setMode (SIG_Drive::tAbsoluteMode);
                else if (mode == "simpleservo")
                        d->setMode (SIG_Drive::tServoSimpleMode);
                else
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Can't interpret drive mode '"+mode+"'");
        }
        
        void SIG_RobotCompilerObjects::driveJoint (SIG_Drive *d, QString jointname)
        {
                // this connects objects => to do in pass 2
        }

        void SIG_RobotCompilerObjects::driveMinMaxForce (SIG_Drive *d,
                                                         DL_Scalar minf, DL_Scalar maxf)
        {
                d->setForces (minf, maxf);
        }
        
        void SIG_RobotCompilerObjects::driveFinish (SIG_Drive *d)
        {
                // nothing left to do
        }
        
        SIG_JointSensor *SIG_RobotCompilerObjects::jointSensorFind (QString name)
        {
                if (target->lookupSensor (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate sensor name '"+name+"'");
                SIG_JointSensor *js = new SIG_JointSensor (target, name, sensornumber++);
                target->addSensor (js);
                return js;
        }

        void SIG_RobotCompilerObjects::jointSensorJoint (SIG_JointSensor *js, QString jointname)
        {
                // this will be left for pass 2
        }

        void SIG_RobotCompilerObjects::jointSensorFinish (SIG_JointSensor *js)
        {
                // there is nothing to do
        }

        SIG_PitchRollSensor *SIG_RobotCompilerObjects::pitchRollSensorFind (QString name)
        {
                if (target->lookupSensor (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate sensor name '"+name+"'");
                SIG_PitchRollSensor *prs = new SIG_PitchRollSensor (target, name, sensornumber++);
                target->addSensor (prs);
                return prs;
        }

        void SIG_RobotCompilerObjects::pitchRollSensorLink(SIG_PitchRollSensor *prs, QString linkname)
        {
                // this will be left for pass 2
        }

        void SIG_RobotCompilerObjects::pitchRollSensorFinish (SIG_PitchRollSensor *prs)
        {
                // there is nothing to do
        }


        SIG_ContactSensor *SIG_RobotCompilerObjects::contactSensorFind (QString name)
        {
                if (target->lookupSensor (name))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Duplicate sensor name '"+name+"'");
                SIG_ContactSensor *cs = new SIG_ContactSensor (target, name, sensornumber++);
                target->addSensor (cs);
                return cs;
        }

        void SIG_RobotCompilerObjects::contactSensorLink(SIG_ContactSensor *cs, QString linkname)
        {
                // this will be left for pass 2
        }

        void SIG_RobotCompilerObjects::contactSensorFinish (SIG_ContactSensor *cs)
        {
                // there is nothing to do
        }


        SIG_Geometry *SIG_RobotCompilerObjects::surfaceFind (QString name)
        {
                return new SIG_Geometry ();
        }

        SIG_Polygon *SIG_RobotCompilerObjects::surfaceNewPoly (SIG_Geometry *g)
        {
                return new SIG_Polygon (g);
        }

        void SIG_RobotCompilerObjects::surfaceNewPoint (SIG_Polygon *p,
                                                        DL_Scalar x,
                                                        DL_Scalar y,
                                                        DL_Scalar z)
        {
                p->appendVertex (DL_vector (x, y, z));
        }

        void SIG_RobotCompilerObjects::surfaceFinish (SIG_Geometry *g, QString name)
        {
                SIG_Body *b = target->lookupBody (name);
                if (!b)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "There is no body with file \"" + name + "\"");
                b->setGeometry (g);
        }

        void SIG_RobotCompilerObjects::modifierScaleall (DL_Scalar scalingFactor)
        {
                // target->setOverallScalingFactor (scalingFactor);
        }
}
