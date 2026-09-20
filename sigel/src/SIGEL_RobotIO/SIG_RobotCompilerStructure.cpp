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
#include "SIGEL_RobotIO/SIG_RobotCompilerStructure.h"

namespace SIGEL_RobotIO {
	bool checkLinkHasPoint (SIG_Link *l, QString pn, SIG_Joint *j)
        {
                if (!l->hasPoint (pn))
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 QString ("There is no point ") +
                                                 pn + " in link " + l->getName () +
                                                 ", as requested in joint " +
                                                 j->getName ());
                return true;
        }
        SIG_RobotCompilerStructure::SIG_RobotCompilerStructure
        (SIG_RobotScanner &sc, SIG_Robot *tg, QString homepath)
                : SIG_RobotCompiler (sc, tg, homepath)
        { }
        
        SIG_RobotCompilerStructure::~SIG_RobotCompilerStructure (void)
        { }

        SIG_Material *SIG_RobotCompilerStructure::materialFind (QString name)
        {
                return target->lookupMaterial (name);
        }

        void SIG_RobotCompilerStructure::materialDensity (SIG_Material *material,
                                                        DL_Scalar density)
        {
                // all done in pass 1
        }

        void SIG_RobotCompilerStructure::materialFriction (SIG_Material *material,
                                                         QString otherSide,
                                                         DL_Scalar frictionconst)
        {
                SIG_Material *oppo = target->lookupMaterial (otherSide);
                if (!oppo)
                        throw SIG_SemanticError
                                (__FILE__, __LINE__,
                                 "There is no material \"" + otherSide +
                                 "\" to which a friction constant is to be set in \"" +
                                 material->getName () + "\"");
                material->setFrictionValue (oppo, frictionconst);
        }

        void SIG_RobotCompilerStructure::materialElasticity (SIG_Material *material,
                                                           DL_Scalar elasconst)
        {
                // all done in pass 1
        }

        void SIG_RobotCompilerStructure::materialColour (SIG_Material *material,
                                                       DL_Scalar red,
                                                       DL_Scalar green,
                                                       DL_Scalar blue)
        {
                // all done in pass 1
        }

        void SIG_RobotCompilerStructure::materialFinish (SIG_Material *material)
        {
                // all done in pass 1
        }

        SIG_Link *SIG_RobotCompilerStructure::linkFind (QString name)
        {
                return target->lookupLink (name);
        }
        
        void SIG_RobotCompilerStructure::linkIsTorso (SIG_Link *link)
        {
                // done in ...Objects
        }
        
        void SIG_RobotCompilerStructure::linkGeometryFile (SIG_Link *link,
                                                         QString geometryfile)
        {
                // done in ...Objects
        }
        
        void SIG_RobotCompilerStructure::linkMaterial (SIG_Link *link,
                                                       QString material)
        {
                SIG_Material *m = target->lookupMaterial (material);
                if (!m)
                        throw SIG_SemanticError
                                (__FILE__, __LINE__,
                                 "There is no material \"" + material +
                                 "\" for link \"" + link->getName () + "\"");
                link->setMaterial (m);
        }
        
        void SIG_RobotCompilerStructure::linkPoint (SIG_Link *link,
                                                    QString pointname,
                                                    DL_Scalar x,
                                                    DL_Scalar y,
                                                    DL_Scalar z)
        {
                // done in ...Objects
        }
        
        void SIG_RobotCompilerStructure::linkNoCollide (SIG_Link *link,
                                                        QString nocollide)
        {
                SIG_Link *nc = target->lookupLink (nocollide);
                if (!nc)
                        throw SIG_SemanticError
                                (__FILE__, __LINE__,
                                 "There is no link \"" + nocollide +
                                 "\" to be marked as not colliding with \"" +
                                 link->getName () + "\"");
                link->addNoCollide (nc);
        }
        
        void SIG_RobotCompilerStructure::linkFinish (SIG_Link *link)
        {
                // nothing to be done.
        }

        SIG_RotationalJoint *SIG_RobotCompilerStructure::rjointFind (QString name)
        {
                SIG_Joint *gj = target->lookupJoint (name);
                if (gj->getJointType () != SIG_Joint::tRotationalJoint)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "internal: " + name + " is not tRotationalJoint");
                SIG_RotationalJoint *j = static_cast<SIG_RotationalJoint *> (gj);
                return j;
        }

        void SIG_RobotCompilerStructure::rjointLinking (SIG_RotationalJoint *rj,
                                                      QString Alink, QString AB, QString AD, QString AH,
                                                      QString Blink, QString BB, QString BD, QString BH)
        {
                SIG_Link *leftlink, *rightlink;
                DL_vector lb, ld, lh, rb, rd, rh;

                leftlink = target->lookupLink (Alink);
                if (!leftlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Alink + "\" available.");
                rightlink = target->lookupLink (Blink);
                if (!rightlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Blink + "\" available.");
                rj->setLeftLink (leftlink);
                rj->setRightLink (rightlink);
                checkLinkHasPoint (leftlink, AB, rj);
                checkLinkHasPoint (leftlink, AD, rj);
                checkLinkHasPoint (leftlink, AH, rj);
                checkLinkHasPoint (rightlink, BB, rj);
                checkLinkHasPoint (rightlink, BD, rj);
                checkLinkHasPoint (rightlink, BH, rj);
                rj->setLeftPoints (leftlink->getPoint (AB),
                                   leftlink->getPoint (AD),
                                   leftlink->getPoint (AH));
                rj->setRightPoints (rightlink->getPoint (BB),
                                    rightlink->getPoint (BD),
                                    rightlink->getPoint (BH));
        }

        void SIG_RobotCompilerStructure::rjointExtents (SIG_RotationalJoint *rj,
                                                      DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                // already done
        }

        void SIG_RobotCompilerStructure::rjointFinish (SIG_RotationalJoint *rj)
        {
                // nothing to be done for finish.
        }

        SIG_TranslationalJoint *SIG_RobotCompilerStructure::tjointFind (QString name)
        {
                SIG_Joint *gj = target->lookupJoint (name);
                if (gj->getJointType () != SIG_Joint::tTranslationalJoint)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "internal: " + name + " is not tTranslationalJoint");
                SIG_TranslationalJoint *j = static_cast<SIG_TranslationalJoint *> (gj);
                return j;
        }
        
        void SIG_RobotCompilerStructure::tjointLinking (SIG_TranslationalJoint *tj,
                                                      QString Alink, QString AB, QString AD, QString AF,
                                                      QString Blink, QString BB, QString BD, QString BF)
        {
                SIG_Link *leftlink, *rightlink;
                DL_vector lb, ld, lh, rb, rd, rh;

                leftlink = target->lookupLink (Alink);
                if (!leftlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Alink + "\" available.");
                rightlink = target->lookupLink (Blink);
                if (!rightlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Blink + "\" available.");
                tj->setLeftLink (leftlink);
                tj->setRightLink (rightlink);
                checkLinkHasPoint (leftlink, AB, tj);
                checkLinkHasPoint (leftlink, AD, tj);
                checkLinkHasPoint (leftlink, AF, tj);
                checkLinkHasPoint (rightlink, BB, tj);
                checkLinkHasPoint (rightlink, BD, tj);
                checkLinkHasPoint (rightlink, BF, tj);
                tj->setLeftPoints (leftlink->getPoint (AB),
                                   leftlink->getPoint (AD),
                                   leftlink->getPoint (AF));
                tj->setRightPoints (rightlink->getPoint (BB),
                                    rightlink->getPoint (BD),
                                    rightlink->getPoint (BF));
        }
        
        void SIG_RobotCompilerStructure::tjointExtents (SIG_TranslationalJoint *tj,
                                                      DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                // already done
        }
        
        void SIG_RobotCompilerStructure::tjointFinish (SIG_TranslationalJoint *tj)
        {
                // Sometimes, finish methods get the blues, because they
                // suspect that they are useless bums in a busy world.
        }

        SIG_CylindricalJoint *SIG_RobotCompilerStructure::cjointFind (QString name)
        {
                SIG_Joint *cj = target->lookupJoint (name);
                if (cj->getJointType () != SIG_Joint::tCylindricalJoint)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "internal: " + name + " is not tCylindricalJoint");
                SIG_CylindricalJoint *j = static_cast<SIG_CylindricalJoint *> (cj);
                return j;
        }

        void SIG_RobotCompilerStructure::cjointLinking (SIG_CylindricalJoint *cj,
                            QString Alink, QString AB, QString AD, QString AH,
                            QString Blink, QString BB, QString BD, QString BH)
        {
                SIG_Link *leftlink, *rightlink;
                leftlink = target->lookupLink (Alink);
                if (!leftlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Alink + "\" available.");
                rightlink = target->lookupLink (Blink);
                if (!rightlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Blink + "\" available.");
                cj->setLeftLink (leftlink);
                cj->setRightLink (rightlink);
                checkLinkHasPoint (leftlink, AB, cj);
                checkLinkHasPoint (leftlink, AD, cj);
                checkLinkHasPoint (leftlink, AH, cj);
                checkLinkHasPoint (rightlink, BB, cj);
                checkLinkHasPoint (rightlink, BD, cj);
                checkLinkHasPoint (rightlink, BH, cj);
                cj->setLeftPoints (leftlink->getPoint (AB),
                                   leftlink->getPoint (AD),
                                   leftlink->getPoint (AH));
                cj->setRightPoints (rightlink->getPoint (BB),
                                    rightlink->getPoint (BD),
                                    rightlink->getPoint (BH));
        }

        void SIG_RobotCompilerStructure::cjointRotExtents (SIG_CylindricalJoint *cj,
                               DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                // already done
        }

        void SIG_RobotCompilerStructure::cjointTraExtents (SIG_CylindricalJoint *cj,
                               DL_Scalar mn, DL_Scalar mx, DL_Scalar ii)
        {
                // already done
        }

        void SIG_RobotCompilerStructure::cjointFinish (SIG_CylindricalJoint *cj)
        {
                // nothing to do
        }

        SIG_GlueJoint *SIG_RobotCompilerStructure::glueFind (QString name)
        {
                SIG_Joint *gj = target->lookupJoint (name);
                if (gj->getJointType () != SIG_Joint::tGlueJoint)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "internal: " + name + " is not tGlueJoint");
                SIG_GlueJoint *j = static_cast<SIG_GlueJoint *> (gj);
                return j;
        }
        
        void SIG_RobotCompilerStructure::glueLinking (SIG_GlueJoint *glue,
                                                      QString Alink, QString AA, QString AB, QString AC,
                                                      QString Blink, QString BP, QString BQ, QString BR)
        {
                SIG_Link *leftlink, *rightlink;
                leftlink = target->lookupLink (Alink);
                if (!leftlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Alink + "\" available");
                rightlink = target->lookupLink (Blink);
                if (!rightlink)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link \"" + Blink + "\" available");
                glue->setLeftLink (leftlink);
                glue->setRightLink (rightlink);
                checkLinkHasPoint (leftlink, AA, glue);
                checkLinkHasPoint (leftlink, AB, glue);
                checkLinkHasPoint (leftlink, AC, glue);
                checkLinkHasPoint (rightlink, BP, glue);
                checkLinkHasPoint (rightlink, BQ, glue);
                checkLinkHasPoint (rightlink, BR, glue);
                glue->setPlaneA (leftlink->getPoint (AA),
                                 leftlink->getPoint (AB),
                                 leftlink->getPoint (AC));
                glue->setPlaneB (rightlink->getPoint (BP),
                                 rightlink->getPoint (BQ),
                                 rightlink->getPoint (BR));
        }
        
        void SIG_RobotCompilerStructure::glueFinish (SIG_GlueJoint *glue)
        {
                // as usual: nothing to do
        }
        
        SIG_Drive *SIG_RobotCompilerStructure::driveFind (QString name)
        {
                return target->lookupDrive (name);
                        
        }

        void SIG_RobotCompilerStructure::driveMode (SIG_Drive *d, QString mode)
        {
                // already done in pass 1
        }

        void SIG_RobotCompilerStructure::driveJoint (SIG_Drive *d, QString jointname)
        {
                SIG_Joint *j = target->lookupJoint (jointname);
                if (!j)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No joint '" + jointname + "' available");
                d->setJoint (j);
        }

        void SIG_RobotCompilerStructure::driveMinMaxForce (SIG_Drive *d, DL_Scalar minf, DL_Scalar maxf)
        {
                // already done in pass 1
        }

        void SIG_RobotCompilerStructure::driveFinish (SIG_Drive *d)
        {
                // nothing, nothing, nothing! finish methods are really overpaid.
                // and i can tell that without knowing their wages!
        }

        SIG_JointSensor *SIG_RobotCompilerStructure::jointSensorFind (QString name)
        {
                SIG_Sensor *s = target->lookupSensor (name);
                if (s->getSensorType () != SIG_Sensor::tJointSensor)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "internal: " + name + " is not tJointSensor");
                return static_cast<SIG_JointSensor *> (s);
        }

        void SIG_RobotCompilerStructure::jointSensorJoint (SIG_JointSensor *js, QString jointname)
        {
                SIG_Joint *j = target->lookupJoint (jointname);
                if (!j)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No joint called '"+jointname+"'");
                js->setJoint (j);
        }

        void SIG_RobotCompilerStructure::jointSensorFinish (SIG_JointSensor *js)
        {
                // there is nothing to do
        }

        SIG_PitchRollSensor *SIG_RobotCompilerStructure::pitchRollSensorFind (QString name)
        {
                SIG_Sensor *s = target->lookupSensor (name);
                if (s->getSensorType () != SIG_Sensor::tPitchRollSensor)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "internal: " + name + " is not tPitchRollSensor");
                return static_cast<SIG_PitchRollSensor *> (s);
        }

        void SIG_RobotCompilerStructure::pitchRollSensorLink (SIG_PitchRollSensor *prs, QString linkname)
        {
                SIG_Link *l = target->lookupLink(linkname);
                if (!l)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link called '"+linkname+"'");
                prs->setLink(l);
	    }

        void SIG_RobotCompilerStructure::pitchRollSensorFinish (SIG_PitchRollSensor *prs)
        {
                // there is nothing to do
        }


        SIG_ContactSensor *SIG_RobotCompilerStructure::contactSensorFind (QString name)
        {
                SIG_Sensor *s = target->lookupSensor (name);
                if (s->getSensorType () != SIG_Sensor::tContactSensor)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "internal: " + name + " is not tContactSensor");
                return static_cast<SIG_ContactSensor *> (s);
        }

        void SIG_RobotCompilerStructure::contactSensorLink (SIG_ContactSensor *cs, QString linkname)
        {
                SIG_Link *l = target->lookupLink(linkname);
                if (!l)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "No link called '"+linkname+"'");
                cs->setLink(l);
	    }

        void SIG_RobotCompilerStructure::contactSensorFinish (SIG_ContactSensor *cs)
        {
                // there is nothing to do
        }
}
