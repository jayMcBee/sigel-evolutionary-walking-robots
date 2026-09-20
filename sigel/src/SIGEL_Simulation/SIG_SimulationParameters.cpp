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
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

#include "SIGEL_Tools/SIG_IO.h"

SIGEL_Simulation::SIG_SimulationParameters::SIG_SimulationParameters()
  : timeToSimulate(0,0,10),
    stepSize(0.02),
    maximalError(0.1),
    maximalIterations(100),
    maximalCollisionLoops(10),
    analytical(true),
    skipFrames(0),
    solveMode(smtConjugateGradient),
    integrator(itRungeKutta4),
    dynaMechsIntegrator( RungeKutta4 ),
    simulationLibrary( DynaMechs ),
    randomSeed(0),
    maximalSOLIDIterations(1),
    jointLimitsK_spring(50),
    jointLimitsB_damper(5),
    jointFrictionU_c(0.35)
{ };

SIGEL_Simulation::SIG_SimulationParameters::SIG_SimulationParameters(QTime timeToSimulate,
								     double stepSize,
								     double maximalError,
								     int maximalIterations,
			   					     int maximalCollisionLoops,
        		  					     bool analytical, 
        							     int skipFrames,
								     SolveModeType solveMode,
								     IntegratorType integrator,
								     DynaMechsIntegrator dynaMechsIntegrator,
								     SimulationLibrary simulationLibrary,
								     int maximalSOLIDIterations,
								     int randomSeed )
  : timeToSimulate(timeToSimulate),
    stepSize(stepSize),
    maximalError(maximalError),
    randomSeed(randomSeed),
    maximalIterations(maximalIterations),
    maximalCollisionLoops(maximalCollisionLoops),
    analytical(analytical),
    skipFrames(skipFrames),
    solveMode(solveMode),
    integrator(integrator),
    dynaMechsIntegrator( dynaMechsIntegrator ),
    simulationLibrary( simulationLibrary ),
    maximalSOLIDIterations(maximalSOLIDIterations)
{ };

void SIGEL_Simulation::SIG_SimulationParameters::readFromFile(QTextStream& file)
{
  QString s;
  while (!file.atEnd()) {
    s=file.readLine();

    if ( s == "TIMETOSIMULATE") {
     s=file.readLine();
     int hour=s.toInt();
     s=file.readLine();
     int minute=s.toInt();
     s=file.readLine();
     int second=s.toInt();
     s=file.readLine();
     int msec=s.toInt();
     timeToSimulate.setHMS(hour,minute,second,msec);
    }

    if ( s == "STEPSIZE") {
      s=file.readLine();
      stepSize=s.toDouble();
		}

    if ( s == "MAXIMALERROR") {
      s=file.readLine();
      maximalError=s.toDouble();
		}

    if ( s == "RANDOMSEED") {
      s=file.readLine();
      randomSeed=s.toInt();
		}

    if ( s == "MAXIMALITERATIONS") {
      s=file.readLine();
      maximalIterations=s.toInt();
		}

    if ( s == "SKIPFRAMES") {
      s=file.readLine();
      skipFrames=s.toInt();
		}

    if ( s == "ANALYTICAL") {
      s=file.readLine();
      int an=s.toInt();
      if (an==1)
        analytical=true;
      else
        analytical=false;
		}

    if ( s == "MAXIMALCOLLISIONLOOPS") {
      s=file.readLine();
      maximalCollisionLoops=s.toInt();
		}

    if ( s == "SOLVEMODE") {
      s=file.readLine();
      solveMode=static_cast<SolveModeType>(s.toInt());
		}

    if ( s == "INTEGRATOR") {
      s=file.readLine();
      integrator=static_cast<IntegratorType>(s.toInt());
		}

    if ( s == "DYNAMECHSINTEGRATOR") {
      s=file.readLine();
      dynaMechsIntegrator=static_cast<DynaMechsIntegrator>(s.toInt());
		}

    if ( s == "SIMULATIONLIBRARY") {
      s=file.readLine();
      simulationLibrary=static_cast<SimulationLibrary>(s.toInt());
		}

    if ( s == "MAXIMALSOLIDITERATIONS") {
      s=file.readLine();
      maximalSOLIDIterations=s.toInt();
		}

    if ( s == "JOINTLIMITSK_SPRING") {
      s=file.readLine();
      jointLimitsK_spring=s.toDouble();
		}

    if ( s == "JOINTLIMITSB_DAMPER") {
      s=file.readLine();
      jointLimitsB_damper=s.toDouble();
		}

    if ( s == "JOINTFRICTIONU_C") {
      s=file.readLine();
      jointFrictionU_c=s.toDouble();
	  }
  }

};

