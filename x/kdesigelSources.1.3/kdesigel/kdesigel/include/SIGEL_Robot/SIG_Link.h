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

#include <qstring.h>
#include <qlist.h>
#include <qtextstream.h>
#include <pointvector.h>
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
      SIG_Link (SIG_Robot *par, QString n, int nr);
      SIG_Link (SIG_Robot *par, QTextStream & tx);
      ~SIG_Link (void);

      QString getName (void) const;
      int getNumber (void) const;
      bool isRootLink (void) const;

      void setBody (SIG_Body *b);
      SIG_Body const *getBody (void) const;
      void setMaterial (SIG_Material *mtrl);
      SIG_Material const *getMaterial (void) const;
      void addPoint (QString pointname, DL_vector point);
      DL_vector getPoint (QString id) const;
      bool hasPoint (QString pointname) const;
      QDictIterator<DL_vector> getPointIter (void) const;
      int getNrOfPoints (void) const;

      void instantiateGeometry (void);
      void transformToDynaMo (void);
      void transformToDynaMechs ( SIG_Joint *predecessor,
				  DL_vector predBase = DL_vector(0, 0, 0),
				  DL_vector predDir = DL_vector(0, 0, 0),
				  DL_vector predHand = DL_vector(0, 0, 0) );
      void transformPoints (DL_vector mov, DL_matrix rot);
      /**
       * addNoCollide marks a pair of links, this and the given one,
       * as links for which no collisions should be reported.
       * @param link The link this link should not collide with.
       * @param negotiate DO NOT TOUCH! Defaults to also entering this
       *                  link into the no collision list within the given
       *                  link.
       */
      void addNoCollide (SIG_Link *link, bool negotiate = true);
      QList<SIG_Link> getNoCollides () const;
      void addJoint (SIG_Joint *joint);
      QList<SIG_Joint> getJoints () const;

      SIG_Geometry const *getGeometry (void) const;
      SIG_Mirtich const *getMirtich( void )  { return mirtich; }
      void getPhysics (DL_Scalar & m,
                       DL_vector & com,
                       DL_matrix & it);
      void propagateInitialLocation (SIG_Link *comingfrom);
      void setInitialLocation (DL_vector p, DL_matrix o, SIG_Link *comingfrom = 0);
      void getInitialLocation (DL_vector & p, DL_matrix & o) const;
      bool isInitiated (void) const;
      bool isMDHVisited (void) const;
      void calculateCommonNormal( DL_vector a,
				  DL_vector u,
				  DL_vector b,
				  DL_vector v,
				  DL_vector &c,
				  DL_vector &w );

      void writeToFileTransfer (QTextStream & tx) const;
      
    private:
      SIG_Robot *parent;
      QString name;
      int number;
      SIG_Body *body;
      SIG_Geometry *geometry;
      SIG_Mirtich *mirtich;
      SIG_Material *material;
      QDict<DL_vector> points;
      QList<SIG_Joint> adjacentJoints;
      QList<SIG_Link> noCollide;
      DL_vector initialLocation;
      DL_matrix initialOrientation;
      bool initiated, mdh_visited;
    };
}

#endif // SIGEL_ROBOT_SIG_LINK_H


