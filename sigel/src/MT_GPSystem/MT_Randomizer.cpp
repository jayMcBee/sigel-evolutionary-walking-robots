#include "MT_GPSystem/MT_Randomizer.h"

// administrative methods


MT_Randomizer::MT_Randomizer()
{
	srand( static_cast<unsigned>( time( nullptr ) ) );

	// initialization
	LastError = MT_ErrorCode::None;

	RandomXPoints.resize( CrossoverPointIndexCount );
	for( int i = 0; i < CrossoverPointIndexCount; i++ )
	{
		RandomXPoints[i] = 0;
	}

	ParentSize = 10;
	OffspringSize = 100;
	NumberOfVariables = 10;
	ProgramLengthMax = 50;
	TournamentPartition.resize( OffspringSize );

	ProbSearchOperator.resize( 3 );
	for( int i = 0; i < 3; i++ )
	{
		ProbSearchOperator[i] = 0;
	}

	ProbMutationPower.resize( 2 );
	for( int i = 0; i < 2; i++ )
	{
		ProbMutationPower[i] = 0;
	}

	ProbCrossOverPoints.resize( 3 );
	for( int i = 0; i < 3; i++ )
	{
		ProbCrossOverPoints[i] = 0;
	}

	ProbInstruction.resize( 18 );
	for( int i = 0; i < 18; i++ )
	{
		ProbInstruction[i] = 0;
	}

	createConstant( 50, true, 0, 1000 );
}

MT_Randomizer::MT_Randomizer( QTextStream &File )
{
	// set seed for the random method
	srand( static_cast<unsigned>( time( nullptr ) ) );

	LastError = MT_ErrorCode::None;
	RandomXPoints.resize( CrossoverPointIndexCount );
	for( int i = 0; i < CrossoverPointIndexCount; i++ )
	{
		RandomXPoints[i] = 0;
	}

	QString Randomizer( "Randomizer:" );
	QString PresentLine = File.readLine();

	while( ( PresentLine != Randomizer ) && !( File.atEnd() ) )
	{
		PresentLine = File.readLine();
	}

	if( ( PresentLine == Randomizer ) && !( File.atEnd() ) )
	{
		ParentSize = ( File.readLine() ).toInt();
		OffspringSize = ( File.readLine() ).toInt();
		NumberOfVariables = ( File.readLine() ).toInt();
		ProgramLengthMax = ( File.readLine() ).toInt();
		TournamentPartition.resize( OffspringSize );

		ProbSearchOperator.resize( 3 );
		for( int i = 0; i < 3; i++ )
		{
			ProbSearchOperator[i] = ( File.readLine() ).toDouble();
		}

		ProbMutationPower.resize( 2 );
		for( int i = 0; i < 2; i++ )
		{
			ProbMutationPower[i] = ( File.readLine() ).toDouble();
		}

		ProbCrossOverPoints.resize( 3 );
		for( int i = 0; i < 3; i++ )
		{
			ProbCrossOverPoints[i] = ( File.readLine() ).toDouble();
		}

		ProbInstruction.resize( 18 );
		for( int i = 0; i < 17; i++ )
		{
			ProbInstruction[i] = ( File.readLine() ).toDouble();
		}

		PresentLine = File.readLine();
		if( PresentLine.toDouble() >= 1.0 )
		{
			ProbInstruction[17] = PresentLine.toDouble();
			PresentLine = File.readLine();
		}
		else
		{
			ProbInstruction[17] = ProbInstruction[16];
		}

		QString ConString( "Constant:" );

		while( ( PresentLine != ConString ) && !( File.atEnd() ) )
		{
			PresentLine = File.readLine();
		}

		if( ( PresentLine == ConString ) && !( File.atEnd() ) )
		{
			PresentLine = File.readLine();

			if( PresentLine == "newConstant" )
			{
				// Constants are newly generated - for the default constructor
				PresentLine = File.readLine();
				createConstant( PresentLine.toInt(), true, 0, 1000 );
			}
			else
			{
				// Otherwise the current line gives the number of constants still to be loaded
				int NumOfConst = PresentLine.toInt();

				Constant.resize( NumOfConst );
				for( int i = 0; i < NumOfConst; i++ )
				{
					PresentLine = File.readLine();
					Constant[i] = PresentLine.toDouble();
				}
			}
		}
		else
		{
			LastError = MT_ErrorCode::ConstantSectionMissing;
		}
	}
	else
	{
		LastError = MT_ErrorCode::RandomizerSectionMissing;
	}
}

