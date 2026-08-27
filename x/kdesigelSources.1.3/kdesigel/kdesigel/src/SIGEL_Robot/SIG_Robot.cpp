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
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include "SIGEL_Robot/SIG_RotationalJoint.h"
#include "SIGEL_Robot/SIG_TranslationalJoint.h"
#include "SIGEL_Robot/SIG_CylindricalJoint.h"
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Robot/SIG_JointSensor.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"


namespace SIGEL_Robot {

        SIG_Robot::SIG_Robot ()
                : language (new SIG_LanguageParameters ()),
                  rootlink (0),
		  initialLocation(0, 0, 0)
        {
	  initialOrientation.makeone();
	}

        SIG_Robot::SIG_Robot (QTextStream & tx)
                : language (0),
		  rootlink (0),
		  initialLocation(0, 0, 0)
        {
	  initialOrientation.makeone();
	  readFromFileTransfer (tx);
        }

        SIG_Robot::SIG_Robot (const SIG_Robot & rob)
	  : language (0),
	    rootlink (0),
	    initialLocation(0, 0, 0)
        {
	        initialOrientation.makeone();
                QString s;
                QTextStream ts (&s, QIODeviceBase::WriteOnly);
                rob.writeToFileTransfer (ts);
                QTextStream rs (&s, QIODeviceBase::ReadOnly);
                readFromFileTransfer (rs);
        }

        SIG_Robot::~SIG_Robot (void)
        {
                clear ();
                delete language;
        }

        void SIG_Robot::clear ()
        {
		initialLocation = DL_vector (0, 0, 0);
		initialOrientation.makeone ();

                delete language;
                language = new SIG_LanguageParameters ();
                rootlink = 0;

                // Freed in the reverse of construction order, as before.
                qDeleteAll (sensors);
                qDeleteAll (drives);
                qDeleteAll (joints);
                qDeleteAll (links);
                qDeleteAll (materials);
                qDeleteAll (bodies);

                sensors.clear ();
                drives.clear ();
                joints.clear ();
                links.clear ();
                materials.clear ();
                bodies.clear ();
        }

        void SIG_Robot::addBody (SIG_Body *b)
        {
                bodies.append (b);
        }

        void SIG_Robot::addMaterial (SIG_Material *m)
        {
                materials.append (m);
        }

        void SIG_Robot::addLink (SIG_Link *l, bool isroot)
        {
                links.append (l);
                if (isroot) {
                        // Something seems to be missing here, but I no longer
                        // have the faintest idea what.
                }
        }

        void SIG_Robot::addJoint (SIG_Joint *j)
        {
                joints.append (j);
        }

        void SIG_Robot::addDrive (SIG_Drive *d)
        {
                drives.append (d);
        }

        void SIG_Robot::addSensor (SIG_Sensor *s)
        {
                sensors.append (s);
        }

        void SIG_Robot::setLangParam (SIG_LanguageParameters *lp, int delprev)
        {
                if (delprev)
                        delete language;
                language = lp;
        }

        SIG_LanguageParameters *SIG_Robot::getLangParam (void) const
        {
                return language;
        }

        SIG_Body *SIG_Robot::lookupBody (QString n) const
        {
                for (SIG_Body *e : bodies)
                        if (e->getName () == n)
                                return e;
                return 0;
        }

        SIG_Material *SIG_Robot::lookupMaterial (QString n) const
        {
                for (SIG_Material *e : materials)
                        if (e->getName () == n)
                                return e;
                return 0;
        }

        SIG_Link *SIG_Robot::lookupLink (QString n) const
        {
                for (SIG_Link *e : links)
                        if (e->getName () == n)
                                return e;
                return 0;
        }

        SIG_Joint *SIG_Robot::lookupJoint (QString n) const
        {
                for (SIG_Joint *e : joints)
                        if (e->getName () == n)
                                return e;
                return 0;
        }

        SIG_Drive *SIG_Robot::lookupDrive (QString n) const
        {
                for (SIG_Drive *e : drives)
                        if (e->getName () == n)
                                return e;
                return 0;
        }

        SIG_Sensor *SIG_Robot::lookupSensor (QString n) const
        {
                for (SIG_Sensor *e : sensors)
                        if (e->getName () == n)
                                return e;
                return 0;
        }

