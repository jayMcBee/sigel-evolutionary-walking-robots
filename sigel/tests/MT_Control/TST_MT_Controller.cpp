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
#include "MT_Control/TST_MT_Controller.h"

#include "MT_Control/MT_Controller.h"
#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Polygon.h"

#include <QAbstractButton>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QLineEdit>
#include <QSettings>
#include <QStandardPaths>
#include <QTextStream>
#include <QtTest>

// MT_Controller.cpp reads this global, which sigel.cpp and sigel_slave.cpp define.
bool guiEnabled = false;

const char *const TST_MT_Controller::gpSetup = R"(Generation:
0

Randomizer:
10
100
10
50
500
750
1000
500
500
400
700
1000
0
1000
2000
3000
4000
5000
6000
7000
8000
9000
10000
11000
12000
13000
14000
15000
16000
17000

Constant:
newConstant
30

Statistics:
0

OffspringSize:
100


Population:
%1

BrutSize:
2

TournamentManager:
1
10

FitnessTranier:
1
0
20
250
10

Trainingset:
0
20
0
)";

void TST_MT_Controller::initTestCase()
{
	previousRoot = qgetenv( "SIGEL_ROOT" );
	previousPlatform = qgetenv( "QT_QPA_PLATFORM" );
	previousFolder = QDir::currentPath();

	// QFileDialog keeps its history in the user's QtProject.conf.
	settingsFolder = std::make_unique<QTemporaryDir>();
	QVERIFY( settingsFolder->isValid() );
	QSettings::setPath( QSettings::NativeFormat, QSettings::UserScope, settingsFolder->path() );

	// A platform theme such as gtk3 would bring native dialogs that the timer cannot see.
	QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs, true );
	qputenv( "QT_QPA_PLATFORM", "offscreen" );
	application = std::make_unique<QApplication>( applicationArgumentCount, applicationArguments );

	dialogTimer.setInterval( 5 );
	QObject::connect( &dialogTimer, &QTimer::timeout, this, [this]() { answerNextDialog(); } );
}

void TST_MT_Controller::cleanupTestCase()
{
	application.reset();
	QCoreApplication::setAttribute( Qt::AA_DontUseNativeDialogs, false );
	QSettings::setPath( QSettings::NativeFormat, QSettings::UserScope, QStandardPaths::writableLocation( QStandardPaths::GenericConfigLocation ) );
	settingsFolder.reset();
	restoreEnvironment( "QT_QPA_PLATFORM", previousPlatform );
	restoreEnvironment( "SIGEL_ROOT", previousRoot );
}

void TST_MT_Controller::init()
{
	guiEnabled = false;
	dialogAnswers.clear();
	proposedFileName.clear();
	unanswerableDialogs = 0;

	folder = std::make_unique<QTemporaryDir>();
	QVERIFY( folder->isValid() );
	QVERIFY( QDir::setCurrent( folder->path() ) );
	qputenv( "SIGEL_ROOT", folder->path().toLocal8Bit() );
	writeFile( "stdConf.mt", defaultConfiguration() );

	experiment = std::make_unique<SIGEL_GP::SIG_GPMetaGPExperiment>();
	addRobot( *experiment );
	dialogTimer.start();
}

void TST_MT_Controller::cleanup()
{
	const int leftAnswers = dialogAnswers.size();
	const int unanswered = unanswerableDialogs;

	experiment.reset();
	dialogTimer.stop();
	guiEnabled = false;
	QDir::setCurrent( previousFolder );
	folder.reset();

	QCOMPARE( leftAnswers, 0 );
	QCOMPARE( unanswered, 0 );
}

void TST_MT_Controller::newControllerIsOffAndHasNoKind()
{
	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::NoMetaGP );
	QVERIFY( !controller().startWOSigel );
}

void TST_MT_Controller::useMetaEnablesTheSelectedKind()
{
	QVERIFY( controller().useMeta( true ) );

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );
}

void TST_MT_Controller::useMetaWithTheCurrentStateReturnsFalse()
{
	QVERIFY( !controller().useMeta( false ) );
	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::NoMetaGP );

	QVERIFY( controller().useMeta( true ) );
	QVERIFY( !controller().useMeta( true ) );
	QVERIFY( controller().IsEnabled() );
}

void TST_MT_Controller::useMetaKeepsTheKindThroughDisableAndEnable()
{
	controller().useMeta( true );

	QVERIFY( controller().useMeta( false ) );
	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );

	QVERIFY( controller().useMeta( true ) );
	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );
}

void TST_MT_Controller::disableDialogKeepsTheSystem()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) ) ) );
	MT_Substitute *before = substitute();
	QVERIFY( before );

	guiEnabled = true;
	expectButton( QMessageBox::Ignore );
	QVERIFY( controller().useMeta( false ) );
	guiEnabled = false;

	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( substitute(), before );
}

void TST_MT_Controller::disableDialogRemovesTheSystem()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) ) ) );
	QVERIFY( substitute() );

	guiEnabled = true;
	expectButton( QMessageBox::Discard );
	QVERIFY( controller().useMeta( false ) );
	guiEnabled = false;

	QVERIFY( !controller().IsEnabled() );

	// With nothing left to delete, switchSystem does not ask.
	QVERIFY( controller().useMeta( true ) );
	QVERIFY( controller().switchSystem( MT_SubstituteKind::Classifier ) );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );
}

