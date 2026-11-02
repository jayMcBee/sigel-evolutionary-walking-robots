#include "MT_GUI/MT_EstimationWidget.h"
#include "MT_Control/MT_Substitute.h"

#include <QMessageBox>
#include <QSpinBox>
#include <QComboBox>
#include <QLayout>
#include <QGroupBox>

MT_EstimationWidget::MT_EstimationWidget( QWidget *parent, const char *name, Qt::WindowFlags fl )
	: MT_EstimationWidgetBase( parent, name, fl ), MT_WidgetBase( parent )
{
	lastIndex = 0;
	tol0 = tol1 = 0.0;
	usedSys = MT_SubstituteKind::Evaluator;

	toleranceSpinBox = new DISpinBox( 3, estimationGroupBox );
	estimationGridLayout->addWidget( toleranceSpinBox, 1, 2 );

	QObject::connect( strategyComboBox, SIGNAL(activated(int)), SLOT(slotStrategyChanged(int)) );
}

void MT_EstimationWidget::slotStrategyChanged( int index )
{
	// remember the last set value
	if( toleranceSpinBox->getType() == MT_SpinBoxValueType::Integer )
	{
		switch( lastIndex )
		{
		case 0:
			tol0 = toleranceSpinBox->doubleValue();
			break;
		case 1:
			tol1 = toleranceSpinBox->doubleValue();
			break;
		case 2:
			tol2 = toleranceSpinBox->doubleValue();
			break;
		case 3:
			tol3 = toleranceSpinBox->doubleValue();
			break;
		case 4:
			tol4 = toleranceSpinBox->doubleValue();
			break;
		}
	}
	else
	{
		switch( lastIndex )
		{
		case 0:
			tol0 = toleranceSpinBox->intValue();
			break;
		case 1:
			tol1 = toleranceSpinBox->intValue();
			break;
		case 2:
			tol2 = toleranceSpinBox->intValue();
			break;
		case 3:
			tol3 = toleranceSpinBox->intValue();
			break;
		case 4:
			tol4 = toleranceSpinBox->intValue();
			break;
		}
	}

	lastIndex = index;
	if( usedSys == MT_SubstituteKind::Evaluator )
	{
		switch( index )
		{
		case 0:
			toleranceSpinBox->setRange( 3, 0.0, 100.0 );
			toleranceSpinBox->setDoubleValue( tol0 );
			break;
		case 1:
			toleranceSpinBox->setRange( 3, 0.0, 100.0 );
			toleranceSpinBox->setDoubleValue( tol1 );
			break;
		case 2:
			toleranceSpinBox->setRange( 3, 0.0, 100.0 );
			toleranceSpinBox->setDoubleValue( tol2 );
			break;
		case 3:
			toleranceSpinBox->setRange( 3, 0.0, 100.0 );
			toleranceSpinBox->setDoubleValue( tol3 );
			break;
		case 4:
		default:
			toleranceSpinBox->setRange( 3, 0.0, 100.0 );
			toleranceSpinBox->setDoubleValue( tol4 );
			break;
		}
	}
	else
	{
		switch( index )
		{
		case 0:
			toleranceSpinBox->setRange( 0, 100 );
			toleranceSpinBox->setIntValue( static_cast< int >( tol0 ) );
			break;
		case 1:
			toleranceSpinBox->setRange( 0, 100 );
			toleranceSpinBox->setIntValue( tol1 );
			break;
		case 2:
			toleranceSpinBox->setRange( 0, 100 );
			toleranceSpinBox->setIntValue( tol2 );
			break;
		case 3:
			toleranceSpinBox->setRange( 0, 100 );
			toleranceSpinBox->setIntValue( tol3 );
			break;
		case 4:
		default:
			toleranceSpinBox->setRange( 0, 100 );
			toleranceSpinBox->setIntValue( tol4 );
			break;
		}
	}
}

