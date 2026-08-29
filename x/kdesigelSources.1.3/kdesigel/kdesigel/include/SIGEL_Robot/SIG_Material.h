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
#ifndef SIGEL_ROBOT_SIG_MATERIAL_H
#define SIGEL_ROBOT_SIG_MATERIAL_H

namespace SIGEL_Robot { class SIG_Material; }

#include <QList>
#include <qstring.h>
#include <qtextstream.h>
#include <pointvector.h>
#include "SIGEL_Robot/SIG_Robot.h"

namespace SIGEL_Robot
{
  /**
   * SIG_Material models properties of the material
   * used to form links.
   *
   * Objects instantiated from this class contain
   * information about the density, the elasticity, and the
   * colour of the material.
   */
  class SIG_Material {
  private:
          struct FrictionValue {
                  SIG_Material *otherSide;
                  DL_Scalar value;
          };
          
          SIG_Robot *parent;
          QString name;
          DL_Scalar elasticity, density;
          QList<FrictionValue *> friction;
          DL_vector colour;
  public:
          /**
           * Kind of a standard constructor.
           *
           * Sets all material values to a default value.
           */
          SIG_Material (SIG_Robot *par, QString n);
          /**
           * Reads the material attributes from a stream.
           */
          SIG_Material (SIG_Robot *par, QTextStream & tx);
          /**
           * Destructor.
           */
          ~SIG_Material (void);
          /**
           * Returns the name of the material.
           */
          QString getName (void) const;
          /**
           * Sets the density of the material.
           *
           * According to the SI system, this has to be in
           * kilograms per cubic meter.
           */
          void setDensity (DL_Scalar dens);
          /**
           * Returns the density of the material.
           *
           * This value denotes the kilograms per cubic meter.
           */
          DL_Scalar getDensity () const;
          /**
           * Sets the elasticity value that the corresponding
           * dyna objects should have.
           *
           * Please refer to the Dynamo manuals for the
           * interpretation of this value.
           */
          void setElasticity (DL_Scalar elas);
          /**
           * Returns the elasticity of the material.
           *
           * Please refer to the Dynamo manuals for the
           * interpretation of this value.
           */
          DL_Scalar getElasticity () const;
          /**
           * Sets the colour of the material.
           */
          void setColour (DL_vector col);
          /**
           * Returns the colour of the material.
           */
          DL_vector getColour () const;
          /**
           * setFrictionValue sets or adds a friction constant to the material.
           * @param otherObj the opposite material to which the constant
           *                 will be set
           * @param fricval  the actual constant
           * @param negotiate DO NOT TOUCH! If set to true (default) the
           *                 constant will also be set in the opposite
           *                 material object.
           */
          void setFrictionValue (SIG_Material *otherObj,
                                 DL_Scalar fricval,
                                 bool negotiate = true);
          /**
           * Returns the friction constant between this material
           * and the material otherObj.
           *
           * otherObj->getFrictionValue (this) returns the
           * same value.
           */
          DL_Scalar getFrictionValue (SIG_Material *otherObj) const;
          /**
           * Writes the material object into a stream.
           */
          void writeToFileTransfer (QTextStream & tx) const;
  };
}

#endif // SIGEL_ROBOT_SIG_MATERIAL_H
