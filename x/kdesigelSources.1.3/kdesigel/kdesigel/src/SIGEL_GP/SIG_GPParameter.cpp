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
#include "compat/q2compat.h"
#include "SIGEL_GP/SIG_GPParameter.h"


SIGEL_GP::SIG_GPParameter::SIG_GPParameter()
: randomSeed(0),
  minIndLength(3),
  maxIndLength(1024),
  resEvGen(0),
  reproductionProb(340),
  xoverProb(330),
  mutationProb(330),
  tournamentsPerGeneration(0.5),
  maxAge(50),
  terminationDurationDays(0),
  terminationDurationHours(0),
  terminationDurationMinutes(0),
  terminationDurationSeconds(0),
  terminationUsesDate(true),
  terminationGenerationNo(0),
  terminationModel(byUser),
  saveExit(true),
  liveUndead(false),
  poolImageGeneration(0),
  maxFitness(50),
  parsimonyPressure(false),
  fitnessName( "SimpleFitnessFunction" ),
  hostList(),
  timeOutMinutes(0)
{
  terminationTime=terminationTime.currentDateTime();
  setPriority(veryLow);
  instructionProb.resize( 15 );
  instructionProb.fill( 1000 );
};

SIGEL_GP::SIG_GPParameter::SIG_GPParameter(QString parameter)
{ };

SIGEL_GP::SIG_GPParameter::~SIG_GPParameter()
{
  // This class owns the hosts in hostList.
  hostList.deleteContents();
};

void SIGEL_GP::SIG_GPParameter::setRandomSeed (int seed)
{
    randomSeed=seed;
};

int SIGEL_GP::SIG_GPParameter::getRandomSeed() const
{ 
   return randomSeed;
};

bool SIGEL_GP::SIG_GPParameter::getParsimonyPressure()
{
    return parsimonyPressure;
};
  
 
void SIGEL_GP::SIG_GPParameter::setParsimonyPressure( bool pressure )
{
    parsimonyPressure = pressure;
};

void SIGEL_GP::SIG_GPParameter::setMinIndLength (int minIndLen)
{
    minIndLength=minIndLen;
};

int SIGEL_GP::SIG_GPParameter::getMinIndLength() const
{
    return minIndLength;
};

void SIGEL_GP::SIG_GPParameter::setMaxIndLength (int maxIndLen)
{
    maxIndLength=maxIndLen;
};

int SIGEL_GP::SIG_GPParameter::getMaxIndLength() const
{
   return maxIndLength;
};

void SIGEL_GP::SIG_GPParameter::setReproductionProb(int prob)
{
    reproductionProb=prob;
};

int SIGEL_GP::SIG_GPParameter::getReproductionProb() const
{
   return reproductionProb;
};

void SIGEL_GP::SIG_GPParameter::setXoverProb(int prob)
{
   xoverProb=prob;
};

int SIGEL_GP::SIG_GPParameter::getXoverProb() const
{
   return xoverProb;
};

void SIGEL_GP::SIG_GPParameter::setMutationProb(int prob)
{
   mutationProb=prob;
};

int  SIGEL_GP::SIG_GPParameter::getMutationProb() const
{
   return mutationProb;
};

void  SIGEL_GP::SIG_GPParameter::setTournamentsPerGeneration(double prob)
{
   tournamentsPerGeneration=prob;
};

double  SIGEL_GP::SIG_GPParameter::getTournamentsPerGeneration() const
{
   return tournamentsPerGeneration;
};

void SIGEL_GP::SIG_GPParameter::setMaxAge(long int age)
{
   maxAge=age;
};

long int SIGEL_GP::SIG_GPParameter::getMaxAge() const
{
   return maxAge;
};

void SIGEL_GP::SIG_GPParameter::setPassiveTime(unsigned long pTime)
{
   passiveTime=pTime;
};

unsigned long SIGEL_GP::SIG_GPParameter::getPassiveTime() const
{
   return passiveTime;
};

void SIGEL_GP::SIG_GPParameter::setMaxTouchsPerLoop(int touchs)
{
  maxTouchsPerLoop=touchs;
};

int SIGEL_GP::SIG_GPParameter::getMaxTouchsPerLoop() const
{
   return maxTouchsPerLoop;
};