void TST_MT_Controller::disableDialogRemovedSystemStartsFromTheDefault()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation, 14 ) ) ) );
	QVERIFY( substitute() );

	guiEnabled = true;
	expectButton( QMessageBox::Discard );
	QVERIFY( controller().useMeta( false ) );
	guiEnabled = false;

	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );

	const QString saved = saveSetup( "after" );
	const QStringList expected = QStringList( "usedSystem=1" ) + estimationLines( defaultEvaluatorEstimation );
	QCOMPARE( saved.split( '\n' ).mid( 0, expected.size() ), expected );
	QVERIFY( saved.contains( QString( "Population:\n%1\n" ).arg( defaultPopulationSize ) ) );
}

void TST_MT_Controller::disableDialogSavesAndRemovesTheSystem()
{
	const QString source = writeFile( "run.mexp", configuration( 2, loadedClassifierEstimation ) );
	QVERIFY( controller().readFromFile( source ) );
	QVERIFY( substitute() );

	guiEnabled = true;
	expectButton( QMessageBox::Save );
	expectFileName( "disabled" );
	QVERIFY( controller().useMeta( false ) );
	guiEnabled = false;

	QVERIFY( !controller().IsEnabled() );
	QVERIFY( readFile( folder->filePath( "disabled.mexp" ) ).startsWith( QString( "usedSystem=2\n" ) + "Classifier:\n4\n60\n5\n1\n21\n22\n" ) );
	QCOMPARE( readFile( source ), configuration( 2, loadedClassifierEstimation ) );

	// With nothing left to delete, switchSystem does not ask.
	QVERIFY( controller().useMeta( true ) );
	QVERIFY( controller().switchSystem( MT_SubstituteKind::Evaluator ) );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );
}

void TST_MT_Controller::disableDialogCancelledSaveStaysEnabled()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) ) ) );
	MT_Substitute *before = substitute();

	guiEnabled = true;
	expectButton( QMessageBox::Save );
	expectFileName( QString() );
	QVERIFY( !controller().useMeta( false ) );
	guiEnabled = false;

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( substitute(), before );
}

void TST_MT_Controller::switchSystemBeforeASystemExists_data()
{
	QTest::addColumn<int>( "wanted" );

	QTest::newRow( "Evaluator" ) << 1;
	QTest::newRow( "Classifier" ) << 2;
}

void TST_MT_Controller::switchSystemBeforeASystemExists()
{
	QFETCH( int, wanted );
	const MT_SubstituteKind kind = static_cast<MT_SubstituteKind>( wanted );
	QVERIFY( controller().useMeta( true ) );

	QVERIFY( controller().switchSystem( kind ) );

	QCOMPARE( controller().UsedSystem(), kind );
	QVERIFY( controller().IsEnabled() );
}

void TST_MT_Controller::switchSystemToTheKindInUseNeverAsks()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) ) ) );
	MT_Substitute *before = substitute();

	QVERIFY( controller().switchSystem( MT_SubstituteKind::Evaluator ) );

	QCOMPARE( substitute(), before );
}

void TST_MT_Controller::switchSystemAnsweredYesDeletesTheSystem()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation, 14 ) ) ) );
	QVERIFY( substitute() );

	expectButton( QMessageBox::Yes );
	QVERIFY( controller().switchSystem( MT_SubstituteKind::Classifier ) );

	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );
	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().getFitnessTrainer(), nullptr );

	// The new system starts from the default file, not from the deleted one.
	QCOMPARE( substituteLines( controller().getClassifier() ), estimationLines( defaultClassifierEstimation ) );
	controller().configureSystem( nullptr );
	QVERIFY( saveSetup( "after" ).contains( QString( "Population:\n%1\n" ).arg( defaultPopulationSize ) ) );
}

void TST_MT_Controller::switchSystemAnsweredNoKeepsTheSystem()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) ) ) );
	MT_Substitute *before = substitute();
	QVERIFY( before );

	expectButton( QMessageBox::No );
	QVERIFY( !controller().switchSystem( MT_SubstituteKind::Classifier ) );

	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );
	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().getFitnessTrainer(), before );
	QCOMPARE( controller().getClassifier(), nullptr );
}

void TST_MT_Controller::switchSystemAnsweredYesClosesTheWindow()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );
	const auto windowCount = []() { return QApplication::topLevelWidgets().size(); };
	const int openWindows = windowCount();
	QVERIFY( openWindows > 0 );

	expectButton( QMessageBox::Yes );
	QVERIFY( controller().switchSystem( MT_SubstituteKind::Classifier ) );

	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );
	QVERIFY( windowCount() < openWindows );
}

void TST_MT_Controller::switchSystemKeepsTheKindThroughDisableAndEnable_data()
{
	QTest::addColumn<bool>( "systemExists" );

	QTest::newRow( "no system" ) << false;
	QTest::newRow( "system exists" ) << true;
}

