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
    jointLimitsK_spring(50),
    jointLimitsB_damper(5),
    jointFrictionU_c(0.35),
    randomSeed(0),
    dynaMechsIntegrator( RungeKutta4 )
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

    if ( s == "RANDOMSEED") {
      s=file.readLine();
      randomSeed=s.toInt();
		}

    if ( s == "DYNAMECHSINTEGRATOR") {
      s=file.readLine();
      dynaMechsIntegrator=static_cast<DynaMechsIntegrator>(s.toInt());
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
  // Nothing reads MAXIMALERROR, MAXIMALITERATIONS, SKIPFRAMES, ANALYTICAL,
  // MAXIMALCOLLISIONLOOPS, SOLVEMODE, INTEGRATOR or MAXIMALSOLIDITERATIONS;
  // fixed values keep the file format.
  file << "MAXIMALERROR\n";
  file << 0.1 << "\n";
  file << "RANDOMSEED\n";
  file << randomSeed << "\n";
  file << "MAXIMALITERATIONS\n";
  file << 100 << "\n";
  file << "SKIPFRAMES\n";
  file << 0 << "\n";
  file << "ANALYTICAL\n";
  file << 1 << "\n";
  file << "MAXIMALCOLLISIONLOOPS\n";
  file << 10 << "\n";
  file << "SOLVEMODE\n";
  file << 1 << "\n";
  file << "INTEGRATOR\n";
  file << 3 << "\n";
  file << "DYNAMECHSINTEGRATOR\n";
  file << (static_cast<int>(dynaMechsIntegrator)) << "\n";
  file << "SIMULATIONLIBRARY\n";
  file << 1 << "\n";   // DynaMechs, the only library
  file << "MAXIMALSOLIDITERATIONS\n";
  file << 1 << "\n";
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

void SIGEL_Simulation::SIG_SimulationParameters::setRandomSeed(int newRandomSeed)
{
  randomSeed = newRandomSeed;
};

int SIGEL_Simulation::SIG_SimulationParameters::getRandomSeed() const
{
  return randomSeed;
};

void SIGEL_Simulation::SIG_SimulationParameters::setDynaMechsIntegrator( DynaMechsIntegrator newDynaMechsIntegrator )
{
  dynaMechsIntegrator = newDynaMechsIntegrator;
};

SIGEL_Simulation::SIG_SimulationParameters::DynaMechsIntegrator SIGEL_Simulation::SIG_SimulationParameters::getDynaMechsIntegrator() const
{
  return dynaMechsIntegrator;
};

void SIGEL_Simulation::SIG_SimulationParameters::setNoise(float _noise) {
	noise = _noise;
};

float SIGEL_Simulation::SIG_SimulationParameters::getNoise() {
    return noise;
};
