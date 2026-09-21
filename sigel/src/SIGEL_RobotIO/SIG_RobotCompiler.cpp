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
#include "SIGEL_RobotIO/SIG_RobotCompiler.h"
#include "SIGEL_RobotIO/SIG_RobotIOExceptions.h"

namespace SIGEL_RobotIO {
        SIG_RobotCompiler::SIG_RobotCompiler (SIG_RobotScanner &sc,
                                              SIG_Robot *tg,
                                              QString homepath)
                : myScanner (sc),
                  target (tg),
                  dirprefix (homepath)
        { }

        SIG_RobotCompiler::~SIG_RobotCompiler (void)
        { }

        /* to be deleted 
        QString SIG_RobotCompiler::prefixFile (QString filename)
        {
                if (filename.length () == 0)
                        throw SIG_SemanticError (__FILE__, __LINE__,
                                                 "Empty filename");
                // Qt 2's QString::at was bounds-safe; Qt 6's is not.
		if (filename.isEmpty() || filename.at (0) != '/')
                        return dirprefix + filename;
                else
                        return filename;
        }
        */

        void SIG_RobotCompiler::expect (int symboltype)
        {
                int st;
                QString sym;

                myScanner.readSymbol (st, sym);
                if (st != symboltype)
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "Wrong symbol: " +
                                               QString::number (st) + " instead of " +
                                               QString::number (symboltype) + ".",
                                               "(unknown)",
                                               myScanner.currentLine ());
        }

        QString SIG_RobotCompiler::expectWord (void)
        {
                int st;
                QString sym;

                myScanner.readSymbol (st, sym);
                if (st != RobotSymbol::word)
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "Word expected.",
                                               "(unknown)",
                                               myScanner.currentLine ());
                return sym;
        }

        DL_Scalar SIG_RobotCompiler::expectNumber (void)
        {
                int st;
                QString sym;

                myScanner.readSymbol (st, sym);
                if (st != RobotSymbol::number)
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "Number expected.",
                                               "(unknown)",
                                               myScanner.currentLine ());
                return sym.toDouble ();
        }

        QString SIG_RobotCompiler::expectString (void)
        {
                int st;
                QString sym;

                myScanner.readSymbol (st, sym);
                if (st != RobotSymbol::string)
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "String expected.",
                                               "(unknown)",
                                               myScanner.currentLine ());
                return sym;
        }

        bool SIG_RobotCompiler::compileNextEntity (void)
        {
                int symType;
                QString symbol;

                do {
                        myScanner.readSymbol (symType, symbol);
                } while (symType == Symbol::None);

                if (symType == Symbol::EOS) {
                        return false;
                } else if (symType != RobotSymbol::word) {
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'material','link','joint','drive','sensor','scaleall' expected.",
                                               "(unknown)", myScanner.currentLine ());
                } else {
                        if (symbol == "material")
                                nextIsMaterial ();
                        else if (symbol == "link")
                                nextIsLink ();
                        else if (symbol == "joint")
                                nextIsJoint ();
                        else if (symbol == "glue")
                                nextIsGlue ();
                        else if (symbol == "drive")
                                nextIsDrive ();
                        else if (symbol == "sensor")
                                nextIsSensor ();
                        else if (symbol == "surface")
                                nextIsSurface ();
                        else if (symbol == "scaleall")
                                nextModScaleall ();
                        else
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'material','link','joint','drive','sensor','scaleall' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                }
                return true;
        }

        void SIG_RobotCompiler::nextIsMaterial (void)
        {
                int symtype;
                QString symstr;
                SIG_Material *mater;

                QString objectName = expectWord ();
                expect (RobotSymbol::openingBrace);

                // Insertion point: find or create the material object.
                mater = materialFind (objectName);

                symstr = expectWord ();
                if (symstr != "density")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'density' expected",
                                               "(unknown)",
                                               myScanner.currentLine ());
                DL_Scalar density = expectNumber ();
                expect (RobotSymbol::semicolon);
                // Insertion point: the material density is known.
                materialDensity (mater, density);

                myScanner.peekSymbol (symtype, symstr);
                while ((symtype == RobotSymbol::word) &&
                       (symstr == "friction")) {
                        myScanner.nextSymbol ();

                        DL_Scalar konstante = expectNumber ();

                        symstr = expectWord ();
                        if (symstr != "on")
                                throw SIG_SyntaxError
                                        (__FILE__, __LINE__,
                                         "'on' expected", "(unknown)",
                                         myScanner.currentLine ());

                        QString anderes_material = expectWord ();

                        expect (RobotSymbol::semicolon);

                        // Insertion point: friction constant complete
                        materialFriction (mater, anderes_material, konstante);
                        myScanner.peekSymbol (symtype, symstr);
                }

                if ((symtype == RobotSymbol::word) &&
                    (symstr == "elasticity")) {
                        myScanner.nextSymbol ();

                        DL_Scalar elas = expectNumber ();
                        expect (RobotSymbol::semicolon);

                        //Insertion point: elasticity constant given
                        materialElasticity (mater, elas);
                        myScanner.peekSymbol (symtype, symstr);
                }

                if ((symtype == RobotSymbol::word) &&
                    (symstr == "colour")) {
                        myScanner.nextSymbol ();

                        DL_Scalar rot, gruen, blau;

                        if (expectWord () != "red")
                                throw SIG_SyntaxError
                                        (__FILE__, __LINE__,
                                         "'red' expected", "(unknown)",
                                         myScanner.currentLine ());
                        rot = expectNumber ();

                        if (expectWord () != "green")
                                throw SIG_SyntaxError
                                        (__FILE__, __LINE__,
                                         "'green' expected", "(unknown)",
                                         myScanner.currentLine ());
                        gruen = expectNumber ();

                        if (expectWord () != "blue")
                                throw SIG_SyntaxError
                                        (__FILE__, __LINE__,
                                         "'blue' expected", "(unknown)",
                                         myScanner.currentLine ());
                        blau = expectNumber ();

                        expect (RobotSymbol::semicolon);
                        // Insertion point: colour fully read
                        materialColour (mater, rot, gruen, blau);

                        myScanner.peekSymbol (symtype, symstr);
                }

                expect (RobotSymbol::closingBrace);
                // Insertion point: end end end
                materialFinish (mater);
        }

        void SIG_RobotCompiler::nextIsLink (void)
        {
                int symtype;
                QString symstr;
                SIG_Link *thema;
                
                QString objectName = expectWord ();
                expect (RobotSymbol::openingBrace);

                // Insertion point: create or look up the object
                thema = linkFind (objectName);

                symstr = expectWord ();
                if (symstr == "torso") {
                        // Insertion point: trunk link
                        linkIsTorso (thema);
                        expect (RobotSymbol::semicolon);
                        symstr = expectWord ();
                }

                if (symstr != "geometry")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'geometry' expected",
                                               "(unknown)",
                                               myScanner.currentLine ());
                symstr = expectString ();
                // Insertion point: geometry file
                linkGeometryFile (thema, symstr);
                expect (RobotSymbol::semicolon);

                symstr = expectWord ();
                if (symstr != "material")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'material' expected.",
                                               "(unknown)",
                                               myScanner.currentLine ());
                symstr = expectWord ();
                // Insertion point: the material is known
                linkMaterial (thema, symstr);
                expect (RobotSymbol::semicolon);

                myScanner.peekSymbol (symtype, symstr);
                while ((symtype == RobotSymbol::word) &&
                       (symstr == "point")) {
                        myScanner.nextSymbol ();
                        
                        QString pointname;
                        DL_Scalar xval, yval, zval;

                        pointname = expectWord ();
                        expect (RobotSymbol::equals);
                        expect (RobotSymbol::openingParen);
                        xval = expectNumber ();
                        expect (RobotSymbol::comma);
                        yval = expectNumber ();
                        expect (RobotSymbol::comma);
                        zval = expectNumber ();
                        expect (RobotSymbol::closingParen);
                        expect (RobotSymbol::semicolon);

                        // Insertion point: a point is known.
                        linkPoint (thema, pointname, xval, yval, zval);
                        myScanner.peekSymbol (symtype, symstr);
                }

                if ((symtype == RobotSymbol::word) &&
                    (symstr == "no_collision")) {
                        myScanner.nextSymbol ();

                        QString ncl;

                        do {
                                ncl = expectWord ();
                                // Insertion point: non-collision specification
                                linkNoCollide (thema, ncl);
                                myScanner.readSymbol (symtype, symstr);
                        } while (symtype == RobotSymbol::comma);

                        if (symtype != RobotSymbol::semicolon)
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "';' expected.",
                                                       "(unknown)",
                                                       myScanner.currentLine ());
                }

                expect (RobotSymbol::closingBrace);
                // Insertion point: end of reading
                linkFinish (thema);
        }

        void SIG_RobotCompiler::nextIsJoint (void)
        {
                QString symstr;
                int symtype;

                QString jointtype = expectWord ();
                if (jointtype == "rotational") {
                        QString jointname = expectWord ();
                        expect (RobotSymbol::openingBrace);
                        SIG_RotationalJoint *rj = rjointFind (jointname);

                        QString Alink, A1, A2, A3, Blink, B1, B2, B3;
                        if (expectWord () != "between")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'between' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        Alink = expectWord ();
                        expect (RobotSymbol::openingParen);
                        A1 = expectWord ();
                        expect (RobotSymbol::comma);
                        A2 = expectWord ();
                        expect (RobotSymbol::comma);
                        A3 = expectWord ();
                        expect (RobotSymbol::closingParen);
                        if (expectWord () != "and")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'and' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        Blink = expectWord ();
                        expect (RobotSymbol::openingParen);
                        B1 = expectWord ();
                        expect (RobotSymbol::comma);
                        B2 = expectWord ();
                        expect (RobotSymbol::comma);
                        B3 = expectWord ();
                        expect (RobotSymbol::closingParen);
                        expect (RobotSymbol::semicolon);
                        rjointLinking (rj, Alink, A1, A2, A3, Blink, B1, B2, B3);

                        if (expectWord () != "minimal")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'minimal' expected",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar mn = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "maximal")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'maximal' expected",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar mx = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "init")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'init' expected",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar ii = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        rjointExtents (rj, mn, mx, ii);
                        
                        expect (RobotSymbol::closingBrace);
                        rjointFinish (rj);
                } else if (jointtype == "translational") {
                        QString jointname = expectWord ();
                        expect (RobotSymbol::openingBrace);
                        SIG_TranslationalJoint *tj = tjointFind (jointname);

                        QString Alink, A1, A2, A3, Blink, B1, B2, B3;
                        if (expectWord () != "between")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'between' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        Alink = expectWord ();
                        expect (RobotSymbol::openingParen);
                        A1 = expectWord ();
                        expect (RobotSymbol::comma);
                        A2 = expectWord ();
                        expect (RobotSymbol::comma);
                        A3 = expectWord ();
                        expect (RobotSymbol::closingParen);
                        if (expectWord () != "and")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'and' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        Blink = expectWord ();
                        expect (RobotSymbol::openingParen);
                        B1 = expectWord ();
                        expect (RobotSymbol::comma);
                        B2 = expectWord ();
                        expect (RobotSymbol::comma);
                        B3 = expectWord ();
                        expect (RobotSymbol::closingParen);
                        expect (RobotSymbol::semicolon);
                        tjointLinking (tj, Alink, A1, A2, A3, Blink, B1, B2, B3);
                        
                        if (expectWord () != "minimal")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'minimal' expected",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar mn = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "maximal")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'maximal' expected",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar mx = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "init")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'init' expected",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar ii = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        tjointExtents (tj, mn, mx, ii);
                        
                        expect (RobotSymbol::closingBrace);
                        tjointFinish (tj);
                } else if (jointtype == "cylindrical") {
                        QString jointname = expectWord ();
                        expect (RobotSymbol::openingBrace);
                        SIG_CylindricalJoint *cj = cjointFind (jointname);

                        QString Alink, A1, A2, A3, Blink, B1, B2, B3;
                        if (expectWord () != "between")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'between' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        Alink = expectWord ();
                        expect (RobotSymbol::openingParen);
                        A1 = expectWord ();
                        expect (RobotSymbol::comma);
                        A2 = expectWord ();
                        expect (RobotSymbol::comma);
                        A3 = expectWord ();
                        expect (RobotSymbol::closingParen);
                        if (expectWord () != "and")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'and' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        Blink = expectWord ();
                        expect (RobotSymbol::openingParen);
                        B1 = expectWord ();
                        expect (RobotSymbol::comma);
                        B2 = expectWord ();
                        expect (RobotSymbol::comma);
                        B3 = expectWord ();
                        expect (RobotSymbol::closingParen);
                        expect (RobotSymbol::semicolon);
                        cjointLinking (cj, Alink, A1, A2, A3, Blink, B1, B2, B3);

                        if (expectWord () != "minimal_rot")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'minimal_rot' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar mn = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "maximal_rot")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'maximal_rot' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar mx = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "init_rot")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'init_rot' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        DL_Scalar ii = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        cjointRotExtents (cj, mn, mx, ii);

                        if (expectWord () != "minimal_trans")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'minimal_trans' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        mn = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "maximal_trans")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'maximal_trans' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        mx = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        if (expectWord () != "init_trans")
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "'init_trans' expected.",
                                                       "(unknown)", myScanner.currentLine ());
                        ii = expectNumber ();
                        expect (RobotSymbol::semicolon);
                        cjointTraExtents (cj, mn, mx, ii);

                        expect (RobotSymbol::closingBrace);
                        cjointFinish (cj);
                } else
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "Unknown joint type '" + jointtype + "'",
                                               "(unknown)", myScanner.currentLine ());
        }

        void SIG_RobotCompiler::nextIsGlue (void)
        {
                QString name;
                SIG_GlueJoint *glue;
                QString Alink, A1, A2, A3, Blink, B1, B2, B3;
                
                name = expectWord ();

                expect (RobotSymbol::openingBrace);
                glue = glueFind (name);
                if (expectWord () != "between")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'between' expected", "(unknown)",
                                               myScanner.currentLine ());
                Alink = expectWord ();
                expect (RobotSymbol::openingParen);
                A1 = expectWord ();
                expect (RobotSymbol::comma);
                A2 = expectWord ();
                expect (RobotSymbol::comma);
                A3 = expectWord ();
                expect (RobotSymbol::closingParen);
                if (expectWord () != "and")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'and' expected.",
                                               "(unknown)", myScanner.currentLine ());
                Blink = expectWord ();
                expect (RobotSymbol::openingParen);
                B1 = expectWord ();
                expect (RobotSymbol::comma);
                B2 = expectWord ();
                expect (RobotSymbol::comma);
                B3 = expectWord ();
                expect (RobotSymbol::closingParen);
                expect (RobotSymbol::semicolon);
                glueLinking (glue, Alink, A1, A2, A3, Blink, B1, B2, B3);

                expect (RobotSymbol::closingBrace);
                glueFinish (glue);
        }

        void SIG_RobotCompiler::nextIsDrive (void)
        {
                QString name, m;
                SIG_Drive *thedrive;

                name = expectWord ();
                thedrive = driveFind (name);

                expect (RobotSymbol::openingBrace);
                m = expectWord ();
                if ((m != "force") &&
                    (m != "relative") &&
                    (m != "absolute") &&
                    (m != "simpleservo"))
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "Drive mode ('force', 'relative', 'absolute', 'simpleservo') expected.",
                                               "(unknown)", myScanner.currentLine ());
                driveMode (thedrive, m);
                driveJoint (thedrive, expectWord ());
                expect (RobotSymbol::semicolon);
                if (expectWord () != "minimalforce")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'minimalforce' expected",
                                               "(unknown)", myScanner.currentLine ());
                DL_Scalar mini = expectNumber ();
                expect (RobotSymbol::semicolon);
                if (expectWord () != "maximalforce")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'maximalforce' expected",
                                               "(unknown)", myScanner.currentLine ());
                driveMinMaxForce (thedrive, mini, expectNumber ());
                expect (RobotSymbol::semicolon);
                expect (RobotSymbol::closingBrace);
        }

        void SIG_RobotCompiler::nextIsSensor (void)
        {
                QString name, styp;

                name = expectWord ();
                expect (RobotSymbol::openingBrace);
                styp = expectWord ();

				// joint sensor type
                if (styp == "joint") {
                        SIG_JointSensor *js = jointSensorFind (name);
                        jointSensorJoint (js, expectWord ());
                        expect (RobotSymbol::semicolon);
                        jointSensorFinish (js);
                }

				// pitch sensor type
				else if (styp == "pitch") {
                        SIG_PitchRollSensor *prs = pitchRollSensorFind (name);
						prs->SetPitchType();
                        pitchRollSensorLink(prs, expectWord ());
                        expect (RobotSymbol::semicolon);
                        pitchRollSensorFinish (prs);
                }

				// roll sensor type
				else if (styp == "roll") {
                        SIG_PitchRollSensor *prs = pitchRollSensorFind (name);
						prs->SetRollType();
                        pitchRollSensorLink(prs, expectWord ());
                        expect (RobotSymbol::semicolon);
                        pitchRollSensorFinish (prs);
                }

				// contact sensor type
				else if (styp == "contact") {
                        SIG_ContactSensor *cs = contactSensorFind (name);
                        contactSensorLink(cs, expectWord ());
                        expect (RobotSymbol::semicolon);
                        contactSensorFinish (cs);
                }

				// whoopsie -- dunno this type !
				else
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "Invalid sensor type '"+styp+"'",
                                               "(unknown)", myScanner.currentLine ());
                expect (RobotSymbol::closingBrace);
        }

        void SIG_RobotCompiler::nextIsSurface (void)
        {

                QString symstr;
                int symtype;

                SIG_Geometry *geom;
                SIG_Polygon *poly;

                if (expectWord () != "for")
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "'for' expected",
                                               "(unknown)",
                                               myScanner.currentLine ());
                QString filename = expectString ();

                geom = surfaceFind (filename);

                myScanner.readSymbol (symtype, symstr);
                while (symtype == RobotSymbol::openingParen) {
                        poly = surfaceNewPoly (geom);

                        do {
                                DL_Scalar x = expectNumber ();
                                expect (RobotSymbol::comma);
                                DL_Scalar y = expectNumber ();
                                expect (RobotSymbol::comma);
                                DL_Scalar z = expectNumber ();

                                surfaceNewPoint (poly, x, y, z);

                                myScanner.readSymbol (symtype, symstr);
                        } while (symtype == RobotSymbol::slash);
                        if (symtype != RobotSymbol::closingParen)
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "')' expected",
                                                       "(unknown)",
                                                       myScanner.currentLine ());
                        myScanner.readSymbol (symtype, symstr);
                }
                if (symtype != RobotSymbol::semicolon)
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "';' expected",
                                               "(unknown)",
                                               myScanner.currentLine ());

                surfaceFinish (geom, filename);
        }

        void SIG_RobotCompiler::nextModScaleall (void)
        {
                DL_Scalar sf = expectNumber ();
                expect (RobotSymbol::semicolon);

                modifierScaleall (sf);
        }

        void SIG_RobotCompiler::runPass (void)
        {
                while (compileNextEntity ());
        }
}
