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
#ifndef SIGEL_SIMULATION_SIG_SIMULATIONPARAMETERS_H
#define SIGEL_SIMULATION_SIG_SIMULATIONPARAMETERS_H

#include <qdatetime.h>
#include <qtextstream.h>

namespace SIGEL_Simulation
{
  
  /**
   * This class holds all technical parameters of interest for a simulation run.
   *
   * This class is neither encapsulating the input for the simulation (like a
   * robot for example) nor actual simulation parameters (like the actual
   * frame number), but holds technical parameters that determine the quality
   * of the simulation (stepsize and allover simulation time for example).
   */
  class SIG_SimulationParameters
    {
    public:
      enum DynaMechsIntegrator {
	/// use Euler
	Euler,
	/// use Runge Kutta 4
	RungeKutta4,
	/// use Runge Kutta 45 (with adaptive stepsize)
	RungeKutta45
      };

      /**
       * The standard-constructor of the SIG_SimulationParameter class.
       *
       * It initializes all attributes to some type-dependendant standard value.
       * Before it makes sense to simulate with these SIG_SimulationParameter object,
       * the right values should be set by the appropriate methods.
       */
      SIG_SimulationParameters();
      /** This constructor gets all the data out of the file */
      SIG_SimulationParameters(QTextStream& file);
      /** All the data is written to a file */
      void writeToFile(QTextStream& file);
      /** All the data is read from the file */
      void readFromFile(QTextStream& file);

      /**
       * Sets the timeToSimulate attribute.
       * @param newTimeToSimulate The new time to simulate.
       */
      void setTimeToSimulate(QTime newTimeToSimulate);

      /**
       * Returns the amount of real time that the simulator has to simulate.
       * @return The real time that the simulator has to simulate.
       */
      QTime getTimeToSimulate() const;

      /**
       * Sets the stepSize attribute.
       * @param newStepSize The new step size.
       */
      void setStepSize(double newStepSize);

      /**
       * Returns the step size that the simulation uses.
       * @return The step size that the simulation uses.
       */
      double getStepSize() const;

      void setJointLimitsK_spring( double newValue );

      double getJointLimitsK_spring() const;

      void setJointLimitsB_damper( double newValue );

      double getJointLimitsB_damper() const;

      void setJointFrictionU_c( double newValue );

      double getJointFrictionU_c() const;

      /**
       * Sets the randomSeed attribute.
       * @param newRandomSeed The random seed.
       */
      void setRandomSeed(int newRandomSeed);

      /**
       * Returns the random seed used by the simulation.
       * @return The random seed used by the simulation.
       */
      int getRandomSeed() const;

      /**
       * Sets the dynamechs integrator to newDynaMechsIntegrator.
       * @param newDynaMechsIntegrator The integrator to be used by dynaMechs.
       */
      void setDynaMechsIntegrator( DynaMechsIntegrator newDynaMechsIntegrator );

      /**
       * Gets the dynamechs integrator.
       * @return The integrator to be used by dynaMechs.
       */
      DynaMechsIntegrator getDynaMechsIntegrator() const;

		/**
			*/
		void setNoise(float newNoise);

		/**
			*/
		float getNoise();

    private:

		/**
			*
			*/
		float noise;
      
      /**
       * The amount of real time that has to be simulated by the simulator.
       */
      QTime timeToSimulate;

      /**
       * The step size that is used for the simulation.
       */
      double stepSize;

      double jointLimitsK_spring;

      double jointLimitsB_damper;

      double jointFrictionU_c;

      /**
       * The random seed used in the simulation.
       */
      int randomSeed;

      /**
       * The integrator used in DynaMechs.
       */
      DynaMechsIntegrator dynaMechsIntegrator;
    };
  
}

#endif // SIGEL_SIMULATION_SIG_SIMULATIONPARAMETERS_H
