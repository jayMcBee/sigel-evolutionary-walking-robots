# Build for the Qt 6 port -- PORTING.md §3.
#
# Two halves: the vendored third-party libraries, then SIGEL's own code and a
# program that runs one fitness evaluation.
#
#   make            build/sigel_eval -- one fitness evaluation
#   make vendor     the five vendored libraries only
#   make pvm        libpvm3.a and pvmd3, built by PVM's own make
#   make pvm-link   build/pvm_link -- SIGEL's PVM code against real PVM
#   make core       the nine SIGEL core modules only
#   make clean      remove build/ and PVM's products
#   make unpatch    revert the vendored tree to the tarball contents
#
# make B=build-fast SAN= SIGSAN=   the same thing without the sanitizers, in a
# separate directory. One evaluation is 0.2 s either way.
#
# The vendored tree is not tracked (it comes out of
# vendor/supportingLibs.tar.gz), so our edits to it live in vendor/patches/ and
# are applied here against a stamp file inside that tree. tar does not delete
# files it does not carry, so re-extracting the tarball over the tree leaves the
# stamp behind: rm -rf the vendored tree first, or the build silently keeps
# objects built from patched sources.
#
# ONE EXCEPTION: pvm3/ no longer comes from vendor/supportingLibs.tar.gz. It is
# upstream PVM 3.4.6 out of vendor/pvm3.4.6.tgz, and that tarball still
# carries 3.4.3. So after any rm -rf of the vendored tree, restore pvm3/ from
# vendor/pvm3.4.6.tgz as well -- the guard below tells you so if you forget.
#
# ALWAYS rm -rf pvm3/ before re-extracting, never extract over it. PVM builds
# in its own tree, and vendor/pvm3.4.6.tgz carries 1997-2007 mtimes -- older
# than any object. Extracting over the top restores vanilla sources, leaves
# lib/LINUX64 and src/LINUX64 in place, and `make pvm' then says "Nothing to be
# done" while libpvm3.a still holds objects built from patched sources that are
# no longer there. The version guard cannot see this: the header still reads
# 3.4.6.
#
# vendor/patches/pvm3-*.patch are the nine PVM gets here: one with the four
# config lines it needs to build, and every Debian source patch that touches one
# of the 28 objects PVM compiles. They go through the same stamp as the rest, so
# nothing below needed changing to pick them up.
#
# Vendored code is built with -w -fpermissive, which SIGEL's own code does not
# get: check.sh already treats these headers as -isystem for the same reason.
# -fpermissive covers exactly newmat1.cpp and newmat9.cpp, which pass a string
# literal to char* and a long to ios_base::fmtflags.
#
# The vendored libraries get UndefinedBehaviorSanitizer too, minus three checks
# they trip by construction: alignment and signed overflow throughout qhull and
# the f2c translation of LINPACK's ssvdc, and vptr in cv97, whose CLinkedList
# header node is a bare CLinkedListNode<T> that CLinkedList.h:37 downcasts to T.
# SIGEL's own code gets the full set.

