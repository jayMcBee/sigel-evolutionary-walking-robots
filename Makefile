# Builds SIGEL: first the vendored libraries, then SIGEL's own code.
#
#   make            sigel, sigel_slave and manage_dyn_slave, then sigelApp/,
#                   the folder SIGEL starts from: cd sigelApp && ./sigelLauncher
#   make coredrive  build/coredrive: one fitness evaluation, for the checks
#   make test       build the unit tests in sigel/tests/ and run them
#   make vendor     the vendored libraries except PVM
#   make pvm        libpvm3.a and pvmd3, built by PVM's own make
#   make pvm-link   build/pvm_link: SIGEL's PVM code against real PVM
#   make core       the SIGEL core modules only
#   make clean      remove build/ and PVM's build; sigelApp/ stays, and needs
#                   `make' again before SIGEL can start PVM
#   make unpatch    revert the vendored tree to the tarball contents
#
#   make B=build-asan ...   the same with the address and undefined-behaviour
#                           sanitizers, in build-asan/; only the checks use it
#   make CPPFLAGS=... ...   SIGEL's own code with extra flags such as
#                           -DSIG_DEBUG; see the README. Run `make clean' first,
#                           because make does not notice a change of flags.
#
# The vendored tree is not tracked: it comes from vendor/supportingLibs.tar.gz,
# pvm3/ from vendor/pvm3.4.6.tgz, and our changes are vendor/patches/, applied
# once against a stamp file. tar never deletes, so remove the tree, pvm3/
# included, before extracting again: extracting over it keeps the stamp and the
# old objects, and make finds nothing to do.
#
# Vendored C++ builds with -w -fpermissive; only newmat1.cpp and newmat9.cpp
# need -fpermissive. In build-asan it gets the sanitizers without three checks it
# fails by design: alignment and signed overflow in the f2c translation of
# LINPACK's ssvdc, and vptr in cv97's CLinkedList.

# A recipe that fails after writing its target must not leave it behind, or the
# next make takes it as up to date.
.DELETE_ON_ERROR:

SL   := downloads/supportingLibs
SHIM := shim
B    := build
# One spelling for each folder, so that the targets match the names in the .d
# files.
override B := $(patsubst $(CURDIR)/%,%,$(patsubst ./%,%,$(patsubst %/,%,$(B))))
LIB  := $(B)/lib
OBJ  := $(B)/obj

# The sanitizers are on in build-asan only, and only the checks build it.
ifeq ($(B),build-asan)
SAN  := -fsanitize=address,undefined -fno-omit-frame-pointer \
        -fno-sanitize=alignment,signed-integer-overflow,vptr
else
SAN  :=
endif
VCXX := g++ -std=c++17 -O1 -g -w -fpermissive $(SAN)
VCC  := gcc -std=gnu17 -O1 -g -w $(SAN)

