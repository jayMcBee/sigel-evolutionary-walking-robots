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
                Q2DictIterator<SIG_Body> itbody (bodies);
                Q2DictIterator<SIG_Material> itmaterial (materials);
                Q2DictIterator<SIG_Link> itlink (links);
                Q2DictIterator<SIG_Joint> itjoint (joints);
                Q2DictIterator<SIG_Drive> itdrive (drives);
                Q2DictIterator<SIG_Sensor> itsensor (sensors);

		initialLocation = DL_vector (0, 0, 0);
		initialOrientation.makeone ();

                delete language;
                language = new SIG_LanguageParameters ();
                rootlink = 0;

                while (itsensor.current ()) { delete itsensor.current (); ++itsensor; }
                while (itdrive.current ()) { delete itdrive.current (); ++itdrive; }
                while (itjoint.current ()) { delete itjoint.current (); ++itjoint; }
                while (itlink.current ()) { delete itlink.current (); ++itlink; }
                while (itmaterial.current ()) { delete itmaterial.current (); ++itmaterial; }
                while (itbody.current ()) { delete itbody.current (); ++itbody; }

                sensors.clear ();
                drives.clear ();
                joints.clear ();
                links.clear ();
                materials.clear ();
                bodies.clear ();
        }

        void SIG_Robot::addBody (SIG_Body *b)
        {
                bodies.insert (b->getName (), b);
        }

        void SIG_Robot::addMaterial (SIG_Material *m)
        {
                materials.insert (m->getName (), m);
        }

        void SIG_Robot::addLink (SIG_Link *l, bool isroot)
        {
                links.insert (l->getName (), l);
                if (isroot) {
                        // Something seems to be missing here, but I no longer
                        // have the faintest idea what.
                }
        }

        void SIG_Robot::addJoint (SIG_Joint *j)
        {
                joints.insert (j->getName (), j);
        }

        void SIG_Robot::addDrive (SIG_Drive *d)
        {
                drives.insert (d->getName (), d);
        }

        void SIG_Robot::addSensor (SIG_Sensor *s)
        {
                sensors.insert (s->getName (), s);
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
                return bodies.find (n);
        }

        SIG_Material *SIG_Robot::lookupMaterial (QString n) const
        {
                return materials.find (n);
        }

        SIG_Link *SIG_Robot::lookupLink (QString n) const
        {
                return links.find (n);
        }

        SIG_Joint *SIG_Robot::lookupJoint (QString n) const
        {
                return joints.find (n);
        }

        SIG_Drive *SIG_Robot::lookupDrive (QString n) const
        {
                return drives.find (n);
        }

        SIG_Sensor *SIG_Robot::lookupSensor (QString n) const
        {
                return sensors.find (n);
        }

        Q2DictIterator<SIG_Body> SIG_Robot::getBodyIter (void) const
        {
                return Q2DictIterator<SIG_Body> (bodies);
        }

        Q2DictIterator<SIG_Material> SIG_Robot::getMaterialIter (void) const
        {
                return Q2DictIterator<SIG_Material> (materials);
        }

        Q2DictIterator<SIG_Link> SIG_Robot::getLinkIter (void) const
        {
                return Q2DictIterator<SIG_Link> (links);
        }

        Q2DictIterator<SIG_Joint> SIG_Robot::getJointIter (void) const
        {
                return Q2DictIterator<SIG_Joint> (joints);
        }

        Q2DictIterator<SIG_Drive> SIG_Robot::getDriveIter (void) const
        {
                return Q2DictIterator<SIG_Drive> (drives);
        }

        Q2DictIterator<SIG_Sensor> SIG_Robot::getSensorIter (void) const
        {
                return Q2DictIterator<SIG_Sensor> (sensors);
        }

        SIG_Link const *SIG_Robot::getRootLink (void) const
        {
                return rootlink;
        }

        int SIG_Robot::getNrOfPoints (void) const
        {
                int summa = 0;
                Q2DictIterator<SIG_Link> lit (links);
                SIG_Link *l;
                while (l = lit.current ()) {
                        summa += l->getNrOfPoints ();
                        ++lit;
                }
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
                Q2DictIterator<SIG_Body> iter = getBodyIter ();
                while (iter.current ()) {
                        iter.current()->load ();
                        ++iter;
                }
        }

        void SIG_Robot::instantiateGeometries (void)
        {
                Q2DictIterator<SIG_Link> iter = getLinkIter ();
                while (iter.current ()) {
                        iter.current ()->instantiateGeometry ();
                        ++iter;
                }
        }

        void SIG_Robot::prepareDynaMo (void)
        {
                instantiateGeometries ();

                Q2DictIterator<SIG_Link> iter (links);
                while (iter.current ()) {
                        iter.current ()->transformToDynaMo ();
                        ++iter;
                }

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
                Q2DictIterator<SIG_Body> itbody (bodies);
                Q2DictIterator<SIG_Material> itmaterial (materials);
                Q2DictIterator<SIG_Link> itlink (links);
                Q2DictIterator<SIG_Joint> itjoint (joints);
                Q2DictIterator<SIG_Drive> itdrive (drives);
                Q2DictIterator<SIG_Sensor> itsensor (sensors);

                tx << "StreamedRobot\n";

		vectorToStream (tx, initialLocation);
		matrixToStream (tx, initialOrientation);
                
                while (itbody.current ()) {
                        itbody.current ()->writeToFileTransfer (tx);
                        ++itbody;
                }
                
                while (itmaterial.current ()) {
                        itmaterial.current ()->writeToFileTransfer (tx);
                        ++itmaterial;
                }

                while (itlink.current ()) {
                        itlink.current ()->writeToFileTransfer (tx);
                        ++itlink;
                }

                while (itjoint.current ()) {
                        itjoint.current ()->writeToFileTransfer (tx);
                        ++itjoint;
                }

                while (itdrive.current ()) {
                        itdrive.current ()->writeToFileTransfer (tx);
                        ++itdrive;
                }

                while (itsensor.current ()) {
                        itsensor.current ()->writeToFileTransfer (tx);
                        ++itsensor;
                }

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
                        rootlink = links.find (tmpstr);

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
	  Q2DictIterator<SIG_Link> itlink (modRob.links);

	  tot_mass = 0;
	  sprintf(outStr, "<B><U>Link Information</U></B><BR><UL>");

	  // first get the mass for all links
	  while (itlink.current())
	    {
	      // get link info
	      itlink.current()->getPhysics(link_mass, com, it);
	      tot_mass += link_mass;

	      // compute volume
	      dens = (itlink.current()->getMaterial())->getDensity();
	      link_vol = link_mass/dens;

	      // append link info to string
	      sprintf(hlp_txt, "<LI><B>\"%s\":</B> &nbsp; Mass: %5.2f kg, &nbsp; Volume: %4.5f m^3, &nbsp; Density: %5.1f kg/m^3</LI>",
		      itlink.current()->getName().toUtf8().constData(), link_mass, link_vol, dens);

	      // not too long?
	      if ( strlen(outStr)+strlen(hlp_txt) < maxOutLen)
	      { strcat(outStr, hlp_txt);
	      }
	      else
	      { break;
	      }

	      ++itlink;
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
