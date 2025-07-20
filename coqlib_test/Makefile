# Makefile pour la démo coqlib.
# Compileur
CC := cc
# Repertoires des fichiers sources, objets...
SRCDIR :=  src
OBJDIR :=  obj
BINDIR :=  bin
BINNAME := coqlib_test
BIN :=     $(BINDIR)/$(BINNAME)
OS_NAME := $(shell uname -s)

CFLAGS=-g -DDEBUG -Wall -I../coqlib/src
# Pas de SDL...
# `sdl2-config --cflags`  -Iinclude  -Wno-unknown-pragmas

ifeq ($(OS_NAME), Darwin)
	LFLAGS=-L../coqlib/lib -framework OpenGL -framework OpenAL \
		-lglfw -lfreetype
	LFLAGS2=-lcoqlib
else
	LFLAGS=  
	LFLAGS2= -L../coqlib/lib -lcoqlib -lunwind -lopenal -lm -lglad -lglfw -lfreetype
endif

# Lister les fichiers c.
SRCS := $(shell find $(SRCDIR) -name "*.c")
# Liste des fichiers `objets` (c compilés) correspondants.
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(BIN)
	@echo Running app $(BIN)
	@./$(BIN)

$(BIN): $(OBJS)
	@echo Linking $@
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(LFLAGS) $(OBJS) -o $@ $(LFLAGS2)
# 	@$(CC) -o $@ $(LFLAGS) $(OBJS)  $(LFLAGS2)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling object $@...
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Removing objects and bin.
	@rm -r $(OBJDIR) $(BINDIR)
