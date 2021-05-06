.PHONY: all clean

all: gam-solve

ROOTCONFIG := root-config
#ARCH       := $(shell $(ROOTCONFIG) --arch)
#PLATFORM   := $(shell $(ROOTCONFIG) --platform)

ROOTCFLAGS := $(shell $(ROOTCONFIG) --cflags)
ROOTGLIBS  := $(shell $(ROOTCONFIG) --glibs)
ROOTLIBS   := $(shell $(ROOTCONFIG) --libs)
GCC         = g++ -std=c++17

CXXFLAGS := -g -Ofast -fno-math-errno -Wall

obj/%.o: src/%.cxx src/%.h
	@echo "Compiling $@"
	@$(GCC) -c $(CXXFLAGS) -Isrc/ $< $(ROOTCFLAGS) -o $@

obj/solve.o: src/solve.cxx obj/Source.o obj/Material.o
	@echo "Compiling $@"
	@$(GCC) -c $(CXXFLAGS) -Isrc/ $< $(ROOTCFLAGS) -o $@

gam-solve: obj/solve.o obj/Source.o obj/Material.o
	@echo "Linking $@"
	@$(GCC) $^ $(ROOTLIBS) $(ROOTGLIBS) -lGeom -lboost_program_options -o $@


clean:
	rm -fv gam-solve obj/*.o
