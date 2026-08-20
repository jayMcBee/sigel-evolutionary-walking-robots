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
#include "SIGEL_RobotIO/SIG_RobotBuilder.h"
#include <qfile.h>
#include <qtextstream.h>

namespace SIGEL_RobotIO {
        SIG_RobotBuilder::SIG_RobotBuilder (QString file)
                : SIG_RobotFactory (),
                  filename (file)
        { }
        
        SIG_RobotBuilder::~SIG_RobotBuilder (void)
        { }
        
        SIG_Robot *SIG_RobotBuilder::build (void)
        {
                robot = new SIG_Robot ();

                buildInto (*robot);

                return robot;
        }

        void SIG_RobotBuilder::buildInto (SIG_Robot & rob)
        {
                // normalization and initiation have been
                // removed due to the disentanglement of
                // the parser passes.
                rob.clear ();
                
                robot = &rob;

                firstPass ();
                secondPass ();
                rob.loadGeometries ();
        }
        
        QString SIG_RobotBuilder::loadFile (QString name)
        {
                QFile f;
                f.setFileName (name);
                f.open (QIODeviceBase::ReadOnly);
                QTextStream ts (&f);
                QString stri = ts.readAll ();
                f.close ();
                return stri;
        }
        
        void SIG_RobotBuilder::firstPass (void)
        {
                QString homepath;
                int occ = filename.lastIndexOf ('/');
                if (occ < 0)
                        homepath = "./";
                else
                        homepath = filename.left (occ + 1);
                
                SIG_RobotScanner *s = new SIG_RobotScanner
                        (loadFile (filename));
                SIG_RobotCompilerObjects *c = new SIG_RobotCompilerObjects
                        (*s, robot, homepath);
                c->runPass ();
                delete c;
                delete s;
        }
        
        void SIG_RobotBuilder::secondPass (void)
        {
                QString homepath;
                int occ = filename.lastIndexOf ('/');
                if (occ < 0)
                        homepath = "./";
                else
                        homepath = filename.left (occ + 1);
                
                SIG_RobotScanner *s = new SIG_RobotScanner
                        (loadFile (filename));
                SIG_RobotCompilerStructure *c = new SIG_RobotCompilerStructure
                        (*s, robot, homepath);
                c->runPass ();
                delete c;
                delete s;
        }
        
}
