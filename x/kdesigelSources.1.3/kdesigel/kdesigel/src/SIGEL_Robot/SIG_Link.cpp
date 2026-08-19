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
#include "SIGEL_Robot/SIG_Link.h"

#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_WrongKinematicsException.h"
#include "SIGEL_Robot/IFunctions.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"
#include "SIGEL_Tools/SIG_IO.h"

#include <cmath>
#include <cstdio>

#include "SIGEL_Simulation/SIG_DynaSystem.h"

using namespace SIGEL_Tools;

namespace SIGEL_Robot {
        SIG_Link::SIG_Link(SIG_Robot *par, QString n, int nr)
                : parent (par),
                  name (n),
                  number (nr),
                  body (0),
                  geometry (0),
                  mirtich (0),
                  material (0),
                  adjacentJoints (),
                  noCollide (),
                  initialLocation (0.0, 0.0, 0.0),
                  initiated (false), mdh_visited (false)
        {
                initialOrientation.makeone ();
        }

        SIG_Link::SIG_Link (SIG_Robot *par, QTextStream & tx)
                : parent (par),
                  geometry (0),
                  mirtich (0)
        {
                QString tmpstr;
                int anum;
                
                // "Link" has already been read by SIG_Robot.
                tx >> name >> number;

                tx >> anum;
                for (int i = 0; i < anum; i++) {
                        tx >> tmpstr;
                        DL_vector *dl = new DL_vector
                                (SIG_Robot::streamToVector (tx));
                        points.insert (tmpstr, dl);
                }

                // To understand this, please read my paper. Holger.
                tx >> anum;
                for (int j = 0; j < anum; j++) {
                        tx >> tmpstr;
                        SIG_Link *l = parent->lookupLink (tmpstr);
                        if (l)
                                addNoCollide (l);
                }
                
                initialLocation = SIG_Robot::streamToVector (tx);
                initialOrientation = SIG_Robot::streamToMatrix (tx);
                tx >> tmpstr;
                initiated = (tmpstr == "y");
                tx >> tmpstr;
                mdh_visited = (tmpstr == "y");

                tx >> tmpstr;
                body = parent->lookupBody (tmpstr);
                tx >> tmpstr;
                material = parent->lookupMaterial (tmpstr);

                tx >> tmpstr;
                if (tmpstr == "y") {
                        geometry = new SIG_Geometry (tx);
                        mirtich=new SIG_Mirtich (geometry, name + "(" + body->getName () + ")");
                }
        }

        SIG_Link::~SIG_Link()
        {
                points.setAutoDelete (TRUE);
                adjacentJoints.setAutoDelete (FALSE);
                noCollide.setAutoDelete (FALSE);

                if (geometry) {
                        delete mirtich;
                        delete geometry;
                }
        }

        QString SIG_Link::getName (void) const
        {
                return name;
        }
        
        int SIG_Link::getNumber (void) const
        {
                return number;
        }

        bool SIG_Link::isRootLink (void) const
        {
                return parent->getRootLink () == this;
        }
        
        void SIG_Link::setBody (SIG_Body *b)
        {
                body = b;
        }
        
        SIG_Body const *SIG_Link::getBody (void) const
        {
                return body;
        }
        
        void SIG_Link::setMaterial (SIG_Material *mtrl)
        {
                material = mtrl;
        }
        
        SIG_Material const *SIG_Link::getMaterial (void) const
        {
                return material;
        }

        void SIG_Link::addPoint (QString pointname, DL_vector point)
        {
                points.insert (pointname, new DL_vector (&point));
        }
        
        DL_vector SIG_Link::getPoint (QString id) const
        {
                DL_vector tmp;
                DL_vector *t;
                t = points.find (id);
                if (t)
                        tmp = *t;
                return tmp;
        }

        bool SIG_Link::hasPoint (QString id) const
        {
                return (points.find (id) != 0);
        }

        QDictIterator<DL_vector> SIG_Link::getPointIter (void) const
        {
                return QDictIterator<DL_vector> (points);
        }

        int SIG_Link::getNrOfPoints (void) const
        {
                return points.count ();
        }

        void SIG_Link::instantiateGeometry (void)
        {
                if (geometry) {
                        delete mirtich;
                        delete geometry;
                }
                
                geometry = new SIG_Geometry (body->getGeometry ());
                mirtich = new SIG_Mirtich (geometry, name + "(" + body->getName () + ")");
        }

