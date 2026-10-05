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
#include <qheaderview.h>
#include <qlabel.h>
#include <qstyle.h>

#include "SIGEL_MasterGUI/SIG_RobotView.h"
#include "SIGEL_Robot/SIG_PitchRollSensor.h"
#include "SIGEL_Robot/SIG_Sensor.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Tools/SIG_Exception.h"


namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_Robot which is a child of 'parent', with the
 *  name 'name' and widget flags set to 'f' 
 */
SIG_RobotView::SIG_RobotView( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
  : SIG_RobotBase( parent, name, fl ), theExperiment( theExperiment )
{
  // The names take the room that the three number columns leave.
  listviewLinks->header()->setStretchLastSection( false );
  listviewLinks->header()->setSectionResizeMode( 0, QHeaderView::Stretch );
  for ( int column = 1; column < listviewLinks->columnCount(); column++ )
    {
      listviewLinks->header()->setSectionResizeMode( column, QHeaderView::ResizeToContents );
      listviewLinks->headerItem()->setTextAlignment( column, Qt::AlignRight | Qt::AlignVCenter );
    }

  QObject::connect( listviewIssues,
		    SIGNAL( itemSelectionChanged() ),
		    this,
		    SLOT( slotIssueSelected() ) );
}

void SIG_RobotView::putIntoExperiment()
{

}

void SIG_RobotView::getOutOfExperiment()
{  SIGEL_Robot::SIG_PitchRollSensor *prs;

   // need to know where we are located for accessing the pixmaps
  char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
  QString sigelRootString( sigelRootCString );

  // first clear the listboxes...
  listboxBodies->clear();
  listboxMaterials->clear();
  listviewLinks->clear();
  listboxJoints->clear();
  listboxDrives->clear();
  listboxSensors->clear();
  shownIssues.clear();
  listviewIssues->clear();
  texteditIssueDetail->setPlainText( "Press Check to examine the robot." );

   // now fill the 6 listboxes describing the robot properties;
   // Manage the Bodies listbox
   const QList<SIGEL_Robot::SIG_Body *> &bodyItList = theExperiment.robot.getBodies();
   textlabelBodies->setText( QString( "Bodies (%1)" ).arg( bodyItList.size() ) );
   for ( auto *bodyIt : bodyItList )
   {
      listboxBodies->addItem( bodyIt->getName() );
   }

   // Manage the Materials listbox
   const QList<SIGEL_Robot::SIG_Material *> &materialItList = theExperiment.robot.getMaterials();
   textlabelMaterials->setText( QString( "Materials (%1)" ).arg( materialItList.size() ) );
   for ( auto *materialIt : materialItList )
   {
      listboxMaterials->addItem( materialIt->getName() );
   }

   showLinks( sigelRootString );

   // Manage the Joints listbox
   const QList<SIGEL_Robot::SIG_Joint *> &jointItList = theExperiment.robot.getJoints();
   textlabelJoints->setText( QString( "Joints (%1)" ).arg( jointItList.size() ) );
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
   textlabelDrives->setText( QString( "Drives (%1)" ).arg( driveItList.size() ) );
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
   textlabelSensors->setText( QString( "Sensors (%1)" ).arg( sensorItList.size() ) );
   for ( auto *sensorIt : sensorItList )
   {
		// insert item with sensor name and pixmap indicating type
		switch (sensorIt->getSensorType())
		{	case SIGEL_Robot::SIG_Sensor::tJointSensor:     listboxSensors->addItem( new QListWidgetItem( QIcon( QPixmap( sigelRootString + "/pixmaps/sensor-J.xpm" ) ), sensorIt->getName() ) );
		                                                   break;

			case SIGEL_Robot::SIG_Sensor::tPitchRollSensor: prs = static_cast<SIGEL_Robot::SIG_PitchRollSensor*>(sensorIt);
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

void SIG_RobotView::showLinks( const QString &sigelRootString )
{
  const QList<SIGEL_Robot::SIG_Link *> &links = theExperiment.robot.getLinks();
  QString label = QString( "Links (%1)" ).arg( links.size() );

  // A robot whose masses cannot be computed still shows its links.
  QList<double> masses;
  try
    {
      masses = theExperiment.robot.getLinkMasses();
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      // The message's later lines name the source file that threw.
      label += "      No masses: " + e.getMessage().section( '\n', 0, 0 );
    }

  double totalMass = 0;
  for ( int i = 0; i < links.size(); i++ )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( listviewLinks );
      item->setText( 0, links[i]->getName() );

      if ( links[i] == theExperiment.robot.getRootLink() )
        {
          item->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/links-R.xpm" ) ) );
          item->setToolTip( 0, "The root link: the torso, from which the robot is built." );
        }
      else
        {
          item->setIcon( 0, QIcon( QPixmap( sigelRootString + "/pixmaps/links-L.xpm" ) ) );
          item->setToolTip( 0, "A link." );
        }

      if ( i < masses.size() )
        {
          double density = links[i]->getMaterial()->getDensity();
          item->setText( 1, QString::number( masses[i], 'f', 2 ) );
          item->setText( 2, QString::number( masses[i] / density, 'f', 5 ) );
          item->setText( 3, QString::number( density, 'f', 1 ) );
          totalMass += masses[i];
        }

      for ( int column = 1; column < listviewLinks->columnCount(); column++ )
        item->setTextAlignment( column, Qt::AlignRight | Qt::AlignVCenter );
    }

  if ( !links.isEmpty() && masses.size() == links.size() )
    label += QString( "      Total mass: %1 kg" ).arg( totalMass, 0, 'f', 2 );

  textlabelLinks->setText( label );
}

void SIG_RobotView::showIssues( const QList<SIGEL_RobotCheck::SIG_RobotIssue> &issues )
{
  shownIssues = issues;
  listviewIssues->clear();
  texteditIssueDetail->clear();

  if ( shownIssues.isEmpty() )
    {
      texteditIssueDetail->setPlainText( "The check found nothing." );
      return;
    }

  for ( const SIGEL_RobotCheck::SIG_RobotIssue &issue : shownIssues )
    {
      QTreeWidgetItem *item = new QTreeWidgetItem( listviewIssues );

      switch ( issue.kind )
        {
        case SIGEL_RobotCheck::SIG_RobotIssue::tError:
          item->setIcon( 0, style()->standardIcon( QStyle::SP_MessageBoxCritical ) );
          item->setText( 0, "Error" );
          break;

        case SIGEL_RobotCheck::SIG_RobotIssue::tWarning:
          item->setIcon( 0, style()->standardIcon( QStyle::SP_MessageBoxWarning ) );
          item->setText( 0, "Warning" );
          break;

        case SIGEL_RobotCheck::SIG_RobotIssue::tSuggestion:
          item->setIcon( 0, style()->standardIcon( QStyle::SP_MessageBoxInformation ) );
          item->setText( 0, "Suggestion" );
          break;
        }

      item->setText( 1, issue.title );
    }

  listviewIssues->resizeColumnToContents( 0 );
  listviewIssues->setCurrentItem( listviewIssues->topLevelItem( 0 ) );
}

void SIG_RobotView::slotIssueSelected()
{
  int row = listviewIssues->indexOfTopLevelItem( listviewIssues->currentItem() );
  if ( row < 0 || row >= shownIssues.size() )
    {
      texteditIssueDetail->clear();
      return;
    }

  const SIGEL_RobotCheck::SIG_RobotIssue &issue = shownIssues[ row ];

  QString detail = "<table cellspacing=\"4\">";
  if ( !issue.part.isEmpty() )
    detail += "<tr><td><b>Parts:</b></td><td>" + issue.part.toHtmlEscaped() + "</td></tr>";
  detail += "<tr><td><b>Analysis:</b></td><td>" + issue.analysis.toHtmlEscaped() + "</td></tr>";
  detail += "<tr><td><b>Advice:</b></td><td>" + issue.advice.toHtmlEscaped() + "</td></tr>";
  detail += "</table>";

  texteditIssueDetail->setHtml( detail );
}

}  //    { namespace SIGEL_MasterGUI }
