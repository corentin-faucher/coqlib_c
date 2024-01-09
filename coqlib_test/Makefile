# Makefile pour la démo coqlib avec glfw.
# Compileur
CC := cc
# Repertoires des fichiers sources, objets...
SRCDIR :=  src
OBJDIR :=  obj
BINDIR :=  bin
BINNAME := coqlib_test
BIN :=     $(BINDIR)/$(BINNAME)

# Flags du compileur
# -g : Generate debug information.
# -Wall : All warnings.
CFLAGS=-g -DDEBUG -DWITH_OPENGL -Wall -Iinclude \
       -I../coqlib/include `sdl2-config --cflags`
LFLAGS=-L../coqlib/lib -framework OpenGL -framework OpenAL \
       -lcoqlib -lglfw `sdl2-config --libs` -lSDL2_ttf -lSDL2_image
# -F/Library/Frameworks -framework SDL2 -framework SDL2_ttf -framework SDL2_image

# Lister les fichiers c.
SRCS := $(shell find $(SRCDIR) -name "*.c")
# Liste des fichiers `objets` (c compilés) correspondants.
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))

all: $(BIN)
	@echo Running app $@
	@./$(BIN)

$(BIN): $(OBJS)
	@echo Linking $@
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(CFLAGS) $(LFLAGS) $(OBJS) -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling object $@...
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Removing objects and bin.
	@rm -r $(OBJDIR) $(BINDIR)