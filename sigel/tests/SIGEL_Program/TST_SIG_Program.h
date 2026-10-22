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
#ifndef SIGEL_PROGRAM_TST_SIG_PROGRAM_H
#define SIGEL_PROGRAM_TST_SIG_PROGRAM_H

#include <QObject>
#include <QStringList>

namespace SIGEL_Program
{

  class SIG_Program;

  /**
   * The unit tests of SIG_Program. TST_SIG_GPOperations tests clampLengthToLimits.
   */
  class TST_SIG_Program : public QObject
  {
    Q_OBJECT

  private slots:

    void defaultConstructorGivesAnEmptyProgram();

    void appendLineAddsTheLineAtTheEnd();

    void insertLinePutsTheLineAtTheIndex_data();

    void insertLinePutsTheLineAtTheIndex();

    void deleteLineRemovesTheLineAndMovesTheNextLinesUp_data();

    void deleteLineRemovesTheLineAndMovesTheNextLinesUp();

    void deleteLinePastTheEndDeletesNothing();

    void getLineOutsideTheProgramThrows_data();

    void getLineOutsideTheProgramThrows();

    void setLineOverwritesOneLine();

    void setLineOutsideTheProgramThrows_data();

    void setLineOutsideTheProgramThrows();

    void getPrgLinesGivesTheLinesOfTheProgram();

    void printToStringGivesTheShownTextOfEveryLine();

    void writeToFileGivesTheFileTextOfEveryLine();

    void readFromFileReadsWhatWriteToFileWrote();

    void readFromFileSkipsLinesWithoutAnInstruction();

    void readFromFileReplacesTheOldLines();

    void exportProgramWritesTheFileTextAndImportProgramReadsIt();

    void exportProgramReplacesAnExistingFile();

    void importProgramOfAMissingFileKeepsTheProgram();

    void importProgramOfABadFileThrowsAndKeepsTheProgram();

    void generateRandomProgramKeepsTheLengthInsideTheLimits();

    void generateRandomProgramWithMaximumZeroGivesTheMinimumLength();

    void generateRandomProgramReplacesTheOldLines();

    void generateRandomProgramUsesTheInstructionProbabilities();

    void generateRandomProgramUsesOnlyTheAllowedCommands();

    void constructorWithParametersGivesTheProgramOfGenerateRandomProgram();

    void assignmentCopiesEveryLine();

    void assignmentLeavesTheSourceAlone();

    void assignmentOverALongerProgramLeavesNoOldLines();

    void selfAssignmentKeepsTheProgram();

  private:

    static constexpr int seeds = 100;

    /** Appends length lines to program; line i is ADD firstOperand + i, -i. */
    void appendLines( SIG_Program &program, int firstOperand, long length );

    /** The shown texts of the lines that appendLines appends. */
    QStringList expectedTexts( int firstOperand, long length );

    QStringList lineTexts( SIG_Program const &program );
  };

}

#endif // SIGEL_PROGRAM_TST_SIG_PROGRAM_H
