ARTOOLKITDIR := ../ARToolKit
INC_DIR      := $(ARTOOLKITDIR)/include
LIB_DIR      := $(ARTOOLKITDIR)/lib

DIREXE := exec/
DIRSRC := src/
DIROBJ := obj/
CCLDL  := -lARgsub -lARvideo -lARMulti -lAR -lglut -lGL -lGLU -lglut -lm -lvlc
CCFLG  := -Wall -I$(INC_DIR) -c
CC     := gcc

all: dirs Piano

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

Piano: $(DIROBJ)main.o $(DIROBJ)utils.o
	$(CC) -o $(DIREXE)$@ $^ -L$(LIB_DIR) $(CCLDL)

$(DIROBJ)%.o: $(DIRSRC)%.c
	$(CC) $(CCFLG) $^ -o $@

run:
	./$(DIREXE)Piano

clean:
	rm -rf *~ core $(DIROBJ) $(DIREXE) $(DIRSRC)*~