void MT_Randomizer::loadSetup( QTextStream &File )
{
	QString Randomizer( "Randomizer:" );
	QString PresentLine = File.readLine();

	while( ( PresentLine != Randomizer ) && !( File.atEnd() ) )
	{
		PresentLine = File.readLine();
	}

	if( ( PresentLine != Randomizer ) || File.atEnd() )
	{
		LastError = MT_ErrorCode::RandomizerSetupSectionMissing;
		return;
	}

	ParentSize = ( File.readLine() ).toInt();
	OffspringSize = ( File.readLine() ).toInt();
	TournamentPartition.resize( OffspringSize );
	NumberOfVariables = ( File.readLine() ).toInt();
	ProgramLengthMax = ( File.readLine() ).toInt();

	for( int i = 0; i < 3; i++ )
	{
		ProbSearchOperator[i] = ( File.readLine() ).toDouble();
	}

	for( int i = 0; i < 2; i++ )
	{
		ProbMutationPower[i] = ( File.readLine() ).toDouble();
	}

	for( int i = 0; i < 3; i++ )
	{
		ProbCrossOverPoints[i] = ( File.readLine() ).toDouble();
	}

	for( int i = 0; i < 17; i++ )
	{
		ProbInstruction[i] = ( File.readLine() ).toDouble();
	}

	PresentLine = File.readLine();
	if( PresentLine.toDouble() >= 1.0 )
	{
		ProbInstruction[17] = PresentLine.toDouble();
		PresentLine = File.readLine();
	}
	else
	{
		ProbInstruction[17] = ProbInstruction[16];
	}

	QString ConString( "Constant:" );
	PresentLine = File.readLine();

	while( ( PresentLine != ConString ) && !( File.atEnd() ) )
	{
		PresentLine = File.readLine();
	}

	if( ( PresentLine != ConString ) || File.atEnd() )
	{
		LastError = MT_ErrorCode::ConstantSetupSectionMissing;
		return;
	}

	PresentLine = File.readLine();
	if( PresentLine == "newConstant" )
	{
		PresentLine = File.readLine();
		createConstant( PresentLine.toInt(), true, 0, 1000 );
		return;
	}

	int NumOfConst = PresentLine.toInt();

	Constant.resize( NumOfConst );
	for( int i = 0; i < NumOfConst; i++ )
	{
		PresentLine = File.readLine();
		Constant[i] = PresentLine.toDouble();
	}
}

void MT_Randomizer::writeToFileRandi( QTextStream &File )
{
	File << ( "Randomizer:\n" );
	File << ParentSize << Qt::endl;
	File << OffspringSize << Qt::endl;
	File << NumberOfVariables << Qt::endl;
	File << ProgramLengthMax << Qt::endl;

	File << ProbSearchOperator[0] << Qt::endl;
	File << ProbSearchOperator[1] << Qt::endl;
	File << ProbSearchOperator[2] << Qt::endl;

	File << ProbMutationPower[0] << Qt::endl;
	File << ProbMutationPower[1] << Qt::endl;

	File << ProbCrossOverPoints[0] << Qt::endl;
	File << ProbCrossOverPoints[1] << Qt::endl;
	File << ProbCrossOverPoints[2] << Qt::endl;

	for( int i = 0; i < 18; i++ )
	{
		File << ProbInstruction[i] << Qt::endl;
	}

	File << Qt::endl;

	File << ( "Constant:" ) << Qt::endl;
	File << Constant.size() << Qt::endl;
	for( int i = 0; i < Constant.size(); i++ )
	{
		File << Constant[i] << Qt::endl;
	}

	File << Qt::endl;
}

// normal set/ get method

void MT_Randomizer::returnIndividualsValue( int **Length, int **NumOfVar, QList<double> **Con, QList<double> **ProbOfFu )
{
	*Length = &ProgramLengthMax;
	*NumOfVar = &NumberOfVariables;

	*Con = &Constant;
	*ProbOfFu = &ProbInstruction;
}

void MT_Randomizer::returnSearchValue( QList<double> **ProbMPower, QList<double> **ProbSOperator, QList<double> **ProbXPoints )
{
	*ProbMPower = &ProbMutationPower;
	*ProbSOperator = &ProbSearchOperator;
	*ProbXPoints = &ProbCrossOverPoints;
}

MT_ErrorCode MT_Randomizer::getLastError()
{
	return LastError;
}

void MT_Randomizer::setParentSize( int SizeOfParent )
{
	ParentSize = SizeOfParent;
}

void MT_Randomizer::setOffspringSize( int OffSize )
{
	OffspringSize = OffSize;
	TournamentPartition.resize( OffspringSize );
}

int MT_Randomizer::getProgramLengthMax()
{
	return ProgramLengthMax;
}