void SIGEL_GP::SIG_GPParameter::setToDoSweepsPerLoop(int sweeps)
{
   toDoSweepsPerLoop=sweeps;
};

int SIGEL_GP::SIG_GPParameter::getToDoSweepsPerLoop() const
{
   return toDoSweepsPerLoop;
};

void SIGEL_GP::SIG_GPParameter::setTerminationTime(QDateTime tTime)
{
   terminationTime=tTime;
};

QDateTime SIGEL_GP::SIG_GPParameter::getTerminationTime() const
{
   return terminationTime;
};

void SIGEL_GP::SIG_GPParameter::setTerminationDurationDays( int newValue )
{
  terminationDurationDays = newValue;
};

int SIGEL_GP::SIG_GPParameter::getTerminationDurationDays() const
{
  return terminationDurationDays;
};

void SIGEL_GP::SIG_GPParameter::setTerminationDurationHours( int newValue )
{
  terminationDurationHours = newValue;
};

int SIGEL_GP::SIG_GPParameter::getTerminationDurationHours() const
{
  return terminationDurationHours;
};

void SIGEL_GP::SIG_GPParameter::setTerminationDurationMinutes( int newValue )
{
  terminationDurationMinutes = newValue;
};

int SIGEL_GP::SIG_GPParameter::getTerminationDurationMinutes() const
{
  return terminationDurationMinutes;
};

void SIGEL_GP::SIG_GPParameter::setTerminationDurationSeconds( int newValue )
{
  terminationDurationSeconds = newValue;
};

int SIGEL_GP::SIG_GPParameter::getTerminationDurationSeconds() const
{
  return terminationDurationSeconds;
};

void SIGEL_GP::SIG_GPParameter::setTerminationGenerationNo (int tGenNo)
{
    terminationGenerationNo=tGenNo;
};

int SIGEL_GP::SIG_GPParameter::getTerminationGenerationNo() const
{
   return terminationGenerationNo;
};

void SIGEL_GP::SIG_GPParameter::setTerminationModel(terminationType termi)
{
   terminationModel=termi;
};

SIGEL_GP::SIG_GPParameter::terminationType SIGEL_GP::SIG_GPParameter::getTerminationModel() const
{
   return terminationModel;
};

void SIGEL_GP::SIG_GPParameter::setSaveExit(bool sexit)
{
   saveExit=sexit;
};

bool SIGEL_GP::SIG_GPParameter::getSaveExit() const
{
   return saveExit;
};

void SIGEL_GP::SIG_GPParameter::setGraveYardDirectory(QDir directory)
{
   graveYardDirectory=directory;
};

QDir SIGEL_GP::SIG_GPParameter::getGraveYardDirectory() const
{
   return graveYardDirectory;
};

void SIGEL_GP::SIG_GPParameter::setLiveUndead(bool grave)
{
   liveUndead=grave;
};

bool SIGEL_GP::SIG_GPParameter::getLiveUndead() const
{
   return liveUndead;
};

void SIGEL_GP::SIG_GPParameter::setPoolImageDirectory(QDir PIDir)
{
   poolImageDirectory=PIDir;
};

QDir SIGEL_GP::SIG_GPParameter::getPoolImageDirectory() const
{
   return poolImageDirectory;
};

void SIGEL_GP::SIG_GPParameter::setPoolImageGeneration(int gener)
{
   poolImageGeneration=gener;
};

int SIGEL_GP::SIG_GPParameter::getPoolImageGeneration() const
{
   return poolImageGeneration;
};


void SIGEL_GP::SIG_GPParameter::setPriority(priorityLevel prio)
{
    priority=prio;

    // for each case, a special set of commands is given, depending on
    // the right configuration of each prioritylevel, which sets the
    // secondary priority values. these are:
    // passiveTime unsigned long
    // maxTouchesPerLoop int
    // toDoSweepsPerLoop int
 
    switch (priority)
      {
      case veryLow:
	setPassiveTime(1000);
	setMaxTouchsPerLoop(-1);
	setToDoSweepsPerLoop(1);
	break;
      case low:
	setPassiveTime(100);
	setMaxTouchsPerLoop(-1);
	setToDoSweepsPerLoop(1);
	break;
      case normal:
	setPassiveTime(50);
	setMaxTouchsPerLoop(-1);
	setToDoSweepsPerLoop(1);
	break;
      case high:
	setPassiveTime(10);
	setMaxTouchsPerLoop(-1);
	setToDoSweepsPerLoop(1);
	break;
      case veryHigh:
	setPassiveTime(1);
	setMaxTouchsPerLoop(-1);
	setToDoSweepsPerLoop(1);
	break;
      };
};  
  