# A recipe that fails AFTER creating its target leaves that target on disk,
# newer than its prerequisites -- so the next make says "up to date" and the
# check never runs again. sigel_eval's SIG_GPExperiment assertion is exactly
# that shape. Found by the C5 review.
.DELETE_ON_ERROR:

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
PATCHES := $(wildcard vendor/patches/*.patch)

# PVM: the vendored 3.4.3 was replaced by upstream 3.4.6 -- PORTING.md Phase P.
# 3.4.3 has no conf/LINUX64.def at all, so it cannot describe this machine.
# (It has no aarch64 in lib/pvmgetarch either, but neither has 3.4.6 -- that
# line is one of the four config lines Phase P adds, not something the version
# bump supplies.)
#
# Re-extract vendor/supportingLibs.tar.gz over the tree and 3.4.3 comes back:
# tar overwrites but never deletes, the paths and the file count both still look
# right, and nothing errors. So check the version on every make rather than in
# the patch stamp -- the stamp file survives exactly this accident, which is
# the same trap the header above warns about for the patches.
#
# An absent pvm3/include/pvm3.h is somebody who has not extracted the vendored
# tree at all; that is not this accident, so it passes and the build says so on
# its own. A header that is present but unparseable IS a fault and stops here.
#
# clean and unpatch are exempt. They are how you recover from the accident this
# guard reports, and refusing to run them until PVM is fixed by hand is the
# wrong way round.
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

# SOLID and qhull went with the Dynamo backend on 2026-08-28
# (physics_backends.md). SOLID was the collision library the Dynamo path used;
# qhull was built only to give SOLID its convex hulls, which is what -DQHULL
# selected. Measured before removing: zero dt* API calls left anywhere in
# SIGEL, zero in vendored DynaMechs, and zero qh_* references outside qhull
# itself. The only surviving "SOLID" in the tree is the maximalSOLIDIterations
# parameter, a number still parsed and written but read by nothing -- dead the
# same way six other simulation parameters now are.
VENDOR_LIBS := $(LIB)/libnewmat.a $(LIB)/libdm.a $(LIB)/libcv97.a \
               $(LIB)/libdynalib.a $(LIB)/libfparser.a

# PVM is the one vendored library we do NOT compile ourselves -- PORTING.md
# Phase P. Two reasons. Its own build already works once patched, so an object
# list here would be a reimplementation with nothing to gain; and pvmd3 is a
# daemon that libpvm3 starts by path, looking under $PVM_ROOT/lib/$PVM_ARCH,
# so the products have to sit in that layout inside the vendored tree rather
# than in build/ with everything else.
#
# The `s' target builds src only. PVM's `default' would also build the console
# (which wants readline), libfpvm and libgpvm3; SIGEL's 2003 link line names
# -lpvm3 and nothing else, so none of those is built.
#
# LINUX64 is hardcoded rather than read from lib/pvmgetarch, because a target
# name is expanded when this file is read -- before vendor/patches/ has been
# applied, and the aarch64 line is one of those patches. ia64, x86_64 and
# aarch64 all map to LINUX64 (lib/pvmgetarch:71-74).
#
# On any OTHER arch PVM's build succeeds into lib/$(PVM_ARCH) and leaves
# lib/LINUX64 empty -- and make does not check that a recipe made its targets,
# so `make pvm' would exit 0 having produced nothing. Hence the test after the
# recipe: without it that failure is silent.
PVM_DIR  := $(SL)/pvm3
PVM_LIB  := $(PVM_DIR)/lib/LINUX64/libpvm3.a
PVM_D    := $(PVM_DIR)/lib/LINUX64/pvmd3

.PHONY: all vendor core clean unpatch pvm pvm-link
all: $(B)/sigel_eval
vendor: $(VENDOR_LIBS)
pvm: $(PVM_LIB) $(PVM_D)
pvm-link: $(B)/pvm_link

# One rule for both products; PVM's own make builds them together. Depending on
# the patch stamp is what rebuilds this when a vendor/patches/pvm3-*.patch
# changes.
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

# Reverse order. Two patches touch pvm3/src/pvmd.c, so undoing them in
# application order leaves the second one's hunks shifted -- it still lands, with
# an offset and a .orig backup, and a larger shift would miss outright.
unpatch:
	@for p in $$(printf '%s\n' $(PATCHES) | tac); do patch -R -p1 -d $(SL) < $$p; done
	rm -f $(STAMP)

# PVM's objects and products live in the vendored tree, not in build/, so they
# need naming here or `make clean' would leave them.
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

# DynaMechs. gldraw.cpp is the OpenGL renderer. It cannot be left out: it
# defines the virtual draw() of every dm* class, so their vtables reference it
# and the linker pulls it in. Hence -lGL -lGLU below, which is never called.
$(LIB)/libdm.a: INCS := -I$(SL)/dynamechs/dm -I$(SHIM)
$(LIB)/libdm.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,$(wildcard $(SL)/dynamechs/dm/*.cpp))

# CyberVRML97. Sample.cpp is a demo program with its own main().
$(LIB)/libcv97.a: INCS := -I$(SL)/cv97 -I$(SHIM)
$(LIB)/libcv97.a: $(patsubst $(SL)/%.cpp,$(OBJ)/%.o,\
  $(filter-out $(SL)/cv97/Sample.cpp,$(wildcard $(SL)/cv97/*.cpp)))

# Dynamo -- the maths library and what it drags with it. -ldynalib is the name
# the 2003 link line uses.
#
# SIGEL's Dynamo BACKEND was deleted on 2026-08-28 (physics_backends.md), but
# Dynamo is also the maths library the whole of SIGEL is built on: DL_vector,
# DL_point, DL_matrix, DL_Scalar, 1,101 references across 93 files. So the
# archive stays; what changes is which of its 60 .cpp go into it.
#
# 14 compiled, of which the linker actually pulls 12. pointvector.cpp and
# list.cpp really are empty -- "no non-inline methods", zero defined symbols,
# never extracted -- and they are here only so the maths half of the library is
# named rather than implied. The 12 that matter are where the "maths half /
# physics half" story breaks: matrix.cpp carries 27
# out-of-line DL_matrix members, all of which sigel_eval links, AND it includes
# dyna_system.h so that DL_matrix::invert can report a singular matrix through
# the physics engine's global callback:
#
#   matrix.cpp:233  DL_dsystem->get_companion()->Msg("singular matrix ...")
#
# That one call, plus the DL_geo vtable matrix.o emits, is an undefined
# reference to dyna_system.o and geo.o, and their closure is nine more physics
# translation units. Measured with nm over all 60 objects, not guessed; each
# entry below the blank line names the symbol that pulled it in:
#
#   dyna_system  DL_dsystem            geo          DL_geo::move
#   dyna         DL_dyna::newkinenergy constraint   DL_constraint::reset_undo
#   constraint_manager DL_constraints  euler        DL_euler::DL_euler
#   m_integrator DL_m_integrator::stepsize          supvec  DL_supvec::A2q
#   force_drawable DL_force_drawable::get_fd_info   vector4 DL_vector4::assign
#   largematrix  DL_largematrix::prep_for_solve
#
# So the two halves are NOT cleanly separable, and this list is the honest
# answer rather than the three-file one. 46 of the 60 stop being compiled,
# 10,084 of 13,567 lines; of the 3,483 still compiled, 3,056 are physics that
# only DL_matrix::invert's error path can reach. Nothing here is stubbed and no
# symbol is defined away -- breaking the coupling would mean patching a
# vendored diagnostic out, which is a separate decision.
#
# The whole of Dynamo/Src/Inc stays on the include path: the maths headers live
# there next to the physics ones.
dynamo_SRC := pointvector matrix list \
              constraint constraint_manager dyna dyna_system euler \
              force_drawable geo largematrix m_integrator supvec vector4
$(LIB)/libdynalib.a: INCS := -I$(SL)/Dynamo/Src/Inc -I$(SHIM)
$(LIB)/libdynalib.a: $(patsubst %,$(OBJ)/Dynamo/Src/Cpp/%.o,$(dynamo_SRC))

# fparser. The 2003 Makefile.am compiles this straight into each program.
$(LIB)/libfparser.a: INCS := -I$(SL)/fparser -I$(SHIM)
$(LIB)/libfparser.a: $(OBJ)/fparser/fparser.o

# ---------------------------------------------------------------------------
# SIGEL's own code, §7 Phase R.
#
# QtWidgets is on the include path and the link line. Nothing here opens a
# window: SIG_GPPopulation is the one core file that names a widget, and it
# guards every use with if (qApp).
#
# THAT GUARD DOES NOT MEAN WHAT THIS COMMENT USED TO SAY. It said qApp "is null
# without a QApplication". SIG_GPPopulation.cpp:23 includes <QApplication>, so
# its qApp is the QtWidgets macro -- a static_cast of QCoreApplication::instance()
# -- and is non-null whenever ANY QCoreApplication exists, a plain one included.
# It is harmless today by scope, not by the guard: the -me path's QCoreApplication
# lives only inside the if(mtEvolve) block (sigel.cpp:285-306), loadExperiment
# (:258) and saveExperiment (:320) both run outside it, and neither MT_GPSystem
# nor MT_Control references SIG_GPPopulation. Those 14 sites are not UB either
# way -- a static_cast of a NULL pointer is well defined. Found by review
# 2026-09-07, alongside the same mistake in MT_Controller.cpp.
#
# These get -Wall -Wextra and no -fpermissive, unlike the vendored code, and
# UndefinedBehaviorSanitizer as well as AddressSanitizer.

SRC   := sigel
QTINC := $(shell qmake6 -query QT_INSTALL_HEADERS)
QTLIB := $(shell qmake6 -query QT_INSTALL_LIBS)
QTBIN := $(shell qmake6 -query QT_INSTALL_LIBEXECS)
MOC   := $(QTBIN)/moc
UIC   := $(QTBIN)/uic
RCC   := $(QTBIN)/rcc

SIGSAN := -fsanitize=address,undefined -fno-omit-frame-pointer
SIGINC := -Ishim -I$(SRC)/include -I$(B)/ui -isystem $(QTINC) \
          $(addprefix -isystem $(QTINC)/,QtCore QtGui QtWidgets \
                                        QtOpenGL QtOpenGLWidgets) \
          $(addprefix -isystem $(SL)/,newmat09 dynamechs/dm Dynamo/Src/Inc \
                                      fparser cv97 pvm3/include)
SIGCXX := g++ -std=c++17 -O1 -g -Wall -Wextra \
              -DMINMAX_H $(SIGSAN)

CORE := SIGEL_Tools SIGEL_Environment MT_GPSystem SIGEL_Robot SIGEL_Program \
        SIGEL_RobotIO SIGEL_Simulation MT_Control SIGEL_GP

# These three exclusions all existed for one reason -- the GUI was not ported --
# and C6/C7/C8 removed it. MT_Controller.cpp constructs an MT_MainWindow and
# makes 23 mainWindow-> accesses (MT_GUI, C6); SIG_GUIGPManager.cpp is its
# counterpart in SIGEL_GP and reaches into SIG_GUIGPExperiment (SIGEL_MasterGUI, C7);
# the non-WIN ZORC fitness function was still on the Qt 2 API until C8. All
# three now compile, and C9 needs all three: linking `sigel' without them fails
# on 20 undefined MT_Controller symbols plus SIG_GUIGPManager's vtable.
#
# WIN_SIG_GPRemoteZORCFitnessFunction.cpp is the one real exclusion left. It
# needs HANDLE and OVERLAPPED from windows.h and has no Qt 2 API left in it, so
# it cannot be compiled on this platform at any point -- it is excluded because
# of the platform, not because of the port.
EXCLUDE_SIGEL_GP   := $(SRC)/src/SIGEL_GP/WIN_SIG_GPRemoteZORCFitnessFunction.cpp

CORE_LIBS := $(patsubst %,$(LIB)/lib%.a,$(CORE))
core: $(CORE_LIBS)

# The five GUI modules, ported in C3-C7 and until C9 built by nothing at all --
# check.sh only ever ran them through -fsyntax-only, so no GUI object file had
# ever existed and no vtable had ever been emitted.
GUI := SIGEL_CommonGUI SIGEL_Visualisation SIGEL_SlaveGUI MT_GUI SIGEL_MasterGUI
GUI_LIBS := $(patsubst %,$(LIB)/lib%.a,$(GUI))
gui: $(GUI_LIBS)

# The slave is not a cut-down master: it links its OWN three GUI modules and
# never the master's two. Handing it all five drags SIG_GUIGPExperiment in through
# the master's moc objects, which drags in MT_Controller, which is exactly the
# master SIG_GPExperiment the slave must not have -- the assertion on the link
# caught precisely that.
GUI_SLAVE := SIGEL_CommonGUI SIGEL_Visualisation SIGEL_SlaveGUI
GUI_SLAVE_LIBS := $(patsubst %,$(LIB)/lib%.a,$(GUI_SLAVE))

$(OBJ)/sigel/%.o: $(SRC)/src/%.cpp $(STAMP)
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(SIGINC) -c $< -o $@

# ---------------------------------------------------------------------------
# The 20 Designer forms -- PORTING.md Phase C, §7.
#
# Qt 2 shipped the .ui as <!DOCTYPE UI> with no version attribute; Qt 4.8's
# uic3 -convert carries that to version="4.0", which Qt 6's uic reads. That
# conversion happened ONCE, in a Qt 4.8 container, and its result is committed
# -- the forms in ui/ are Qt 6 forms now, so nothing here needs docker.
#
# Qt 6's uic emits only Ui::<Form>, a struct with setupUi(). The QWidget-derived
# class the hand-written subclasses inherit from is a committed source file per
# form, in the module's own include/ and src/ -- see SIG_GPParameterBase.h.
#
# All 20 are converted as of C2. An unconverted form fed to Qt 6's uic fails
# loudly (it cannot read the Qt 2 <!DOCTYPE UI> format at all), so this list
# staying in step with ui/ is checked by the build rather than by inspection.
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
QRCS  :=  SIGEL_MasterUI/SIG_GPParameterBase \
            SIGEL_SlaveUI/SIG_SimulationWidgetBase

UI_HDRS  := $(patsubst %,$(B)/ui/ui_%.h,$(notdir $(FORMS)))
QRC_OBJS := $(patsubst %,$(OBJ)/qrc/%.o,$(notdir $(QRCS)))
QRC_MASTER := $(OBJ)/qrc/SIG_GPParameterBase.o
QRC_SLAVE  := $(OBJ)/qrc/SIG_SimulationWidgetBase.o

# The qrc objects are built here, not just generated, so that check.sh covers
# rcc at all: a malformed .qrc fails the gate instead of waiting for C7 to link.
# It does NOT catch a .qrc naming a missing file -- this rule depends on the
# .qrc, not on the files listed inside it, so a deleted image leaves the stale
# object in place and make exits 0. check.sh's forward resource check is what
# fires there. (An earlier version of this comment claimed the rule covered it;
# corrected by the C1 review, which measured it.)
# C7 must name these on the link line EXPLICITLY -- a resource object that ends
# up inside a static archive with nothing referencing it is dropped, and the
# icons silently vanish again.
.PHONY: forms
forms: $(UI_HDRS) $(QRC_OBJS)

# One pattern rule cannot see the module subdirectory, so generate the rule.
define form_rule
$(B)/ui/ui_$(notdir $(1)).h: $(SRC)/ui/$(1).ui
	@mkdir -p $$(dir $$@)
	$(UIC) $$< -o $$@
endef
$(foreach f,$(FORMS),$(eval $(call form_rule,$(f))))

# The images Qt 2 embedded in the .ui. uic3 -extract pulled them out into a
# .qrc next to the form; rcc puts them back INSIDE the binary, which is what
# Qt 2 did and what a runtime file path would not do.
define qrc_rule
$(B)/qrc/qrc_$(notdir $(1)).cpp: $(SRC)/ui/$(1).qrc
	@mkdir -p $$(dir $$@)
	$(RCC) --name $(notdir $(1)) $$< -o $$@
endef
$(foreach q,$(QRCS),$(eval $(call qrc_rule,$(q))))

$(OBJ)/qrc/%.o: $(B)/qrc/qrc_%.cpp
	@mkdir -p $(dir $@)
	$(SIGCXX) -MMD -MP $(SIGINC) -c $< -o $@

# The Q_OBJECT classes in core. Their vtable and typeinfo live in the generated
# code, so without these the link fails on SIG_Simulation. SIG_DynaSystem.h was
# the third entry until the Dynamo backend was deleted (physics_backends.md).
# This was a hand-written list of the two core Q_OBJECT classes. The GUI adds
# 54 more, and a hand-kept list of 56 is a list that goes stale silently: a
# missing entry is not a compile error, it is an undefined vtable at link time
# or -- worse, for a class whose vtable something else emits -- a signal that
# never fires at run time. Derived from the source instead.
MOC_HDRS := $(patsubst $(SRC)/include/%,%,$(shell grep -rl Q_OBJECT \
              $(addprefix $(SRC)/include/,$(CORE) $(GUI)) 2>/dev/null | sort))
MOC_OBJS := $(patsubst %.h,$(OBJ)/moc/%.o,$(MOC_HDRS))
# MT_Control is a CORE module, but its only Q_OBJECT is MT_Controller, whose
# meta-object references MT_MainWindow -- so putting it on the slave's line
# drags the whole master GUI in behind it. The slave links the Clean
# SIG_GPExperiment precisely so that it never has an MT_Controller at all.
# sigel_eval and pvm_link are headless harnesses: they link no GUI archive at
# all, so they get the core-only meta-objects they always had. Handing them the
# derived full set drags GUI vtables onto a link line with no GUI library
# behind it. MT_Control is out for the same reason as in the slave.
MOC_OBJS_CORE  := $(patsubst %.h,$(OBJ)/moc/%.o,\
                    $(filter $(addsuffix /%,$(filter-out MT_Control,$(CORE))),$(MOC_HDRS)))

SLAVE_MODULES  := $(filter-out MT_Control,$(CORE)) $(GUI_SLAVE)
MOC_OBJS_SLAVE := $(patsubst %.h,$(OBJ)/moc/%.o,\
                    $(filter $(addsuffix /%,$(SLAVE_MODULES)),$(MOC_HDRS)))

# make treats these as intermediate and DELETES them after linking, so the next
# make regenerates the .cpp, recompiles the .o, and relinks -- which leaves the
# binary looking out of date to the gate scripts' `make -q' guard even though
# nothing changed. Keep them.
.SECONDARY: $(patsubst %.h,$(B)/moc/%.cpp,$(MOC_HDRS))

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
$(foreach m,$(GUI),$(eval $(call core_lib,$(m))))

# One fitness evaluation. --start-group because the core modules have cycles:
# SIGEL_GP calls SIGEL_Simulation, which reaches back through SIG_Robot.
#
# WHICH SIG_GPExperiment GETS LINKED WAS DECIDED BY ARCHIVE MEMBER ORDER, AND
# NOTHING PINNED IT. §9 records that the class is defined twice on purpose --
# SIG_GPExperiment.cpp for `sigel', SIG_GPExperimentClean.cpp for the slave,
# differing in whether the constructor builds an MT_Controller -- and that
# sigel_eval is the slave's role and must get Clean. But both land in
# libSIGEL_GP.a, the linker takes the FIRST member that defines the symbol, and
# $(wildcard) does not sort: a clean build happened to put Clean first, an
# incremental one put the master first and the link then failed on
# MT_Controller. Sorting would be worse, not better -- alphabetically the
# master wins.
#
# So name Clean explicitly, ahead of the archives, exactly as 2003 compiled it
# into the slave target. The assertion after the link is the P4 pattern.
# TODAY the linker fails first, on the master's undefined MT_Controller, so the
# assertion is belt-and-braces; it becomes the ONLY guard once C6 ports MT_GUI
# and MT_Controller links, at which point the master would link silently and
# sigel_eval would start constructing an MT_Controller per experiment.
CLEAN_OBJ := $(OBJ)/sigel/SIGEL_GP/SIG_GPExperimentClean.o
# ...and its opposite. `sigel' needs the MASTER variant, and needs it named just
# as explicitly: BOTH files define the identical set of symbols for the class,
# so the linker takes whichever archive member it meets first, and $(wildcard)
# happens to put Clean at member 5 and the master at 6. Leaving it to that
# order gave `sigel' the Clean constructor -- which never assigns
# mtController (a raw pointer with no initialiser, SIG_GPExperiment.h:221),
# so every experiment in the GUI carried a garbage MT_Controller pointer and
# -mtevolve dereferenced it. Found by the C9 review; see PORTING.md.
MASTER_OBJ := $(OBJ)/sigel/SIGEL_GP/SIG_GPExperiment.o

# The two variants differ only in the constructor, so the honest test of which
# one linked is the constructor's SIZE against the object it should have come
# from. The previous guard counted MT_Controller symbols in the binary, which
# says nothing: `sigel' links MT_Control and MT_GUI whatever happens, so it
# counted 45 and passed while linking exactly the wrong variant.
ctor_size = nm -C -S $(1) | awk '/SIG_GPExperiment::SIG_GPExperiment\(\)$$/{print $$2; exit}'

$(B)/sigel_eval: checks/programs/sigel_eval.cpp $(MOC_OBJS_CORE) $(CLEAN_OBJ) $(CORE_LIBS) $(VENDOR_LIBS)
	$(SIGCXX) $(SIGINC) $< $(MOC_OBJS_CORE) $(CLEAN_OBJ) -o $@ \
	  -Wl,--start-group $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group \
	  -L$(QTLIB) -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui -lQt6Core -lGL -lGLU -lm
	@want=`$(call ctor_size,$(CLEAN_OBJ))`; got=`$(call ctor_size,$@)`; \
	 test -n "$$want" && test "$$got" = "$$want" || { \
	   echo "sigel_eval linked the WRONG SIG_GPExperiment: constructor is $$got," \
	        "Clean's is $$want -- see PORTING.md section 9." >&2; exit 1; }

# ---------------------------------------------------------------------------
# Does SIGEL's own PVM code link and run against real PVM? -- PORTING.md
# Phase P, step P4.  ./checks/pvm-check.sh runs this.
#
# The two objects are named on the command line rather than left to the
# archive, so the linker takes them whether or not anything references them.
# They are every object in the built core with an undefined pvm_*: measured
# with nm over build/lib/lib*.a, seven symbols each and pvm_recv shared, 13
# distinct. The rest of the core and the vendored libraries follow because
# those two drag in most of SIGEL.
#
# That list is hand-written, so the recipe asserts it is still complete: if any
# other core object gains a pvm_* call it would sit unreferenced in its archive,
# never be linked, and P4 would stay green while covering less.
#
# MT_Controller.o is exempted, and the assertion caught it the moment C9 lifted
# its exclusion. Its ONLY pvm_* is a single pvm_halt() on a fatal error path
# (MT_Controller.cpp:456, immediately followed by exit(1)) -- not a data round
# trip, which is the thing P4 exists to prove. Linking it here would pull the
# entire master GUI into a headless harness to cover one teardown call. The
# exemption is the reason, not a silencing: any OTHER new pvm_* still fails.
#
# -ltirpc is NOT optional even though the link succeeds without it. libasan
# exports weak xdr_double, xdr_int, xdrmem_create and friends as interceptors,
# so under the sanitizers PVM's XDR references bind to those with nothing
# behind them. glibc still has the same names but only as compat symbols
# (xdr_double@GLIBC_2.17), which ld will not bind a new reference to. Hence
# the program runs a round trip rather than only linking.
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
# The two programs -- PORTING.md Phase C, step C9.
#
# Nothing built these before C9: they are src/*.cpp, outside every module list,
# and check.sh only ever ran them through -fsyntax-only. Linking is what proves
# the GUI port, because a missing moc, an unemitted vtable and a resource that
# never made it into the binary are all invisible to a compile.
#
# The .qrc objects are named on the link line rather than left inside an
# archive: nothing references their symbols, so the linker would drop them from
# a static library and the form icons would silently vanish. The Makefile's own
# forms section says so; this is the line it was talking about.
#
# sigel gets the MASTER SIG_GPExperiment -- the variant whose constructor builds
# an MT_Controller -- straight out of libSIGEL_GP.a. sigel_slave must get the
# Clean variant instead, so that object is named explicitly ahead of the
# archives, exactly as sigel_eval does it. Both are asserted after the link.
.PHONY: programs
programs: $(B)/sigel $(B)/sigel_slave

SIGLIBS = $(PVM_LIB) -ltirpc \
          -L$(QTLIB) -lQt6OpenGLWidgets -lQt6OpenGL -lQt6Widgets -lQt6Gui -lQt6Core \
          -lGL -lGLU -lm

$(B)/sigel: $(SRC)/src/sigel.cpp $(MOC_OBJS) $(QRC_OBJS) $(GUI_LIBS) $(CORE_LIBS) \
            $(VENDOR_LIBS) $(PVM_LIB)
	$(SIGCXX) $(SIGINC) $< $(MOC_OBJS) $(QRC_MASTER) $(MASTER_OBJ) -o $@ \
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

# The GUI behaviour driver -- PORTING.md Phase C, step C10.
#
# Same link shape as $(B)/sigel, and for the same reason: it constructs the
# real SIG_MainWindow, so it needs the MASTER SIG_GPExperiment. Left to the
# archives the linker picks Clean, whose constructor never builds an
# MT_Controller, and the first tree selection calls IsEnabled() through an
# uninitialised pointer. That produced a convincing false defect before the
# assertion below existed -- the crash is in the harness, not the port.
#
# -lQt6Test is the only addition: QTest is what posts the mouse, key and
# context-menu events.
.PHONY: guidrive
guidrive: $(B)/guidrive

$(B)/guidrive: checks/programs/guidrive.cpp $(MOC_OBJS) $(QRC_OBJS) $(GUI_LIBS) $(CORE_LIBS) \
               $(VENDOR_LIBS) $(PVM_LIB)
	$(SIGCXX) -DQT_CORE_LIB -DQT_GUI_LIB -DQT_WIDGETS_LIB -DQT_TESTLIB_LIB \
	  $(SIGINC) -isystem $(QTINC)/QtTest $< $(MOC_OBJS) $(QRC_MASTER) $(QRC_SLAVE) $(MASTER_OBJ) -o $@ \
	  -Wl,--start-group $(GUI_LIBS) $(CORE_LIBS) $(VENDOR_LIBS) -Wl,--end-group \
	  -lQt6Test $(SIGLIBS)
	@want=`$(call ctor_size,$(MASTER_OBJ))`; got=`$(call ctor_size,$@)`; \
	 test -n "$$want" && test "$$got" = "$$want" || { \
	   echo "guidrive linked the WRONG SIG_GPExperiment: constructor is $$got," \
	        "the master's is $$want -- see PORTING.md section 9." >&2; exit 1; }

-include $(shell find $(OBJ) -name '*.d' 2>/dev/null)