        void SIG_Link::transformToDynaMo (void)
        {
                DL_vector v;
                DL_matrix m;
                mirtich->moveToOriginAndMajorAxes (v, m);
                transformPoints (v, m);
        }

        void SIG_Link::transformPoints (DL_vector mov, DL_matrix rot)
        {
                QDictIterator<DL_vector> pit (points);
                DL_vector *pt;
                while (pt = pit.current ()) {
                        pt->plusis (&mov);
                        DL_vector v (pt);
                        rot.times (&v, pt);
                        ++pit;
                }

                QListIterator<SIG_Joint> jit (adjacentJoints);
                SIG_Joint *j;
                while (j = jit.current ()) {
                        j->transformPoints (this, mov, rot);
                        ++jit;
                }
        }
        
        void SIG_Link::addNoCollide (SIG_Link *link, bool negotiate)
        {
                if (noCollide.containsRef (link) == 0) {
                        noCollide.append (link);
                        if (negotiate)
                                link->addNoCollide (this, false);
                }
        }
        
        QList<SIG_Link> SIG_Link::getNoCollides () const
        {
                return noCollide;
        }
        
        void SIG_Link::addJoint (SIG_Joint *joint)
        {
                adjacentJoints.append (joint);
        }
        
        QList<SIG_Joint> SIG_Link::getJoints () const
        {
                return adjacentJoints;
        }

        SIG_Geometry const *SIG_Link::getGeometry (void) const
        {
                return geometry;
        }

        void SIG_Link::getPhysics (DL_Scalar & m,
                                   DL_vector & com,
                                   DL_matrix & it)
        {
                mirtich->computePhysics (material->getDensity (), m, com, it);
        }

        void SIG_Link::propagateInitialLocation (SIG_Link *comingfrom)
        {
                // It is very imported, that the following assignment
                // is done first within this procedure. This is due
                // to recursion.
                initiated = true;

                SIG_Joint *j;
                SIG_Link *l;
                QListIterator<SIG_Joint> li (adjacentJoints);
                while (j = li.current ()) {
                        DL_vector transla, fglobtransla;
                        DL_matrix rota, fglobrota;

                        l = j->otherSide (this);
                        if (l != comingfrom) {
                                j->getGeomRelation (transla, rota, this);

                                initialOrientation.times (&rota, &fglobrota);
                                initialOrientation.times (&transla, &fglobtransla);
                                fglobtransla.plusis (&initialLocation);

                                l->setInitialLocation (fglobtransla, fglobrota, this);
                        }

                        ++li;
                }
        }
        
        void SIG_Link::setInitialLocation (DL_vector p, DL_matrix o, SIG_Link *comingfrom)
        {
                if (!initiated) {
                        initialLocation = p;
                        initialOrientation = o;
                        
                        propagateInitialLocation (comingfrom);
                } else {
                        DL_Scalar const ILnull = 0.000001;
                        for (int i = 0; i < 3; i++) {
                                if (fabs (p.get (i) - initialLocation.get (i)) > ILnull)
                                        throw SIG_InitialLocationError
                                                (__FILE__, __LINE__,
                                                 "Ambiguous values for position in link \"" +
                                                 name + "\".");
                                for (int j = 0; j < 3; j++)
                                        if (fabs (o.get (i,j) - initialOrientation.get (i,j)) > ILnull)
                                                throw SIG_InitialLocationError
                                                        (__FILE__, __LINE__,
                                                         "Ambiguous values for orientation in link \"" +
                                                         name + "\".");
                        }
                }
        }
        
        void SIG_Link::getInitialLocation (DL_vector & p, DL_matrix & o) const
        {
                p = initialLocation;
                o = initialOrientation;
        }
        
        bool SIG_Link::isInitiated (void) const
        {
                return initiated;
        }

