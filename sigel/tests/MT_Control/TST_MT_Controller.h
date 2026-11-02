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
#ifndef MT_CONTROL_TST_MT_CONTROLLER_H
#define MT_CONTROL_TST_MT_CONTROLLER_H

#include "SIGEL_GP/SIG_GPMetaGPExperiment.h"

#include <QApplication>
#include <QByteArray>
#include <QList>
#include <QMessageBox>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTimer>

#include <memory>

/**
 * What the dialog timer does with the next dialog. A message box gets a click on
 * button. A file dialog gets fileName, and an empty fileName cancels it.
 */
struct TST_MT_DialogAnswer
{
	QMessageBox::StandardButton button = QMessageBox::NoButton;
	QString fileName;
};

/**
 * The unit tests of MT_Controller, run on the controller of a
 * SIG_GPMetaGPExperiment.
 *
 * The class makes its own QApplication on the offscreen platform, because
 * sigel/tests/main.cpp has none. Each test gets its own folder as SIGEL_ROOT and
 * as current folder, with a stdConf.mt in it.
 *
 * loadEstimationState, saveEstimationState and writeEstimationSettings are
 * private. The tests reach them through the public calls that use them, and read
 * the result in the saved file or in the substitute's own writeToFile.
 *
 * A timer answers the dialogs from the list that expectButton and expectFileName
 * fill. cleanup fails a test that leaves an answer unused or meets a dialog with
 * no answer waiting.
 */
class TST_MT_Controller : public QObject
{
	Q_OBJECT

private slots:

	void initTestCase();

	void cleanupTestCase();

	void init();

	void cleanup();

	void newControllerIsOffAndHasNoKind();

	void newControllerGivesNoSubstitute();

	void useMetaEnablesTheSelectedKind();

	void useMetaWithTheCurrentStateReturnsFalse();

	void useMetaKeepsTheKindThroughDisableAndEnable();

	void disableDialogKeepsTheSystem();

	void disableDialogRemovesTheSystem();

	void disableDialogRemovedSystemStartsFromTheDefault();

	void disableDialogSavesAndRemovesTheSystem();

	void disableDialogCancelledSaveStaysEnabled();

	void switchSystemBeforeASystemExists_data();

	void switchSystemBeforeASystemExists();

	void switchSystemToTheKindInUseNeverAsks();

	void switchSystemAnsweredYesDeletesTheSystem();

	void switchSystemAnsweredNoKeepsTheSystem();

	void switchSystemAnsweredYesClosesTheWindow();

	void switchSystemKeepsTheKindThroughDisableAndEnable_data();

	void switchSystemKeepsTheKindThroughDisableAndEnable();

	void readFromFileSetsTheKindOfTheFile_data();

	void readFromFileSetsTheKindOfTheFile();

	void readFromFileKeepsTheLoadedKindThroughDisableAndEnable_data();

	void readFromFileKeepsTheLoadedKindThroughDisableAndEnable();

	void readFromFileRefusesAFileWithoutASubstitute_data();

	void readFromFileRefusesAFileWithoutASubstitute();

	void readFromFileRefusalDisablesARunningSystem();

	void readFromFileWithoutANameLoadsTheDefault_data();

	void readFromFileWithoutANameLoadsTheDefault();

	void missingFileAnsweredStandardLoadsTheDefault();

	void missingFileAnsweredStandardWithoutADefaultFails();

	void missingFileAnsweredDeactivateFails();

	void missingFileAnsweredDeactivateDisablesARunningSystem();

	void readFromFileReadsAMexpFileAndSavesToIt();

	void readFromFileThenSaveThenBuildsTheSystem();

	void saveNameIsClearedByRemove();

	void saveNameIsKeptByDisable();

	void saveNameIsClearedByLoadingTheDefault();

	void saveNameIsClearedByTheStandardAnswer();

	void saveNameIsClearedByTheDeactivateAnswer();

	void writeToFileWithMetaOffWritesNothing();

	void writeToFileOfAnUnchangedLoadedFileKeepsTheFile();

	void writeToFileRoundTrip_data();

	void writeToFileRoundTrip();

	void writeToFileOfTheDefaultWithoutAGuiUsesLastExperiment_data();

	void writeToFileOfTheDefaultWithoutAGuiUsesLastExperiment();

	void writeToFileAsksForANameAndAddsMexp_data();

	void writeToFileAsksForANameAndAddsMexp();