void TST_MT_Controller::switchSystemKeepsTheKindThroughDisableAndEnable()
{
	QFETCH( bool, systemExists );

	controller().useMeta( true );
	if( systemExists )
	{
		QVERIFY( controller().getFitnessTrainer() );
		expectButton( QMessageBox::Yes );
	}

	QVERIFY( controller().switchSystem( MT_SubstituteKind::Classifier ) );
	QVERIFY( controller().useMeta( false ) );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );

	QVERIFY( controller().useMeta( true ) );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );
}

void TST_MT_Controller::readFromFileSetsTheKindOfTheFile_data()
{
	QTest::addColumn<int>( "usedSystem" );
	QTest::addColumn<QString>( "estimation" );

	QTest::newRow( "Evaluator" ) << 1 << QString( loadedEvaluatorEstimation );
	QTest::newRow( "Classifier" ) << 2 << QString( loadedClassifierEstimation );
}

void TST_MT_Controller::readFromFileSetsTheKindOfTheFile()
{
	QFETCH( int, usedSystem );
	QFETCH( QString, estimation );

	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( usedSystem, estimation ) ) ) );

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), static_cast<MT_SubstituteKind>( usedSystem ) );
}

void TST_MT_Controller::readFromFileKeepsTheLoadedKindThroughDisableAndEnable_data()
{
	QTest::addColumn<int>( "usedSystem" );
	QTest::addColumn<QString>( "estimation" );

	QTest::newRow( "Evaluator" ) << 1 << QString( loadedEvaluatorEstimation );
	QTest::newRow( "Classifier" ) << 2 << QString( loadedClassifierEstimation );
}

void TST_MT_Controller::readFromFileKeepsTheLoadedKindThroughDisableAndEnable()
{
	QFETCH( int, usedSystem );
	QFETCH( QString, estimation );
	const MT_SubstituteKind kind = static_cast<MT_SubstituteKind>( usedSystem );

	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( usedSystem, estimation ) ) ) );
	QCOMPARE( controller().UsedSystem(), kind );

	QVERIFY( controller().useMeta( false ) );
	QVERIFY( controller().useMeta( true ) );

	QCOMPARE( controller().UsedSystem(), kind );
}

void TST_MT_Controller::readFromFileRefusesAFileWithoutASubstitute_data()
{
	QTest::addColumn<QString>( "firstLine" );

	QTest::newRow( "usedSystem=0" ) << "usedSystem=0";
	QTest::newRow( "unknown number" ) << "usedSystem=3";
	QTest::newRow( "no usedSystem line" ) << "Evaluator:";
}

void TST_MT_Controller::readFromFileRefusesAFileWithoutASubstitute()
{
	QFETCH( QString, firstLine );

	const QString text = firstLine + "\n" + loadedEvaluatorEstimation + QString( gpSetup ).arg( defaultPopulationSize );
	QVERIFY( !controller().readFromFile( writeFile( "bad.mexp", text ) ) );

	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::NoMetaGP );
}

void TST_MT_Controller::readFromFileWithoutANameLoadsTheDefault_data()
{
	QTest::addColumn<int>( "selected" );
	QTest::addColumn<QString>( "estimation" );

	QTest::newRow( "Evaluator" ) << 1 << QString( defaultEvaluatorEstimation );
	QTest::newRow( "Classifier" ) << 2 << QString( defaultClassifierEstimation );
}

void TST_MT_Controller::readFromFileWithoutANameLoadsTheDefault()
{
	QFETCH( int, selected );
	QFETCH( QString, estimation );
	const MT_SubstituteKind kind = static_cast<MT_SubstituteKind>( selected );
	QVERIFY( controller().useMeta( true ) );

	QVERIFY( controller().switchSystem( kind ) );
	QVERIFY( controller().readFromFile( QString() ) );

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), kind );
	QCOMPARE( substituteLines( substitute() ), estimationLines( estimation ) );
}

void TST_MT_Controller::missingFileAnsweredStandardLoadsTheDefault()
{
	guiEnabled = true;
	expectButton( QMessageBox::RestoreDefaults );
	QVERIFY( controller().readFromFile( folder->filePath( "missing.mexp" ) ) );
	guiEnabled = false;

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );
	QCOMPARE( substituteLines( substitute() ), estimationLines( defaultEvaluatorEstimation ) );
}

void TST_MT_Controller::missingFileAnsweredStandardWithoutADefaultFails()
{
	QVERIFY( QFile::remove( folder->filePath( "stdConf.mt" ) ) );

	guiEnabled = true;
	expectButton( QMessageBox::RestoreDefaults );
	QVERIFY( !controller().readFromFile( folder->filePath( "missing.mexp" ) ) );
	guiEnabled = false;

	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::NoMetaGP );
}

void TST_MT_Controller::missingFileAnsweredDeactivateFails()
{
	guiEnabled = true;
	expectButton( QMessageBox::Abort );
	QVERIFY( !controller().readFromFile( folder->filePath( "missing.mexp" ) ) );
	guiEnabled = false;

	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::NoMetaGP );
}

void TST_MT_Controller::readFromFileReadsAMexpFileAndSavesToIt()
{
	const QString path = writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation, 12 ) );
	QVERIFY( controller().readFromFile( path ) );
	QVERIFY( substitute() );

	QString saved;
	QTextStream stream( &saved );
	QVERIFY( controller().writeToFile( stream, "ignored" ) );

	QCOMPARE( saved, QString( "MetaInUse\nfileName=" ) + path + "\n#####\n" );
	QVERIFY( readFile( path ).contains( "OffspringSize:\n100\n" ) );
	QVERIFY( readFile( path ).contains( "Generation:\n0\n" ) );
	QVERIFY( readFile( path ) != configuration( 1, loadedEvaluatorEstimation, 12 ) );
}