        void SIG_Link::writeToFileTransfer (QTextStream & tx) const
        {
                QDictIterator<DL_vector> piter (points);
                QListIterator<SIG_Link> nciter (noCollide);
                tx << "Link "
                   << getName () << ' '
                   << getNumber () << ' '
                   << points.count () << ' ';
                while (piter.current ()) {
                        tx << piter.currentKey () << ' ';
                        SIG_Robot::vectorToStream (tx, *piter.current ());
                        ++piter;
                }
                tx << noCollide.count () << ' ';
                while (nciter.current ()) {
                        tx << nciter.current ()->getName () << ' ';
                        ++nciter;
                }
                SIG_Robot::vectorToStream (tx, initialLocation);
                SIG_Robot::matrixToStream (tx, initialOrientation);
                tx << (initiated ? "y " : "n ");
                tx << (mdh_visited ? "y " : "n ");
                tx << body->getName () << ' '
                   << material->getName () << ' ';
                if (geometry) {
                        tx << "y\n";
                        geometry->writeToFileTransfer (tx);
                } else
                        tx << "n\n";
        }

        bool SIG_Link::isMDHVisited (void) const
	{
	  return mdh_visited;
	}

        void SIG_Link::transformToDynaMechs ( SIG_Joint *predecessor,
					      DL_vector predBase,
					      DL_vector predDir,
					      DL_vector predHand )
	{
#ifdef SIG_DEBUG
	  SIGEL_Tools::SIG_IO::cerr << "\n\nTransforming link "
				    << getName()
				    <<" to DynaMechs!\n";
#endif

	  mdh_visited = true;

	  QList< SIG_Joint > successors;

	  SIG_Joint *actAdjacentJoint = adjacentJoints.first();

	  while (actAdjacentJoint)
	    {
	      if (actAdjacentJoint->continuable( this ))
		successors.append( actAdjacentJoint );

	      actAdjacentJoint = adjacentJoints.next();
	    };

	  SIG_Joint *realSuccessor = successors.first();

	  bool transformZ = predecessor;
	  bool transformX = realSuccessor;

	  DL_vector realNewXAxis;
	  DL_vector realNewYAxis;
	  DL_vector realNewZAxis;

	  DL_vector realNewOrigin;

	  SIG_Joint *actSuccessor = successors.first();

	  do
	    {
#ifdef SIG_DEBUG
	      if (actSuccessor)
		SIGEL_Tools::SIG_IO::cerr << "Transforming link "
					  << getName()
					  << " accordingly to successor "
					  << actSuccessor->getName()
					  << ".\n";
	      else
		SIGEL_Tools::SIG_IO::cerr << "Tansforming link "
					  << getName()
					  << " without successor.\n";
#endif

	      bool processingRealSuccessor = (actSuccessor == realSuccessor);

	      DL_vector newXAxis(1, 0, 0);
	      DL_vector newZAxis(0, 0, 1);

	      DL_vector newOrigin(0, 0, 0);

	      if (transformZ)
		{
		  newZAxis = predDir;
		  newZAxis.minusis( &predBase );
		  newZAxis.normalize();

#ifdef SIG_DEBUG
		  SIGEL_Tools::SIG_IO::cerr << "Z-Axis transformed:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << newZAxis.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

		};

	      if (transformX)
		{
		  bool successorsLeftLink = ( this == actSuccessor->getLeftLink() );

		  DL_vector succBase;
		  DL_vector succDir;
		  DL_vector succHand;

		  switch (actSuccessor->getJointType())
		    {
		    case SIG_Joint::tTranslationalJoint:
		      {
			SIG_TranslationalJoint *translationalJoint = static_cast< SIG_TranslationalJoint * >(actSuccessor);

			succBase = ( successorsLeftLink ) ? translationalJoint->getLeftBase() : translationalJoint->getRightBase();
			succDir = ( successorsLeftLink ) ? translationalJoint->getLeftDir() : translationalJoint->getRightDir();
			succHand = ( successorsLeftLink ) ? translationalJoint->getLeftFix() : translationalJoint->getRightFix();
		      };
		      break;
		    case SIG_Joint::tRotationalJoint:
		      {
			SIG_RotationalJoint *rotationalJoint = static_cast< SIG_RotationalJoint * >(actSuccessor);

			succBase = ( successorsLeftLink ) ? rotationalJoint->getLeftBase() : rotationalJoint->getRightBase();
			succDir = ( successorsLeftLink ) ? rotationalJoint->getLeftDir() : rotationalJoint->getRightDir();
			succHand = ( successorsLeftLink ) ? rotationalJoint->getLeftHand() : rotationalJoint->getRightHand();
		      };
		      break;
		    };

		  DL_vector otherAxis = succDir;
		  otherAxis.minusis( &succBase );
		  otherAxis.normalize();

		  calculateCommonNormal( predBase,
					 newZAxis,
					 succBase,
					 otherAxis,
					 newOrigin,
					 newXAxis );

#ifdef SIG_DEBUG
		  SIGEL_Tools::SIG_IO::cerr << "X-Axis transformed:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << newXAxis.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";

		  SIGEL_Tools::SIG_IO::cerr << "Origin transformed:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << newOrigin.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

		}
	      else if (transformZ)
		{
		  int firstIndex;	      

		  for (int i=0; i<3; i++)
		    if (newZAxis.get( i )!=0)
		      {
			firstIndex = i;
			break;
		      };

		  int secondIndex = (firstIndex + 1) % 3;

		  int thirdIndex = (secondIndex + 1) % 3;

		  newXAxis.set( firstIndex, - newZAxis.get( secondIndex ) );
		  newXAxis.set( secondIndex, newZAxis.get( firstIndex ) );
		  newXAxis.set( thirdIndex, 0 );

		  newOrigin = predBase;

#ifdef SIG_DEBUG
		  SIGEL_Tools::SIG_IO::cerr << "X-Axis transformed:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << newXAxis.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";

		  SIGEL_Tools::SIG_IO::cerr << "Origin transformed:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << newOrigin.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

		};

	      newXAxis.normalize();

	      double screwD = 0;
	      double screwTheta = 0;

	      if (processingRealSuccessor)
		{
		  realNewXAxis = newXAxis;
		  realNewZAxis = newZAxis;

		  realNewOrigin = newOrigin;
		}
	      else
		{
		  DL_vector originDistanceVector = newOrigin;
		  originDistanceVector.minusis( &realNewOrigin );

		  screwD = originDistanceVector.norm();

		  double const minimalDistanceMeasure = 0.00001;

		  if (screwD > minimalDistanceMeasure)
		    {
		      DL_vector translationDirectionVector = originDistanceVector;

		      translationDirectionVector.normalize();

		      if (translationDirectionVector.inprod( &newZAxis ) < 0)
			screwD *= -1;
		    };

		  screwTheta = tolerantACos( realNewXAxis.inprod( &newXAxis ) );

#ifdef _WINDOWS
			double const pi = ::atan( 1 ) * 4;
#else
			double const pi = std::atan( 1 ) * 4;
#endif			

		  double const maximalParallelityMeasure = 0.00001;

		  DL_vector xxNormalVector;
		  realNewXAxis.crossprod( &newXAxis, &xxNormalVector );

		  if (xxNormalVector.norm() > maximalParallelityMeasure)
		    {
		      xxNormalVector.normalize();

		      if (newZAxis.inprod( &xxNormalVector ) < 0)
			screwTheta = 2 * pi - screwTheta;
		    };

#ifdef SIG_DEBUG
		  SIGEL_Tools::SIG_IO::cerr << "screwD: " << screwD
					    << " screwTheta: " << screwTheta
					    << "\n";

		  DL_vector debugScrewTranslationVector = newZAxis;
		  debugScrewTranslationVector.timesis( screwD );

		  DL_vector debugTranslatedOrigin = realNewOrigin;
		  debugTranslatedOrigin.plusis( &debugScrewTranslationVector );

		  DL_matrix debugScrewRotationMatrix = rotationMatrix( newZAxis,
								       screwTheta );

		  DL_vector debugHelpXAxis = realNewXAxis;
		  DL_vector debugRotatedXAxis;

		  debugScrewRotationMatrix.times( &debugHelpXAxis, &debugRotatedXAxis );

		  realNewZAxis.crossprod( &realNewXAxis, &realNewYAxis );
		  realNewYAxis.normalize();

		  DL_vector debugNewYAxis;
		  newZAxis.crossprod( &newXAxis, &debugNewYAxis );
		  debugNewYAxis.normalize();

		  DL_vector debugHelpYAxis = realNewYAxis;
		  DL_vector debugRotatedYAxis;

		  debugScrewRotationMatrix.times( &debugHelpYAxis, &debugRotatedYAxis );

		  SIGEL_Tools::SIG_IO::cerr << "New origin:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << newOrigin.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";

		  SIGEL_Tools::SIG_IO::cerr << "Translated real new origin:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << debugTranslatedOrigin.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";

		  SIGEL_Tools::SIG_IO::cerr << "New X-Axis:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << newXAxis.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";

		  SIGEL_Tools::SIG_IO::cerr << "Rotated real new X-Axis:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << debugRotatedXAxis.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";

		  SIGEL_Tools::SIG_IO::cerr << "New Y-Axis:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << debugNewYAxis.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";

		  SIGEL_Tools::SIG_IO::cerr << "Rotated real new Y-Axis:";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << debugRotatedYAxis.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

		};

	      if (actSuccessor)
		{
		  actSuccessor->transformToDynaMechs( this,
						      screwD,
						      screwTheta );

		  actSuccessor = successors.next();
		};
	    }
	  while (actSuccessor);

	  if (transformX || transformZ)
	    {
	      realNewZAxis.crossprod( &realNewXAxis, &realNewYAxis );
	      realNewYAxis.normalize();

#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "Y-Axis transformed:";
	      for (int i=0; i<3; i++)
		SIGEL_Tools::SIG_IO::cerr << " " << realNewYAxis.get( i );
	      SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

	      NEWMAT::Matrix newCoordinates( 3, 3 );
	      newCoordinates.Column( 1 ) = SIG_TypeConverter::toColumnVector( realNewXAxis );
	      newCoordinates.Column( 2 ) = SIG_TypeConverter::toColumnVector( realNewYAxis );
	      newCoordinates.Column( 3 ) = SIG_TypeConverter::toColumnVector( realNewZAxis );

	      NEWMAT::Matrix rotation = newCoordinates.i();

	      NEWMAT::ColumnVector translation = SIG_TypeConverter::toColumnVector( realNewOrigin ) * -1;

	      if (this->isRootLink())
		{
		  parent->initialLocation = SIG_TypeConverter::toDL_vector( translation * -1 );
		  parent->initialOrientation = SIG_TypeConverter::toDL_matrix( rotation.i() );

#ifdef SIG_DEBUG
		  SIGEL_Tools::SIG_IO::cerr << "Initial robot location: ";
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << parent->initialLocation.get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";
		  SIGEL_Tools::SIG_IO::cerr << "Initial robot orientation:\n";
		  for (int i=0; i<3; i++)
		    {
		      for (int j=0; j<3; j++)
			SIGEL_Tools::SIG_IO::cerr << parent->initialOrientation.get( i, j ) << " ";
		      SIGEL_Tools::SIG_IO::cerr << "\n";
		    };
#endif

		};

	      geometry->translate( SIG_TypeConverter::toDL_vector( translation ) );
	      geometry->rotate( SIG_TypeConverter::toDL_matrix( rotation ) );

	      transformPoints( SIG_TypeConverter::toDL_vector( translation ),
			       SIG_TypeConverter::toDL_matrix( rotation ) );

#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "Rotation matrix:\n";
	      for (int i=1; i<=3; i++)
		{
		  for (int j=1; j<=3; j++)
		    SIGEL_Tools::SIG_IO::cerr << rotation( i, j ) << " ";
		  SIGEL_Tools::SIG_IO::cerr << "\n";
		};
	      SIGEL_Tools::SIG_IO::cerr << "Translation vector:";
	      for (int i=1; i<=3; i++)
		SIGEL_Tools::SIG_IO::cerr << " " << translation( i );
	      SIGEL_Tools::SIG_IO::cerr << "\n";
	      SIGEL_Tools::SIG_IO::cerr << "Transformed points:\n";
	      QDictIterator< DL_vector > pit( points );
	      pit.toFirst();
	      while (pit.current())
		{
		  for (int i=0; i<3; i++)
		    SIGEL_Tools::SIG_IO::cerr << " " << pit.current()->get( i );
		  SIGEL_Tools::SIG_IO::cerr << "\n";
		  ++pit;
		};
#endif	      

	      mirtich->computeAgain();
	    };

#ifdef SIG_DEBUG
		  SIGEL_Tools::SIG_IO::cerr << "Link "
					    << getName()
					    << " transformed.\n";
#endif
	}

