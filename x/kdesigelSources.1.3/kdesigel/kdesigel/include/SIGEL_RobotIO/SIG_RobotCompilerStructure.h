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
#ifndef SIGEL_ROBOTIO_SIG_ROBOTCOMPILERSTRUCTURE_H
#define SIGEL_ROBOTIO_SIG_ROBOTCOMPILERSTRUCTURE_H

namespace SIGEL_RobotIO { class SIG_RobotCompilerStructure; }

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_RobotIO/SIG_RobotScanner.h"
#include "SIGEL_RobotIO/SIG_RobotCompiler.h"
#include "SIGEL_RobotIO/SIG_RobotIOExceptions.h"

namespace SIGEL_RobotIO {
        class SIG_RobotCompilerStructure : public SIG_RobotCompiler {
        public:
                SIG_RobotCompilerStructure (SIG_RobotScanner &sc,
                                            SIG_Robot *tg,
                                            QString homepath);
                virtual ~SIG_RobotCompilerStructure (void);

                virtual SIG_Material *materialFind (QString name);
                virtual void materialDensity (SIG_Material *material,
                                              DL_Scalar density);
                virtual void materialFriction (SIG_Material *material,
                                               QString otherSide,
                                               DL_Scalar frictionconst);
                virtual void materialElasticity (SIG_Material *material,
                                                 DL_Scalar elasconst);
                virtual void materialColour (SIG_Material *material,
                                             DL_Scalar red,
                                             DL_Scalar green,
                                             DL_Scalar blue);
                virtual void materialFinish (SIG_Material *material);

                virtual SIG_Link *linkFind (QString name);
                virtual void linkIsTorso (SIG_Link *link);
                virtual void linkGeometryFile (SIG_Link *link,
                                               QString geometryfile);
                virtual void linkMaterial (SIG_Link *link,
                                           QString material);
                virtual void linkPoint (SIG_Link *link,
                                        QString pointname,
                                        DL_Scalar x,
                                        DL_Scalar y,
                                        DL_Scalar z);
                virtual void linkNoCollide (SIG_Link *link,
                                            QString nocollide);
                virtual void linkFinish (SIG_Link *link);

                virtual SIG_RotationalJoint *rjointFind (QString name);
                virtual void rjointLinking (SIG_RotationalJoint *rj,
                                            QString Alink, QString AB, QString AD, QString AH,
                                            QString Blink, QString BB, QString BD, QString BH);
                virtual void rjointExtents (SIG_RotationalJoint *rj,
                                            DL_Scalar mn, DL_Scalar mx, DL_Scalar ii);
                virtual void rjointFinish (SIG_RotationalJoint *rj);

                virtual SIG_TranslationalJoint *tjointFind (QString name);
                virtual void tjointLinking (SIG_TranslationalJoint *tj,
                                            QString Alink, QString AB, QString AD, QString AF,
                                            QString Blink, QString BB, QString BD, QString BF);
                virtual void tjointExtents (SIG_TranslationalJoint *tj,
                                            DL_Scalar mn, DL_Scalar mx, DL_Scalar ii);
                virtual void tjointFinish (SIG_TranslationalJoint *tj);

                virtual SIG_CylindricalJoint *cjointFind (QString name);
                virtual void cjointLinking (SIG_CylindricalJoint *cj,
                                            QString Alink, QString AB, QString AD, QString AH,
                                            QString Blink, QString BB, QString BD, QString BH);
                virtual void cjointRotExtents (SIG_CylindricalJoint *cj,
                                               DL_Scalar mn, DL_Scalar mx, DL_Scalar ii);
                virtual void cjointTraExtents (SIG_CylindricalJoint *cj,
                                               DL_Scalar mn, DL_Scalar mx, DL_Scalar ii);
                virtual void cjointFinish (SIG_CylindricalJoint *cj);

                virtual SIG_GlueJoint *glueFind (QString name);
                virtual void glueLinking (SIG_GlueJoint *glue,
                                          QString Alink, QString AA, QString AB, QString AC,
                                          QString Blink, QString BP, QString BQ, QString BR);
                virtual void glueFinish (SIG_GlueJoint *glue);

                virtual SIG_Drive *driveFind (QString name);
                virtual void driveMode (SIG_Drive *d, QString mode);
                virtual void driveJoint (SIG_Drive *d, QString jointname);
                virtual void driveMinMaxForce (SIG_Drive *d, DL_Scalar minf, DL_Scalar maxf);
                virtual void driveFinish (SIG_Drive *d);

                virtual SIG_JointSensor *jointSensorFind (QString name);
                virtual void jointSensorJoint (SIG_JointSensor *js, QString jointname);
                virtual void jointSensorFinish (SIG_JointSensor *js);

				virtual SIG_ContactSensor *contactSensorFind (QString name);
				virtual void contactSensorLink(SIG_ContactSensor *cs, QString linkname);
		        virtual void contactSensorFinish (SIG_ContactSensor *cs);

				virtual SIG_PitchRollSensor *pitchRollSensorFind (QString name);
				virtual void pitchRollSensorLink(SIG_PitchRollSensor *prs, QString linkname);
		        virtual void pitchRollSensorFinish (SIG_PitchRollSensor *prs);

                virtual SIG_Geometry *surfaceFind (QString name) { return NULL; }
                virtual SIG_Polygon *surfaceNewPoly (SIG_Geometry *g) { return NULL; }
                virtual void surfaceNewPoint (SIG_Polygon *p,
                                              DL_Scalar x,
                                              DL_Scalar y,
                                              DL_Scalar z) {}
                virtual void surfaceFinish (SIG_Geometry *g, QString name) {}
        };
}

#endif