        const QList<SIG_Body *> &SIG_Robot::getBodies (void) const
        {
                return bodies;
        }

        const QList<SIG_Material *> &SIG_Robot::getMaterials (void) const
        {
                return materials;
        }

        const QList<SIG_Link *> &SIG_Robot::getLinks (void) const
        {
                return links;
        }

        const QList<SIG_Joint *> &SIG_Robot::getJoints (void) const
        {
                return joints;
        }

        const QList<SIG_Drive *> &SIG_Robot::getDrives (void) const
        {
                return drives;
        }

        const QList<SIG_Sensor *> &SIG_Robot::getSensors (void) const
        {
                return sensors;
        }

        SIG_Link const *SIG_Robot::getRootLink (void) const
        {
                return rootlink;
        }

        int SIG_Robot::getNrOfPoints (void) const
        {
                int summa = 0;
                for (SIG_Link *l : links)
                        summa += l->getNrOfPoints ();
                return summa;
        }

        void SIG_Robot::setRootLink (SIG_Link *l)
        {
                rootlink = l;
        }

        void SIG_Robot::initiate (void)
        {
                if (rootlink) {
                        DL_vector rl (0, 0, 0);
                        DL_matrix ro;
                        ro.makeone ();
                        
                        rootlink->setInitialLocation (rl, ro);
                }
        }

        void SIG_Robot::loadGeometries (void)
        {
                for (SIG_Body *b : bodies)
                        b->load ();
        }

        void SIG_Robot::instantiateGeometries (void)
        {
                for (SIG_Link *l : links)
                        l->instantiateGeometry ();
        }

        void SIG_Robot::prepareDynaMo (void)
        {
                instantiateGeometries ();

                for (SIG_Link *l : links)
                        l->transformToDynaMo ();

		initiate();
        }

        void SIG_Robot::prepareDynaMechs (void)
        {
	  instantiateGeometries ();

	  rootlink->transformToDynaMechs( 0 );

	  initiate();

	  Q2PtrList< SIG_Joint > rootJoints = rootlink->getJoints();

	  SIG_Joint *actJoint = rootJoints.first();

	  while (actJoint)
	    {
	      actJoint->calculateMDH( rootlink );

	      actJoint = rootJoints.next();
	    };
        }

        void SIG_Robot::writeToFileTransfer (QTextStream & tx) const
        {
                tx << "StreamedRobot\n";

		vectorToStream (tx, initialLocation);
		matrixToStream (tx, initialOrientation);
                
                for (SIG_Body *e : bodies)
                        e->writeToFileTransfer (tx);
                
                for (SIG_Material *e : materials)
                        e->writeToFileTransfer (tx);

                for (SIG_Link *e : links)
                        e->writeToFileTransfer (tx);

                for (SIG_Joint *e : joints)
                        e->writeToFileTransfer (tx);

                for (SIG_Drive *e : drives)
                        e->writeToFileTransfer (tx);

                for (SIG_Sensor *e : sensors)
                        e->writeToFileTransfer (tx);

                tx << "RobotComplete\n";

                if (rootlink)
                        tx << rootlink->getName () << '\n';
                else
                        tx << "-\n";

                if (language)
                        language->writeToFileTransfer (tx);
                else
                        tx << "-\n";
        }

