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
#ifndef SIGEL_ROBOTIO_SIG_ROBOTCOMPILER_H
#define SIGEL_ROBOTIO_SIG_ROBOTCOMPILER_H

namespace SIGEL_RobotIO { class SIG_RobotCompiler; }

#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Robot/SIG_CylindricalJoint.h"
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_RobotIO/SIG_RobotScanner.h"
#include <pointvector.h>

using SIGEL_Robot::SIG_Body;
using SIGEL_Robot::SIG_Robot;
using SIGEL_Robot::SIG_Material;
using SIGEL_Robot::SIG_Link;
using SIGEL_Robot::SIG_Joint;
using SIGEL_Robot::SIG_RotationalJoint;
using SIGEL_Robot::SIG_TranslationalJoint;
using SIGEL_Robot::SIG_CylindricalJoint;
using SIGEL_Robot::SIG_GlueJoint;
using SIGEL_Robot::SIG_Geometry;
using SIGEL_Robot::SIG_Polygon;
using SIGEL_Robot::SIG_Drive;
using SIGEL_Robot::SIG_Sensor;
using SIGEL_Robot::SIG_JointSensor;
using SIGEL_Robot::SIG_PitchRollSensor;
using SIGEL_Robot::SIG_ContactSensor;

namespace SIGEL_RobotIO {
        class SIG_RobotCompiler {
        protected:
                SIG_Scanner &myScanner;
                SIG_Robot *target;
                QString dirprefix;
        public:
                SIG_RobotCompiler (SIG_RobotScanner &sc,
                                   SIG_Robot *tg,
                                   QString homepath);
                virtual ~SIG_RobotCompiler (void);

                // QString prefixFile (QString filename);

                void expect (int symboltype);
                QString expectWord (void);
                DL_Scalar expectNumber (void);
                QString expectString (void);

                virtual SIG_Material *materialFind (QString name) = 0;
                virtual void materialDensity (SIG_Material *material,
                                              DL_Scalar density) = 0;
                virtual void materialFriction (SIG_Material *material,
                                               QString otherSide,
                                               DL_Scalar frictionconst) = 0;
                virtual void materialElasticity (SIG_Material *material,
                                                 DL_Scalar elasconst) = 0;
                virtual void materialColour (SIG_Material *material,
                                             DL_Scalar red,
                                             DL_Scalar green,
                                             DL_Scalar blue) = 0;
                virtual void materialFinish (SIG_Material *material) = 0;

                virtual SIG_Link *linkFind (QString name) = 0;
                virtual void linkIsTorso (SIG_Link *link) = 0;
                virtual void linkGeometryFile (SIG_Link *link,
                                               QString geometryfile) = 0;
                virtual void linkMaterial (SIG_Link *link,
                                           QString material) = 0;
                virtual void linkPoint (SIG_Link *link,
                                        QString pointname,
                                        DL_Scalar x,
                                        DL_Scalar y,
                                        DL_Scalar z) = 0;
                virtual void linkNoCollide (SIG_Link *link,
                                            QString nocollide) = 0;
                virtual void linkFinish (SIG_Link *link) = 0;

                virtual SIG_RotationalJoint *rjointFind (QString name) = 0;
                virtual void rjointLinking (SIG_RotationalJoint *rj,
                                            QString Alink, QString AB, QString AD, QString AH,
                                            QString Blink, QString BB, QString BD, QString BH) = 0;
                virtual void rjointExtents (SIG_RotationalJoint *rj,
                                            DL_Scalar mn, DL_Scalar mx, DL_Scalar ii) = 0;
                virtual void rjointFinish (SIG_RotationalJoint *rj) = 0;

                virtual SIG_TranslationalJoint *tjointFind (QString name) = 0;
                virtual void tjointLinking (SIG_TranslationalJoint *tj,
                                            QString Alink, QString AB, QString AD, QString AF,
                                            QString Blink, QString BB, QString BD, QString BF) = 0;
                virtual void tjointExtents (SIG_TranslationalJoint *tj,
                                            DL_Scalar mn, DL_Scalar mx, DL_Scalar ii) = 0;
                virtual void tjointFinish (SIG_TranslationalJoint *tj) = 0;

                virtual SIG_CylindricalJoint *cjointFind (QString name) = 0;
                virtual void cjointLinking (SIG_CylindricalJoint *cj,
                                            QString Alink, QString AB, QString AD, QString AH,
                                            QString Blink, QString BB, QString BD, QString BH) = 0;
                virtual void cjointRotExtents (SIG_CylindricalJoint *cj,
                                               DL_Scalar mn, DL_Scalar mx, DL_Scalar ii) = 0;
                virtual void cjointTraExtents (SIG_CylindricalJoint *cj,
                                               DL_Scalar mn, DL_Scalar mx, DL_Scalar ii) = 0;
                virtual void cjointFinish (SIG_CylindricalJoint *cj) = 0;

                virtual SIG_GlueJoint *glueFind (QString name) = 0;
                virtual void glueLinking (SIG_GlueJoint *glue,
                                          QString Alink, QString AA, QString AB, QString AC,
                                          QString Blink, QString BP, QString BQ, QString BR) =0;
                virtual void glueFinish (SIG_GlueJoint *glue) = 0;

                virtual SIG_Drive *driveFind (QString name) = 0;
                virtual void driveMode (SIG_Drive *d, QString mode) = 0;
                virtual void driveJoint (SIG_Drive *d, QString jointname) = 0;
                virtual void driveMinMaxForce (SIG_Drive *d,
                                               DL_Scalar minf, DL_Scalar maxf) = 0;
                virtual void driveFinish (SIG_Drive *d) = 0;

                virtual SIG_JointSensor *jointSensorFind (QString name) = 0;
                virtual void jointSensorJoint (SIG_JointSensor *js, QString jointname) = 0;
                virtual void jointSensorFinish (SIG_JointSensor *js) = 0;

				virtual SIG_PitchRollSensor *pitchRollSensorFind (QString name)=0;
				virtual void pitchRollSensorLink(SIG_PitchRollSensor *prs, QString linkname)=0;
		        virtual void pitchRollSensorFinish (SIG_PitchRollSensor *prs)=0;

				virtual SIG_ContactSensor *contactSensorFind (QString name)=0;
				virtual void contactSensorLink(SIG_ContactSensor *cs, QString linkname)=0;
		        virtual void contactSensorFinish (SIG_ContactSensor *cs)=0;

                virtual SIG_Geometry *surfaceFind (QString name) = 0;
                virtual SIG_Polygon *surfaceNewPoly (SIG_Geometry *g) = 0;
                virtual void surfaceNewPoint (SIG_Polygon *p,
                                              DL_Scalar x,
                                              DL_Scalar y,
                                              DL_Scalar z) = 0;
                virtual void surfaceFinish (SIG_Geometry *g, QString name) = 0;

                virtual void modifierScaleall (DL_Scalar scalingFactor) { }

                virtual void nextIsMaterial (void);
                virtual void nextIsLink (void);
                virtual void nextIsJoint (void);
                virtual void nextIsGlue (void);
                virtual void nextIsDrive (void);
                virtual void nextIsSensor (void);
                virtual void nextIsSurface (void);

                virtual void nextModScaleall (void);

                bool compileNextEntity (void);
                void runPass (void);
        };
}

#endif