        void SIG_Link::calculateCommonNormal( DL_vector a,
					      DL_vector u,
					      DL_vector b,
					      DL_vector v,
					      DL_vector &c,
					      DL_vector &w )
	{
	  u.normalize();
	  v.normalize();

#ifdef SIG_DEBUG
	  SIGEL_Tools::SIG_IO::cerr << "Calculating common normal:\n"
				    << "a:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << a.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";
	  SIGEL_Tools::SIG_IO::cerr << "u:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << u.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";
	  SIGEL_Tools::SIG_IO::cerr << "b:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << b.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";
	  SIGEL_Tools::SIG_IO::cerr << "v:";
	  for (int i=0; i<3; i++)
	    SIGEL_Tools::SIG_IO::cerr << " " << v.get( i );
	  SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

	  c.init(0, 0, 0);
	  w.init(0, 0, 0);

	  double const minimalDistanceMeasure = 0.000001;

	  DL_vector uvNormal;
	  u.crossprod( &v, &uvNormal );
	  double parallelityMeasure = uvNormal.norm();

	  double const maximalParallelityMeasure = 0.000001; // Let's try other values!

	  bool uvParallel = ( parallelityMeasure <= maximalParallelityMeasure );

	  if (uvParallel)
	    {
#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "u and v are parallel.\n";
#endif

	      c = a;

	      double t0 = ( u.inprod( &c ) - u.inprod( &b ) ) / u.inprod( &v );
	      DL_vector p = b;
	      DL_vector dir = v;
	      dir.timesis( t0 );
	      p.plusis( &dir );

	      w = p;
	      w.minusis( &c );

	      if (w.norm() < minimalDistanceMeasure)
		{
		  QString message = "Axes of adjacent joints meeting in link " + getName() + " are overlapping!";
		  throw SIG_WrongKinematicsException( __FILE__,
						      __LINE__,
						      message );
		};

	      w.normalize();
	    }
	  else
	    {
#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "u and v are not parallel.\n";
#endif

	      u.crossprod( &v, &w );
	      w.normalize();

	      NEWMAT::Matrix linEqSystem( 3, 3 );
	      linEqSystem.Column( 1 ) = SIG_TypeConverter::toColumnVector( u );
	      linEqSystem.Column( 2 ) = SIG_TypeConverter::toColumnVector( w );
	      linEqSystem.Column( 3 ) = SIG_TypeConverter::toColumnVector( v ) * -1;

	      NEWMAT::ColumnVector rightSide = SIG_TypeConverter::toColumnVector( b ) - SIG_TypeConverter::toColumnVector( a );

	      NEWMAT::ColumnVector solutions = linEqSystem.i() * rightSide;

#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "Linear equation system:\n";
	      for (int i=1; i<=3; i++)
		{
		  for (int j=1; j<=3; j++)
		    SIGEL_Tools::SIG_IO::cerr << linEqSystem( i, j ) << " ";
		  SIGEL_Tools::SIG_IO::cerr << "\n";
		};

	      SIGEL_Tools::SIG_IO::cerr << "Right side";
	      for (int i=1; i<=3; i++)
		SIGEL_Tools::SIG_IO::cerr << " " << rightSide( i );
	      SIGEL_Tools::SIG_IO::cerr << "\n";

	      SIGEL_Tools::SIG_IO::cerr << "Solution vector (t0, t1, t2)^T:";
	      for (int i=1; i<=3; i++)
		SIGEL_Tools::SIG_IO::cerr << " " << solutions( i );
	      SIGEL_Tools::SIG_IO::cerr << "\n";
#endif

	      c = u;
	      c.timesis( solutions( 1 ) );
	      c.plusis( &a );

	      DL_vector cut = w;
	      cut.timesis( solutions( 2 ) );
	      cut.plusis( &c );

	      DL_vector hDir = cut;
	      hDir.minusis( &c );

	      if (hDir.norm() < minimalDistanceMeasure)
		{
		  QString message = "Axes of adjacent joints meeting in link " + getName() + " cut!";
		  throw SIG_WrongKinematicsException( __FILE__,
						      __LINE__,
						      message );
		};

	      hDir.normalize();

	      if (w.inprod( &hDir ) == -1)
		w.timesis( -1 );

	      w.normalize();
	    };

#ifdef SIG_DEBUG
	      SIGEL_Tools::SIG_IO::cerr << "c:";
	      for (int i=0; i<3; i++)
		SIGEL_Tools::SIG_IO::cerr << " " << c.get( i );
	      SIGEL_Tools::SIG_IO::cerr << "\n";

	      SIGEL_Tools::SIG_IO::cerr << "w:";
	      for (int i=0; i<3; i++)
		SIGEL_Tools::SIG_IO::cerr << " " << w.get( i );
	      SIGEL_Tools::SIG_IO::cerr << "\n";
#endif
	}
}