STAMP := $(SL)/.sigel-patched
PATCHES := $(wildcard vendor/patches/*.patch)

# The supportingLibs archive carries PVM 3.4.3, which cannot build here, and
# extracting it over the tree brings 3.4.3 back without any error. So the
# version is checked on every make. clean and unpatch are exempt: they are the
# way out.
PVM_GUARD := $(filter-out clean unpatch,$(or $(MAKECMDGOALS),all))
ifneq ($(PVM_GUARD),)
ifneq ($(wildcard $(SL)/pvm3/include/pvm3.h),)
PVM_VERSION := $(shell sed -n 's/^\#define[[:space:]]*PVM_VER[[:space:]]*"\(.*\)"/\1/p' \
                       $(SL)/pvm3/include/pvm3.h)
ifneq ($(PVM_VERSION),3.4.6)
$(error vendored pvm3 is "$(PVM_VERSION)", expected 3.4.6 -- \
        rm -rf $(SL)/pvm3 && tar xzf vendor/pvm3.4.6.tgz -C $(SL) --strip-components=1 ./pvm3)
endif
endif
endif

VENDOR_LIBS := $(LIB)/libnewmat.a $(LIB)/libdm.a $(LIB)/libcv97.a \
               $(LIB)/libfparser.a

# PVM is built by its own make, inside its own tree: libpvm3 starts pvmd3 by
# path, under $PVM_ROOT/lib/$PVM_ARCH. Target `s' builds only what SIGEL links.
# LINUX64 is fixed because make reads it before the patches add aarch64.
PVM_DIR  := $(SL)/pvm3
PVM_LIB  := $(PVM_DIR)/lib/LINUX64/libpvm3.a
PVM_D    := $(PVM_DIR)/lib/LINUX64/pvmd3

.PHONY: all vendor core clean unpatch pvm pvm-link coredrive test
# sigelApp/ takes the plain build only, so a sanitized `make' does not fill it.
ifeq ($(B),build)
all: programs sigelApp
else
all: programs
endif
coredrive: $(B)/coredrive
vendor: $(VENDOR_LIBS)
pvm: $(PVM_LIB) $(PVM_D)
pvm-link: $(B)/pvm_link

# PVM's make builds both. On another architecture it builds into another
# folder and still succeeds, so the recipe checks lib/LINUX64.
$(PVM_LIB) $(PVM_D) &: $(STAMP)
	cd $(PVM_DIR) && PVM_ROOT=$$PWD $(MAKE) s
	@test -f $(PVM_LIB) && test -x $(PVM_D) || { \
	  echo "PVM built nothing in lib/LINUX64 -- this machine's" \
	       "lib/pvmgetarch says $$($(PVM_DIR)/lib/pvmgetarch)." >&2; \
	  exit 1; }

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

# Reverse order: two patches change pvm3/src/pvmd.c.
unpatch:
	@for p in $$(printf '%s\n' $(PATCHES) | tac); do patch -R -p1 -d $(SL) < $$p; done
	rm -f $(STAMP)

# PVM builds inside the vendored tree, so clean removes that too.
clean:
	rm -rf $(B)
	rm -rf $(PVM_DIR)/lib/LINUX64 $(PVM_DIR)/src/LINUX64

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

# DynaMechs. gldraw.cpp defines the draw() of every dm class, so it is linked,
# and with it -lGL -lGLU.
$(LIB)/libdm.a: INCS := -I$(SL)/dynamechs/dm -I$(SHIM)
$(LIB)/libdm.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,$(wildcard $(SL)/dynamechs/dm/*.cpp))

# CyberVRML97. Sample.cpp is a demo program with its own main().
$(LIB)/libcv97.a: INCS := -I$(SL)/cv97 -I$(SHIM)
$(LIB)/libcv97.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,\
  $(filter-out $(SL)/cv97/Sample.cpp,$(wildcard $(SL)/cv97/*.cpp)))

# fparser. The 2003 Makefile.am compiles this straight into each program.
$(LIB)/libfparser.a: INCS := -I$(SL)/fparser -I$(SHIM)
$(LIB)/libfparser.a: $(OBJ)/fparser/fparser.o

# ---------------------------------------------------------------------------
# SIGEL's own code: -Wall -Wextra, and in build-asan all the sanitizer checks.

SRC   := sigel
QTINC := $(shell qmake6 -query QT_INSTALL_HEADERS)
QTLIB := $(shell qmake6 -query QT_INSTALL_LIBS)
QTBIN := $(shell qmake6 -query QT_INSTALL_LIBEXECS)
MOC   := $(QTBIN)/moc
UIC   := $(QTBIN)/uic
RCC   := $(QTBIN)/rcc

ifeq ($(B),build-asan)
SIGSAN := -fsanitize=address,undefined -fno-omit-frame-pointer
else
SIGSAN :=
endif
# An MD5 over SIGEL's sources: the master refuses a fitness from a slave built
# from other source. The header is rewritten only when the hash changes, so
# `make -q' still finds an unchanged tree up to date.
SOURCE_HASH := $(shell find $(SRC)/src $(SRC)/include -type f -print0 | LC_ALL=C sort -z \
                 | xargs -0 md5sum | md5sum | cut -c1-32)
SOURCE_HASH_HDR := $(B)/gen/SIG_SourceHash.h
$(shell mkdir -p $(B)/gen; grep -qs '"$(SOURCE_HASH)"' $(SOURCE_HASH_HDR) \
  || echo '#define SIG_SOURCE_HASH "$(SOURCE_HASH)"' > $(SOURCE_HASH_HDR))

SIGINC := -Ishim -I$(SRC)/include -I$(B)/ui -I$(B)/gen -isystem $(QTINC) \
          $(addprefix -isystem $(QTINC)/,QtCore QtGui QtWidgets \
                                        QtOpenGL QtOpenGLWidgets) \
          $(addprefix -isystem $(SL)/,newmat09 dynamechs/dm \
                                      fparser cv97 pvm3/include)
# -fPIC, not -fPIE: with a Qt built with reduce_relocations, as Debian 12's is,
# -fPIE makes QApplication's constructor crash.
SIGCXX := g++ -std=c++20 -O1 -g -fPIC -Wall -Wextra -Wold-style-cast \
              -Wnon-virtual-dtor $(SIGSAN) $(CPPFLAGS)

CORE := SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program \
        SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP SIGEL_RobotCheck

CORE_LIBS := $(patsubst %,$(LIB)/lib%.a,$(CORE))
core: $(CORE_LIBS)

GUI := SIGEL_CommonGUI SIGEL_Visualisation SIGEL_SlaveGUI MT_GUI SIGEL_MasterGUI
GUI_LIBS := $(patsubst %,$(LIB)/lib%.a,$(GUI))
gui: $(GUI_LIBS)

# The slave links its own three GUI modules only: the master's would pull in
# MT_Controller.
GUI_SLAVE := SIGEL_CommonGUI SIGEL_Visualisation SIGEL_SlaveGUI
GUI_SLAVE_LIBS := $(patsubst %,$(LIB)/lib%.a,$(GUI_SLAVE))

# ---------------------------------------------------------------------------
# The Designer forms. uic makes ui_<Form>.h; the QWidget class of each form is
# a source file in its module, see SIG_GPParameterBase.h.
FORMS :=  MT_UI/MT_AddConstantsWidgetBase MT_UI/MT_AddIndividualsWidget \
            MT_UI/MT_EstimationWidgetBase MT_UI/MT_ExperimentWidgetBase \
            MT_UI/MT_IndividualWidgetBase MT_UI/MT_PopulationWidgetBase \
            MT_UI/MT_SearchWidgetBase MT_UI/MT_SelectionWidgetBase \
            MT_UI/MT_StatisticsWidgetBase \
            SIGEL_MasterUI/SIG_EditHostDialogBase \
            SIGEL_MasterUI/SIG_EnvironmentBase \
            SIGEL_MasterUI/SIG_ExperimentViewBase \
            SIGEL_MasterUI/SIG_GPParameterBase \
            SIGEL_MasterUI/SIG_IndividualListBase \
            SIGEL_MasterUI/SIG_IndividualViewBase \
            SIGEL_MasterUI/SIG_LanguageParametersBase \
            SIGEL_MasterUI/SIG_RobotBase \
            SIGEL_MasterUI/SIG_SimulationParameterBase \
            SIGEL_SlaveUI/SIG_MovieSettingsDialogBase \
            SIGEL_SlaveUI/SIG_SimulationWidgetBase
QRCS  :=  SIGEL_SlaveUI/SIG_SimulationWidgetBase

UI_HDRS  := $(patsubst %,$(B)/ui/ui_%.h,$(notdir $(FORMS)))
QRC_OBJS := $(patsubst %,$(OBJ)/qrc/%.o,$(notdir $(QRCS)))
QRC_SLAVE  := $(OBJ)/qrc/SIG_SimulationWidgetBase.o

# The images of the forms are compiled in with rcc. Their objects are named on
# the link line: in an archive nothing references them, and the linker would
# drop them. A .qrc that names a missing file is caught by check.sh, not here.
.PHONY: forms
forms: $(UI_HDRS) $(QRC_OBJS)

# One pattern rule cannot see the module subdirectory, so generate the rule.
define form_rule
$(B)/ui/ui_$(notdir $(1)).h: $(SRC)/ui/$(1).ui
	@mkdir -p $$(dir $$@)
	$(UIC) $$< -o $$@
endef
$(foreach f,$(FORMS),$(eval $(call form_rule,$(f))))

define qrc_rule
$(B)/qrc/qrc_$(notdir $(1)).cpp: $(SRC)/ui/$(1).qrc
	@mkdir -p $$(dir $$@)
	$(RCC) --name $(notdir $(1)) $$< -o $$@
endef
$(foreach q,$(QRCS),$(eval $(call qrc_rule,$(q))))

$(OBJ)/qrc/%.o: $(B)/qrc/qrc_%.cpp
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(SIGINC) -c $< -o $@

# The patched vendored headers and the form headers must exist first; the .d
# files track later changes.
$(OBJ)/sigel/%.o: $(SRC)/src/%.cpp $(STAMP) | $(UI_HDRS)
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(SIGINC) -c $< -o $@

# The Q_OBJECT headers, found in the source, so that none is missed.
MOC_HDRS := $(patsubst $(SRC)/include/%,%,$(shell grep -rl Q_OBJECT \
              $(addprefix $(SRC)/include/,$(CORE) $(GUI)) 2>/dev/null | sort))
MOC_OBJS := $(patsubst %.h,$(OBJ)/moc/%.o,$(MOC_HDRS))
# The slave, coredrive, sigel_tests and pvm_link get no MT_Control meta-object:
# the one of MT_Controller pulls in the master GUI. coredrive, sigel_tests and
# pvm_link link no GUI module, so they get no GUI meta-objects either.
MOC_OBJS_CORE  := $(patsubst %.h,$(OBJ)/moc/%.o,\
                    $(filter $(addsuffix /%,$(filter-out MT_Control,$(CORE))),$(MOC_HDRS)))

SLAVE_MODULES  := $(filter-out MT_Control,$(CORE)) $(GUI_SLAVE)
MOC_OBJS_SLAVE := $(patsubst %.h,$(OBJ)/moc/%.o,\
                    $(filter $(addsuffix /%,$(SLAVE_MODULES)),$(MOC_HDRS)))

# Keep the generated moc files; otherwise every make generates them again and
# `make -q' fails.
.SECONDARY: $(patsubst %.h,$(B)/moc/%.cpp,$(MOC_HDRS))

$(B)/moc/%.cpp: $(SRC)/include/%.h
	@mkdir -p $(dir $@)
	$(MOC) $(subst -isystem ,-I,$(SIGINC)) $< -o $@   # moc rejects -isystem

$(OBJ)/moc/%.o: $(B)/moc/%.cpp $(STAMP) | $(UI_HDRS)
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(SIGINC) -c $< -o $@

define core_lib
$(LIB)/lib$(1).a: $$(patsubst $(SRC)/src/%.cpp,$(OBJ)/sigel/%.o,\
  $$(wildcard $(SRC)/src/$(1)/*.cpp))
endef
$(foreach m,$(CORE),$(eval $(call core_lib,$(m))))
$(foreach m,$(GUI),$(eval $(call core_lib,$(m))))

# SIG_MainWindow.cpp puts the build time in the window title, so it is compiled
# again whenever another SIGEL object is. A target that always rebuilds would
# make every `make -q' fail.
TITLE_OBJ := $(OBJ)/sigel/SIGEL_MasterGUI/SIG_MainWindow.o
$(TITLE_OBJ): $(filter-out $(TITLE_OBJ),$(patsubst $(SRC)/src/%.cpp,$(OBJ)/sigel/%.o,\
  $(foreach m,$(CORE) $(GUI),$(wildcard $(SRC)/src/$(m)/*.cpp))))

# SIG_GPExperiment is defined twice: SIG_GPExperiment.cpp for sigel and
# guidrive, and SIG_GPExperimentClean.cpp, which builds no MT_Controller, for the
# slave, coredrive and the tests. Both are in libSIGEL_GP.a, and the linker takes
# the first one it meets. So each program names its object, and the recipes of
# sigel, guidrive, sigel_slave and coredrive check by the size of the
# constructor that the right one was linked.
CLEAN_OBJ := $(OBJ)/sigel/SIGEL_GP/SIG_GPExperimentClean.o
MASTER_OBJ := $(OBJ)/sigel/SIGEL_GP/SIG_GPExperiment.o

ctor_size = nm -C -S $(1) | awk '/SIG_GPExperiment::SIG_GPExperiment\(\)$$/{print $$2; exit}'

# One fitness evaluation. --start-group because the core modules call each
# other in a cycle.
$(B)/coredrive: checks/programs/coredrive.cpp $(MOC_OBJS_CORE) $(CLEAN_OBJ) $(CORE_LIBS) $(VENDOR_LIBS)
	$(SIGCXX) $(SIGINC) $< $(MOC_OBJS_CORE) $(CLEAN_OBJ) -o $@ \
	  -Wl,--start-group $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group \
	  -L$(QTLIB) -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui -lQt6Core -lGL -lGLU -lm
	@want=`$(call ctor_size,$(CLEAN_OBJ))`; got=`$(call ctor_size,$@)`; \
	 test -n "$$want" && test "$$got" = "$$want" || { \
	   echo "coredrive linked the WRONG SIG_GPExperiment: constructor is $$got," \
	        "Clean's is $$want -- see PORTING.md section 9." >&2; exit 1; }

# The unit tests, with Qt Test, linked like coredrive. Each test class has a
# Q_OBJECT header. main.cpp names the classes by hand, so `make test' stops if
# it misses one: that class would be built and never run.
test: $(B)/sigel_tests
	@for h in $(TEST_HDRS); do c=`basename $$h .h`; \
	   grep -q "push_back( std::make_unique<\(.*::\)\{0,1\}$$c>() )" $(SRC)/tests/main.cpp || { \
	     echo "$(SRC)/tests/main.cpp does not run the test class $$c" >&2; exit 1; }; done
	$(B)/sigel_tests

TESTINC   := $(SIGINC) -I$(SRC)/tests -isystem $(QTINC)/QtTest
TEST_SRCS := $(SRC)/tests/main.cpp $(sort $(wildcard $(SRC)/tests/*/*.cpp))
TEST_HDRS := $(sort $(wildcard $(SRC)/tests/*/*.h))
TEST_OBJS := $(patsubst $(SRC)/tests/%.cpp,$(OBJ)/tests/%.o,$(TEST_SRCS)) \
             $(patsubst $(SRC)/tests/%.h,$(OBJ)/tests/%.moc.o,$(TEST_HDRS))

# Kept for the reason given at the other moc files above.
.SECONDARY: $(patsubst $(SRC)/tests/%.h,$(B)/moc/tests/%.cpp,$(TEST_HDRS))

$(B)/moc/tests/%.cpp: $(SRC)/tests/%.h
	@mkdir -p $(dir $@)
	$(MOC) $(subst -isystem ,-I,$(TESTINC)) $< -o $@   # moc rejects -isystem

$(OBJ)/tests/%.moc.o: $(B)/moc/tests/%.cpp $(STAMP) | $(UI_HDRS)
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(TESTINC) -c $< -o $@

$(OBJ)/tests/%.o: $(SRC)/tests/%.cpp $(STAMP) | $(UI_HDRS)
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(TESTINC) -c $< -o $@

$(B)/sigel_tests: $(TEST_OBJS) $(MOC_OBJS_CORE) $(CLEAN_OBJ) $(CORE_LIBS) $(VENDOR_LIBS)
	$(SIGCXX) $(TEST_OBJS) $(MOC_OBJS_CORE) $(CLEAN_OBJ) -o $@ \
	  -Wl,--start-group $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group \
	  -L$(QTLIB) -lQt6Test -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui -lQt6Core -lGL -lGLU -lm

# ---------------------------------------------------------------------------
# pvm_link: SIGEL's PVM code against real PVM; checks/pvm-check.sh runs it.
# PVM_OBJS are the core objects that call pvm_*, named so that the linker takes
# them. The recipe fails if another core object calls pvm_*, except
# MT_Controller.o, whose one pvm_halt() would pull in the master GUI.
# -ltirpc is needed under the sanitizers: without it, the weak xdr_* functions
# of libasan take PVM's XDR calls, with nothing behind them.
PVM_OBJS := $(OBJ)/sigel/SIGEL_GP/SIG_GPFitnessTrainer.o \
            $(OBJ)/sigel/SIGEL_GP/SIG_GPPVMData.o

$(B)/pvm_link: checks/programs/pvm_link.cpp $(PVM_OBJS) $(MOC_OBJS_CORE) $(CORE_LIBS) $(VENDOR_LIBS) \
               $(PVM_LIB) $(PVM_D)
	$(SIGCXX) $(SIGINC) $< $(PVM_OBJS) $(MOC_OBJS_CORE) -o $@ \
	  -Wl,--start-group $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group \
	  $(PVM_LIB) -ltirpc \
	  -L$(QTLIB) -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui -lQt6Core -lGL -lGLU -lm
	@bad=`nm --undefined-only --print-file-name $(CORE_LIBS) 2>/dev/null \
	      | sed -n 's/.*:\(.*\.o\): *U pvm_.*/\1/p' | sort -u \
	      | grep -v -x -e SIG_GPFitnessTrainer.o -e SIG_GPPVMData.o \
	                 -e MT_Controller.o`; \
	  test -z "$$bad" || { \
	    echo "PVM_OBJS is out of date: these also need pvm_* and are not" \
	         "on the link line, so P4 no longer covers them:" >&2; \
	    echo "$$bad" >&2; exit 1; }

# ---------------------------------------------------------------------------
# sigel, sigel_slave and manage_dyn_slave.
.PHONY: programs
programs: $(B)/sigel $(B)/sigel_slave $(B)/manage_dyn_slave

SIGLIBS = $(PVM_LIB) -ltirpc \
          -L$(QTLIB) -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui -lQt6Core \
          -lGL -lGLU -lm

$(B)/sigel: $(SRC)/src/sigel.cpp $(MOC_OBJS) $(QRC_OBJS) $(GUI_LIBS) $(CORE_LIBS) \
            $(VENDOR_LIBS) $(PVM_LIB)
	$(SIGCXX) $(SIGINC) $< $(MOC_OBJS) $(MASTER_OBJ) -o $@ \
	  -Wl,--start-group $(GUI_LIBS) $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group $(SIGLIBS)
	@want=`$(call ctor_size,$(MASTER_OBJ))`; got=`$(call ctor_size,$@)`; \
	 test -n "$$want" && test "$$got" = "$$want" || { \
	   echo "sigel linked the WRONG SIG_GPExperiment: constructor is $$got," \
	        "the master's is $$want. Clean leaves mtController uninitialised" \
	        "-- see PORTING.md section 9." >&2; exit 1; }

$(B)/sigel_slave: $(SRC)/src/sigel_slave.cpp $(MOC_OBJS_SLAVE) $(QRC_SLAVE) $(CLEAN_OBJ) \
                  $(GUI_SLAVE_LIBS) $(CORE_LIBS) $(VENDOR_LIBS) $(PVM_LIB)
	$(SIGCXX) $(SIGINC) $< $(MOC_OBJS_SLAVE) $(QRC_SLAVE) $(CLEAN_OBJ) -o $@ \
	  -Wl,--start-group $(GUI_SLAVE_LIBS) $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group $(SIGLIBS)
	@want=`$(call ctor_size,$(CLEAN_OBJ))`; got=`$(call ctor_size,$@)`; \
	 test -n "$$want" && test "$$got" = "$$want" || { \
	   echo "sigel_slave linked the WRONG SIG_GPExperiment: constructor is $$got," \
	        "Clean's is $$want -- see PORTING.md section 9." >&2; exit 1; }

# manage_dyn_slave is C and needs only the C library. -std=gnu17, not c17: the
# strict modes hide gethostname() and the member h_addr of struct hostent.
$(B)/manage_dyn_slave: $(SRC)/src/manage_dyn_slave.c
	@mkdir -p $(dir $@)
	gcc -std=gnu17 -O1 -g -Wall -Wextra $(SIGSAN) $< -o $@

# ---------------------------------------------------------------------------
# sigelApp/, the folder SIGEL starts from: the three programs, the scripts,
# stdConf.mt, pixmaps/, textures/ and a link to PVM. The data are copies, not
# links: SIGEL writes in the folder it runs from, and the tracked files must not
# change. cp -p leaves alone a copy that SIGEL has rewritten since. The programs come from build/ through a temporary
# name, so a running copy is not overwritten. `make clean' leaves this folder.
APP      := sigelApp
APP_DATA := sigelLauncher povrayLauncher sigelDynClient stdConf.mt \
            $(patsubst $(SRC)/%,%,$(wildcard $(SRC)/pixmaps/* $(SRC)/textures/*))

.PHONY: sigelApp
sigelApp: $(APP)/sigel $(APP)/sigel_slave $(APP)/manage_dyn_slave \
          $(addprefix $(APP)/,$(APP_DATA)) $(APP)/supportingLibs/pvm3

$(APP)/sigel $(APP)/sigel_slave $(APP)/manage_dyn_slave: $(APP)/%: build/%
	@mkdir -p $(dir $@)
	cp $< $@.tmp && mv -f $@.tmp $@

$(addprefix $(APP)/,$(APP_DATA)): $(APP)/%: $(SRC)/%
	@mkdir -p $(dir $@)
	cp -p $< $@

# PVM stays where it was built; the launcher's PVM_ROOT reaches it through this.
$(APP)/supportingLibs/pvm3: | $(PVM_LIB)
	@mkdir -p $(dir $@)
	ln -sfn ../../$(PVM_DIR) $@

# The GUI behaviour driver for check.sh. It links like sigel, with Qt Test.
.PHONY: guidrive
guidrive: $(B)/guidrive

$(B)/guidrive: checks/programs/guidrive.cpp $(MOC_OBJS) $(QRC_OBJS) $(GUI_LIBS) $(CORE_LIBS) \
               $(VENDOR_LIBS) $(PVM_LIB)
	$(SIGCXX) -DQT_CORE_LIB -DQT_GUI_LIB -DQT_WIDGETS_LIB -DQT_TESTLIB_LIB \
	  $(SIGINC) -isystem $(QTINC)/QtTest $< $(MOC_OBJS) $(QRC_SLAVE) $(MASTER_OBJ) -o $@ \
	  -Wl,--start-group $(GUI_LIBS) $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group \
	  -lQt6Test $(SIGLIBS)
	@want=`$(call ctor_size,$(MASTER_OBJ))`; got=`$(call ctor_size,$@)`; \
	 test -n "$$want" && test "$$got" = "$$want" || { \
	   echo "guidrive linked the WRONG SIG_GPExperiment: constructor is $$got," \
	        "the master's is $$want -- see PORTING.md section 9." >&2; exit 1; }

-include $(shell find $(OBJ) -name '*.d' 2>/dev/null)
