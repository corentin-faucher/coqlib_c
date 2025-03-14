# Makefile pour coqlib.
# Permet de généger le fichier libcoqlib.a.
# Compileur
CC := cc
# Repertoires des fichiers sources, objets...
SRCDIR := src
OBJDIR := obj
LIBDIR := lib
LIBNAME := coqlib
# Flags du compileur
CFLAGS=-g -Wall -Wno-deprecated-declarations -Wno-unknown-pragmas -Iinclude \
       -DDEBUG -DWITH_OPENGL `sdl2-config --cflags` \
       -I/usr/local/include/freetype2
#-F/Library/Frameworks -framework SDL2

# Lister les fichiers c.
SRCS := $(shell find $(SRCDIR) -name "*.c")
# Liste des fichiers `objets` (c compilés) correspondants.
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
# Emplacement de la librairie.
LIB := $(LIBDIR)/lib$(LIBNAME).a

# Rule primaire : créé la librairie (package de .o)
$(LIB): $(OBJS)
	@echo Creating library $@.
	@test -d $(@D) || mkdir -p $(@D)
	@ar -rcs $@ $(OBJS)
	@echo Done!

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling object $@...
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Removing objects and lib.
	@rm -r $(LIBDIR) $(OBJDIR)
