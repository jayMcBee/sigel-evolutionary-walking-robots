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
#ifndef SIGEL_ROBOT_SIG_LINK_H
#define SIGEL_ROBOT_SIG_LINK_H

namespace SIGEL_Robot { class SIG_Link; }

#include <QList>
#include <memory>
#include <qstring.h>
#include <qtextstream.h>
#include "SIGEL_Tools/SIG_Vector.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_Mirtich.h"

namespace SIGEL_Robot
{
  
  /**
   * This class models a link.
   *
   * SIG_Joint is a linking pin to all information
   * needed to work with a link. Geometry information
   * is saved in a SIG_Body object, material information
   * in a SIG_Material object. Furthermore a list of
   * significant points is managed.
   */
  class SIG_Link
    {
    public:
      /** A significant point on the link, with the name it is declared under. */
      struct NamedPoint { QString name; SIG_Vector *value; };

    public:
      SIG_Link (SIG_Robot *par, QString n, int nr);
      SIG_Link (SIG_Robot *par, QTextStream & tx);
      ~SIG_Link ();

      QString getName () const;
      int getNumber () const;
      bool isRootLink () const;

      void setBody (SIG_Body *b);
      SIG_Body const *getBody () const;
      void setMaterial (SIG_Material *mtrl);
      SIG_Material const *getMaterial () const;
      void addPoint (QString pointname, SIG_Vector point);
      SIG_Vector getPoint (QString id) const;
      bool hasPoint (QString pointname) const;
      const QList<NamedPoint> &getPoints () const;
      int getNrOfPoints () const;

      void instantiateGeometry ();
      void transformToDynaMechs ( SIG_Joint *predecessor,
				  SIG_Vector predBase = SIG_Vector(0, 0, 0),
				  SIG_Vector predDir = SIG_Vector(0, 0, 0),
				  SIG_Vector predHand = SIG_Vector(0, 0, 0) );
      void transformPoints (SIG_Vector mov, SIG_Matrix rot);
      /**
       * addNoCollide marks a pair of links, this and the given one,
       * as links for which no collisions should be reported.
       * @param link The link this link should not collide with.
       * @param negotiate DO NOT TOUCH! Defaults to also entering this
       *                  link into the no collision list within the given
       *                  link.
       */
      void addNoCollide (SIG_Link *link, bool negotiate = true);
      QList<SIG_Link *> getNoCollides () const;
      void addJoint (SIG_Joint *joint);
      QList<SIG_Joint *> getJoints () const;

      SIG_Geometry const *getGeometry () const;
      SIG_Mirtich const *getMirtich()  { return mirtich.get(); }
      void getPhysics (double & m,
                       SIG_Vector & com,
                       SIG_Matrix & it);
      void propagateInitialLocation (SIG_Link *comingfrom);
      void setInitialLocation (SIG_Vector p, SIG_Matrix o, SIG_Link *comingfrom = nullptr);
      void getInitialLocation (SIG_Vector & p, SIG_Matrix & o) const;
      bool isInitiated () const;
      bool isMDHVisited () const;
      void calculateCommonNormal( SIG_Vector a,
				  SIG_Vector u,
				  SIG_Vector b,
				  SIG_Vector v,
				  SIG_Vector &c,
				  SIG_Vector &w );

      void writeToFileTransfer (QTextStream & tx) const;
      
    private:
      SIG_Robot *parent;
      QString name;
      int number;
      SIG_Body *body;
      std::unique_ptr< SIG_Geometry > geometry;
      std::unique_ptr< SIG_Mirtich > mirtich;
      SIG_Material *material;
      // A SIG_Vector has no name of its own, so unlike SIG_Robot's six lists this
      // one needs to carry the key.
      QList<NamedPoint> points;
      QList<SIG_Joint *> adjacentJoints;
      QList<SIG_Link *> noCollide;
      SIG_Vector initialLocation;
      SIG_Matrix initialOrientation;
      bool initiated, mdh_visited;
    };

}

#endif // SIGEL_ROBOT_SIG_LINK_H
