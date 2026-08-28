# Build for the Qt 6 port -- PORTING.md §3.
#
# Two halves: the vendored third-party libraries, then SIGEL's own code and a
# program that runs one fitness evaluation.
#
#   make            build/sigel_eval -- one fitness evaluation
#   make vendor     the five vendored libraries only
#   make core       the nine SIGEL core modules only
#   make clean      remove build/
#   make unpatch    revert the vendored tree to the tarball contents
#
# make B=build-fast SAN= SIGSAN=   the same thing without the sanitizers, in a
# separate directory. One evaluation is 0.2 s either way.
#
# The vendored tree is not tracked (it comes out of supportingLibs.tar.gz), so
# the edits gcc 15 needs live in patches/ and are applied here against a
# stamp file inside that tree. tar does not delete files it does not carry, so
# re-extracting the tarball over the tree leaves the stamp behind: rm -rf the
# vendored tree first, or the build silently keeps objects built from patched
# sources.
#
# ONE EXCEPTION: pvm3/ no longer comes from supportingLibs.tar.gz. It is
# upstream PVM 3.4.6 out of the tracked pvm3.4.6.tgz, and that tarball still
# carries 3.4.3. So after any rm -rf of the vendored tree, restore pvm3/ from
# pvm3.4.6.tgz as well -- the guard below tells you so if you forget.
#
# patches/pvm3-*.patch are the nine that make PVM build here: the four config
# lines, and every Debian source patch that touches one of the 28 objects PVM
# compiles. They go through the same stamp as the rest, so nothing below needed
# changing to pick them up.
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

# PVM: the vendored 3.4.3 was replaced by upstream 3.4.6 -- PORTING.md Phase P.
# 3.4.3 has no conf/LINUX64.def at all, so it cannot describe this machine.
# (It has no aarch64 in lib/pvmgetarch either, but neither has 3.4.6 -- that
# line is one of the four config lines Phase P adds, not something the version
# bump supplies.)
#
# Re-extract supportingLibs.tar.gz over the tree and 3.4.3 comes back: tar
# overwrites but never deletes, the paths and the file count both still look
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
        rm -rf $(SL)/pvm3 && tar xzf pvm3.4.6.tgz -C $(SL) --strip-components=1 ./pvm3)
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

# Reverse order. Two patches touch pvm3/src/pvmd.c, so undoing them in
# application order leaves the second one's hunks shifted -- it still lands, with
# an offset and a .orig backup, and a larger shift would miss outright.
unpatch:
	@for p in $$(printf '%s\n' $(PATCHES) | tac); do patch -R -p1 -d $(SL) < $$p; done
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

# The Q_OBJECT classes in core. Their vtable and typeinfo live in the generated
# code, so without these the link fails on SIG_Simulation. SIG_DynaSystem.h was
# the third entry until the Dynamo backend was deleted (physics_backends.md).
MOC_HDRS := MT_GPSystem/MT_GPManager.h \
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
