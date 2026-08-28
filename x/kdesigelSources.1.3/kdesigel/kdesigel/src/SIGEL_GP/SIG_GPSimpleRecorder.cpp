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
#include "SIGEL_GP/SIG_GPSimpleRecorder.h"
#include <exception>

#include <cmath>

SIGEL_GP::SIG_GPSimpleRecorder::SIG_GPSimpleRecorder()
: SIGEL_Simulation::SIG_Recorder(),
		    start(0,0,0),
		    end(0,0,0)
{
  
};

void SIGEL_GP::SIG_GPSimpleRecorder::init()
{
  // NOTE: 2003 declared throw(SIG_RecorderNoQueriesSetException,
  // SIG_RecorderBadRecordingOrderException) here. Unlike record() and
  // finish(), init() is called from SIG_Simulation's CONSTRUCTOR, which
  // every fitness function builds OUTSIDE its own try block. Without this
  // boundary a SIG_Exception from simulationQueries->getLinkPosition()
  // escapes to sigel_slave.cpp:361, which reports fitness 0.0 and the master
  // accepts it as a real result. (The example used to be named here as
  // SIG_DynaSystemWrongNumberException; that class went with the Dynamo
  // backend on 2026-08-28 -- physics_backends.md -- but the escape path is
  // unchanged and still swallows anything thrown from this constructor.)
  try {

  		SIG_Recorder::init();

  int rootLinkNumber = simulationQueries->getRootNumber();

  start = simulationQueries->getLinkPosition( rootLinkNumber );

  startRotation = simulationQueries->getLinkOrientation( rootLinkNumber );

  }
  catch (SIGEL_Simulation::SIG_RecorderNoQueriesSetException &) { throw; }
  catch (SIGEL_Simulation::SIG_RecorderBadRecordingOrderException &) { throw; }
  catch (...) { std::terminate(); }
};

void SIGEL_GP::SIG_GPSimpleRecorder::record()
{ };

void SIGEL_GP::SIG_GPSimpleRecorder::finish()
{
  SIG_Recorder::finish();

  int rootLinkNumber = simulationQueries->getRootNumber();

  end = simulationQueries->getLinkPosition( rootLinkNumber );
  endRotation = simulationQueries->getLinkOrientation( rootLinkNumber );
};