void TST_MT_Controller::readFromFileThenSaveThenBuildsTheSystem()
{
	// The padding makes the file larger than the read block of the stream.
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, QString( loadedEvaluatorEstimation ) + QString( 40000, QChar( '\n' ) ), 14 ) ) ) );

	// saveSystem closes the file that createGPSystem reads next.
	QCOMPARE( savedFileName(), folder->filePath( "run.mexp" ) );

	QCOMPARE( substituteLines( substitute() ), estimationLines( loadedEvaluatorEstimation ) );
	controller().configureSystem( nullptr );
	QVERIFY( saveSetup( "after" ).contains( "Population:\n14\n" ) );
}

void TST_MT_Controller::saveNameIsClearedByRemove()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) ) ) );

	guiEnabled = true;
	expectButton( QMessageBox::Discard );
	QVERIFY( controller().useMeta( false ) );
	guiEnabled = false;
	QVERIFY( controller().useMeta( true ) );

	QCOMPARE( savedFileName(), QString( "lastExperiment.mexp" ) );
}

void TST_MT_Controller::saveNameIsKeptByDisable()
{
	const QString path = writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) );
	QVERIFY( controller().readFromFile( path ) );

	guiEnabled = true;
	expectButton( QMessageBox::Ignore );
	QVERIFY( controller().useMeta( false ) );
	guiEnabled = false;
	QVERIFY( controller().useMeta( true ) );

	QCOMPARE( savedFileName(), path );
}

void TST_MT_Controller::writeToFileWithMetaOffWritesNothing()
{
	QString saved;
	QTextStream stream( &saved );

	QVERIFY( controller().writeToFile( stream, "ignored" ) );

	QVERIFY( saved.isEmpty() );
	QVERIFY( !QFile::exists( folder->filePath( "lastExperiment.mexp" ) ) );
}

void TST_MT_Controller::writeToFileOfAnUnchangedLoadedFileKeepsTheFile()
{
	const QString text = configuration( 1, loadedEvaluatorEstimation );
	const QString path = writeFile( "run.mexp", text );
	QVERIFY( controller().readFromFile( path ) );

	QString saved;
	QTextStream stream( &saved );
	QVERIFY( controller().writeToFile( stream, "ignored" ) );

	QCOMPARE( saved, QString( "MetaInUse\nfileName=" ) + path + "\n#####\n" );
	QCOMPARE( readFile( path ), text );
}

void TST_MT_Controller::writeToFileRoundTrip_data()
{
	QTest::addColumn<int>( "usedSystem" );
	QTest::addColumn<QString>( "estimation" );
	QTest::addColumn<bool>( "withSubstitute" );

	QTest::newRow( "Evaluator with its substitute" ) << 1 << QString( loadedEvaluatorEstimation ) << true;
	QTest::newRow( "Classifier with its substitute" ) << 2 << QString( loadedClassifierEstimation ) << true;
	QTest::newRow( "Evaluator from the estimation state" ) << 1 << QString( loadedEvaluatorEstimation ) << false;
	QTest::newRow( "Classifier from the estimation state" ) << 2 << QString( loadedClassifierEstimation ) << false;
}

void TST_MT_Controller::writeToFileRoundTrip()
{
	QFETCH( int, usedSystem );
	QFETCH( QString, estimation );
	QFETCH( bool, withSubstitute );

	const QString path = writeFile( "run.mexp", configuration( usedSystem, estimation ) );
	QVERIFY( controller().readFromFile( path ) );
	if( withSubstitute )
	{
		QVERIFY( substitute() );
	}
	else
	{
		controller().configureSystem( nullptr );
	}

	QString saved;
	QTextStream stream( &saved );
	QVERIFY( controller().writeToFile( stream, "ignored" ) );

	SIGEL_GP::SIG_GPMetaGPExperiment reloadExperiment;
	addRobot( reloadExperiment );
	MT_Controller &reloaded = *reloadExperiment.mtController;
	QVERIFY( reloaded.readFromFile( path ) );

	QVERIFY( reloaded.IsEnabled() );
	QCOMPARE( reloaded.UsedSystem(), static_cast<MT_SubstituteKind>( usedSystem ) );
	MT_Substitute *reloadedSubstitute = usedSystem == 1 ? reloaded.getFitnessTrainer() : reloaded.getClassifier();
	QCOMPARE( substituteLines( reloadedSubstitute ), estimationLines( estimation ) );
}

void TST_MT_Controller::writeToFileOfTheDefaultWithoutAGuiUsesLastExperiment_data()
{
	QTest::addColumn<int>( "selected" );

	QTest::newRow( "Evaluator" ) << 1;
	QTest::newRow( "Classifier" ) << 2;
}

