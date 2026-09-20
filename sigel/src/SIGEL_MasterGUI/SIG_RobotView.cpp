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
SIG_RobotView::SIG_RobotView( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
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
   const QList<SIGEL_Robot::SIG_Body *> &bodyItList = theExperiment.robot.getBodies();
   textlabelBodies->setText( "Bodies:  " + QString::number( bodyItList.size() ) );
   for ( auto *bodyIt : bodyItList )
   {
      listboxBodies->addItem( bodyIt->getName() );
   }

   // Manage the Materials listbox
   const QList<SIGEL_Robot::SIG_Material *> &materialItList = theExperiment.robot.getMaterials();
   textlabelMaterials->setText( "Materials:  " + QString::number( materialItList.size() ) );
   for ( auto *materialIt : materialItList )
   {
      listboxMaterials->addItem( materialIt->getName() );
   }

   // Manage the Links listbox
   const QList<SIGEL_Robot::SIG_Link *> &linkItList = theExperiment.robot.getLinks();
   textlabelLinks->setText( "Links:  " + QString::number( linkItList.size() ) );
   for ( auto *linkIt : linkItList )
   {
      listboxLinks->addItem( linkIt->getName() );
   }

   // Manage the Joints listbox
   const QList<SIGEL_Robot::SIG_Joint *> &jointItList = theExperiment.robot.getJoints();
   textlabelJoints->setText( "Joints:  " + QString::number( jointItList.size() ) );
   for ( auto *jointIt : jointItList )
   {
      // put small icons indicating the joint type
      switch (jointIt->getJointType())
      {
         case SIGEL_Robot::SIG_Joint::tCylindricalJoint  :  listboxJoints->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/joints-C.xpm" ) ), jointIt->getName() ) );
                                    break;

         case SIGEL_Robot::SIG_Joint::tRotationalJoint   :  listboxJoints->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/joints-R.xpm" ) ), jointIt->getName() ) );
                                    break;

         case SIGEL_Robot::SIG_Joint::tTranslationalJoint:  listboxJoints->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/joints-T.xpm" ) ), jointIt->getName() ) );
                                    break;
      }
   }

   // Manage the Drives Listbox
   const QList<SIGEL_Robot::SIG_Drive *> &driveItList = theExperiment.robot.getDrives();
   textlabelDrives->setText( "Drives:  " + QString::number( driveItList.size() ) );
   for ( auto *driveIt : driveItList )
   {
      // put small icon indicating the type of drive we use
      switch (driveIt->getMode())
      {
         case SIGEL_Robot::SIG_Drive::tForceMode      :  listboxDrives->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/motor-F.xpm" ) ), driveIt->getName() ) );
                                                         break;

         case SIGEL_Robot::SIG_Drive::tRelativeMode   :  listboxDrives->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/motor-R.xpm" ) ), driveIt->getName() ) );
                                                         break;

         case SIGEL_Robot::SIG_Drive::tAbsoluteMode   :  listboxDrives->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/motor-A.xpm" ) ), driveIt->getName() ) );
                                                         break;

         case SIGEL_Robot::SIG_Drive::tServoSimpleMode:  listboxDrives->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/motor-S.xpm" ) ), driveIt->getName() ) );
                                                         break;
      }
   }

   // Manage the Sensors Listbox
   const QList<SIGEL_Robot::SIG_Sensor *> &sensorItList = theExperiment.robot.getSensors();
   textlabelSensors->setText( "Sensors:  " + QString::number( sensorItList.size() ) );
   for ( auto *sensorIt : sensorItList )
   {
		// insert item with sensor name and pixmap indicating type
		switch (sensorIt->getSensorType())
		{	case SIGEL_Robot::SIG_Sensor::tJointSensor:     listboxSensors->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/sensor-J.xpm" ) ), sensorIt->getName() ) );
		                                                   break;

			case SIGEL_Robot::SIG_Sensor::tPitchRollSensor: prs = (SIGEL_Robot::SIG_PitchRollSensor *)sensorIt;
                                                         if (prs->IsPitchType())
                                                         {	listboxSensors->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/sensor-P.xpm" ) ), sensorIt->getName() ) );
                                                         }
                                                         else
                                                         {	listboxSensors->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/sensor-R.xpm" ) ), sensorIt->getName() ) );
                                                         }
                                                         break;

			case SIGEL_Robot::SIG_Sensor::tContactSensor:   listboxSensors->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/sensor-C.xpm" ) ), sensorIt->getName() ) );
															            break;
		}
   }
}


}  //    { namespace SIGEL_MasterGUI }
