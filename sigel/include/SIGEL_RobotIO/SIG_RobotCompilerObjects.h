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
#ifndef SIGEL_ROBOTIO_SIG_ROBOTCOMPILEROBJECTS_H
#define SIGEL_ROBOTIO_SIG_ROBOTCOMPILEROBJECTS_H

namespace SIGEL_RobotIO { class SIG_RobotCompilerObjects; }

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_RobotIO/SIG_RobotScanner.h"
#include "SIGEL_RobotIO/SIG_RobotCompiler.h"

namespace SIGEL_RobotIO {

        class SIG_RobotCompilerObjects : public SIG_RobotCompiler {
        private:
                int linknumber, jointnumber, drivenumber, sensornumber;
        public:
                SIG_RobotCompilerObjects (SIG_RobotScanner &sc,
                                          SIG_Robot *tg,
                                          QString homepath);

                SIG_Material *materialFind (QString name) override;
                void materialDensity (SIG_Material *material,
                                      double density) override;
                void materialFriction (SIG_Material *material,
                                       QString otherSide,
                                       double frictionconst) override;
                void materialElasticity (SIG_Material *material,
                                         double elasconst) override;
                void materialColour (SIG_Material *material,
                                     double red,
                                     double green,
                                     double blue) override;
                void materialFinish (SIG_Material *material) override;

                SIG_Link *linkFind (QString name) override;
                void linkIsTorso (SIG_Link *link) override;
                void linkGeometryFile (SIG_Link *link,
                                       QString geometryfile) override;
                void linkMaterial (SIG_Link *link,
                                   QString material) override;
                void linkPoint (SIG_Link *link,
                                QString pointname,
                                double x,
                                double y,
                                double z) override;
                void linkNoCollide (SIG_Link *link,
                                    QString nocollide) override;
                void linkFinish (SIG_Link *link) override;

                SIG_RotationalJoint *rjointFind (QString name) override;
                void rjointLinking (SIG_RotationalJoint *rj,
                                    QString Alink, QString AB, QString AD, QString AH,
                                    QString Blink, QString BB, QString BD, QString BH) override;
                void rjointExtents (SIG_RotationalJoint *rj,
                                    double mn, double mx, double ii) override;
                void rjointFinish (SIG_RotationalJoint *rj) override;

                SIG_TranslationalJoint *tjointFind (QString name) override;
                void tjointLinking (SIG_TranslationalJoint *tj,
                                    QString Alink, QString AB, QString AD, QString AF,
                                    QString Blink, QString BB, QString BD, QString BF) override;
                void tjointExtents (SIG_TranslationalJoint *tj,
                                    double mn, double mx, double ii) override;
                void tjointFinish (SIG_TranslationalJoint *tj) override;

                SIG_CylindricalJoint *cjointFind (QString name) override;
                void cjointLinking (SIG_CylindricalJoint *cj,
                                    QString Alink, QString AB, QString AD, QString AH,
                                    QString Blink, QString BB, QString BD, QString BH) override;
                void cjointRotExtents (SIG_CylindricalJoint *cj,
                                       double mn, double mx, double ii) override;
                void cjointTraExtents (SIG_CylindricalJoint *cj,
                                       double mn, double mx, double ii) override;
                void cjointFinish (SIG_CylindricalJoint *cj) override;

                SIG_GlueJoint *glueFind (QString name) override;
                void glueLinking (SIG_GlueJoint *glue,
                                  QString Alink, QString AA, QString AB, QString AC,
                                  QString Blink, QString BP, QString BQ, QString BR) override;
                void glueFinish (SIG_GlueJoint *glue) override;

                SIG_Drive *driveFind (QString name) override;
                void driveMode (SIG_Drive *d, QString mode) override;
                void driveJoint (SIG_Drive *d, QString jointname) override;
                void driveMinMaxForce (SIG_Drive *d, double minf, double maxf) override;
                void driveFinish (SIG_Drive *d) override;

                SIG_JointSensor *jointSensorFind (QString name) override;
                void jointSensorJoint (SIG_JointSensor *js, QString jointname) override;
                void jointSensorFinish (SIG_JointSensor *js) override;

                SIG_PitchRollSensor *pitchRollSensorFind (QString name) override;
                void pitchRollSensorLink (SIG_PitchRollSensor *prs, QString linkname) override;
                void pitchRollSensorFinish (SIG_PitchRollSensor *prs) override;

				SIG_ContactSensor *contactSensorFind (QString name) override;
				void contactSensorLink(SIG_ContactSensor *cs, QString linkname) override;
		        void contactSensorFinish (SIG_ContactSensor *cs) override;

                SIG_Geometry *surfaceFind (QString name) override;
                SIG_Polygon *surfaceNewPoly (SIG_Geometry *g) override;
                void surfaceNewPoint (SIG_Polygon *p,
                                      double x,
                                      double y,
                                      double z) override;
                void surfaceFinish (SIG_Geometry *g, QString name) override;

                void modifierScaleall (double scalingFactor) override;
        };

}

#endif
