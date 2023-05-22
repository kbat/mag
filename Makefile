.PHONY: all clean

all: gam-solve gam

ROOTCONFIG := root-config
#ARCH       := $(shell $(ROOTCONFIG) --arch)
#PLATFORM   := $(shell $(ROOTCONFIG) --platform)

ROOTCFLAGS := $(shell $(ROOTCONFIG) --cflags)
ROOTGLIBS  := $(shell $(ROOTCONFIG) --glibs)
ROOTLIBS   := $(shell $(ROOTCONFIG) --libs)

EIGENINC   := /usr/include/eigen3

GCC         = g++ -std=c++17

CXXFLAGS := -g -Ofast -fno-math-errno -Wall

obj/%.o: src/%.cxx src/%.h
	@echo "Compiling $@"
	@$(GCC) -c $(CXXFLAGS) -Isrc/ -I$(EIGENINC) $< $(ROOTCFLAGS) -o $@

obj/solve.o: src/solve.cxx
	@echo "Compiling $@"
	@$(GCC) -c $(CXXFLAGS) -Isrc/ -I$(EIGENINC) $< $(ROOTCFLAGS) -o $@

gam-solve: obj/solve.o obj/SDEF.o obj/Test.o obj/Source.o obj/Material.o obj/Solver.o obj/SolverArguments.o obj/functions.o obj/Markov.o
	@echo "Linking $@"
	@$(GCC) $^ $(ROOTLIBS) $(ROOTGLIBS) -lGeom -L$(HOME)/.conda/envs/WORK/lib -lboost_program_options -o $@

obj/gam.o: src/gam.cxx
	@echo "Compiling $@"
	@$(GCC) -c $(CXXFLAGS) -Isrc/ -I$(EIGENINC) $< $(ROOTCFLAGS) -o $@

gam: obj/gam.o obj/SDEF.o obj/Source.o obj/Material.o obj/Solver.o obj/Optimiser.o obj/OptArguments.o obj/functions.o obj/Markov.o
	@echo "Linking $@"
	@$(GCC) $^ $(ROOTLIBS) $(ROOTGLIBS) -lGeom -L$(HOME)/.conda/envs/WORK/lib -lboost_program_options -ltbb -o $@

valgrind: gam-solve
	valgrind \
	--tool=memcheck	\
	--smc-check=all-non-file --error-limit=no --leak-check=yes --num-callers=2 \
	--leak-check=full --track-origins=yes --show-leak-kinds=all \
	--suppressions=${ROOTSYS}/etc/valgrind-root.supp \
	./gam-solve -test 2 1

	# valgrind --smc-check=all-non-file -v --error-limit=no --leak-check=yes --num-callers=50 \
	# --leak-check=full --track-origins=yes --suppressions=${ROOTSYS}/etc/valgrind-root.supp \
	# ./gam-solve -test 2 1

test: gam-solve
	cd test/solver && root -b -q -l tests.C

clean:
	rm -fv gam gam-solve obj/*.o
