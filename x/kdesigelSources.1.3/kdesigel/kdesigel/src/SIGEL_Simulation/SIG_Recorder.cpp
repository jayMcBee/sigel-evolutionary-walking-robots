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
#ifdef _WINDOWS
#pragma warning( disable : 4290 )
#endif

#include "SIGEL_Simulation/SIG_Recorder.h"

SIGEL_Simulation::SIG_Recorder::SIG_Recorder()
  : initialized(false), finished(false)
{
  simulationQueries = 0;
};

void SIGEL_Simulation::SIG_Recorder::init()
  throw(SIGEL_Simulation::SIG_RecorderNoQueriesSetException,
	SIGEL_Simulation::SIG_RecorderBadRecordingOrderException)
{ 
  if (simulationQueries==0)
    throw SIG_RecorderNoQueriesSetException(__FILE__,__LINE__,"Init called before setSimulationQueries");
  if (initialized)
    throw SIG_RecorderBadRecordingOrderException(__FILE__,__LINE__,"Init called more than once");
  initialized=true;
};

void SIGEL_Simulation::SIG_Recorder::record()
  throw(SIGEL_Simulation::SIG_RecorderNoQueriesSetException,
	SIGEL_Simulation::SIG_RecorderBadRecordingOrderException)
{
  if (simulationQueries==0)
    throw SIG_RecorderNoQueriesSetException(__FILE__,__LINE__,"Record called before setSimulationQueries");
  if (!initialized)
    throw SIG_RecorderBadRecordingOrderException(__FILE__,__LINE__,"Record called before Init");
  if (finished)
    throw SIG_RecorderBadRecordingOrderException(__FILE__,__LINE__,"Record called after Finish");
};

void SIGEL_Simulation::SIG_Recorder::finish()
  throw(SIGEL_Simulation::SIG_RecorderNoQueriesSetException,
	SIGEL_Simulation::SIG_RecorderBadRecordingOrderException)
{
  if (simulationQueries==0)
    throw SIG_RecorderNoQueriesSetException(__FILE__,__LINE__,"Finish called before setSimulationQueries");
  if (!initialized)
    throw SIG_RecorderBadRecordingOrderException(__FILE__,__LINE__,"Finish called before Init");
  if (finished)
    throw SIG_RecorderBadRecordingOrderException(__FILE__,__LINE__,"Finish called more than once");
  finished = true;
};

#ifdef _WINDOWS
void SIGEL_Simulation::SIG_Recorder::setSimulationQueries(SIG_SimulationQueries &newSimulationQueries)
#else
void SIGEL_Simulation::SIG_Recorder::setSimulationQueries(SIG_SimulationQueries const &newSimulationQueries)
#endif
{
  simulationQueries=&newSimulationQueries;
};
