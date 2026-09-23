# Libraries

The third-party code SIGEL 2.0 is built on. `vendor/README.md` says how the
vendored libraries are unpacked and patched. The original project's list is
at https://sigel.sourceforge.net/seiten/links_en.html.

## From the system

| library | version here | SIGEL uses it for | upstream |
|---|---|---|---|
| Qt 6: Core, Gui, Widgets, OpenGL, OpenGLWidgets | 6.10.2 | the interface and the 3-D view's widget. Qt Test is used by the checks only | https://www.qt.io/ |
| OpenGL and GLU | libGL 1.7.0 (libglvnd), GLU 9.0.2 (Mesa) | drawing the 3-D view | https://www.khronos.org/opengl/, https://gitlab.freedesktop.org/mesa/glu |
| libtirpc | 1.3.7 | PVM's data encoding | https://sourceforge.net/projects/libtirpc/ |

## Vendored

| library | version here | SIGEL uses it for | upstream | status |
|---|---|---|---|---|
| PVM | 3.4.6 | running the fitness evaluations on slave processes and other machines | https://www.netlib.org/pvm3/ | 3.4.6 is the newest release on netlib. SIGEL 1.3 used 3.4.3 |
| DynaMechs | 4.0, by `dm.h` | the physics simulation | https://dynamechs.sourceforge.net/ | the only file on SourceForge is `dynamechs_4.0pre1.zip`; it is not known whether the vendored copy is that one |
| Dynamo | as shipped with SIGEL | the maths types `DL_vector`, `DL_matrix` and `DL_Scalar`. The link also pulls in some of its physics code, because `matrix.cpp` calls it. SIGEL tried Dynamo as its physics engine and dropped it | https://web.archive.org/web/20031211131933/http://www.win.tue.nl/~bartb/dynamo/ | the original site is gone. `future_refactorings.md` item 64 removes it |
| NEWMAT | 09 | matrix maths | http://www.robertnz.net/nm_intro.htm | upstream has newmat10 and newmat11 (beta) |
| CyberVRML97 for C++ (`cv97`) | as shipped with SIGEL | reading the VRML 97 geometry of the robots' links | https://github.com/cybergarage/cyber-vrml97-cc | |
| Function Parser for C++ (`fparser`) | 2.1 | the floor function of an environment: the terrain height as a formula in x and z | http://warp.povusers.org/FunctionParser/ | upstream is at 4.5.2 |