void TST_MT_Controller::writeToFileOfTheDefaultWithoutAGuiUsesLastExperiment()
{
	QFETCH( int, selected );
	QVERIFY( controller().useMeta( true ) );

	QVERIFY( controller().switchSystem( static_cast<MT_SubstituteKind>( selected ) ) );
	QVERIFY( controller().readFromFile( QString() ) );

	QString saved;
	QTextStream stream( &saved );
	QVERIFY( controller().writeToFile( stream, "ignored" ) );

	QCOMPARE( saved, QString( "MetaInUse\nfileName=lastExperiment.mexp\n#####\n" ) );
	QCOMPARE( readFile( folder->filePath( "lastExperiment.mexp" ) ), QString( "usedSystem=%1\n" ).arg( selected ) + defaultConfiguration() );
}

void TST_MT_Controller::writeToFileAsksForANameAndAddsMexp_data()
{
	QTest::addColumn<QString>( "typedName" );

	QTest::newRow( "without the suffix" ) << "chosen";
	QTest::newRow( "with the suffix" ) << "chosen.mexp";
}

void TST_MT_Controller::writeToFileAsksForANameAndAddsMexp()
{
	QFETCH( QString, typedName );
	const QString expected = folder->filePath( "chosen.mexp" );

	experiment->experimentName = "walker";
	QVERIFY( controller().useMeta( true ) );
	QVERIFY( controller().readFromFile( QString() ) );

	QString saved;
	QTextStream stream( &saved );
	guiEnabled = true;
	expectFileName( typedName );
	QVERIFY( controller().writeToFile( stream, "ignored" ) );
	guiEnabled = false;

	QCOMPARE( saved, QString( "MetaInUse\nfileName=" ) + expected + "\n#####\n" );
	QVERIFY( QFile::exists( expected ) );
	QCOMPARE( QFileInfo( proposedFileName ).fileName(), QString( "mt_walker" ) );
}

void TST_MT_Controller::writeToFileCancelledDialogFails()
{
	QVERIFY( controller().useMeta( true ) );
	QVERIFY( controller().readFromFile( QString() ) );

	QString saved;
	QTextStream stream( &saved );
	guiEnabled = true;
	expectFileName( QString() );
	QVERIFY( !controller().writeToFile( stream, "ignored" ) );
	guiEnabled = false;

	QVERIFY( saved.isEmpty() );
}

// A failed save that keeps evolutionMutex locked hangs the second writeToFile.
void TST_MT_Controller::writeToFileSavesAfterACancelledDialog()
{
	QVERIFY( controller().useMeta( true ) );
	QVERIFY( controller().readFromFile( QString() ) );
	QVERIFY( substitute() );

	QString cancelled;
	QTextStream cancelledStream( &cancelled );
	guiEnabled = true;
	expectFileName( QString() );
	QVERIFY( !controller().writeToFile( cancelledStream, "ignored" ) );
	QVERIFY( cancelled.isEmpty() );

	QString saved;
	QTextStream savedStream( &saved );
	expectFileName( "second" );
	QVERIFY( controller().writeToFile( savedStream, "ignored" ) );
	guiEnabled = false;

	QCOMPARE( saved, QString( "MetaInUse\nfileName=" ) + folder->filePath( "second.mexp" ) + "\n#####\n" );
	QVERIFY( readFile( folder->filePath( "second.mexp" ) ).contains( "Generation:\n0\n" ) );
}

void TST_MT_Controller::saveEstimationStateWritesTheLoadedValues_data()
{
	QTest::addColumn<int>( "usedSystem" );
	QTest::addColumn<QString>( "estimation" );

	QTest::newRow( "Evaluator" ) << 1 << QString( loadedEvaluatorEstimation );
	QTest::newRow( "Classifier" ) << 2 << QString( loadedClassifierEstimation );
}

// With no substitute, saveSystem takes the estimation state.
void TST_MT_Controller::saveEstimationStateWritesTheLoadedValues()
{
	QFETCH( int, usedSystem );
	QFETCH( QString, estimation );

	const QString path = writeFile( "run.mexp", configuration( usedSystem, estimation ) );
	QVERIFY( controller().readFromFile( path ) );
	controller().configureSystem( nullptr );

	QString saved;
	QTextStream stream( &saved );
	QVERIFY( controller().writeToFile( stream, "ignored" ) );

	const QStringList lines = readFile( path ).split( '\n' );
	const QStringList expected = QStringList( QString( "usedSystem=%1" ).arg( usedSystem ) ) + estimationLines( estimation );
	QCOMPARE( lines.mid( 0, expected.size() ), expected );
}

void TST_MT_Controller::substituteGetsTheLoadedEstimation_data()
{
	QTest::addColumn<int>( "usedSystem" );
	QTest::addColumn<QString>( "estimation" );

	QTest::newRow( "Evaluator" ) << 1 << QString( loadedEvaluatorEstimation );
	QTest::newRow( "Classifier" ) << 2 << QString( loadedClassifierEstimation );
}

// The substitute reads what writeEstimationSettings made of the loaded state.
void TST_MT_Controller::substituteGetsTheLoadedEstimation()
{
	QFETCH( int, usedSystem );
	QFETCH( QString, estimation );

	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( usedSystem, estimation ) ) ) );

	QCOMPARE( substituteLines( substitute() ), estimationLines( estimation ) );
}