SIGEL_GP::SIG_GPParameter::priorityLevel SIGEL_GP::SIG_GPParameter::getPriority() const
{
   return priority;
};

double SIGEL_GP::SIG_GPParameter::getMaxFitness() const
{
   return maxFitness;
   
};

void SIGEL_GP::SIG_GPParameter::setMaxFitness(double maxfit)
{
   maxFitness=maxfit;
};


QString SIGEL_GP::SIG_GPParameter::getFitnessName() const
{
   return fitnessName;
};

void SIGEL_GP::SIG_GPParameter::setFitnessName(QString name)
{
   fitnessName=name;
};

void SIGEL_GP::SIG_GPParameter::readFromFile(QTextStream & file)
{
  QString s;
  hostList.deleteContents();

  while (!file.atEnd()) {
   s = file.readLine();

   if ( s == "RANDOMSEED") {
      s=file.readLine();
      randomSeed=s.toInt();
		}

   if ( s == "MININDLENGTH") {
      s=file.readLine();
      minIndLength=s.toInt();
		}

   if ( s == "MAXINDLENGTH") {
      s=file.readLine();
      maxIndLength=s.toInt();
		}

    if ( s == "RESEVGEN") {
      s = file.readLine();
      setResetEveryGeneration(s.toInt());
    }

   if ( s == "REPRODUCTIONPROB") {
      s=file.readLine();
      reproductionProb=s.toInt();
		}

   if ( s == "XOVERPROB") {
      s=file.readLine();
      xoverProb=s.toInt();
		}

   if ( s == "MUTATIONPROB") {
      s=file.readLine();
      mutationProb=s.toInt();
		}

   if ( s == "TOURNAMENTSPERGENERATION") {
      s=file.readLine();
      tournamentsPerGeneration=s.toDouble();
		}

   if ( s == "MAXAGE") {
      s=file.readLine();
      maxAge=s.toInt();
		}

    if ( s == "TERMINATIONTIME") {
      s=file.readLine();
      int year=s.toInt();
      s=file.readLine();
      int month=s.toInt();
      s=file.readLine();
      int day=s.toInt();
      s=file.readLine();
      int hour=s.toInt();
      s=file.readLine();
      int minute=s.toInt();
      s=file.readLine();
      int second=s.toInt();
      QDate date(year,month,day);
      QTime time(hour,minute,second,0);
      terminationTime.setDate(date);
      terminationTime.setTime(time);
		}

   if ( s == "TERMINATIONGENERATIONNO") {
      s=file.readLine();
      terminationGenerationNo=s.toInt();
		}

   if ( s == "TERMINATIONMODEL") {
      s=file.readLine();
      terminationModel=static_cast<terminationType>(s.toInt());
		}

    if ( s == "SAVEEXIT") {
      s=file.readLine();
      int se=s.toInt();
      if (se==1)
        saveExit=true;
      else
        saveExit=false;
		}

    if ( s == "GRAVEYARDDIRECTORY") {
      s=file.readLine();
      graveYardDirectory.setPath(s);
		}

    if ( s == "LIVEUNDEAD") {
      s=file.readLine();
      int lu=s.toInt();
      if (lu==1)
        liveUndead=true;
      else
        liveUndead=false;
		}

    if ( s == "POOLIMAGEDIRECTORY") {
      s=file.readLine();
      poolImageDirectory.setPath(s);
		}

    if ( s == "POOLIMAGEGENERATION") {
      s=file.readLine();
      poolImageGeneration=s.toInt();
		}

    if ( s == "PRIORITY") {
      s=file.readLine();
      setPriority(static_cast<priorityLevel>(s.toInt()));
		}

    if ( s == "FITNESSNAME") {
      s=file.readLine();
      setFitnessName(s);
		}

    if ( s == "TERMINATIONUSESDATE") {
      s = file.readLine();
      int terminationUsesDateInt = s.toInt();
      if (terminationUsesDateInt == 1)
        terminationUsesDate = true;
      else
        terminationUsesDate = false;
		}

    if ( s == "PROBABILITY_ADD") {
      s=file.readLine();
      setProbability( SIGEL_Program::ADD, s.toInt() );
		}

    if ( s == "PROBABILITY_SUB") {
      s=file.readLine();
      setProbability( SIGEL_Program::SUB, s.toInt() );
		}

    if ( s == "PROBABILITY_MUL") {
      s=file.readLine();
      setProbability( SIGEL_Program::MUL, s.toInt() );
		}

    if ( s == "PROBABILITY_DIV") {
      s=file.readLine();
      setProbability( SIGEL_Program::DIV, s.toInt() );
		}

    if ( s == "PROBABILITY_LOAD") {
      s=file.readLine();
      setProbability( SIGEL_Program::LOAD, s.toInt() );
		}

    if ( s == "PROBABILITY_COPY") {
      s=file.readLine();
      setProbability( SIGEL_Program::COPY, s.toInt() );
		}

    if ( s == "PROBABILITY_MIN") {
      s=file.readLine();
      setProbability( SIGEL_Program::MIN, s.toInt() );
		}

   if ( s == "PROBABILITY_MAX") {
      s=file.readLine();
      setProbability( SIGEL_Program::MAX, s.toInt() );
		}

    if ( s == "PROBABILITY_CMP") {
      s=file.readLine();
      setProbability( SIGEL_Program::CMP, s.toInt() );
		}

    if ( s == "PROBABILITY_JMP") {
      s=file.readLine();
      setProbability( SIGEL_Program::JMP, s.toInt() );
		}

    if ( s == "PROBABILITY_MOVE") {
      s=file.readLine();
      setProbability( SIGEL_Program::MOVE, s.toInt() );
		}

    if ( s == "PROBABILITY_SENSE") {
      s=file.readLine();
      setProbability( SIGEL_Program::SENSE, s.toInt() );
		}

    if ( s == "PROBABILITY_DELAY") {
      s=file.readLine();
      setProbability( SIGEL_Program::DELAY, s.toInt() );
		}

    if ( s == "PROBABILITY_NOP") {
      s=file.readLine();
      setProbability( SIGEL_Program::NOP, s.toInt() );
		}

    if ( s == "PROBABILITY_MOD") {
      s=file.readLine();
      setProbability( SIGEL_Program::MOD, s.toInt() );
		}

    if ( s == "TIMEOUTMINUTES") {
      s = file.readLine();
      setTimeOutMinutes( s.toInt() );
		}

    if ( s == "TERMINATIONDURATIONDAYS") {
      s = file.readLine();
      setTerminationDurationDays( s.toInt() );
		}

    if ( s == "TERMINATIONDURATIONHOURS") {
      s = file.readLine();
      setTerminationDurationHours( s.toInt() );
		}

    if ( s == "TERMINATIONDURATIONMINUTES") {
      s = file.readLine();
      setTerminationDurationMinutes( s.toInt() );
		}

    if ( s == "TERMINATIONDURATIONSECONDS") {
      s = file.readLine();
      setTerminationDurationSeconds( s.toInt() );
		}

    if ( s == "PVMHOST") {
      s = file.readLine();
      SIGEL_GP::SIG_GPPVMHost *newHost = new SIGEL_GP::SIG_GPPVMHost( s );
      hostList.append( newHost );
		}

  }

};