        void SIG_Robot::readFromFileTransfer (QTextStream & tx)
        {
                QString objtype, tmpstr;

                clear ();

                tx >> tmpstr;
                if (tmpstr != "StreamedRobot")
                        throw SIG_UnstreamingError (__FILE__, __LINE__,
                                                    "'StreamedRobot' should be first word");
		initialLocation = streamToVector (tx);
		initialOrientation = streamToMatrix (tx);
                tx >> objtype;
                while (objtype != "RobotComplete") {
                        if (objtype == "Body")
                                addBody (new SIG_Body (this, tx));
                        else if (objtype == "Material")
                                addMaterial (new SIG_Material (this, tx));
                        else if (objtype == "Link")
                                addLink (new SIG_Link (this, tx));
                        else if (objtype == "TranslationalJoint")
                                addJoint (new SIG_TranslationalJoint (this, tx));
                        else if (objtype == "RotationalJoint")
                                addJoint (new SIG_RotationalJoint (this, tx));
                        else if (objtype == "CylindricalJoint")
                                addJoint (new SIG_CylindricalJoint (this, tx));
                        else if (objtype == "GlueJoint")
                                addJoint (new SIG_GlueJoint (this, tx));
                        else if (objtype == "Drive")
                                addDrive (new SIG_Drive (this, tx));
                        else if (objtype == "JointSensor")
                                addSensor (new SIG_JointSensor (this, tx));
                        else if (objtype == "PitchRollSensor")
                                addSensor (new SIG_PitchRollSensor (this, tx));
                        else if (objtype == "ContactSensor")
                                addSensor (new SIG_ContactSensor (this, tx));

                        tx >> objtype;
                }

                tx >> tmpstr;
                if (tmpstr == "-")
                        rootlink = 0;
                else
                        rootlink = lookupLink (tmpstr);

                tx >> tmpstr;
                if (tmpstr == "LanguageParameters") {
                        delete language;
                        language = new SIG_LanguageParameters (tx);
                } else
                        language = 0;
        }

        void SIG_Robot::vectorToStream (QTextStream & tx, DL_vector vec)
        {
                tx << vec.get (0) << ' '
                   << vec.get (1) << ' '
                   << vec.get (2) << ' ';
        }

        DL_vector SIG_Robot::streamToVector (QTextStream & tx)
        {
                DL_Scalar x, y, z;
                tx >> x >> y >> z;
                return DL_vector (x, y, z);
        }

        void SIG_Robot::matrixToStream (QTextStream & tx, DL_matrix mat)
        {
                tx << mat.get (0,0) << ' ' << mat.get (0,1) << ' ' << mat.get (0,2) << ' '
                   << mat.get (1,0) << ' ' << mat.get (1,1) << ' ' << mat.get (1,2) << ' '
                   << mat.get (2,0) << ' ' << mat.get (2,1) << ' ' << mat.get (2,2) << ' ';
        }

        DL_matrix SIG_Robot::streamToMatrix (QTextStream & tx)
        {
                DL_matrix mat;
                for (int i = 0; i < 3; i++) {
                        for (int j = 0; j < 3; j++) {
                                DL_Scalar v;
                                tx >> v;
                                mat.set (i, j, v);
                        }
                }
                return mat;
        }

   void SIG_Robot::getRobotInformation(char *outStr, int maxOutLen)
	{ DL_Scalar link_mass, tot_mass;
	  DL_vector com;
	  DL_matrix it;
	  char   hlp_txt[256];
	  double link_vol,
	    dens;

	  // copy robot or it will be shred..
	  SIG_Robot modRob( *this );
	  modRob.prepareDynaMechs();
	  tot_mass = 0;
	  sprintf(outStr, "<B><U>Link Information</U></B><BR><UL>");

	  // first get the mass for all links
	  for (SIG_Link *link : modRob.links)
	    {
	      // get link info
	      link->getPhysics(link_mass, com, it);
	      tot_mass += link_mass;

	      // compute volume
	      dens = (link->getMaterial())->getDensity();
	      link_vol = link_mass/dens;

	      // append link info to string
	      sprintf(hlp_txt, "<LI><B>\"%s\":</B> &nbsp; Mass: %5.2f kg, &nbsp; Volume: %4.5f m^3, &nbsp; Density: %5.1f kg/m^3</LI>",
		      link->getName().toUtf8().constData(), link_mass, link_vol, dens);

	      // not too long?
	      if ( strlen(outStr)+strlen(hlp_txt) < maxOutLen)
	      { strcat(outStr, hlp_txt);
	      }
	      else
	      { break;
	      }
	    }

	  if (strlen(outStr)+16 < maxOutLen)
	  {  strcat(outStr, "</UL><BR>");
	  }

	  // do something with the information..
	  if (strlen(outStr)+128 < maxOutLen)
	  {  sprintf(hlp_txt, "<B><U>Robot Information</U></B><BR><UL><LI><B>Torso link:</B> &nbsp; '%s'</LI><LI><B>Total mass:</B> &nbsp; %7.2f kg</LI></UL>", modRob.rootlink->getName().toUtf8().constData(), tot_mass);
	     strcat(outStr, hlp_txt);
	  }
  }

}