void TST_MT_Controller::substituteKeepsEachStrategy_data()
{
	QTest::addColumn<int>( "usedSystem" );
	QTest::addColumn<QString>( "estimation" );

	for( int strategy : { 1, 2, 9, 10, 13 } )
	{
		QTest::addRow( "Evaluator %d", strategy ) << 1 << QString( "Evaluator:\n%1\n0.25\n7\n0\n\n" ).arg( strategy );
	}

	for( int strategy : { 3, 4, 5, 6, 8 } )
	{
		QTest::addRow( "Classifier %d", strategy ) << 2 << QString( "Classifier:\n%1\n60\n5\n0\n\n" ).arg( strategy );
	}
}

void TST_MT_Controller::substituteKeepsEachStrategy()
{
	QFETCH( int, usedSystem );
	QFETCH( QString, estimation );

	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( usedSystem, estimation ) ) ) );

	QCOMPARE( substituteLines( substitute() ), estimationLines( estimation ) );
}

void TST_MT_Controller::negativeStrategyFromAFileIsReadAsPositive_data()
{
	QTest::addColumn<int>( "usedSystem" );
	QTest::addColumn<QString>( "estimation" );
	QTest::addColumn<QString>( "expected" );

	QTest::newRow( "Evaluator" ) << 1 << "Evaluator:\n-2\n0.25\n7\n0\n\n" << "Evaluator:\n2\n0.25\n7\n0\n\n";
	QTest::newRow( "Classifier" ) << 2 << "Classifier:\n-4\n60\n5\n0\n\n" << "Classifier:\n4\n60\n5\n0\n\n";
}

// A negative strategy matches no strategy of the substitute, so MetaGP would never estimate.
void TST_MT_Controller::negativeStrategyFromAFileIsReadAsPositive()
{
	QFETCH( int, usedSystem );
	QFETCH( QString, estimation );
	QFETCH( QString, expected );

	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( usedSystem, estimation ) ) ) );

	QCOMPARE( substituteLines( substitute() ), estimationLines( expected ) );
}

void TST_MT_Controller::getFitnessTrainerOnlyForAnEvaluator()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 2, loadedClassifierEstimation ) ) ) );

	QCOMPARE( controller().getFitnessTrainer(), nullptr );
}

void TST_MT_Controller::getClassifierOnlyForAClassifier()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 1, loadedEvaluatorEstimation ) ) ) );

	QCOMPARE( controller().getClassifier(), nullptr );
}

void TST_MT_Controller::getFitnessTrainerKeepsItsSubstitute()
{
	controller().useMeta( true );

	MT_Substitute *first = controller().getFitnessTrainer();

	QVERIFY( first );
	QCOMPARE( controller().getFitnessTrainer(), first );
	QCOMPARE( substituteLines( first ), estimationLines( defaultEvaluatorEstimation ) );
}

void TST_MT_Controller::getClassifierKeepsItsSubstitute()
{
	QVERIFY( controller().useMeta( true ) );
	QVERIFY( controller().switchSystem( MT_SubstituteKind::Classifier ) );

	MT_Substitute *first = controller().getClassifier();

	QVERIFY( first );
	QCOMPARE( controller().getClassifier(), first );
	QCOMPARE( substituteLines( first ), estimationLines( defaultClassifierEstimation ) );
}

void TST_MT_Controller::startEvolutionWithMetaOffOnlyClearsStartWOSigel()
{
	controller().startWOSigel = true;

	QVERIFY( controller().startEvolution() );

	QVERIFY( !controller().startWOSigel );
	QVERIFY( !controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::NoMetaGP );
}

void TST_MT_Controller::configureSystemWithoutAFileLoadsTheDefault()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );

	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );
	const QStringList lines = saveSetup( "default" ).split( '\n' );
	const QStringList expected = QStringList( "usedSystem=1" ) + estimationLines( defaultEvaluatorEstimation );
	QCOMPARE( lines.mid( 0, expected.size() ), expected );
}

void TST_MT_Controller::slotSaveSetupWritesEstimationAndSetup()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 2, loadedClassifierEstimation, 12 ) ) ) );
	controller().configureSystem( nullptr );

	const QString saved = saveSetup( "setup" );

	const QStringList expected = QStringList( "usedSystem=2" ) + estimationLines( loadedClassifierEstimation );
	QCOMPARE( saved.split( '\n' ).mid( 0, expected.size() ), expected );
	QVERIFY( saved.contains( "OffspringSize:\n100\n" ) );
	QVERIFY( saved.contains( "Population:\n12\n" ) );
}

void TST_MT_Controller::slotSaveSetupKeepsTheMcnfSuffix()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );

	expectFileName( "typed.mcnf" );
	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotSaveSetup" ) );

	QVERIFY( QFile::exists( folder->filePath( "typed.mcnf" ) ) );
	QVERIFY( !QFile::exists( folder->filePath( "typed.mcnf.mcnf" ) ) );
}

void TST_MT_Controller::slotSaveSetupCancelledWritesNothing()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );
	const int filesBefore = QDir( folder->path() ).entryList( QDir::Files ).size();

	expectFileName( QString() );
	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotSaveSetup" ) );

	QCOMPARE( QDir( folder->path() ).entryList( QDir::Files ).size(), filesBefore );
}

