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
#ifndef MT_CONTROL_MT_ESTIMATIONSTATE_H
#define MT_CONTROL_MT_ESTIMATIONSTATE_H

#include <QList>

/**
 * The estimation settings and the counts per SIGEL generation that MT_Controller
 * keeps as a copy of its substitute's data. The MetaGP window shows and edits them.
 */
struct MT_EstimationState
{
	// True while the controller holds loaded data.
	bool inUse;

	int strategy;
	int refreshInterval;
	double tolerance;

	// The number of SIGEL generations that the two lists cover.
	unsigned int generationCount;

	// Per SIGEL generation: the number of simulations, and the number of estimations by MetaGP.
	// MT_Evaluator counts individuals, MT_Classifier counts tournaments.
	QList<unsigned int> *simulationCounts;
	QList<unsigned int> *estimationCounts;
};

#endif // MT_CONTROL_MT_ESTIMATIONSTATE_H