// special method for supply random

unsigned long MT_Randomizer::getRandomInteger( unsigned long upperBoundary )
{
	unsigned long Random;

	if( upperBoundary > 0 )
	{
		Random = fabs( rand() % upperBoundary );
	}
	else
	{
		Random = 0;
	}

	return Random;
}

int MT_Randomizer::getProgLength()
{
	double d = ProgramLengthMax * 0.66;
	int StartLength = d;

	return StartLength;
}

void MT_Randomizer::createConstant( int NumOfConstant, bool Integer, double lowerBound, double upperBound )
{
	double RangeDouble = upperBound - lowerBound;
	int RangeInt = upperBound - lowerBound;

	Constant.resize( NumOfConstant );

	if( Integer )
	{
		// Randomly generate integer constants
		for( int i = 0; i < NumOfConstant; i++ )
		{
			Constant[i] = lowerBound + getRandomInteger( RangeInt );
		}
	}
	else
	{
		// Problem if a double is drawn from a large range
		// RangeDouble < 65 ... three decimal places
		for( int i = 0; i < NumOfConstant; i++ )
		{
			Constant[i] = lowerBound + fabs( getRandomInteger( RangeDouble * 1000 ) / 1000.0 );
		}
	}
}

bool MT_Randomizer::answerMutateLine()
{
	bool Answer;

	if( getRandomInteger( 1000 ) < ProbMutationPower[0] )
	{
		Answer = true;
	}
	else
	{
		Answer = false;
	}

	return Answer;
}

bool MT_Randomizer::answerMutateElement()
{
	bool Answer;

	if( getRandomInteger( 1000 ) < ProbMutationPower[1] )
	{
		Answer = true;
	}
	else
	{
		Answer = false;
	}

	return Answer;
}

QList<int> * MT_Randomizer::getRandomTournamentPartition( int NumberOfTournaments )
{
	// Array holding the current number of players per tournament
	QList<int> Index;
	Index.resize( NumberOfTournaments );
	for( int k = 0; k < NumberOfTournaments; k++ )
	{
		Index[k] = 0;
	}

	// Index of the tournament the player should take part in
	int IndexOfTournament = 0;

	// Number of players per tournament, i.e. the tournament size
	int MaxPlayers = ( OffspringSize / NumberOfTournaments );

	for( int i = 0; i < OffspringSize; i++ )
	{
		IndexOfTournament = getRandomInteger( NumberOfTournaments );

		if( Index[IndexOfTournament] < MaxPlayers )
		{
			// There is still a free slot in tournament "IndexOfTournament"
			TournamentPartition[i] = IndexOfTournament;
			Index[IndexOfTournament]++;
		}
		else
		{
			// Tournament "IndexOfTournament" is full - then ...
			while( IndexOfTournament != -1 )
			{
				IndexOfTournament = ( IndexOfTournament + 1 ) % NumberOfTournaments;
				if( Index[IndexOfTournament] < MaxPlayers )
				{
					TournamentPartition[i] = IndexOfTournament;
					Index[IndexOfTournament]++;
					IndexOfTournament = -1;
				}
			}
		}
	}

	return &TournamentPartition;
}

int MT_Randomizer::getProportionalWinner( QList<int> * Players )
{
	QList<int> PlayersProportional;
	PlayersProportional.resize( Players->size() );
	int Total = 0;

	for( int i = 0; i < ( *Players ).size(); i++ )
	{
		Total = Total + ( *Players )[i];
		PlayersProportional[i] = Total;
	}

	int Winner = getRandomInteger( Total );

	for( int i = 0; i < ( *Players ).size(); i++ )
	{
		if( Winner <= PlayersProportional[i] )
		{
			Winner = i;
			break;
		}
	}
	return Winner;
}

MT_Instruction MT_Randomizer::getRandomInstruction()
{
	int Choice = 1;
	MT_Instruction NewInstruction;

	unsigned long Random = getRandomInteger( ProbInstruction[17] );

	for( int i = 0; i < 18; i++ )
	{
		if( Random < ProbInstruction[i] )
		{
			Choice = i;
			break;
		}
	}

	switch( Choice )
	{
		case 0: NewInstruction = swi; break;
		case 1: NewInstruction = cop; break;
		case 2: NewInstruction = add; break;
		case 3: NewInstruction = sub; break;
		case 4: NewInstruction = mul; break;
		case 5: NewInstruction = mtd; break;
		case 6: NewInstruction = mod; break;
		case 7: NewInstruction = mmin; break;
		case 8: NewInstruction = mmax; break;
		case 9: NewInstruction = cmp; break;
		case 10: NewInstruction = jmp; break;
		case 11: NewInstruction = jle; break;
		case 12: NewInstruction = jeq; break;
		case 13: NewInstruction = nop; break;
		case 14: NewInstruction = lui; break;
		case 15: NewInstruction = lus; break;
		case 16: NewInstruction = lud; break;
		case 17: NewInstruction = lum; break;
	}

	return NewInstruction;
}

