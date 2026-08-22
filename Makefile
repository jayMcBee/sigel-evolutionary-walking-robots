# Build for the Qt 6 port -- PORTING.md §3.
#
# Two halves: the vendored third-party libraries, then SIGEL's own code and a
# program that runs one fitness evaluation.
#
#   make            build/sigel_eval -- one fitness evaluation
#   make vendor     the seven vendored libraries only
#   make core       the nine SIGEL core modules only
#   make clean      remove build/
#   make unpatch    revert the vendored tree to the tarball contents
#
# make B=build-fast SAN= SIGSAN=   the same thing without the sanitizers, in a
# separate directory. One evaluation is 0.2 s either way.
#
# The vendored tree is not tracked (it comes out of supportingLibs.tar.gz), so
# the three edits gcc 15 needs live in patches/ and are applied here against a
# stamp file inside that tree. tar does not delete files it does not carry, so
# re-extracting the tarball over the tree leaves the stamp behind: rm -rf the
# vendored tree first, or the build silently keeps objects built from patched
# sources.
#
# Vendored code is built with -w -fpermissive, which SIGEL's own code does not
# get: check.sh already treats these headers as -isystem for the same reason.
# -fpermissive covers exactly newmat1.cpp and newmat9.cpp, which pass a string
# literal to char* and a long to ios_base::fmtflags. Dynamo/containerlist.h was
# a third -- a missing #include <cstddef> that -fpermissive was hiding, so it is
# a patch instead.
#
# The vendored libraries get UndefinedBehaviorSanitizer too, minus three checks
# they trip by construction: alignment and signed overflow throughout qhull and
# the f2c translation of LINPACK's ssvdc, and vptr in cv97, whose CLinkedList
# header node is a bare CLinkedListNode<T> that CLinkedList.h:37 downcasts to T.
# SIGEL's own code gets the full set.

SL   := x/supportingLibs/supportingLibs
SHIM := shim
B    := build
LIB  := $(B)/lib
OBJ  := $(B)/obj

SAN  := -fsanitize=address,undefined -fno-omit-frame-pointer \
        -fno-sanitize=alignment,signed-integer-overflow,vptr
VCXX := g++ -std=c++17 -O1 -g -w -fpermissive -DMINMAX_H $(SAN)
VCC  := gcc -std=gnu17 -O1 -g -w $(SAN)