void SIGEL_GP::SIG_GPParameter::writeToFile(QTextStream & file)
{
  file << "RANDOMSEED\n";
  file << randomSeed << "\n";
  file << "MININDLENGTH\n";
  file << minIndLength << "\n";
  file << "MAXINDLENGTH\n";
  file << maxIndLength << "\n";
  file << "RESEVGEN\n";
  file << resEvGen << "\n";
  file << "REPRODUCTIONPROB\n";
  file << reproductionProb << "\n";
  file << "XOVERPROB\n";
  file << xoverProb << "\n";
  file << "MUTATIONPROB\n";
  file << mutationProb << "\n";
  file << "TOURNAMENTSPERGENERATION\n";
  file << tournamentsPerGeneration << "\n";
  file << "MAXAGE\n";
  file << maxAge << "\n";
  file << "TERMINATIONTIME\n";
  file << terminationTime.date().year() << "\n";
  file << terminationTime.date().month() << "\n";
  file << terminationTime.date().day() << "\n";
  file << terminationTime.time().hour() << "\n";
  file << terminationTime.time().minute() << "\n";
  file << terminationTime.time().second() << "\n";
  file << "TERMINATIONGENERATIONNO\n";
  file << terminationGenerationNo << "\n";
  file << "TERMINATIONMODEL\n";
  file << static_cast<int>(terminationModel) << "\n";
  file << "SAVEEXIT\n";
  if (saveExit)
    file << 1 << "\n";
  else
    file << 0 << "\n";  
  file << "GRAVEYARDDIRECTORY\n";
  file << graveYardDirectory.path() << "\n";
  file << "LIVEUNDEAD\n";
  if (liveUndead)
    file << 1 << "\n";
  else
    file << 0 << "\n";  
  file << "POOLIMAGEDIRECTORY\n";
  file << poolImageDirectory.path() << "\n";
  file << "POOLIMAGEGENERATION\n";
  file << poolImageGeneration << "\n";
  file << "PRIORITY\n";
  file << static_cast<int>(priority) << "\n";
  file << "FITNESSNAME\n";
  file << fitnessName << "\n";
  file << "TERMINATIONUSESDATE\n";
  if (terminationUsesDate)
    file << "1\n";
  else
    file << "0\n";

  file << "PROBABILITY_ADD\n";
  file << getProbability(SIGEL_Program::ADD) << "\n";  
  file << "PROBABILITY_SUB\n";
  file << getProbability(SIGEL_Program::SUB) << "\n";
  file << "PROBABILITY_MUL\n";
  file << getProbability(SIGEL_Program::MUL) << "\n";
  file << "PROBABILITY_DIV\n";
  file << getProbability(SIGEL_Program::DIV) << "\n";
  file << "PROBABILITY_LOAD\n";
  file << getProbability(SIGEL_Program::LOAD) << "\n";
  file << "PROBABILITY_COPY\n";
  file << getProbability(SIGEL_Program::COPY) << "\n";
  file << "PROBABILITY_MIN\n";
  file << getProbability(SIGEL_Program::MIN) << "\n";
  file << "PROBABILITY_MAX\n";
  file << getProbability(SIGEL_Program::MAX) << "\n";
  file << "PROBABILITY_CMP\n";
  file << getProbability(SIGEL_Program::CMP) << "\n";
  file << "PROBABILITY_JMP\n";
  file << getProbability(SIGEL_Program::JMP) << "\n";
  file << "PROBABILITY_MOVE\n";
  file << getProbability(SIGEL_Program::MOVE) << "\n";
  file << "PROBABILITY_SENSE\n";
  file << getProbability(SIGEL_Program::SENSE) << "\n";
  file << "PROBABILITY_DELAY\n";
  file << getProbability(SIGEL_Program::DELAY) << "\n";
  file << "PROBABILITY_NOP\n";
  file << getProbability(SIGEL_Program::NOP) << "\n";  
  file << "PROBABILITY_MOD\n";
  file << getProbability(SIGEL_Program::MOD) << "\n";
  file << "TIMEOUTMINUTES\n";
  file << getTimeOutMinutes() << "\n";
  file << "TERMINATIONDURATIONDAYS\n";
  file << getTerminationDurationDays() << "\n";
  file << "TERMINATIONDURATIONHOURS\n";
  file << getTerminationDurationHours() << "\n";
  file << "TERMINATIONDURATIONMINUTES\n";
  file << getTerminationDurationMinutes() << "\n";
  file << "TERMINATIONDURATIONSECONDS\n";
  file << getTerminationDurationSeconds() << "\n";

  SIGEL_GP::SIG_GPPVMHost *actHost = hostList.first();

  while (actHost)
    {
      file << "PVMHOST\n";
      file << actHost->print();

      actHost = hostList.next();
    };

};