SIGEL_Simulation::SIG_SimulationParameters::SIG_SimulationParameters(QTextStream& file)
{
  readFromFile(file);
};

void SIGEL_Simulation::SIG_SimulationParameters::writeToFile(QTextStream& file)
{
  file << "TIMETOSIMULATE\n";
  file << timeToSimulate.hour() << "\n";
  file << timeToSimulate.minute() << "\n";
  file << timeToSimulate.second() << "\n";
  file << timeToSimulate.msec() << "\n"; 
  file << "STEPSIZE\n";
  file << stepSize << "\n";
  file << "MAXIMALERROR\n";
  file << maximalError << "\n";
  file << "RANDOMSEED\n";
  file << randomSeed << "\n";
  file << "MAXIMALITERATIONS\n";
  file << maximalIterations << "\n";
  file << "SKIPFRAMES\n";
  file << skipFrames << "\n";
  file << "ANALYTICAL\n";
  if (analytical)
    file << 1 << "\n";
  else
    file << 0 << "\n"; 
  file << "MAXIMALCOLLISIONLOOPS\n";
  file << maximalCollisionLoops << "\n";
  file << "SOLVEMODE\n";
  file << (static_cast<int>(solveMode)) << "\n";
  file << "INTEGRATOR\n";
  file << (static_cast<int>(integrator)) << "\n";
  file << "DYNAMECHSINTEGRATOR\n";
  file << (static_cast<int>(dynaMechsIntegrator)) << "\n";
  file << "SIMULATIONLIBRARY\n";
  file << (static_cast<int>(simulationLibrary)) << "\n";
  file << "MAXIMALSOLIDITERATIONS\n";
  file << maximalSOLIDIterations << "\n";
  file << "JOINTLIMITSK_SPRING\n";
  file << jointLimitsK_spring << "\n";
  file << "JOINTLIMITSB_DAMPER\n";
  file << jointLimitsB_damper << "\n";
  file << "JOINTFRICTIONU_C\n";
  file << jointFrictionU_c << "\n";
};

void SIGEL_Simulation::SIG_SimulationParameters::setTimeToSimulate(QTime newTimeToSimulate)
{
  timeToSimulate = newTimeToSimulate;
};

QTime SIGEL_Simulation::SIG_SimulationParameters::getTimeToSimulate() const
{
  return timeToSimulate;
};

void SIGEL_Simulation::SIG_SimulationParameters::setStepSize(double newStepSize)
{
  stepSize = newStepSize;
}

double SIGEL_Simulation::SIG_SimulationParameters::getStepSize() const
{
  return stepSize;
};

void SIGEL_Simulation::SIG_SimulationParameters::setJointLimitsK_spring( double newValue )
{
  jointLimitsK_spring = newValue;
};

double SIGEL_Simulation::SIG_SimulationParameters::getJointLimitsK_spring() const
{
  return jointLimitsK_spring;
};

void SIGEL_Simulation::SIG_SimulationParameters::setJointLimitsB_damper( double newValue )
{
  jointLimitsB_damper = newValue;
};

double SIGEL_Simulation::SIG_SimulationParameters::getJointLimitsB_damper() const
{
  return jointLimitsB_damper;
};

void SIGEL_Simulation::SIG_SimulationParameters::setJointFrictionU_c( double newValue )
{
  jointFrictionU_c = newValue;
};

