################################################################################
#|  Project: RicoTech
#|     Date: 2016-11-03
#|   Author: Dan Bechard
################################################################################

# Directories
SRC_DIR := src
INC_DIR := include include/SDL include/GL include/stb
OBJ_DIR := obj
LIB_DIR := lib
DLL_DIR := dll
RES_DIR := res
RES_SUBDIRS := chunks font shader mesh texture
BIN_DIR := bin

BIN_EXE := $(BIN_DIR)/RicoTech.exe

#SOURCES := $(wildcard $(SRC_DIR)/*.c)
#SOURCES := $(filter-out $(SRC_DIR)/main_nuke.c, $(SOURCES))
#SOURCES := $(filter-out $(SRC_DIR)/notes.c, $(SOURCES))
SOURCES := $(SRC_DIR)/main.c
#OBJECTS := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SOURCES:.c=.o))
OBJECTS := $(subst $(SRC_DIR)/,$(OBJ_DIR)/,$(SOURCES:.c=.o))
DLLS := $(wildcard $(DLL_DIR)/*.dll)
BIN_DLLS := $(subst $(DLL_DIR)/,$(BIN_DIR)/,$(DLLS))

RESOURCES := $(wildcard $(RES_DIR)/chunks/*.bin)
RESOURCES += $(wildcard $(RES_DIR)/font/*.bff)
RESOURCES += $(wildcard $(RES_DIR)/shader/*.glsl)
RESOURCES += $(wildcard $(RES_DIR)/mesh/*.ric)
RESOURCES += $(wildcard $(RES_DIR)/texture/*.tga)
BIN_RESOURCES := $(subst $(RES_DIR)/,$(BIN_DIR)/,$(RESOURCES))

INCLUDE_DIRS := $(INC_DIR:%=-I%)
RESOURCE_DIRS := $(RES_SUBDIRS:%=$(BIN_DIR)/%)

# Use this with mingw libraries
#_LIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -mwindows
# Use this with MSVC libraries
_LIBS = -lSDL2main -lSDL2 -lopengl32 #-mwindows
LIBS  := -L$(LIB_DIR) $(_LIBS)

# Compiler & flags
GCC_FLAGS = -fmax-errors=3
CLANG_FLAGS = -ferror-limit=3 -fcolor-diagnostics
CC = gcc
CFLAGS = $(GCC_FLAGS) -g -MMD -Wall -Wextra -Werror -Wno-unused-function -O0 #-Og #-Wno-error=incompatible-pointer-types
LDFLAGS = # None

default: prebuild build postbuild

prebuild:
	$(info =========================================================)
	$(info #        ______            _______        _             #)
	$(info #        |  __ \ O        |__   __|      | |            #)
	$(info #        | |__| |_  ___ ___  | | ___  ___| |__          #)
	$(info #        |  _  /| |/ __/ _ \ | |/ _ \/ __| `_ \         #)
	${info #        | | \ \| | |_| (_) || |  __/ |__| | | |        #}
	$(info #        |_|  \_\_|\___\___/ |_|\___|\___|_| |_|        #)
	$(info #                                                       #)
	$(info #              Copyright 2017 Dan Bechard               #)
	$(info =========================================================)

compile-banner:
	$(info )
	$(info ---- Compile [build-compile] ---------------------------------------)
link-banner:
	$(info )
	$(info ---- Link [build-link] ---------------------------------------------)
copydlls-banner:
	$(info )
	$(info ---- Copy DLLs [postbuild-copydlls] --------------------------------)
copyres-banner:
	$(info )
	$(info ---- Copy resources [postbuild-copyres] ----------------------------)

build-compile: compile-banner $(OBJECTS)
build-link: link-banner $(BIN_EXE)
build: build-compile build-link

postbuild-copydlls: copydlls-banner $(BIN_DLLS)
postbuild-copyres: copyres-banner make-res-dirs $(BIN_RESOURCES)
postbuild: postbuild-copydlls postbuild-copyres

# Link executable
$(BIN_EXE): $(OBJECTS)
	$(foreach O,$^,$(info +  [OBJ] ${O}))
	$(info [EXE] $@)
	@$(CC) -o $@ $(OBJECTS) $(LIBS)

# Compile C files into OBJ files and generate dependencies
$(OBJECTS): $(SOURCES)
	$(foreach S,$^,$(info +  [SRC] ${S}))
	$(info [OBJ] $@)
	@$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ -c $<

# Copy DLLs
$(BIN_DLLS): $(DLLS)
	$(foreach O,$^,$(info -> [DLL] ${O}))
	@cp $^ $@

# Make resource directories
make-res-dirs:
#	$(foreach O,$(RESOURCE_DIRS),$(info [MKDIR] ${O}))
	@mkdir -p $(RESOURCE_DIRS)

# Copy resources
$(BIN_DIR)/%: $(RES_DIR)/%
	$(info [RES] $^ -> $@)
	@cp $^ $@

# Include generated dependency files
-include $(OBJECTS:.o=.d)

# Delete all generated files
.PHONY: clean
clean:
	$(info )
	$(info ---- Clean [clean] -------------------------------------------------)
	$(info [DEL] $(BIN_EXE))
	@rm -f $(EXE)
	$(info [DEL] $(OBJ_DIR)/*)
	@rm -rf $(OBJ_DIR)/*
	$(info [DEL] $(BIN_DIR)/*)
	@rm -rf $(BIN_DIR)/*

################################################################################
# Miscellaneous notes
################################################################################
#|  $@ File name of target
#|  $< Name of first prerequisite
#|  $^ Name of all prerequisites, with spaces between them
#|  $? Name of all prerequisites which have changed
#|  %  Wildcard, can be neighbored by prefix, suffix, or both
################################################################################

# Note: To use this, run e.g. `make print-SOURCES`
print-%  : ; @echo $* = $($*)
