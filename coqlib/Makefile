# Makefile pour coqlib.
# Permet de généger le fichier libcoqlib.a.
# Compileur
CC := cc
# Repertoires des fichiers sources, objets...
SRCDIR := src
OBJDIR := obj
LIBDIR := lib
LIBNAME := coqlib
OS_NAME := $(shell uname -s)
# Flags du compileur
# Ici, on ajoute le flag `DEBUG` pour voir les logs, e.g. les printdebug("allo").
# Pour freetype, il faut aller dans le sous-répertoire freetype2.
ifeq ($(OS_NAME), Darwin)
	CFLAGS=-g -Wall -Wno-deprecated-declarations -Wno-unknown-pragmas \
		-DDEBUG -I/usr/local/include/freetype2
else
	CFLAGS=-g -Wall -Wno-deprecated-declarations -Wno-unknown-pragmas \
       -DDEBUG -I/usr/include/freetype2
endif

# Lister les fichiers c.
SRCS := $(shell find $(SRCDIR) -name "*.c")
# Liste des fichiers `objets` (c compilés) correspondants.
OBJS := $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))
# Emplacement de la librairie.
LIB := $(LIBDIR)/lib$(LIBNAME).a

# Rule primaire : créé la librairie (package/archive de .o)
$(LIB): $(OBJS)
	@echo Creating library $@.
	@test -d $(@D) || mkdir -p $(@D)
	@ar -rcs $@ $(OBJS)
	@echo Done!

# Compilations des .c en .o.
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo Compiling object $@...
	@test -d $(@D) || mkdir -p $(@D)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo Removing objects and lib.
	@rm -r $(LIBDIR) $(OBJDIR)