	void writeToFileCancelledDialogFails();

	void writeToFileSavesAfterACancelledDialog();

	void saveEstimationStateWritesTheLoadedValues_data();

	void saveEstimationStateWritesTheLoadedValues();

	void substituteGetsTheLoadedEstimation_data();

	void substituteGetsTheLoadedEstimation();

	void estimationOfTheOtherKindTakesTheDefaultSetup_data();

	void estimationOfTheOtherKindTakesTheDefaultSetup();

	void estimationStateFollowsTheSubstituteIntoTheWindow();

	void getFitnessTrainerOnlyForAnEvaluator();

	void getClassifierOnlyForAClassifier();

	void getFitnessTrainerKeepsItsSubstitute();

	void getClassifierKeepsItsSubstitute();

	void startEvolutionWithMetaOffOnlyClearsStartWOSigel();

	void stopEvolutionRunsSafelyWithAManager_data();

	void stopEvolutionRunsSafelyWithAManager();

	void configureSystemWithoutAFileLoadsTheDefault();

	void slotSaveSetupWritesEstimationAndSetup();

	void slotSaveSetupKeepsTheMcnfSuffix();

	void slotSaveSetupCancelledWritesNothing();

	void slotLoadSetupLoadsAnotherKindWhileEnabled();

	void slotLoadSetupWhileDisabledTakesTheKindOfTheFile();

	void slotLoadSetupKindSurvivesDisableAndEnable();

	void slotLoadSetupCancelledChangesNothing();

	void slotLoadDefaultRestoresTheDefaultSetup();

	void slotLoadDefaultWhileDisabledTakesTheKindOfTheFile();

	void slotLoadDefaultWithoutTheFileChangesNothing();

private:

	static constexpr const char *defaultEvaluatorEstimation = "Evaluator:\n1\n0.5\n10\n0\n\n";
	static constexpr const char *defaultClassifierEstimation = "Classifier:\n3\n75\n10\n0\n\n";

	static constexpr const char *loadedEvaluatorEstimation = "Evaluator:\n2\n0.25\n7\n2\n11\n12\n13\n14\n\n";
	static constexpr const char *loadedClassifierEstimation = "Classifier:\n4\n60\n5\n1\n21\n22\n\n";

	static constexpr int defaultPopulationSize = 10;

	// The part of a configuration file after the estimation sections.
	static const char *const gpSetup;

	std::unique_ptr<QApplication> application;
	int applicationArgumentCount = 1;
	char applicationName[10] = "sigeltest";

	// QApplication keeps these for its whole life.
	char *applicationArguments[2] = { applicationName, nullptr };
	QByteArray previousRoot;
	QByteArray previousPlatform;
	QString previousFolder;

	std::unique_ptr<QTemporaryDir> settingsFolder;
	std::unique_ptr<QTemporaryDir> folder;
	std::unique_ptr<SIGEL_GP::SIG_GPMetaGPExperiment> experiment;

	QTimer dialogTimer;
	QList<TST_MT_DialogAnswer> dialogAnswers;
	QString proposedFileName;
	int unanswerableDialogs = 0;

	MT_Controller &controller();

	// A one-link robot: without a root link MT_Evaluator cannot be built.
	void addRobot( SIGEL_GP::SIG_GPMetaGPExperiment &target );

	// The file's text: the usedSystem line, the estimation, then gpSetup.
	QString configuration( int usedSystem, const QString &estimation, int populationSize = defaultPopulationSize );

	// The text of a controller's default file.
	QString defaultConfiguration();

	// Writes text into the test's folder and returns the absolute path.
	QString writeFile( const QString &name, const QString &text );

	QString readFile( const QString &path );

	// The substitute of the kind in use.
	MT_Substitute *substitute();

	// The lines that substitute's writeToFile gives.
	QStringList substituteLines( MT_Substitute *substitute );

	// The lines of an estimation constant, without the empty line that ends it.
	QStringList estimationLines( const QString &estimation );

	// The fileName line that writeToFile gives; empty if the save fails.
	QString savedFileName();

	// Answers the save dialog of slotSaveSetup with name, and returns what it wrote.
	QString saveSetup( const QString &name );

	void expectButton( QMessageBox::StandardButton button );

	void expectFileName( const QString &fileName );

	void answerNextDialog();

	void restoreEnvironment( const char *name, const QByteArray &value );
};

#endif // MT_CONTROL_TST_MT_CONTROLLER_H