double SIGEL_Simulation::SIG_SimulationParameters::getJointFrictionU_c() const
{
  return jointFrictionU_c;
};

void SIGEL_Simulation::SIG_SimulationParameters::setMaximalError(double newMaximalError)
{
  maximalError = newMaximalError;
};

double SIGEL_Simulation::SIG_SimulationParameters::getMaximalError() const
{
  return maximalError;
};

void SIGEL_Simulation::SIG_SimulationParameters::setRandomSeed(int newRandomSeed)
{
  randomSeed = newRandomSeed;
};

int SIGEL_Simulation::SIG_SimulationParameters::getRandomSeed() const
{
  return randomSeed;
};

void SIGEL_Simulation::SIG_SimulationParameters::setAnalytical(bool newAnalytical)
{
  analytical=newAnalytical;
};

bool SIGEL_Simulation::SIG_SimulationParameters::getAnalytical() const
{
  return analytical;
};

void SIGEL_Simulation::SIG_SimulationParameters::setMaximalIterations(int newMaximalIterations)
{
  maximalIterations=newMaximalIterations;
};

int SIGEL_Simulation::SIG_SimulationParameters::getMaximalIterations() const
{
  return maximalIterations;
};

void SIGEL_Simulation::SIG_SimulationParameters::setSkipFrames(int newSkipFrames)
{
  skipFrames=newSkipFrames;
};

int SIGEL_Simulation::SIG_SimulationParameters::getSkipFrames() const
{
  return skipFrames;
};

void SIGEL_Simulation::SIG_SimulationParameters::setMaximalCollisionLoops(int newMaximalCollisionLoops)
{
  maximalCollisionLoops=newMaximalCollisionLoops;
};

int SIGEL_Simulation::SIG_SimulationParameters::getMaximalCollisionLoops() const
{
  return maximalCollisionLoops;
};

void SIGEL_Simulation::SIG_SimulationParameters::setSolveMode(SolveModeType newSolveMode)
{
  solveMode=newSolveMode;
};

SIGEL_Simulation::SIG_SimulationParameters::SolveModeType SIGEL_Simulation::SIG_SimulationParameters::getSolveMode() const
{
  return solveMode;
};

void SIGEL_Simulation::SIG_SimulationParameters::setIntegrator(IntegratorType newIntegrator)
{
  integrator=newIntegrator;
};

SIGEL_Simulation::SIG_SimulationParameters::IntegratorType SIGEL_Simulation::SIG_SimulationParameters::getIntegrator() const
{
  return integrator;
};

void SIGEL_Simulation::SIG_SimulationParameters::setDynaMechsIntegrator( DynaMechsIntegrator newDynaMechsIntegrator )
{
  dynaMechsIntegrator = newDynaMechsIntegrator;
};

SIGEL_Simulation::SIG_SimulationParameters::DynaMechsIntegrator SIGEL_Simulation::SIG_SimulationParameters::getDynaMechsIntegrator() const
{
  return dynaMechsIntegrator;
};

void SIGEL_Simulation::SIG_SimulationParameters::setSimulationLibrary( SimulationLibrary newSimulationLibrary )
{
  simulationLibrary = newSimulationLibrary;
};

SIGEL_Simulation::SIG_SimulationParameters::SimulationLibrary SIGEL_Simulation::SIG_SimulationParameters::getSimulationLibrary() const
{
  return simulationLibrary;
};

void SIGEL_Simulation::SIG_SimulationParameters::setMaximalSOLIDIterations(int newMaximalSOLIDIterations)
{
  maximalSOLIDIterations=newMaximalSOLIDIterations;
};

int SIGEL_Simulation::SIG_SimulationParameters::getMaximalSOLIDIterations() const
{
  return maximalSOLIDIterations;
};


void SIGEL_Simulation::SIG_SimulationParameters::setNoise(float _noise) {
	noise = _noise;
};

float SIGEL_Simulation::SIG_SimulationParameters::getNoise() {
    return noise;
};