int MT_Randomizer::getRandomSOperator()
{
	int Choice = 0;
	unsigned int Random = getRandomInteger( ProbSearchOperator[2] );
	for( int i = 0; i < 3; i++ )
	{
		if( Random < ProbSearchOperator[i] )
		{
			Choice = i + 1;
			break;
		}
	}

	return Choice;
}

QList<int> * MT_Randomizer::getRandomXPoints( int SizeIndi1, int SizeIndi2 )
{
	int NumberofXPoints;
	unsigned int Random = getRandomInteger( ProbCrossOverPoints[2] );

	if( ( SizeIndi1 > 3 ) && ( SizeIndi2 > 3 ) )
	{
		for( int i = 0; i < 3; i++ )
		{
			if( Random < ProbCrossOverPoints[i] )
			{
				NumberofXPoints = i;
				break;
			}
		}
	}
	else
	{
		NumberofXPoints = 0; // Since the individuals are so small, only one crossover point is chosen
	}

	RandomXPoints[AdditionalPointCount] = NumberofXPoints;

	switch( NumberofXPoints )
	{
		case 0:
			{
				// 1 XPoint
				RandomXPoints[ParentOneFirstPoint] = getRandomInteger( SizeIndi1 ) + 1;
				RandomXPoints[ParentTwoFirstPoint] = getRandomInteger( SizeIndi2 ) + 1;
			}
			break;
		case 1:
			{
				// 2 XPoints
				RandomXPoints[ParentOneFirstPoint] = getRandomInteger( SizeIndi1 - 1 ) + 1;
				RandomXPoints[ParentTwoFirstPoint] = getRandomInteger( SizeIndi2 - 1 ) + 1;

				RandomXPoints[ParentOneSecondPoint] = RandomXPoints[ParentOneFirstPoint] + 1 + getRandomInteger( SizeIndi1 - RandomXPoints[ParentOneFirstPoint] - 1 );
				RandomXPoints[ParentTwoSecondPoint] = RandomXPoints[ParentTwoFirstPoint] + 1 + getRandomInteger( SizeIndi2 - RandomXPoints[ParentTwoFirstPoint] - 1 );
			}
			break;
		case 2:
			{
				// 3 XPoints
				RandomXPoints[ParentOneFirstPoint] = getRandomInteger( SizeIndi1 - 2 ) + 1;
				RandomXPoints[ParentTwoFirstPoint] = getRandomInteger( SizeIndi2 - 2 ) + 1;

				RandomXPoints[ParentOneSecondPoint] = RandomXPoints[ParentOneFirstPoint] + 1 + getRandomInteger( SizeIndi1 - RandomXPoints[ParentOneFirstPoint] - 2 );
				RandomXPoints[ParentTwoSecondPoint] = RandomXPoints[ParentTwoFirstPoint] + 1 + getRandomInteger( SizeIndi2 - RandomXPoints[ParentTwoFirstPoint] - 2 );

				RandomXPoints[ParentOneThirdPoint] = RandomXPoints[ParentOneSecondPoint] + 1 + getRandomInteger( SizeIndi1 - RandomXPoints[ParentOneSecondPoint] - 1 );
				RandomXPoints[ParentTwoThirdPoint] = RandomXPoints[ParentTwoSecondPoint] + 1 + getRandomInteger( SizeIndi2 - RandomXPoints[ParentTwoSecondPoint] - 1 );
			}
			break;
	}

	return &RandomXPoints;
}

MT_Operand * MT_Randomizer::getRandomOperand()
{
	MT_Operand *NewOperand;

	int Terminal = getRandomInteger( NumberOfVariables + Constant.size() );

	if( Terminal < NumberOfVariables )
	{
		NewOperand = new MT_Operand( 1, -1, Terminal );
	}
	else
	{
		NewOperand = new MT_Operand( 0, Constant[Terminal - NumberOfVariables], -1 );
	}

	return NewOperand;
}

MT_Operand * MT_Randomizer::getRandomVariable()
{
	MT_Operand *NewOperand;
	NewOperand = new MT_Operand( 1, -1.0, getRandomInteger( NumberOfVariables ) );
	return NewOperand;
}

int MT_Randomizer::getNumOfVari()
{
	return NumberOfVariables;
}
