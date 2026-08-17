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
#include <qlabel.h>
#include <qlistbox.h>

#include "SIGEL_MasterGUI/SIG_RobotView.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_Drive.h"


namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_Robot which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f' 
 */
SIG_RobotView::SIG_RobotView( QWidget* parent,  const char* name, WFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
  : SIG_RobotBase( parent, name, fl ), theExperiment( theExperiment )
{
}

/*
 *  Destroys the object and frees any allocated resources
 */
SIG_RobotView::~SIG_RobotView()
{
    // no need to delete child widgets, Qt does it all for us
}

void SIG_RobotView::putIntoExperiment()
{

}

void SIG_RobotView::getOutOfExperiment()
{  SIGEL_Robot::SIG_PitchRollSensor *prs;

   // need to know where we are located for accessing the pixmaps
#ifdef _WINDOWS
  char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else
  char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif
  QString sigelRootString( sigelRootCString );

  // first clear the listboxes...
  listboxBodies->clear();
  listboxMaterials->clear();
  listboxLinks->clear();
  listboxJoints->clear();
  listboxDrives->clear();
  listboxSensors->clear();

   // now fill the 6 listboxes describing the robot properties;
   // Manage the Bodies listbox
   QDictIterator<SIGEL_Robot::SIG_Body> bodyIt = theExperiment.robot.getBodyIter();
   textlabelBodies->setText( "Bodies:  " + QString::number( bodyIt.count() ) );
   for( ; bodyIt.current(); ++bodyIt )
   {
      listboxBodies->insertItem( bodyIt.current()->getName() );
   }

   // Manage the Materials listbox
   QDictIterator<SIGEL_Robot::SIG_Material> materialIt = theExperiment.robot.getMaterialIter();
   textlabelMaterials->setText( "Materials:  " + QString::number( materialIt.count() ) );
   for( ; materialIt.current(); ++materialIt )
   {
      listboxMaterials->insertItem( materialIt.current()->getName() );
   }

   // Manage the Links listbox
   QDictIterator<SIGEL_Robot::SIG_Link> linkIt = theExperiment.robot.getLinkIter();
   textlabelLinks->setText( "Links:  " + QString::number( linkIt.count() ) );
   for( ; linkIt.current(); ++linkIt )
   {
      listboxLinks->insertItem( linkIt.current()->getName() );
   }

   // Manage the Joints listbox
   QDictIterator<SIGEL_Robot::SIG_Joint> jointIt = theExperiment.robot.getJointIter();
   textlabelJoints->setText( "Joints:  " + QString::number( jointIt.count() ) );
   for( ; jointIt.current(); ++jointIt )
   {
      // put small icons indicating the joint type
      switch (jointIt.current()->getJointType())
      {
         case SIGEL_Robot::SIG_Joint::tCylindricalJoint  :  listboxJoints->insertItem(  QPixmap( sigelRootString + "/pixmaps/joints-C.xpm" ), jointIt.current()->getName() );
                                    break;

         case SIGEL_Robot::SIG_Joint::tRotationalJoint   :  listboxJoints->insertItem(  QPixmap( sigelRootString + "/pixmaps/joints-R.xpm" ), jointIt.current()->getName() );
                                    break;

         case SIGEL_Robot::SIG_Joint::tTranslationalJoint:  listboxJoints->insertItem(  QPixmap( sigelRootString + "/pixmaps/joints-T.xpm" ), jointIt.current()->getName() );
                                    break;
      }
   }

   // Manage the Drives Listbox
   QDictIterator<SIGEL_Robot::SIG_Drive> driveIt = theExperiment.robot.getDriveIter();
   textlabelDrives->setText( "Drives:  " + QString::number( driveIt.count() ) );
   for( ; driveIt.current(); ++driveIt )
   {
      // put small icon indicating the type of drive we use
      switch (driveIt.current()->getMode())
      {
         case SIGEL_Robot::SIG_Drive::tForceMode      :  listboxDrives->insertItem( QPixmap( sigelRootString + "/pixmaps/motor-F.xpm" ), driveIt.current()->getName() );
                                                         break;

         case SIGEL_Robot::SIG_Drive::tRelativeMode   :  listboxDrives->insertItem( QPixmap( sigelRootString + "/pixmaps/motor-R.xpm" ), driveIt.current()->getName() );
                                                         break;

         case SIGEL_Robot::SIG_Drive::tAbsoluteMode   :  listboxDrives->insertItem( QPixmap( sigelRootString + "/pixmaps/motor-A.xpm" ), driveIt.current()->getName() );
                                                         break;

         case SIGEL_Robot::SIG_Drive::tServoSimpleMode:  listboxDrives->insertItem( QPixmap( sigelRootString + "/pixmaps/motor-S.xpm" ), driveIt.current()->getName() );
                                                         break;
      }
   }

   // Manage the Sensors Listbox
   QDictIterator<SIGEL_Robot::SIG_Sensor> sensorIt = theExperiment.robot.getSensorIter();
   textlabelSensors->setText( "Sensors:  " + QString::number( sensorIt.count() ) );
   for( ; sensorIt.current(); ++sensorIt )
   {
		// insert item with sensor name and pixmap indicating type
		switch (sensorIt.current()->getSensorType())
		{	case SIGEL_Robot::SIG_Sensor::tJointSensor:     listboxSensors->insertItem( QPixmap( sigelRootString + "/pixmaps/sensor-J.xpm" ), sensorIt.current()->getName() );
		                                                   break;

			case SIGEL_Robot::SIG_Sensor::tPitchRollSensor: prs = (SIGEL_Robot::SIG_PitchRollSensor *)sensorIt.current();
                                                         if (prs->IsPitchType())
                                                         {	listboxSensors->insertItem( QPixmap( sigelRootString + "/pixmaps/sensor-P.xpm" ), sensorIt.current()->getName() );
                                                         }
                                                         else
                                                         {	listboxSensors->insertItem( QPixmap( sigelRootString + "/pixmaps/sensor-R.xpm" ), sensorIt.current()->getName() );
                                                         }
                                                         break;

			case SIGEL_Robot::SIG_Sensor::tContactSensor:   listboxSensors->insertItem(  QPixmap( sigelRootString + "/pixmaps/sensor-C.xpm" ), sensorIt.current()->getName() );
															            break;
		}
   }
}


}  //    { namespace SIGEL_MasterGUI }
