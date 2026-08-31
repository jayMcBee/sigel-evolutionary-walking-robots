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
#include <qradiobutton.h>

#include "SIGEL_MasterGUI/SIG_SimulationParameter.h"

#include "SIGEL_Tools/SIG_IO.h" // only for DEBUG!

namespace SIGEL_MasterGUI
{

SIG_SimulationParameter::SIG_SimulationParameter( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
  : SIG_SimulationParameterBase( parent, name, fl ), theExperiment( theExperiment )
{
  lineeditStepSize->setValidator( new QDoubleValidator( lineeditStepSize ) );
  lineeditMaximalError->setValidator( new QDoubleValidator( lineeditMaximalError ) );
  lineeditJointlimitsSpringConstant->setValidator( new QDoubleValidator( lineeditJointlimitsSpringConstant ) );
  lineeditJointlimitsDamperConstant->setValidator( new QDoubleValidator( lineeditJointlimitsDamperConstant ) );
  lineeditJointfrictionConstant->setValidator( new QDoubleValidator( lineeditJointfrictionConstant ) );

  // Qt 2 forced LC_NUMERIC="C" for the whole process (qapplication_x11.cpp:1389)
  // and its QDoubleValidator hard-coded '.' (qvalidator.cpp:387). Qt 6 validators
  // follow the system locale, but the read-back below is QString::toDouble(),
  // which is locale-independent and always wants '.'. Left to disagree, a typed
  // "9,81" validates under a comma-decimal locale and reads back as 0.
  for ( QValidator *v : findChildren<QValidator *>() )
    v->setLocale( QLocale::c() );
}

SIG_SimulationParameter::~SIG_SimulationParameter()
{
    // no need to delete child widgets, Qt does it all for us
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
  
  // get maximal error out of the widget (lineedit with double validator)
  double maximalError = lineeditMaximalError->text().toDouble();
  theExperiment.simulationParameter.setMaximalError( maximalError );

  // get the skipped frames out of the widget (spinbox)
  theExperiment.simulationParameter.setSkipFrames( spinboxSkippedFrames->value() );

  // get the maximal iterations out of the widget (spinbox)
  theExperiment.simulationParameter.setMaximalIterations( spinboxMaximalIterations->value() );

  // get the maximal collision loops out of the widget (spinbox)
  theExperiment.simulationParameter.setMaximalCollisionLoops( spinboxMaximalCollisionLoops->value() );

  // get the integrator out of the widget (combobox)
  switch( comboboxIntegrator->currentIndex() )
    {
    case 0:
      theExperiment.simulationParameter.setIntegrator( SIGEL_Simulation::SIG_SimulationParameters::itRungeKutta4);
      break;
    case 1:
      theExperiment.simulationParameter.setIntegrator( SIGEL_Simulation::SIG_SimulationParameters::itRungeKutta2);
      break;
    case 2:
      theExperiment.simulationParameter.setIntegrator( SIGEL_Simulation::SIG_SimulationParameters::itDoubleEuler);
      break;
    case 3:
      theExperiment.simulationParameter.setIntegrator( SIGEL_Simulation::SIG_SimulationParameters::itEuler);
      break;
    }

  // get the solve mode out of the widget (combobox)
  switch( comboboxSolveMode->currentIndex() )
    {
    case 0:
      theExperiment.simulationParameter.setSolveMode( SIGEL_Simulation::SIG_SimulationParameters::smtSingleValueDecomposition );
      break;
    case 1:
      theExperiment.simulationParameter.setSolveMode( SIGEL_Simulation::SIG_SimulationParameters::smtConjugateGradient );
      break;
    case 2:
      theExperiment.simulationParameter.setSolveMode( SIGEL_Simulation::SIG_SimulationParameters::smtLUDecomposition );
      break;
    }
  
  // get the CM mode out of the widget (radiobuttons)
  if( radiobuttonCMModeAnalytical->isChecked() )
    theExperiment.simulationParameter.setAnalytical( true );
  else
    theExperiment.simulationParameter.setAnalytical( false );

  theExperiment.simulationParameter.setMaximalSOLIDIterations( spinboxMaxSOLIDIterations->value() );

  // Get the simulation library out of the widget
  if( radiobuttonDynamo->isChecked() )
    theExperiment.simulationParameter.setSimulationLibrary( SIGEL_Simulation::SIG_SimulationParameters::DynaMo );
  else
    theExperiment.simulationParameter.setSimulationLibrary( SIGEL_Simulation::SIG_SimulationParameters::DynaMechs );

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

  // Set the maximal error widget (line edit with double validator)
  QString maximalError;
  maximalError.setNum( theExperiment.simulationParameter.getMaximalError() );
  lineeditMaximalError->setText( maximalError );

  // Set the skipped frames widget (spinbox)
  spinboxSkippedFrames->setValue( theExperiment.simulationParameter.getSkipFrames() );

  // Set the maximal iterations widget (spinbox)
  spinboxMaximalIterations->setValue( theExperiment.simulationParameter.getMaximalIterations() );

  // Set the maximal collision loops widget (spinbox)
  spinboxMaximalCollisionLoops->setValue( theExperiment.simulationParameter.getMaximalCollisionLoops() );

  // Set the integrator combobox
  switch( theExperiment.simulationParameter.getIntegrator() )
    {
    case SIGEL_Simulation::SIG_SimulationParameters::itEuler:
      comboboxIntegrator->setCurrentIndex( 3 );
      break;
    case SIGEL_Simulation::SIG_SimulationParameters::itDoubleEuler:
      comboboxIntegrator->setCurrentIndex( 2 );
      break;
    case SIGEL_Simulation::SIG_SimulationParameters::itRungeKutta2:
      comboboxIntegrator->setCurrentIndex( 1 );
      break;
    case SIGEL_Simulation::SIG_SimulationParameters::itRungeKutta4:
      comboboxIntegrator->setCurrentIndex( 0 );
      break;
    }
  
  // Set the solve mode widget (combobox)
  switch( theExperiment.simulationParameter.getSolveMode() )
    {
    case SIGEL_Simulation::SIG_SimulationParameters::smtSingleValueDecomposition:
      comboboxSolveMode->setCurrentIndex( 0 );
      break;
    case SIGEL_Simulation::SIG_SimulationParameters::smtConjugateGradient:
      comboboxSolveMode->setCurrentIndex( 1 );
      break;
    case SIGEL_Simulation::SIG_SimulationParameters::smtLUDecomposition:
      comboboxSolveMode->setCurrentIndex( 2 );
      break;
    }

  // set the CM (Constraints Manager) mode widget (radio button)
  if( theExperiment.simulationParameter.getAnalytical() )
    radiobuttonCMModeAnalytical->setChecked( true );
  else
    radiobuttonCMModeEmpirical->setChecked( true );
  
  spinboxMaxSOLIDIterations->setValue( theExperiment.simulationParameter.getMaximalSOLIDIterations() );
  
  // Set the simulation library widgets.
  if( theExperiment.simulationParameter.getSimulationLibrary() == SIGEL_Simulation::SIG_SimulationParameters::DynaMo )
    radiobuttonDynamo->setChecked( true );
  if( theExperiment.simulationParameter.getSimulationLibrary() == SIGEL_Simulation::SIG_SimulationParameters::DynaMechs )
    radiobuttonDynaMechs->setChecked( true );

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
