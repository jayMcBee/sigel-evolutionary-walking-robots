# Build for the Qt 6 port -- PORTING.md §3.
#
# R1 builds the vendored libraries only. None of this is SIGEL's own code.
#
#   make            the vendored libraries
#   make clean      remove build/
#   make unpatch    revert the vendored tree to the tarball contents
#
# The vendored tree is not tracked (it comes out of supportingLibs.tar.gz), so
# the three edits gcc 15 needs live in patches/ and are applied here against a
# stamp file inside that tree. Re-extracting the tarball removes the stamp and
# the patches are applied again.
#
# Vendored code is built with -w -fpermissive, which SIGEL's own code does not
# get: check.sh already treats these headers as -isystem for the same reason.
# -fpermissive covers newmat1.cpp and newmat9.cpp, which pass a string literal
# to char* and a long to ios_base::fmtflags.
#
# AddressSanitizer only, no UndefinedBehaviorSanitizer. qhull and the f2c
# translation of LINPACK's ssvdc report misaligned access and signed overflow
# throughout; instrumenting them would bury the reports from SIGEL's own code,
# which is what the sanitizer run is for.

SL   := x/supportingLibs/supportingLibs
SHIM := shim
B    := build
LIB  := $(B)/lib
OBJ  := $(B)/obj

SAN  := -fsanitize=address -fno-omit-frame-pointer
VCXX := g++ -std=c++17 -O1 -g -w -fpermissive -DMINMAX_H $(SAN)
VCC  := gcc -std=gnu17 -O1 -g -w $(SAN)

STAMP := $(SL)/.sigel-patched
PATCHES := $(wildcard patches/*.patch)

VENDOR_LIBS := $(LIB)/libnewmat.a $(LIB)/libdm.a $(LIB)/libcv97.a \
               $(LIB)/libdynalib.a $(LIB)/libsolid.a $(LIB)/libqhull.a \
               $(LIB)/libfparser.a

.PHONY: all vendor clean unpatch
all: vendor
vendor: $(VENDOR_LIBS)

$(STAMP): $(PATCHES)
	@for p in $(PATCHES); do echo "  patch $$p"; patch -p1 -d $(SL) < $$p; done
	@touch $@

unpatch:
	@for p in $(PATCHES); do patch -R -p1 -d $(SL) < $$p; done
	rm -f $(STAMP)

clean:
	rm -rf $(B)

$(OBJ)/%.o: $(SL)/%.cpp $(STAMP)
	@mkdir -p $(dir $@)
	$(VCXX) $(INCS) -c $< -o $@

$(OBJ)/%.o: $(SL)/%.c $(STAMP)
	@mkdir -p $(dir $@)
	$(VCC) $(INCS) -c $< -o $@

$(LIB)/lib%.a:
	@mkdir -p $(LIB)
	$(AR) crs $@ $^

# newmat09 -- the object list its own gnu.mak uses for libnewmat.a; the rest of
# the directory is the test and example programs.
newmat_SRC := bandmat cholesky evalue fft hholder jacobi myexcept newmat1 \
  newmat2 newmat3 newmat4 newmat5 newmat6 newmat7 newmat8 newmat9 newmatex \
  newmatnl newmatrm solution sort submat svd
$(LIB)/libnewmat.a: INCS := -I$(SL)/newmat09 -I$(SHIM)
$(LIB)/libnewmat.a: $(patsubst %,$(OBJ)/newmat09/%.o,$(newmat_SRC))

# DynaMechs. gldraw.cpp is the OpenGL renderer; it is archived but nothing
# headless references it, so the linker leaves it out.
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