bool MT_EstimationWidget::onHide( MT_GPManager *manager, MT_EstimationState *estimationState )
{
	MT_EstimationStrategy strategy;
	int refInt;
	double tolerance;

	if( usedSys == MT_SubstituteKind::Evaluator )
	{
		switch( strategyComboBox->currentIndex() )
		{
		case 0:
			strategy = MT_EstimationStrategy::MeanError;
			break;
		case 1:
			strategy = MT_EstimationStrategy::CleanedMeanError;
			break;
		case 2:
			strategy = MT_EstimationStrategy::AdaptiveError;
			break;
		case 3:
			strategy = MT_EstimationStrategy::AdaptiveCleanedError;
			break;
		case 4:
		default:
			strategy = MT_EstimationStrategy::EvaluatorWithoutMetaGP;
		}
	}
	else
	{
		switch( strategyComboBox->currentIndex() )
		{
		case 0:
			strategy = MT_EstimationStrategy::ErrorPercentage;
			break;
		case 1:
			strategy = MT_EstimationStrategy::AdaptiveByGeneration;
			break;
		case 2:
			strategy = MT_EstimationStrategy::AdaptiveByFitness;
			break;
		case 3:
			strategy = MT_EstimationStrategy::SaveTheBest;
			break;
		case 4:
		default:
			strategy = MT_EstimationStrategy::ClassifierWithoutMetaGP;
		}
	}

	refInt = intervalSpinBox->value();

	if( toleranceSpinBox->getType() == MT_SpinBoxValueType::Integer )
	{
		tolerance = static_cast< double >( toleranceSpinBox->intValue() );
	}
	else
	{
		tolerance = toleranceSpinBox->doubleValue();
	}

	if( estimationState )
	{
		estimationState->strategy = strategy;
		estimationState->tolerance = tolerance;
		estimationState->refreshInterval = refInt;
	}
	else
	{
		QMessageBox::critical( this, "Configure MetaGP System", "Couldn't set the information for the current MetaGP system." );
	}

	return true;
}

void MT_EstimationWidget::onShow( MT_GPManager *manager, MT_EstimationState *estimationState )
{
	MT_EstimationStrategy strategy = MT_EstimationStrategy::MeanError;
	int refInt = 0;
	double tolerance = 0.0;

	// read in the values
	if( estimationState )
	{
		strategy = estimationState->strategy;
		refInt = estimationState->refreshInterval;
		tolerance = estimationState->tolerance;
	}
	else
	{
		QMessageBox::critical( this, "Configure MetaGP System", "Couldn't retrieve the information for the current MetaGP system." );
		return;
	}

	// build new dropdownlist depending on the used substituter
	strategyComboBox->clear();

	if( MT_Substitute::kindOfStrategy( strategy ) == MT_SubstituteKind::Evaluator )
	{
		// of type evaluator
		estimationGroupBox->setTitle( "Evaluation" );
		usedSys = MT_SubstituteKind::Evaluator;
		strategyComboBox->addItem( "Mean error" );
		strategyComboBox->addItem( "Cleaned mean error" );
		strategyComboBox->addItem( "Adaptive error" );
		strategyComboBox->addItem( "Adaptive cleaned error" );
		strategyComboBox->addItem( "Don't use MetaGP" );

		switch( strategy )
		{
		case MT_EstimationStrategy::MeanError:
			strategyComboBox->setCurrentIndex( 0 );
			break;
		case MT_EstimationStrategy::CleanedMeanError:
			strategyComboBox->setCurrentIndex( 1 );
			break;
		case MT_EstimationStrategy::AdaptiveError:
			strategyComboBox->setCurrentIndex( 2 );
			break;
		case MT_EstimationStrategy::AdaptiveCleanedError:
			strategyComboBox->setCurrentIndex( 3 );
			break;
		case MT_EstimationStrategy::EvaluatorWithoutMetaGP:
		default:
			strategyComboBox->setCurrentIndex( 4 );
		}
	}
	else
	{
		// of type classifier
		estimationGroupBox->setTitle( "Classification" );
		usedSys = MT_SubstituteKind::Classifier;
		strategyComboBox->addItem( "Error percentage" );
		strategyComboBox->addItem( "Adaptive - generation" );
		strategyComboBox->addItem( "Adaptive - fitness" );
		strategyComboBox->addItem( "Save the best" );
		strategyComboBox->addItem( "Don't use MetaGP" );

		switch( strategy )
		{
		case MT_EstimationStrategy::ErrorPercentage:
			strategyComboBox->setCurrentIndex( 0 );
			break;
		case MT_EstimationStrategy::AdaptiveByGeneration:
			strategyComboBox->setCurrentIndex( 1 );
			break;
		case MT_EstimationStrategy::AdaptiveByFitness:
			strategyComboBox->setCurrentIndex( 2 );
			break;
		case MT_EstimationStrategy::SaveTheBest:
			strategyComboBox->setCurrentIndex( 3 );
			break;
		case MT_EstimationStrategy::ClassifierWithoutMetaGP:
		default:
			strategyComboBox->setCurrentIndex( 4 );
		}
	}
	slotStrategyChanged( strategyComboBox->currentIndex() );
	lastIndex = strategyComboBox->currentIndex();

	// remember the value been set
	switch( lastIndex )
	{
	case 0:
		tol0 = tolerance;
		break;
	case 1:
		tol1 = tolerance;
		break;
	default:
		break;
	}

	// set the values
	intervalSpinBox->setValue( refInt );
	if( toleranceSpinBox->getType() == MT_SpinBoxValueType::Integer )
	{
		toleranceSpinBox->setIntValue( static_cast< int >( tolerance ) );
	}
	else
	{
		toleranceSpinBox->setDoubleValue( tolerance );
	}
}
