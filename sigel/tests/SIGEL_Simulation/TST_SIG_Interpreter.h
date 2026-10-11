/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef SIGEL_SIMULATION_TST_SIG_INTERPRETER_H
#define SIGEL_SIMULATION_TST_SIG_INTERPRETER_H

#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Simulation/SIG_CommandInterface.h"
#include "SIGEL_Simulation/SIG_SimulationQueries.h"

#include <QList>
#include <QObject>

namespace SIGEL_Simulation
{

  /**
   * One call of moveDrive: the drive number and the value of each register.
   */
  struct TST_MoveRecord
  {
    int driveNumber;
    QList<int> registerValues;
  };

  /**
   * A command interface that only keeps a record of each call of moveDrive.
   * A MOVE in the program is thus the way a test reads the registers.
   */
  class TST_RecordingCommandInterface : public SIG_CommandInterface
  {
  public:

    void moveDrive( int driveNumber, QList<SIG_Register> const &registers ) override;

    const QList<TST_MoveRecord> &getMoveRecords() const;

  private:

    QList<TST_MoveRecord> moveRecords;
  };

  /**
   * Simulation queries with one fixed sensor value. sense loads the sensor
   * value into register 0 and the sensor number into register 1.
   * SIG_Interpreter uses no other query; each of them throws.
   */
  class TST_FixedSimulationQueries : public SIG_SimulationQueries
  {
  public:

    TST_FixedSimulationQueries( int sensorValue );

    void sense( int sensorNumber, QList<SIG_Register> &registers ) const override;

    QTime getCurrentSimulationWholeSeconds() const override;

    double getCurrentSimulationSeconds() const override;

    SIG_Vector getLinkPosition( int linkNo ) const override;

    SIG_Matrix getLinkOrientation( int linkNo ) const override;

    int getRootNumber() const override;

    std::vector<double*>* getUsedForces() const override;

    int getLinkCount() const override;

    void checkDynas() const override;

  private:

    int sensorValue;
  };

  /**
   * The unit tests of SIG_Interpreter.
   */
  class TST_SIG_Interpreter : public QObject
  {
    Q_OBJECT

  private slots:

    void registersStartAtZero();

    void memorySizeGivesTheNumberOfRegisters();

    void constructorRefusesRegisterWidthZero();

    void emptyProgramThrows();

    void moveTakesItsOwnDuration();

    void moveThatIsNotAllowedIsSkipped();

    void loadPutsTheValueInTheRegister();

    void loadWrapsTheValueToTheRegisterWidth();

    void registerWidthComesFromTheLanguageParameters();

    void loadThatIsNotAllowedIsSkipped();

    void moveGivesTheRegisterValueAsDriveNumber();

    void moveRegisterOperandWraps();

    void negativeRegisterOperandWithEightRegisters();

    void negativeRegisterOperandWithSixRegisters_data();

    void negativeRegisterOperandWithSixRegisters();

    void timeAccountOfThreeRunsThreeCommands();

    void timeAccountOfZeroRunsNothing();

    void exceededTimeIsSubtractedFromTheNextCall();

    void exceededTimeSmallerThanTheTimeAccountIsSubtractedFromThatCall();

    void exceededTimeLargerThanTheTimeAccountIsSubtractedOverSeveralCalls();

    void remainingLastCommandTimeIsSubtractedOnlyOnce();

  private:

    /**
     * Gives each allowed command the same duration.
     */
    void setAllDurations( SIGEL_Robot::SIG_LanguageParameters &languageParameters, double duration );

    void appendLine( SIGEL_Program::SIG_Program &program, SIGEL_Program::Robotinstruction instruction, int operand1, int operand2 );
  };

}

#endif // SIGEL_SIMULATION_TST_SIG_INTERPRETER_H