STAMP := $(SL)/.sigel-patched
PATCHES := $(wildcard patches/*.patch)

VENDOR_LIBS := $(LIB)/libnewmat.a $(LIB)/libdm.a $(LIB)/libcv97.a \
               $(LIB)/libdynalib.a $(LIB)/libsolid.a $(LIB)/libqhull.a \
               $(LIB)/libfparser.a

.PHONY: all vendor core clean unpatch
all: $(B)/sigel_eval
vendor: $(VENDOR_LIBS)

# A patch that reverse-applies cleanly is already in the tree; skip it. Anything
# else that will not apply is a real error and stops the build.
$(STAMP): $(PATCHES)
	@for p in $(PATCHES); do \
	  if patch -p1 -R -f -s --dry-run -d $(SL) < $$p >/dev/null 2>&1; then \
	    echo "  already applied  $$p"; \
	  else \
	    echo "  patch            $$p"; patch -N -p1 -d $(SL) < $$p || exit 1; \
	  fi; \
	done
	@touch $@

unpatch:
	@for p in $(PATCHES); do patch -R -p1 -d $(SL) < $$p; done
	rm -f $(STAMP)

clean:
	rm -rf $(B)

$(OBJ)/%.o: $(SL)/%.cpp $(STAMP)
	@mkdir -p $(dir $@)
	$(VCXX) -MMD -MP $(INCS) -c $< -o $@

$(OBJ)/%.o: $(SL)/%.c $(STAMP)
	@mkdir -p $(dir $@)
	$(VCC) -MMD -MP $(INCS) -c $< -o $@

$(LIB)/lib%.a:
	@mkdir -p $(LIB)
	@$(RM) $@          # ar replaces members but never removes them
	$(AR) crs $@ $^

# newmat09 -- the object list its own gnu.mak uses for libnewmat.a; the rest of
# the directory is the test and example programs.
newmat_SRC := bandmat cholesky evalue fft hholder jacobi myexcept newmat1 \
  newmat2 newmat3 newmat4 newmat5 newmat6 newmat7 newmat8 newmat9 newmatex \
  newmatnl newmatrm solution sort submat svd
$(LIB)/libnewmat.a: INCS := -I$(SL)/newmat09 -I$(SHIM)
$(LIB)/libnewmat.a: $(patsubst %,$(OBJ)/newmat09/%.o,$(newmat_SRC))

# DynaMechs. gldraw.cpp is the OpenGL renderer. It cannot be left out: it
# defines the virtual draw() of every dm* class, so their vtables reference it
# and the linker pulls it in. Hence -lGL -lGLU below, which is never called.
$(LIB)/libdm.a: INCS := -I$(SL)/dynamechs/dm -I$(SHIM)
$(LIB)/libdm.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,$(wildcard $(SL)/dynamechs/dm/*.cpp))

# CyberVRML97. Sample.cpp is a demo program with its own main().
$(LIB)/libcv97.a: INCS := -I$(SL)/cv97 -I$(SHIM)
$(LIB)/libcv97.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,\
  $(filter-out $(SL)/cv97/Sample.cpp,$(wildcard $(SL)/cv97/*.cpp)))

# Dynamo. -ldynalib is the name the 2003 link line uses.
$(LIB)/libdynalib.a: INCS := -I$(SL)/Dynamo/Src/Inc -I$(SHIM)
$(LIB)/libdynalib.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,$(wildcard $(SL)/Dynamo/Src/Cpp/*.cpp))

# SOLID, with the qhull convex hull path its Make-config recommends.
$(LIB)/libsolid.a: INCS := -I$(SL)/SOLID-2.0/include -I$(SL)/SOLID-2.0/src \
                           -I$(SL)/qhull -I$(SHIM) -DQHULL
$(LIB)/libsolid.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,$(wildcard $(SL)/SOLID-2.0/src/*.cpp))

# qhull. unix.c, rbox.c and the two user_eg*.c are programs, not library code.
qhull_SRC := geom geom2 global io mem merge poly poly2 qhull qset stat user
$(LIB)/libqhull.a: INCS := -I$(SL)/qhull
$(LIB)/libqhull.a: $(patsubst %,$(OBJ)/qhull/%.o,$(qhull_SRC))

# fparser. The 2003 Makefile.am compiles this straight into each program.
$(LIB)/libfparser.a: INCS := -I$(SL)/fparser -I$(SHIM)
$(LIB)/libfparser.a: $(OBJ)/fparser/fparser.o

# ---------------------------------------------------------------------------
# SIGEL's own code, §7 Phase R.
#
# QtWidgets is on the include path and the link line. Nothing here opens a
# window: SIG_GPPopulation is the one core file that names a widget, and it
# guards every use with if (qApp), which is null without a QApplication.
#
# These get -Wall -Wextra and no -fpermissive, unlike the vendored code, and
# UndefinedBehaviorSanitizer as well as AddressSanitizer.

SRC   := x/kdesigelSources.1.3/kdesigel/kdesigel
QTINC := $(shell qmake6 -query QT_INSTALL_HEADERS)
QTLIB := $(shell qmake6 -query QT_INSTALL_LIBS)
MOC   := $(shell qmake6 -query QT_INSTALL_LIBEXECS)/moc

SIGSAN := -fsanitize=address,undefined -fno-omit-frame-pointer
SIGINC := -Ishim -I$(SRC)/include -isystem $(QTINC) \
          $(addprefix -isystem $(QTINC)/,QtCore QtGui QtWidgets) \
          $(addprefix -isystem $(SL)/,newmat09 dynamechs/dm Dynamo/Src/Inc \
                                      fparser cv97 SOLID-2.0/include pvm3/include)
SIGCXX := g++ -std=c++17 -O1 -g -Wall -Wextra \
              -DMINMAX_H $(SIGSAN)

CORE := SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program \
        SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP

# MT_Controller.cpp constructs an MT_MainWindow and makes 23 mainWindow->
# accesses, so it cannot build until MT_GUI is ported -- PORTING.md §9.
# SIG_GUIGPManager.cpp is its counterpart in SIGEL_GP.
EXCLUDE_MT_Control := $(SRC)/src/MT_Control/MT_Controller.cpp
# The two ZORC files drive real hardware over a serial line and ask the user for
# the distance walked through QInputDialog. Both still on the Qt 2 API.
EXCLUDE_SIGEL_GP   := $(SRC)/src/SIGEL_GP/SIG_GUIGPManager.cpp \
                      $(SRC)/src/SIGEL_GP/SIG_GPRemoteZORCFitnessFunction.cpp \
                      $(SRC)/src/SIGEL_GP/WIN_SIG_GPRemoteZORCFitnessFunction.cpp

CORE_LIBS := $(patsubst %,$(LIB)/lib%.a,$(CORE))
core: $(CORE_LIBS)

$(OBJ)/sigel/%.o: $(SRC)/src/%.cpp $(STAMP)
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(SIGINC) -c $< -o $@

# The four Q_OBJECT classes in core. Their vtable and typeinfo live in the
# generated code, so without these the link fails on SIG_Simulation and
# SIG_DynaSystem. MT_Controller.h is here for completeness -- its .cpp is
# excluded above, so nothing references its vtable yet.
MOC_HDRS := MT_GPSystem/MT_GPManager.h SIGEL_Simulation/SIG_DynaSystem.h \
            SIGEL_Simulation/SIG_Simulation.h
MOC_OBJS := $(patsubst %.h,$(OBJ)/moc/%.o,$(MOC_HDRS))

$(B)/moc/%.cpp: $(SRC)/include/%.h
	@mkdir -p $(dir $@)
	$(MOC) $(subst -isystem ,-I,$(SIGINC)) $< -o $@   # moc rejects -isystem

$(OBJ)/moc/%.o: $(B)/moc/%.cpp
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(SIGINC) -c $< -o $@

define core_lib
$(LIB)/lib$(1).a: $$(patsubst $(SRC)/src/%.cpp,$(OBJ)/sigel/%.o,\
  $$(filter-out $$(EXCLUDE_$(1)),$$(wildcard $(SRC)/src/$(1)/*.cpp)))
endef
$(foreach m,$(CORE),$(eval $(call core_lib,$(m))))

# One fitness evaluation. --start-group because the core modules have cycles:
# SIGEL_GP calls SIGEL_Simulation, which reaches back through SIG_Robot.
$(B)/sigel_eval: sigel_eval.cpp $(MOC_OBJS) $(CORE_LIBS) $(VENDOR_LIBS)
	$(SIGCXX) $(SIGINC) $< $(MOC_OBJS) -o $@ \
	  -Wl,--start-group $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group \
	  -L$(QTLIB) -lQt6Widgets -lQt6Gui -lQt6Core -lGL -lm

-include $(shell find $(OBJ) -name '*.d' 2>/dev/null)
