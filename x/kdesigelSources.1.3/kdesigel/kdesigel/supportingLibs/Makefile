.PHONY: pvm3 qt

ifeq ($(findstring linux,$(OSTYPE)),linux)
export PVM_ROOT=$(PWD)/pvm3
#PVM_ARCHDIR := LINUX
endif

ifeq ($(OSTYPE),solaris)
PVM_ROOT=$(PWD)/kdesigel/supportingLibs/pvm3
export PVM_ROOT
CC = gcc
#PVM_ARCHDIR := SUN4SOL2
X11_DIR := /usr/openwin
RUNTIME_LIB_DIR_FLAG := -R
LIBSOCKET_FLAG := -lsocket
endif


all: qhull/libqhull.a  \
newmat09/libnewmat.a \
cv97/libcv97.a \
Dynamo/Src/Lib/libdynalib.a \
dynamechs/lib/libdm.a \
SOLID-2.0/lib/libsolid.a \
pvm3
#qt

distdir:

clean distclean:
	-cd Dynamo/Src/Lib; \
	rm -f *.a;
	-cd Dynamo/Src/Obj; \
	rm -f *.o;
	-cd qhull; \
	make cleanall;
	-cd newmat09; \
	rm -f *.o *.a;
	-cd cv97; \
	rm -f *.o *.a;
	-cd dynamechs/obj; \
	rm -f *.o;
	-cd dynamechs/lib; \
	rm -f *.a;
	-cd pvm3; \
	make clean;
	-cd SOLID-2.0; \
	make distclean;




##### build qhull

qhull/libqhull.a:
	cd qhull; \
	make;


##### build newmat; Matrixoperationen

NEWMAT_OBJECTS = newmat09/bandmat.o \
		 newmat09/cholesky.o \
		 newmat09/evalue.o \
		 newmat09/fft.o \
		 newmat09/hholder.o \
		 newmat09/jacobi.o \
		 newmat09/myexcept.o \
		 newmat09/newmat1.o \
		 newmat09/newmat2.o \
		 newmat09/newmat3.o \
		 newmat09/newmat4.o \
		 newmat09/newmat5.o \
		 newmat09/newmat6.o \
		 newmat09/newmat7.o \
		 newmat09/newmat8.o \
		 newmat09/newmat9.o \
		 newmat09/newmatex.o \
		 newmat09/newmatnl.o \
		 newmat09/newmatrm.o \
		 newmat09/sort.o \
		 newmat09/solution.o \
		 newmat09/submat.o \
		 newmat09/svd.o

newmat09/libnewmat.a: $(NEWMAT_OBJECTS)
	$(AR) r newmat09/libnewmat.a $(NEWMAT_OBJECTS);

$(NEWMAT_OBJECTS) $(CV97_OBJECTS): %.o: %.cpp
	$(CC) -fpermissive -O2 -I./newmat09 -c $< -o $@;


##### build cv97; Bibliothek zum benutzen von VRML-Dateien


CV97_SOURCES = $(wildcard cv97/*.cpp)
CV97_OBJECTS = $(CV97_SOURCES:.cpp=.o)
#PVM_ROOT := $(PWD)/kdesigel/supportingLibs/pvm3

$(CV97_OBJECTS): %.o: %.cpp
	$(CC) -fpermissive -O2 -I./cv97 -c $< -o $@;

cv97/libcv97.a: $(CV97_OBJECTS)
	$(AR) r cv97/libcv97.a $(CV97_OBJECTS);


##### build dynamo; Bibliothek zur Dynamiksimulation

Dynamo/Src/Lib/libdynalib.a:
	cd Dynamo/Src/Cpp; \
	make;


##### build dynamechs; ebenfalls eine Bibliothek zur Dynamiksimualtion

DM_SOURCES = $(wildcard dynamechs/dm/*.cpp)
DM_OBJECTS = $(patsubst dynamechs/dm/%.cpp,dynamechs/obj/%.o,$(DM_SOURCES))

$(DM_OBJECTS): dynamechs/obj/%.o: dynamechs/dm/%.cpp
	$(CC) -fpermissive -O2 -I./dynamechs/dm -c $< -o $@;

dynamechs/lib/libdm.a: $(DM_OBJECTS)
	$(AR) r dynamechs/lib/libdm.a $(DM_OBJECTS);


##### build solid; Bibliothek fuer die Erkennung von Kollisionen

SOLID-2.0/lib/libsolid.a:
	cd SOLID-2.0; \
	make;


##### build pvm3; heterogene (Unix-) Computer als einen Parallelrechner verwenden
pvm3:
	cd pvm3; \
	export PVM_ROOT=$(PWD)/pvm3; \
	make;


##### build qt; gui
qt:
ifeq ($(QTDIR),)
	cd qt; \
	export QTDIR=$(PWD)/qt;\
	echo yes | ./configure -thread -static -no-xft -platform linux-g++; \
	make;
endif
