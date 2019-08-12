.PHONY= doc clean

CC=g++
OPTIONS= -g 
DEBUG= #-D DEBUG
LIBDIR=lib
INCLUDEDIR=include
#_OBJ= tcomplejo.o tvectorcom.o tabbcom.o 
_OBJ= cpu.o
OBJ = $(patsubst %,$(LIBDIR)/%,$(_OBJ))
MAIN = chip8

tad:    src/$(MAIN).cpp $(OBJ)
	$(CC) $(OPTIONS) $(DEBUG) -I$(INCLUDEDIR) src/$(MAIN).cpp $(OBJ) -o $(MAIN)

$(LIBDIR)/%.o : $(LIBDIR)/%.cpp $(INCLUDEDIR)/%.h
	$(CC) $(OPTIONS) $(DEBUG) -c -I$(INCLUDEDIR) -o $@ $<


clean:
	rm -f $(OBJ) 
