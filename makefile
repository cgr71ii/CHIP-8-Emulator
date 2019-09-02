.PHONY= doc clean

CC=g++
OPTIONS= -g -std=c++11 
DEBUG= #-D DEBUG
LIBDIR=lib
INCLUDEDIR=include
TESTDIR=tests

#_OBJ= cpu.o
#OBJ = $(patsubst %,$(LIBDIR)/%,$(_OBJ))

#__OBJ = $(wildcard $(LIBDIR)/*.cpp)
#_OBJ = $(patsubst $(LIBDIR)/%.cpp,%.o,$(__OBJ))
#OBJ = $(patsubst %,$(LIBDIR)/%,$(_OBJ))

_OBJ = $(wildcard $(LIBDIR)/*.cpp)
OBJ = $(patsubst $(LIBDIR)/%.cpp,$(LIBDIR)/%.o,$(_OBJ))

_TESTS = $(wildcard $(TESTDIR)/*.cpp)
TESTS = $(patsubst $(TESTDIR)/%.cpp,$(TESTDIR)/%.o,$(_TESTS))
TESTSEXE = $(patsubst $(TESTDIR)/%.cpp,$(TESTDIR)/%.exe,$(_TESTS))

# Variables for "make cleanw"
OBJW = $(subst /,\,$(OBJ))
TESTSW = $(subst /,\,$(TESTS))
TESTSEXEW = $(subst /,\,$(TESTSEXE))

$(info [INFO] $$_OBJ is [${_OBJ}])
$(info [INFO] $$OBJ is [${OBJ}])

$(info [INFO] $$_TESTS is [${_TESTS}])
$(info [INFO] $$TESTS is [${TESTS}])
$(info [INFO] $$TESTSEXE is [${TESTSEXE}])

MAIN = chip8

$(info --------------------------------)

all: $(OBJ) $(TESTS) $(TESTSEXE) $(MAIN).exe

$(MAIN).exe: src/$(MAIN).cpp
	$(info Building main)
	$(CC) $(OPTIONS) $(DEBUG) -I$(INCLUDEDIR) src/$(MAIN).cpp $(OBJ) -o $(MAIN)

$(TESTDIR)/%.o : $(TESTDIR)/%.cpp $(INCLUDEDIR)/*.h
	$(info Building object test files)
	$(CC) $(OPTIONS) $(DEBUG) -c -I$(INCLUDEDIR) -o $@ $<

$(TESTDIR)/%.exe : $(TESTDIR)/%.o $(INCLUDEDIR)/*.h
	$(info Compiling test files)
	$(CC) $(OPTIONS) $(DEBUG) -I$(INCLUDEDIR) $< $(OBJ) -o $@

$(LIBDIR)/%.o : $(LIBDIR)/%.cpp $(INCLUDEDIR)/%.h
	$(info Building object lib files)
	$(CC) $(OPTIONS) $(DEBUG) -c -I$(INCLUDEDIR) -o $@ $<

clean:
	erase /F /Q $(MAIN).exe $(OBJW) $(TESTSW) $(TESTSEXEW)