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
      enum SimulationLibrary {
	/// use DynaMo
	DynaMo,
	/// use DynaMechs
	DynaMechs
      };

      enum DynaMechsIntegrator {
	/// use Euler
	Euler,
	/// use Runge Kutta 4
	RungeKutta4,
	/// use Runge Kutta 45 (with adaptive stepsize)
	RungeKutta45
      };

      enum SolveModeType {
       /** Solve using LU decomposition (constant framerate) */
       smtLUDecomposition,
       /** Solve using conjugate gradient (very variable framerate, better results, default) */
       smtConjugateGradient,
       /** Solve using single value decomposition (slowest but most stable) */
       smtSingleValueDecomposition
      };
      enum IntegratorType {
       /** use the Euler integrator */
       itEuler,
       /** use the DoubleEuler integrator */
       itDoubleEuler,
       /** use the RungeKutta2 integrator */
       itRungeKutta2,
       /** use the RungeKutta4 integrator */
       itRungeKutta4
      };
      /**
       * The standard-constructor of the SIG_SimulationParameter class.
       *
       * It initializes all attributes to some type-dependendant standard value.
       * Before it makes sense to simulate with these SIG_SimulationParameter object,
       * the right values should be set by the appropriate methods.
       */
      SIG_SimulationParameters();
      /**
       * The constructor of the SIG_SimulationParameter class.
       *
       * The SIG_SimulationParameter object is instantiated in the
       * constructor of the SIG_Simulation object.
       * @param timeToSimulate Specifies the amount of model time to simulate.
       * @param stepSize The step size that dynamo has to use for the simulation.
       * @param maximalError The value of the maximal error to be allowed by dynamo.
       * @param maximalIterations The maximal Iterations of the constraints manager.
       * @param maximalCollisionLoops The maximal number of collision loops used by the constraints manager.
       * @param analytical The analytical Parameter of the constraints manager.
       * @param skipFrames The numbers of frames skipped by the constraints manager.
       * @param solveMode The Solve Mode used by the constraints manager.
       * @param integrator The Integrator used by Dynamo
       * @param randomSeed The random seed that should be used for the simulation.
       */
      SIG_SimulationParameters(QTime timeToSimulate,
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
			       int randomSeed = 0 );
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
       * Returns the step size that that dynamo uses in this simulation run.
       * @return The step size that that dynamo uses in this simulation run.
       */
      double getStepSize() const;

      void setJointLimitsK_spring( double newValue );

      double getJointLimitsK_spring() const;

      void setJointLimitsB_damper( double newValue );

      double getJointLimitsB_damper() const;

      void setJointFrictionU_c( double newValue );

      double getJointFrictionU_c() const;

      /**
       * Sets the maximalError attribute.
       * @param newMaximalError The new maximal error.
       */
      void setMaximalError(double newMaximalError);

      /**
       * Returns the maximal error that is allowed by dynamo.
       * @return The maximal error used by dynamo.
       */
      double getMaximalError() const;

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
       * Sets the analytical attribute.
       * @param newAnalytical sets the constraint manager to analytical or empirical.
       */
      void setAnalytical(bool newAnalytical);

      /**
       * Returns if the constraint manager is set to analytical or empirical.
       * @return the analytical attribute.
       */
      bool getAnalytical() const;

      /**
       * Sets the maximalIterations attribute.
       * @param newMaximalIterations The maximal Iterations used by the constraint manager.
       */
      void setMaximalIterations(int newMaximalIterations);

      /**
       * Returns the maximal iterations used by the constraint manager.
       * @return the maximalIterations attribute.
       */
      int getMaximalIterations() const;

      /**
       * Sets the skipFrames attribute.
       * @param newSkipFrames The amount of frames the constraint manager skips.
       */
      void setSkipFrames(int newSkipFrames);

      /**
       * Returns the amount of frames skipped by the constraint manager.
       * @return the skipFrames attribute.
       */
      int getSkipFrames() const;

      /**
       * Sets the maximalCollisionLoops attribute.
       * @param newMaximalCollisionLoops the amount of collision loops used by the constraint manager.
       */
      void setMaximalCollisionLoops(int newMaximalCollisionLoops);

      /**
       * Returns the maximal collision loops used by the constraint manager.
       * @return the maximalCollisionLoops attribute.
       */
      int getMaximalCollisionLoops() const;

      /**
       * Sets the solveMode attribute.
       * @param newSolveMode this sets the solve mode of the constraint manager.
       */
      void setSolveMode(SolveModeType newSolveMode);

      /**
       * Returns solve Mode of the constraint manager
       * @return the solveMode attribute.
       */
      SolveModeType getSolveMode() const;

      /**
       * Sets the integrator attribute.
       * @param newIntegrator the integrator used by dynamos dyna system.
       */
      void setIntegrator(IntegratorType newIntegrator);

      /**
       * Returns the integrator used by dynamos dyna system.
       * @return the integrator attribute.
       */
      IntegratorType getIntegrator() const;

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
       * Sets the used simulation library to newSimulationLibrary.
       * @param newSimulationLibrary The simulation library to use.
       */
      void setSimulationLibrary( SimulationLibrary newSimulationLibrary );

      /**
       * Gets the used simulation library.
       * @param simulationLibrary The simulation library to use.
       */
      SimulationLibrary getSimulationLibrary() const;
      
      /** Sets the amount of SOLID iterations
       * @param newMaximalSOLIDIterations is the number of iteration the DynaSystem tries to avoid collisions
       */
      void setMaximalSOLIDIterations(int newMaximalSOLIDIterations);

      /** Gets the amount of SOLID iterations
       * @return is the number of iteration the DynaSystem tries to avoid collisions
       */
      int getMaximalSOLIDIterations() const;

		/**
			*/
		void setNoise(float _noise);

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
       * The step size that is used by dynamo for the simulation.
       */
      double stepSize;

      /**
       * The maximal error that is allowed by dynamo.
       */
      double maximalError;

      double jointLimitsK_spring;

      double jointLimitsB_damper;

      double jointFrictionU_c;

      /**
       * The random seed used in the simulation.
       */
      int randomSeed;

      /** The method for the constraints manager,
       *
       * true = analytical (fast, default)
       *
       * false = empirical (slow)
       *
       * (additional information is in the dynamo documentation under constraint_manager)
       */
      bool analytical;

      /** The maximal iterations for the constraints manager,
       *
       * true means analytical, false empirical 
       * (additional information is in the dynamo documentation under constraint_manager)
       */
      int maximalIterations;

      /** The amount of skipped frames for the constraints manager,
       *
       * (additional information is in the dynamo documentation under constraint_manager)
       */
      int skipFrames;

      /** The maximal number collision loops for the constraints manager,
       *
       * true means analytical, false empirical 
       * (additional information is in the dynamo documentation under constraint_manager)
       */
      int maximalCollisionLoops;

      /** The solving mode for the constraints manager
       * 
       * (additional information is in the dynamo documentation under constraint_manager)
       */
      SolveModeType solveMode;

      /** The integrator for dynamo */
      IntegratorType integrator;

      /** The library which is used */
      SimulationLibrary simulationLibrary;

      /**
       * The integrator used in DynaMechs.
       */
      DynaMechsIntegrator dynaMechsIntegrator;
      
      /** The maximal iterations, the DynaSystem does to avoid collisions */
      int maximalSOLIDIterations;
      
    };
  
}

#endif // SIGEL_SIMULATION_SIG_SIMULATIONPARAMETERS_H
