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
#include "SIGEL_Robot/SIG_Material.h"

namespace SIGEL_Robot {

        SIG_Material::SIG_Material (SIG_Robot *par, QString n)
                : parent (par),
                  name (n),
                  elasticity (1.0),
                  density (1.0),
                  friction (),
                  colour (1.0, 1.0, 1.0)
        {
        }

        SIG_Material::SIG_Material (SIG_Robot *par, QTextStream & tx)
                : parent (par)
        {
                QString tmpstr;
                int zahl;

                // "Material" has already been read by SIG_Robot.
                tx >> name
                   >> elasticity
                   >> density;
                
                tx >> zahl;
                for (int i = 0; i < zahl; i++) {
                        QString fname;
                        DL_Scalar fval;
                        SIG_Material *fother;

                        tx >> fname;
                        tx >> fval;
                        // For the following, see my paper. - Holger
                        if (fother = parent->lookupMaterial (fname))
                                setFrictionValue (fother, fval);
                }
                colour = SIG_Robot::streamToVector (tx);
        }

        SIG_Material::~SIG_Material (void)
        {
                qDeleteAll (friction);
                friction.clear ();
        }

        QString SIG_Material::getName (void) const
        {
                return name;
        }

        void SIG_Material::setDensity (DL_Scalar dens)
        {
                density = dens;
        }

        DL_Scalar SIG_Material::getDensity (void) const
        {
                return density;
        }

        void SIG_Material::setElasticity (DL_Scalar elas)
        {
                elasticity = elas;
        }

        DL_Scalar SIG_Material::getElasticity (void) const
        {
                return elasticity;
        }

        void SIG_Material::setColour (DL_vector col)
        {
                colour = col;
        }

        DL_vector SIG_Material::getColour (void) const
        {
                return colour;
        }

        void SIG_Material::setFrictionValue (SIG_Material *otherObj,
                                             DL_Scalar fricval,
                                             bool negotiate)
        {
                FrictionValue *found = 0;
                for (FrictionValue *fv : friction)
                        if (fv->otherSide == otherObj) {
                                fv->value = fricval;
                                found = fv;
                                break;
                        }

                if (!found) {
                        FrictionValue *fv = new FrictionValue;
                        fv->otherSide = otherObj;
                        fv->value = fricval;
                        friction.append (fv);
                }

                if ((negotiate) && (otherObj != this))
                        otherObj->setFrictionValue (this, fricval, false);
        }

        DL_Scalar SIG_Material::getFrictionValue (SIG_Material *otherObj) const
        {
                for (const FrictionValue *fv : friction)
                        if (fv->otherSide == otherObj)
                                return fv->value;
                return 0.6;
        }

        void SIG_Material::writeToFileTransfer (QTextStream & tx) const
        {
                tx << "Material" << ' '
                   << getName () << ' '
                   << elasticity << ' '
                   << density << ' '
                   << friction.count () << ' ';
                for (const FrictionValue *fv : friction)
                        tx << fv->otherSide->getName () << ' '
                           << fv->value << ' ';
                SIG_Robot::vectorToStream (tx, colour);
                tx << '\n';
        }
}
