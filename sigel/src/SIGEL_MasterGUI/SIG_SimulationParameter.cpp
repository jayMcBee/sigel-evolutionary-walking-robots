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
#include <qspinbox.h>
#include <qcombobox.h>

#include "SIGEL_MasterGUI/SIG_SimulationParameter.h"

#include "SIGEL_Tools/SIG_IO.h" // only for DEBUG!

namespace SIGEL_MasterGUI
{

SIG_SimulationParameter::SIG_SimulationParameter( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
  : SIG_SimulationParameterBase( parent, name, fl ), theExperiment( theExperiment )
{
  lineeditStepSize->setValidator( new QDoubleValidator( lineeditStepSize ) );
  lineeditJointlimitsSpringConstant->setValidator( new QDoubleValidator( lineeditJointlimitsSpringConstant ) );
  lineeditJointlimitsDamperConstant->setValidator( new QDoubleValidator( lineeditJointlimitsDamperConstant ) );
  lineeditJointfrictionConstant->setValidator( new QDoubleValidator( lineeditJointfrictionConstant ) );

  // The read-back is QString::toDouble(), which always wants '.'. The C locale
  // alone still takes "0,375" as grouped, so the group separator is rejected.
  QLocale cLocale = QLocale::c();
  cLocale.setNumberOptions( QLocale::RejectGroupSeparator );
  for ( QValidator *v : findChildren<QValidator *>() )
    v->setLocale( cLocale );
}

void SIG_SimulationParameter::putIntoExperiment()
{
  // get time to simulate out of the widget (three spinboxes)
  QTime theTime( spinboxHours->value(),
		 spinboxMins->value(),
		 spinboxSecs->value() );
  theExperiment.simulationParameter.setTimeToSimulate( theTime );
  
  // get step size out of the widget (lineedit with double validator)
  double stepSize = lineeditStepSize->text().toDouble();
  theExperiment.simulationParameter.setStepSize( stepSize );
  
  // get random seed out of the widget (spinbox)
  int randomSeed = spinboxRandomSeed->value();
  theExperiment.simulationParameter.setRandomSeed( randomSeed );
  
  // Get the dynaMechs integrator out of the widgets
  switch( comboboxDynaMechsIntegrator->currentIndex() )
    {
      // euler
    case 0:
      theExperiment.simulationParameter.setDynaMechsIntegrator( SIGEL_Simulation::SIG_SimulationParameters::Euler );
      break;
      // runge kutta 4
    case 1:
      theExperiment.simulationParameter.setDynaMechsIntegrator( SIGEL_Simulation::SIG_SimulationParameters::RungeKutta4 );
      break;
      // runga kutta 45
    case 2:
      theExperiment.simulationParameter.setDynaMechsIntegrator( SIGEL_Simulation::SIG_SimulationParameters::RungeKutta45 );
      break;
    }

  // get the joint constants out of the widgets
  double newSpringConstant = lineeditJointlimitsSpringConstant->text().toDouble();
  theExperiment.simulationParameter.setJointLimitsK_spring( newSpringConstant );
  double newDamperConstant = lineeditJointlimitsDamperConstant->text().toDouble();
  theExperiment.simulationParameter.setJointLimitsB_damper( newDamperConstant );
  double newFrictionConstant = lineeditJointfrictionConstant->text().toDouble();
  theExperiment.simulationParameter.setJointFrictionU_c( newFrictionConstant );
  /*
   * Some stuff has still to be set.
   */
}

void SIG_SimulationParameter::getOutOfExperiment()
{
  // Set the time to simulate widgets (three spinboxes)
  QTime theTime = theExperiment.simulationParameter.getTimeToSimulate();
  spinboxHours->setValue( theTime.hour() );
  spinboxMins->setValue( theTime.minute() );
  spinboxSecs->setValue( theTime.second() );

  // Set the step size widget (line edit with double validator)
  QString stepSize;
  stepSize.setNum( theExperiment.simulationParameter.getStepSize() );
  lineeditStepSize->setText( stepSize );

  // Set the random seed widget (spinbox)
  spinboxRandomSeed->setValue( theExperiment.simulationParameter.getRandomSeed() );

  // Set the dynaMechs integrator widget
  switch( theExperiment.simulationParameter.getDynaMechsIntegrator() )
    {
      // euler
    case SIGEL_Simulation::SIG_SimulationParameters::Euler:
      comboboxDynaMechsIntegrator->setCurrentIndex( 0 );
      break;
      // runge kutta 4
    case SIGEL_Simulation::SIG_SimulationParameters::RungeKutta4:
      comboboxDynaMechsIntegrator->setCurrentIndex( 1 );
      break;
      // runga kutta 45
    case SIGEL_Simulation::SIG_SimulationParameters::RungeKutta45:
      comboboxDynaMechsIntegrator->setCurrentIndex( 2 );
      break;
    }

  lineeditJointlimitsSpringConstant->setText( QString::number( theExperiment.simulationParameter.getJointLimitsK_spring() ) );
  lineeditJointlimitsDamperConstant->setText( QString::number( theExperiment.simulationParameter.getJointLimitsB_damper() ) );
  lineeditJointfrictionConstant->setText( QString::number( theExperiment.simulationParameter.getJointFrictionU_c() ) );
  /*
   * Some stuff has still to be set!
   */
}

}
