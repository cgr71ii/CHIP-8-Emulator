.PHONY= doc clean

CC=g++
OPTIONS= -g -std=c++11 
DEBUG= #-D DEBUG
LIBDIR=lib
INCLUDEDIR=include
TESTDIR=tests
SDL2FLAGS=$(shell pkg-config sdl2 --cflags --libs)

UNAME=$(shell uname)

#_OBJ= cpu.o
#OBJ = $(patsubst %,$(LIBDIR)/%,$(_OBJ))

#__OBJ = $(wildcard $(LIBDIR)/*.cpp)
#_OBJ = $(patsubst $(LIBDIR)/%.cpp,%.o,$(__OBJ))
#OBJ = $(patsubst %,$(LIBDIR)/%,$(_OBJ))

_OBJ = $(wildcard $(LIBDIR)/*.cpp)
OBJ = $(patsubst $(LIBDIR)/%.cpp,$(LIBDIR)/%.o,$(_OBJ))

_TESTS = $(wildcard $(TESTDIR)/*.cpp)
TESTS = $(patsubst $(TESTDIR)/%.cpp,$(TESTDIR)/%.o,$(_TESTS))
TESTSLIN = $(patsubst $(TESTDIR)/%.cpp,$(TESTDIR)/%,$(_TESTS))
TESTSEXE = $(patsubst $(TESTDIR)/%.cpp,$(TESTDIR)/%.exe,$(_TESTS))

# Variables for "make cleanw"
OBJW = $(subst /,\,$(OBJ))
TESTSW = $(subst /,\,$(TESTS))
TESTSEXEW = $(subst /,\,$(TESTSEXE))

ifeq ($(UNAME), Linux)
	EXT=
	TESTSCOMP=$(TESTSLIN)
else
	EXT=.exe
	TESTSCOMP=$(TESTSEXE)
endif

$(info [INFO] $$UNAME is [${UNAME}])
$(info [INFO] $$EXT is [${EXT}])

$(info [INFO] $$_OBJ is [${_OBJ}])
$(info [INFO] $$OBJ is [${OBJ}])

$(info [INFO] $$_TESTS is [${_TESTS}])
$(info [INFO] $$TESTS is [${TESTS}])
$(info [INFO] $$TESTSLIN is [${TESTSLIN}])
$(info [INFO] $$TESTSEXE is [${TESTSEXE}])
$(info [INFO] $$TESTSCOMP is [${TESTSCOMP}])

MAIN = chip8

$(info --------------------------------)

all: $(OBJ) $(TESTS) $(TESTSCOMP) $(MAIN)$(EXT)

$(MAIN)$(EXT): src/$(MAIN).cpp $(OBJ)
	$(info Building main)
	$(CC) $(OPTIONS) $(DEBUG) $(SDL2FLAGS) -I$(INCLUDEDIR) src/$(MAIN).cpp $(OBJ) -o $(MAIN)

$(TESTDIR)/%.o : $(TESTDIR)/%.cpp $(INCLUDEDIR)/*.h $(OBJ)
	$(info Building object test files)
	$(CC) $(OPTIONS) $(DEBUG) -c -I$(INCLUDEDIR) -o $@ $<

$(TESTDIR)/%$(EXT) : $(TESTDIR)/%.o $(INCLUDEDIR)/*.h
	$(info Compiling test files)
	$(CC) $(OPTIONS) $(DEBUG) -I$(INCLUDEDIR) $< $(OBJ) -o $@

$(LIBDIR)/%.o : $(LIBDIR)/%.cpp $(INCLUDEDIR)/%.h
	$(info Building object lib files)
	$(CC) $(OPTIONS) $(DEBUG) -c -I$(INCLUDEDIR) -o $@ $<

clean:
	$(info Use "cleanw" for windows and "cleanl" for linux.)

cleanw:
	erase /F /Q $(MAIN).exe $(OBJW) $(TESTSW) $(TESTSEXEW)

cleanl:
	rm -f $(MAIN) $(OBJ) $(TESTS) $(TESTSLIN)