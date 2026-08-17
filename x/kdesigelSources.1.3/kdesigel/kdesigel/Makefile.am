####### kdevelop will overwrite this part!!! (begin)##########

SUBDIRS = supportingLibs ui src include doc pixmaps templates 

EXTRA_DIST = Makefile.am sigelLauncher Terrain.ter povrayLauncher 

####### kdevelop will overwrite this part!!! (end)############

##### SUBDIRS = supportingLibs src ui include doc pixmaps templates

####### EXTRA_DIST = include pixmaps doc templates supportingLibs Terrain.ter povrayLauncher sigelLauncher

dist-hook:
	cp -r supportingLibs $(distdir)
	cd $(distdir)/supportingLibs; \
	make clean;

include  $(top_srcdir)/makefile-rules
INCLUDES = $(all_includes)

bin_PROGRAMS = sigel_slave sigel manage_dyn_slave

sigel_SOURCES = src/sigel.cpp $(top_srcdir)/kdesigel/supportingLibs/fparser/fparser.cpp
sigel_slave_SOURCES = src/sigel_slave.cpp $(top_srcdir)/kdesigel/supportingLibs/fparser/fparser.cpp $(top_srcdir)/kdesigel/src/SIGEL_GP/SIG_GPExperimentClean.cpp
manage_dyn_slave_SOURCES = src/manage_dyn_slave.c

if LINUX
ldflagsAdd = -L$(top_srcdir)/kdesigel/supportingLibs/pvm3/lib/LINUX \
-Wl,-rpath,/usr/lib,-rpath,/usr/X11R6/lib
endif

if DEBIAN
ldflagsAdd = -L$(top_srcdir)/kdesigel/supportingLibs/pvm3/lib/LINUX \
-Wl,-rpath,/usr/lib,-rpath,/usr/X11R6/lib
ldAdd=-lqt-gl
endif

if SOLARIS
ldflagsAdd = -L$(top_srcdir)/kdesigel/supportingLibs/pvm3/lib/SUN4SOL2 \
-Wl,-R/usr/lib,-R/usr/X11R6/lib,-lsocket
endif

LDFLAGS = -L/usr/X11R6/lib \
-L$(top_srcdir)/kdesigel/supportingLibs/SOLID-2.0/lib \
-L$(top_srcdir)/kdesigel/supportingLibs/qhull \
-L$(top_srcdir)/kdesigel/supportingLibs/newmat09 \
-L$(top_srcdir)/kdesigel/supportingLibs/Dynamo/Src/Lib \
-L$(top_srcdir)/kdesigel/supportingLibs/dynamechs/lib \
-L$(top_srcdir)/kdesigel/supportingLibs/cv97 \
-L$(QTDIR)/lib \
$(ldflagsAdd)

LDADD =	src/SIGEL_SlaveGUI/libSIGEL_SlaveGUI.a \
src/SIGEL_MasterGUI/libSIGEL_MasterGUI.a \
ui/SIGEL_MasterUI/libSIGEL_MasterUI.a \
ui/SIGEL_SlaveUI/libSIGEL_SlaveUI.a \
src/SIGEL_CommonGUI/libSIGEL_CommonGUI.a \
src/SIGEL_GP/libSIGEL_GP.a \
src/SIGEL_Environment/libSIGEL_Environment.a \
src/SIGEL_Program/libSIGEL_Program.a \
src/SIGEL_RealInterface/libSIGEL_RealInterface.a \
src/SIGEL_Robot/libSIGEL_Robot.a \
src/SIGEL_RobotIO/libSIGEL_RobotIO.a \
src/SIGEL_Simulation/libSIGEL_Simulation.a \
src/SIGEL_Tools/libSIGEL_Tools.a \
src/SIGEL_Visualisation/libSIGEL_Visualisation.a \
src/MT_Control/libMT_Control.a \
src/MT_GPSystem/libMT_GPSystem.a \
src/MT_GUI/libMT_GUI.a \
ui/MT_UI/libMT_UI.a \
-lqt-mt $(ldAdd)		\
-ldynalib -lsolid -lqhull -ldm -lnewmat -lpvm3 -lcv97 -lGLU -lGL -lXmu -lXext -lX11 -lm -lresolv -lpthread -lnsl -lSM -lICE -ldl
















