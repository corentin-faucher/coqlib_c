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

CFLAGS=-g -DDEBUG -DWITH_OPENGL -Wall -Iinclude \
       -I../coqlib/include `sdl2-config --cflags`

ifeq ($(OS_NAME), Darwin)
	LFLAGS=-L../coqlib/lib -framework OpenGL -framework OpenAL \
		   `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
	LFLAGS2=-lcoqlib
# -F/Library/Frameworks -framework SDL2 -framework SDL2_ttf -framework SDL2_image
else
	LFLAGS=-L../coqlib/lib -lunwind -lm -lGLEW -lGL \
	       -lSDL2 -lSDL2_ttf -lSDL2_image -lopenal
	LFLAGS2=-lcoqlib
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

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling object $@...
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Removing objects and bin.
	@rm -r $(OBJDIR) $(BINDIR)