Q2PtrList< SIGEL_GP::SIG_GPPVMHost > const &SIGEL_GP::SIG_GPParameter::getHostList() const
{
  return hostList;
};

Q2PtrList< SIGEL_GP::SIG_GPPVMHost > &SIGEL_GP::SIG_GPParameter::getHostList()
{
  return hostList;
};

  
void SIGEL_GP::SIG_GPParameter::setProbability( SIGEL_Program::Robotinstruction instruction, int prob )
{
  /*

  switch( instruction )
    {

    case SIGEL_Program::ADD:
      instructionProb[0] = prob;
      break;

    case SIGEL_Program::SUB:
      instructionProb[1] = prob;
      break;

    case SIGEL_Program::MUL:
      instructionProb[2] = prob;
      break;

    case SIGEL_Program::DIV:
      instructionProb[3] = prob;
      break;

    case SIGEL_Program::LOAD:
      instructionProb[4] = prob;
      break;

    case SIGEL_Program::MIN:
      instructionProb[5] = prob;
      break;

    case SIGEL_Program::MAX:
      instructionProb[6] = prob;
      break;

    case SIGEL_Program::CMP:
      instructionProb[7] = prob;
      break;

    case SIGEL_Program::JMP:
      instructionProb[8] = prob;
      break;

    case SIGEL_Program::NOP:
      instructionProb[9] = prob;
      break;

    case SIGEL_Program::MOD:
      instructionProb[10] = prob;
      break;

    case SIGEL_Program::COPY:
      instructionProb[11] = prob;
      break;

    case SIGEL_Program::MOVE:
      instructionProb[12] = prob;
      break;

    case SIGEL_Program::SENSE:
      instructionProb[13] = prob;
      break;

    case SIGEL_Program::DELAY:
      instructionProb[14]= prob;
      break;
    }

  */
    int instructionID = static_cast< int >( instruction ); 
    instructionProb[instructionID] = prob;

}

  
int  SIGEL_GP::SIG_GPParameter::getProbability( SIGEL_Program::Robotinstruction instruction )
{

  int instructionID = static_cast< int >( instruction );
  int prob          = instructionProb[instructionID];
  
  /*
  switch( instruction )
    {

    case SIGEL_Program::ADD:
      prob = instructionProb[0];
      break;

    case SIGEL_Program::SUB:
      prob = instructionProb[1];
      break;

    case SIGEL_Program::MUL:
      prob = instructionProb[2];
      break;

    case SIGEL_Program::DIV:
      prob = instructionProb[3];
      break;

    case SIGEL_Program::LOAD:
      prob = instructionProb[4];
      break;

    case SIGEL_Program::MIN:
      prob = instructionProb[5];
      break;

    case SIGEL_Program::MAX:
      prob = instructionProb[6];
      break;

    case SIGEL_Program::CMP:
      prob = instructionProb[7];
      break;

    case SIGEL_Program::JMP:
      prob = instructionProb[8];
      break;

    case SIGEL_Program::NOP:
      prob = instructionProb[9];
      break;

    case SIGEL_Program::MOD:
      prob = instructionProb[10];
      break;

    case SIGEL_Program::COPY:
      prob = instructionProb[11];
      break;

    case SIGEL_Program::MOVE:
      prob = instructionProb[12];
      break;

    case SIGEL_Program::SENSE:
      prob = instructionProb[13];
      break;

    case SIGEL_Program::DELAY:
      prob = instructionProb[14];
      break;

    }
   
   */

    return prob;

}

Q2Array< int >& SIGEL_GP::SIG_GPParameter::getInstructionProbabilities()
{
  return instructionProb;
}

bool SIGEL_GP::SIG_GPParameter::getTerminationUsesDate() const
{
  return terminationUsesDate;
};

void SIGEL_GP::SIG_GPParameter::setTerminationUsesDate( bool newValue )
{
  terminationUsesDate = newValue;
};

int SIGEL_GP::SIG_GPParameter::getTimeOutMinutes()
{
  return timeOutMinutes;
};

void SIGEL_GP::SIG_GPParameter::setTimeOutMinutes( int newValue )
{
  timeOutMinutes = newValue;
};

