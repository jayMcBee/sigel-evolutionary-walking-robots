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
#include <qvalidator.h>
#include <QLocale>
#include <QValidator>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qslider.h>
#include <qcheckbox.h>
#include "fparser.h"

#include "SIGEL_MasterGUI/SIG_EnvironmentView.h"

#include "pointvector.h"

namespace SIGEL_MasterGUI
{
/* 
 *  Constructs a SIG_Environment which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SIG_EnvironmentView::SIG_EnvironmentView( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
  : SIG_EnvironmentBase( parent, name, fl ), theExperiment( theExperiment )
{
  // gravity validators
  lineeditGravityX->setValidator( new QDoubleValidator( lineeditGravityX ) );
  lineeditGravityY->setValidator( new QDoubleValidator( lineeditGravityY ) );
  lineeditGravityZ->setValidator( new QDoubleValidator( lineeditGravityZ ) );

  // y plane level validators
  lineeditYPlaneLevel->setValidator( new QDoubleValidator( lineeditYPlaneLevel ) );

  // start position validators
  lineeditStartPositionX->setValidator( new QDoubleValidator( lineeditStartPositionX ) );
  lineeditStartPositionY->setValidator( new QDoubleValidator( lineeditStartPositionY ) );
  lineeditStartPositionZ->setValidator( new QDoubleValidator( lineeditStartPositionZ ) );

  // floor properties validators
	lineeditXDim->setValidator(new QIntValidator(lineeditXDim ) );
	lineeditZDim->setValidator(new QIntValidator(lineeditZDim ) );
	
  // validators for the dynaMechs lineedits
  lineeditPlanarSpringConstant->setValidator( new QDoubleValidator(lineeditPlanarSpringConstant) );
  lineeditNormalSpringConstant->setValidator( new QDoubleValidator(lineeditNormalSpringConstant) );
  lineeditPlanarDamperConstant->setValidator( new QDoubleValidator(lineeditPlanarDamperConstant) );
  lineeditNormalDamperConstant->setValidator( new QDoubleValidator(lineeditNormalDamperConstant) );
  lineeditStaticFrictionCoefficient->setValidator( new QDoubleValidator(lineeditStaticFrictionCoefficient) );
  lineeditKineticFrictionCoefficient->setValidator( new QDoubleValidator(lineeditKineticFrictionCoefficient) );


  // Qt 2 forced LC_NUMERIC="C" for the whole process (qapplication_x11.cpp:1389)
  // and its QDoubleValidator hard-coded '.' (qvalidator.cpp:387). Qt 6 validators
  // follow the system locale, but the read-back below is QString::toDouble(),
  // which is locale-independent and always wants '.'. Left to disagree, a typed
  // "9,81" validates under a comma-decimal locale and reads back as 0.
  for ( QValidator *v : findChildren<QValidator *>() )
    v->setLocale( QLocale::c() );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_EnvironmentView::~SIG_EnvironmentView()
{
    // no need to delete child widgets, Qt does it all for us
}

void SIG_EnvironmentView::putIntoExperiment()
{
  // put the gravity values out of the widgets into the experiment ( lineedits )
  DL_vector gravityVector( lineeditGravityX->text().toDouble(),
			   lineeditGravityY->text().toDouble(),
			   lineeditGravityZ->text().toDouble() );
  theExperiment.environment.setGravity( gravityVector );

  // floor parameters of the dynaMechs Environment
	theExperiment.environment.setFloorDimensionX(lineeditXDim->text().toInt());
	theExperiment.environment.setFloorDimensionZ(lineeditZDim->text().toInt());
	theExperiment.environment.setFloorFunction(lineeditFunction->text());
	theExperiment.environment.setFloorPictureFile(lineeditPictureFile->text());
	theExperiment.environment.setTextureFile(lineeditTextureFile->text());
	theExperiment.environment.setTexAlpha(sliderAlpha->value());
	
  // put the floor material into the experiment
  theExperiment.environment.setFloorMaterialName( comboboxMaterialName->currentText() );
  // put the y plane level into the experiment ( lineedit )
  theExperiment.environment.setYPlaneLevel( lineeditYPlaneLevel->text().toDouble() );

  DL_vector startPositionVector( lineeditStartPositionX->text().toDouble(),
				 lineeditStartPositionY->text().toDouble(),
				 lineeditStartPositionZ->text().toDouble() );
  theExperiment.environment.setStartPosition( startPositionVector );

  // put the dynaMechs values into the experiment
  theExperiment.environment.setGroundPlanarSpringConstant( lineeditPlanarSpringConstant->text().toDouble() );
  theExperiment.environment.setGroundNormalSpringConstant( lineeditNormalSpringConstant->text().toDouble() );
  theExperiment.environment.setGroundPlanarDamperConstant( lineeditPlanarDamperConstant->text().toDouble() );
  theExperiment.environment.setGroundNormalDamperConstant( lineeditNormalDamperConstant->text().toDouble() );
  theExperiment.environment.setFrictionCoeff_u_s( lineeditStaticFrictionCoefficient->text().toDouble() );
  theExperiment.environment.setFrictionCoeff_u_k( lineeditKineticFrictionCoefficient->text().toDouble() );
};

void SIG_EnvironmentView::getOutOfExperiment()
{
  // set the gravity vector according to the experiment (three lineedits)
  DL_vector gravityVector = theExperiment.environment.getGravity();
  lineeditGravityX->setText( QString::number( gravityVector.x) );
  lineeditGravityY->setText( QString::number( gravityVector.y) );
  lineeditGravityZ->setText( QString::number( gravityVector.z) );

  // get the material name
  // first build the combobox
  comboboxMaterialName->clear();
  const QList<SIGEL_Robot::SIG_Material *> &materialIterList = theExperiment.robot.getMaterials();
  for ( auto *materialIter : materialIterList )
    {
      comboboxMaterialName->addItem( materialIter->getName() );
    }
  bool wasAlreadyInserted = false;
  for( int i=0; i < comboboxMaterialName->count(); i++ )
    {
      if( comboboxMaterialName->itemText(i) == theExperiment.environment.getFloorMaterialName() )
	{
	  comboboxMaterialName->setCurrentIndex( i );
	  wasAlreadyInserted = true;
	  break;
	}
    }
  if( !wasAlreadyInserted)
    comboboxMaterialName->addItem( theExperiment.environment.getFloorMaterialName() );

  //  lineeditMaterialName->setText( theExperiment.environment.getFloorMaterialName() );
  // get the y plane level
  QString yPlaneLevel;
  yPlaneLevel.setNum( theExperiment.environment.getYPlaneLevel() );
  lineeditYPlaneLevel->setText( yPlaneLevel );

  // get the start position
  DL_vector startPositionVector = theExperiment.environment.getStartPosition();
  lineeditStartPositionX->setText( QString::number( startPositionVector.x ) );
  lineeditStartPositionY->setText( QString::number( startPositionVector.y ) );
  lineeditStartPositionZ->setText( QString::number( startPositionVector.z ) );

  // get the floor properties
  lineeditXDim->setText( QString::number( theExperiment.environment.getFloorDimensionX() ) );
  lineeditZDim->setText( QString::number( theExperiment.environment.getFloorDimensionZ() ) );
  lineeditFunction->setText( theExperiment.environment.getFloorFunction() );
  lineeditPictureFile->setText( theExperiment.environment.getFloorPictureFile() );
  lineeditTextureFile->setText(theExperiment.environment.getTextureFile());
	sliderAlpha->setValue(theExperiment.environment.getTexAlpha());
	checkboxTextureFile->setChecked(theExperiment.environment.getWithTexture());
	this->slotTextureSelect();
  radiobuttonFunction->setChecked(theExperiment.environment.getFloorFuncSelected());
  radiobuttonPictureFile->setChecked(!theExperiment.environment.getFloorFuncSelected());
  this->slotFloorSelectionChanged();
	
  // get the dynaMechs values
  lineeditPlanarSpringConstant->setText( QString::number( theExperiment.environment.getGroundPlanarSpringConstant() ) );
  lineeditNormalSpringConstant->setText( QString::number( theExperiment.environment.getGroundNormalSpringConstant() ) );
  lineeditPlanarDamperConstant->setText( QString::number( theExperiment.environment.getGroundPlanarDamperConstant() ) );
  lineeditNormalDamperConstant->setText( QString::number( theExperiment.environment.getGroundNormalDamperConstant() ) );
  lineeditStaticFrictionCoefficient->setText( QString::number( theExperiment.environment.getFrictionCoeff_u_s() ) );
  lineeditKineticFrictionCoefficient->setText( QString::number( theExperiment.environment.getFrictionCoeff_u_k() ) );
};
	
	void SIG_EnvironmentView::slotFloorSelectionChanged() {
		if (radiobuttonFunction->isChecked()) {
			lineeditFunction->setEnabled(true);
			lineeditXDim->setEnabled(true);
			lineeditZDim->setEnabled(true);
			lineeditPictureFile->setDisabled(true);
			pushbuttonSelectFile->setDisabled(true);
			theExperiment.environment.setFloorFuncSelected(true);
		}
		else { // picture-file selected
			lineeditPictureFile->setEnabled(true);
			pushbuttonSelectFile->setEnabled(true);
			lineeditFunction->setDisabled(true);
			lineeditXDim->setDisabled(true);
			lineeditZDim->setDisabled(true);
			theExperiment.environment.setFloorFuncSelected(false);
		}
	};
	
	void SIG_EnvironmentView::slotSelectFile() {
		QString picFile = QFileDialog::getOpenFileName( this, QString(), theExperiment.environment.getFloorPictureFile(), "Portable Greymap (*.pnm *.pgm)" );
		if (!picFile.isEmpty()) {
			lineeditPictureFile->setText(picFile);
			this->putIntoExperiment();
		}
	};
	
  void SIG_EnvironmentView::slotSelectTextureFile() {
  QString texFile = QFileDialog::getOpenFileName( this, QString(), theExperiment.environment.getTextureFile(), "Portable Pixmap (*.pnm *.ppm)" );
		if (!texFile.isEmpty()) {
			lineeditTextureFile->setText(texFile);
			this->putIntoExperiment();
		}
  };
  	
	void SIG_EnvironmentView::slotAlpha() {
		this->putIntoExperiment();
	};
	
	void SIG_EnvironmentView::slotTextureSelect() {
		if (checkboxTextureFile->isChecked()) {
			lineeditTextureFile->setEnabled(true);
			pushbuttonSelectTextureFile->setEnabled(true);
			sliderAlpha->setEnabled(true);
			theExperiment.environment.setWithTexture(true);
		}
		else {
			lineeditTextureFile->setDisabled(true);
			pushbuttonSelectTextureFile->setDisabled(true);
			sliderAlpha->setDisabled(true);
			theExperiment.environment.setWithTexture(false);
		}
	};

}