void TST_MT_Controller::slotLoadSetupLoadsAnotherKindWhileEnabled()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );

	writeFile( "other.mcnf", configuration( 2, loadedClassifierEstimation, 14 ) );
	expectFileName( "other.mcnf" );
	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotLoadSetup" ) );

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );

	const QString saved = saveSetup( "after" );
	const QStringList expected = QStringList( "usedSystem=2" ) + estimationLines( loadedClassifierEstimation );
	QCOMPARE( saved.split( '\n' ).mid( 0, expected.size() ), expected );
	QVERIFY( saved.contains( "Population:\n14\n" ) );

	QCOMPARE( substituteLines( controller().getClassifier() ), estimationLines( loadedClassifierEstimation ) );
	QCOMPARE( savedFileName(), QString( "lastExperiment.mexp" ) );
	const QStringList savedSystem = readFile( folder->filePath( "lastExperiment.mexp" ) ).split( '\n' );
	QCOMPARE( savedSystem.mid( 0, expected.size() ), expected );
}

void TST_MT_Controller::slotLoadSetupWhileDisabledTakesTheKindOfTheFile()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );
	QVERIFY( controller().useMeta( false ) );

	writeFile( "other.mcnf", configuration( 2, loadedClassifierEstimation, 14 ) );
	expectFileName( "other.mcnf" );
	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotLoadSetup" ) );

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );

	const QString saved = saveSetup( "after" );
	const QStringList expected = QStringList( "usedSystem=2" ) + estimationLines( loadedClassifierEstimation );
	QCOMPARE( saved.split( '\n' ).mid( 0, expected.size() ), expected );
	QVERIFY( saved.contains( "Population:\n14\n" ) );
}

void TST_MT_Controller::slotLoadSetupKindSurvivesDisableAndEnable()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );

	writeFile( "other.mcnf", configuration( 2, loadedClassifierEstimation ) );
	expectFileName( "other.mcnf" );
	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotLoadSetup" ) );

	QVERIFY( controller().useMeta( false ) );
	QVERIFY( controller().useMeta( true ) );

	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );
	QCOMPARE( controller().getFitnessTrainer(), nullptr );
	QVERIFY( controller().getClassifier() );
}

void TST_MT_Controller::slotLoadSetupCancelledChangesNothing()
{
	QVERIFY( controller().useMeta( true ) );
	controller().configureSystem( nullptr );
	const QString before = saveSetup( "before" );

	expectFileName( QString() );
	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotLoadSetup" ) );

	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );
	QCOMPARE( saveSetup( "after" ), before );
}

void TST_MT_Controller::slotLoadDefaultRestoresTheDefaultSetup()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 2, loadedClassifierEstimation, 14 ) ) ) );
	controller().configureSystem( nullptr );

	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotLoadDefault" ) );

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );

	const QString saved = saveSetup( "after" );
	const QStringList expected = QStringList( "usedSystem=1" ) + estimationLines( defaultEvaluatorEstimation );
	QCOMPARE( saved.split( '\n' ).mid( 0, expected.size() ), expected );
	QVERIFY( saved.contains( QString( "Population:\n%1\n" ).arg( defaultPopulationSize ) ) );
}

void TST_MT_Controller::slotLoadDefaultWhileDisabledTakesTheKindOfTheFile()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 2, loadedClassifierEstimation, 14 ) ) ) );
	controller().configureSystem( nullptr );
	QVERIFY( controller().useMeta( false ) );

	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotLoadDefault" ) );

	QVERIFY( controller().IsEnabled() );
	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Evaluator );

	const QString saved = saveSetup( "after" );
	const QStringList expected = QStringList( "usedSystem=1" ) + estimationLines( defaultEvaluatorEstimation );
	QCOMPARE( saved.split( '\n' ).mid( 0, expected.size() ), expected );
	QVERIFY( saved.contains( QString( "Population:\n%1\n" ).arg( defaultPopulationSize ) ) );
}

void TST_MT_Controller::slotLoadDefaultWithoutTheFileChangesNothing()
{
	QVERIFY( controller().readFromFile( writeFile( "run.mexp", configuration( 2, loadedClassifierEstimation, 14 ) ) ) );
	controller().configureSystem( nullptr );
	QVERIFY( QFile::remove( folder->filePath( "stdConf.mt" ) ) );

	QVERIFY( QMetaObject::invokeMethod( &controller(), "slotLoadDefault" ) );

	QCOMPARE( controller().UsedSystem(), MT_SubstituteKind::Classifier );
	QVERIFY( saveSetup( "after" ).contains( "Population:\n14\n" ) );
}

MT_Controller &TST_MT_Controller::controller()
{
	return *experiment->mtController;
}

