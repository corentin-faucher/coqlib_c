# Makefile pour la démo coqlib avec sdl.
# Compileur
CC := cc
# Repertoires des fichiers sources, objets...
# SRCDIR :=  
OBJDIR :=  obj
BINDIR :=  bin
BINNAME := sdl_test
BIN :=     $(BINDIR)/$(BINNAME)
OS_NAME := $(shell uname -s)

CFLAGS=-g -Wall -DDEBUG -DWITH_OPENGL -Wno-unknown-pragmas \
       -I../coqlib/src `sdl2-config --cflags`

ifeq ($(OS_NAME), Darwin)
	LFLAGS=-g -Wall -DDEBUG -DWITH_OPENGL \
	       -L../coqlib/lib -framework OpenGL -framework OpenAL \
	       -lcoqlib `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
# -F/Library/Frameworks -framework SDL2 -framework SDL2_ttf -framework SDL2_image
else
    LFLAGS=-L../coqlib/lib -lunwind -lm -lGLEW -lGL \
	       -lSDL2 -lSDL2_ttf -lSDL2_image -lopenal
	LFLAGS2=-lcoqlib
endif

# Lister les fichiers c.
SRCS := $(shell find . -name "*.c")
# Liste des fichiers `objets` (c compilés) correspondants.
OBJS := $(patsubst %.c, $(OBJDIR)/%.o, $(SRCS))

all: $(BIN)
	@echo Running app $(BIN)
	@echo $(MAKE)
	@./$(BIN)

$(BIN): $(OBJS)
	@echo Linking $@
	@test -d $(@D) || mkdir -p $(@D)
	$(CC) $(LFLAGS) $(OBJS) -o $@ $(LFLAGS2)

$(OBJDIR)/%.o: %.c
	@echo Compiling object $@...
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Removing objects and bin.
	@rm -r $(OBJDIR) $(BINDIR)