void TST_MT_Controller::addRobot( SIGEL_GP::SIG_GPMetaGPExperiment &target )
{
	const SIG_Vector corners[4] = { SIG_Vector( 0, 0, 0 ), SIG_Vector( 1, 0, 0 ), SIG_Vector( 0, 1, 0 ), SIG_Vector( 0, 0, 1 ) };
	const int faces[4][3] = { { 0, 2, 1 }, { 0, 1, 3 }, { 0, 3, 2 }, { 1, 2, 3 } };

	SIGEL_Robot::SIG_Geometry *geometry = new SIGEL_Robot::SIG_Geometry();
	for( const int ( &face )[3] : faces )
	{
		SIGEL_Robot::SIG_Polygon *polygon = new SIGEL_Robot::SIG_Polygon( geometry );
		for( int corner : face )
		{
			polygon->appendVertex( corners[corner] );
		}
	}

	SIGEL_Robot::SIG_Robot &robot = target.robot;
	SIGEL_Robot::SIG_Body *body = new SIGEL_Robot::SIG_Body( &robot, "body", "folder" );
	SIGEL_Robot::SIG_Material *material = new SIGEL_Robot::SIG_Material( &robot, "material" );
	SIGEL_Robot::SIG_Link *link = new SIGEL_Robot::SIG_Link( &robot, "link", 0 );
	body->setGeometry( geometry );
	link->setBody( body );
	link->setMaterial( material );
	robot.addBody( body );
	robot.addMaterial( material );
	robot.addLink( link );
	robot.setRootLink( link );
}

QString TST_MT_Controller::configuration( int usedSystem, const QString &estimation, int populationSize )
{
	return QString( "usedSystem=%1\n" ).arg( usedSystem ) + estimation + QString( gpSetup ).arg( populationSize );
}

QString TST_MT_Controller::defaultConfiguration()
{
	return configuration( 1, QString( defaultEvaluatorEstimation ) + defaultClassifierEstimation );
}

QString TST_MT_Controller::writeFile( const QString &name, const QString &text )
{
	const QString path = folder->filePath( name );
	QFile file( path );
	if( !file.open( QIODevice::WriteOnly ) )
	{
		QTest::qFail( "cannot write a test file", __FILE__, __LINE__ );
		return path;
	}

	QTextStream( &file ) << text;
	return path;
}

QString TST_MT_Controller::readFile( const QString &path )
{
	QFile file( path );
	if( !file.open( QIODevice::ReadOnly ) )
	{
		return QString();
	}

	return QTextStream( &file ).readAll();
}

MT_Substitute *TST_MT_Controller::substitute()
{
	if( controller().UsedSystem() == MT_SubstituteKind::Classifier )
	{
		return controller().getClassifier();
	}

	return controller().getFitnessTrainer();
}

QStringList TST_MT_Controller::substituteLines( MT_Substitute *substitute )
{
	if( !substitute )
	{
		return QStringList();
	}

	QString text;
	QTextStream stream( &text );
	substitute->writeToFile( stream );
	return estimationLines( text );
}

QStringList TST_MT_Controller::estimationLines( const QString &estimation )
{
	QStringList lines = estimation.split( '\n' );
	while( !lines.isEmpty() && lines.last().isEmpty() )
	{
		lines.removeLast();
	}

	return lines;
}

QString TST_MT_Controller::savedFileName()
{
	QString saved;
	QTextStream stream( &saved );
	if( !controller().writeToFile( stream, "ignored" ) )
	{
		return QString();
	}

	return saved.split( '\n' ).value( 1 ).mid( QString( "fileName=" ).size() );
}

QString TST_MT_Controller::saveSetup( const QString &name )
{
	expectFileName( name );
	QMetaObject::invokeMethod( &controller(), "slotSaveSetup" );
	return readFile( folder->filePath( name + ".mcnf" ) );
}

void TST_MT_Controller::expectButton( QMessageBox::StandardButton button )
{
	TST_MT_DialogAnswer answer;
	answer.button = button;
	dialogAnswers.append( answer );
}

void TST_MT_Controller::expectFileName( const QString &fileName )
{
	TST_MT_DialogAnswer answer;
	answer.fileName = fileName;
	dialogAnswers.append( answer );
}

void TST_MT_Controller::answerNextDialog()
{
	QWidget *modal = QApplication::activeModalWidget();
	QMessageBox *box = qobject_cast<QMessageBox *>( modal );
	QFileDialog *fileDialog = qobject_cast<QFileDialog *>( modal );
	if( !box && !fileDialog )
	{
		return;
	}

	if( dialogAnswers.isEmpty() )
	{
		unanswerableDialogs++;
		modal->close();
		return;
	}

	const TST_MT_DialogAnswer answer = dialogAnswers.takeFirst();
	if( box )
	{
		QAbstractButton *button = box->button( answer.button );
		if( !button )
		{
			unanswerableDialogs++;
			box->close();
			return;
		}

		button->click();
		return;
	}

	if( answer.button != QMessageBox::NoButton )
	{
		unanswerableDialogs++;
		fileDialog->reject();
		return;
	}

	proposedFileName = fileDialog->selectedFiles().value( 0 );
	if( answer.fileName.isEmpty() )
	{
		fileDialog->reject();
		return;
	}

	// QFileDialog::accept takes a name with a folder for a change of folder, and
	// selectFile leaves a name that the dialog proposes in the field.
	QLineEdit *nameField = fileDialog->findChild<QLineEdit *>( "fileNameEdit" );
	if( !nameField )
	{
		unanswerableDialogs++;
		fileDialog->reject();
		return;
	}

	fileDialog->setDirectory( folder->path() );
	nameField->setText( answer.fileName );
	static_cast<QDialog *>( fileDialog )->accept();
}

void TST_MT_Controller::restoreEnvironment( const char *name, const QByteArray &value )
{
	if( value.isNull() )
	{
		qunsetenv( name );
		return;
	}

	qputenv( name, value );
}
