################################################################################
#|  Project: RicoTech
#|     Date: 2018-02-05
#|   Author: Dan Bechard
################################################################################

# Directories
SRC_DIR := src
INC_DIR := include ../dlb/include ../rico/include
OBJ_DIR := obj
LIB_DIR := lib ../rico/lib
DLL_DIR := lib
BIN_DIR := bin
RES_DIR := res

SRC_FILES := chet.c
#LIBS = -lmingw32 -lSDL2main -lSDL2 -lopengl32 -mwindows # Use this with MinGW libs
LIBS = -lrico -lSDL2main -lSDL2 -lopengl32 -lopenal32 # Use this with MSVC libs
LIB_DIRS := $(LIB_DIR:%=-L%)
LIBS := $(LIB_DIRS) $(LIBS)
BIN_EXE := $(BIN_DIR)/chet.exe
RES_SUBDIRS := audio font mesh packs shader texture

INCLUDE_DIRS := $(INC_DIR:%=-I%)

SOURCES := $(SRC_FILES:%=$(SRC_DIR)/%)
OBJECTS := $(subst $(SRC_DIR)/,$(OBJ_DIR)/,$(SOURCES:.c=.o))
DEPS := $(OBJECTS:.o=.d)

RESOURCE_DIRS := $(RES_SUBDIRS:%=$(BIN_DIR)/%)
RESOURCES := $(wildcard $(RES_DIR)/chunks/*.bin)
RESOURCES += $(wildcard $(RES_DIR)/font/*.bff)
RESOURCES += $(wildcard $(RES_DIR)/shader/*.glsl)
RESOURCES += $(wildcard $(RES_DIR)/mesh/*.obj)
RESOURCES += $(wildcard $(RES_DIR)/texture/*.tga)
RESOURCES += $(wildcard $(RES_DIR)/packs/*.pak)
RESOURCES += $(wildcard $(RES_DIR)/audio/*.ric)
BIN_RESOURCES := $(subst $(RES_DIR)/,$(BIN_DIR)/,$(RESOURCES))
DLLS := $(wildcard $(DLL_DIR)/*.dll)
BIN_DLLS := $(subst $(DLL_DIR)/,$(BIN_DIR)/,$(DLLS))

# Compiler & flags
# Optimization flags:
# -O0 Disabled
# -Og Debug
# -O2 Release
# -O3 Extreme (Careful, might make EXE bigger or invoke undefined behavior!)
SHARED_FLAGS = -std=c99 -g -MMD -O0 -Wall -Wextra -Werror -Wno-unused-function #-Wno-missing-field-initializers -Wno-missing-braces -Wno-deprecated-declarations #-Wno-error=incompatible-pointer-types
GCC_FLAGS = -fmax-errors=3
CLANG_FLAGS = -ferror-limit=3 -fcolor-diagnostics -Wno-macro-redefined
CC = gcc #-v
CFLAGS = $(GCC_FLAGS) $(SHARED_FLAGS)
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
	$(info #              Copyright 2018  Dan Bechard              #)
	$(info =========================================================)

compile-banner:
	$(info )
	$(info ---- Compile [build-compile] ---------------------------------------)
link-banner:
	$(info )
	$(info ---- Link [build-link] ---------------------------------------------)
copyres-banner:
	$(info )
	$(info ---- Copy resources [postbuild-copyres] ----------------------------)

build-compile: compile-banner
build-link: link-banner $(BIN_EXE)
build: make-build-dirs build-compile build-link

postbuild-copyres: copyres-banner make-res-dirs $(BIN_RESOURCES) $(BIN_DLLS)
postbuild: postbuild-copyres

################################################################################
# Link executable
$(BIN_EXE): $(OBJECTS)
	$(info [EXE] $@)
	$(foreach O,$^,$(info +  [OBJ] ${O}))
	@$(CC) -o $@ $^ $(LIBS)

# Compile C files into OBJ files and generate dependencies
$(OBJECTS): $(SOURCES)
	$(info [OBJ] $@)
	$(foreach S,$^,$(info +  [SRC] ${S}))
	@$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ -c $<

make-build-dirs:
	@mkdir -p $(BIN_DIR) $(OBJ_DIR)

make-res-dirs:
#	$(foreach O,$(RESOURCE_DIRS),$(info [MKDIR] ${O}))
	@mkdir -p $(RESOURCE_DIRS)

# Copy resources
$(BIN_DIR)/%: $(RES_DIR)/%
	$(info [RES] $^ -> $@)
	@cp $^ $@

# Copy dlls
$(BIN_DIR)/%: $(DLL_DIR)/%
	$(info [DLL] $^ -> $@)
	@cp $^ $@

# Include generated dependency files
-include $(DEPS)

# Delete all generated files
.PHONY: clean
clean:
	$(info )
	$(info ---- Clean [clean] -------------------------------------------------)
	$(info [DEL] $(BIN_EXE))
	-@rm $(BIN_EXE)
	$(info [DEL] $(OBJ_DIR)/*)
	-@rm -r $(OBJ_DIR)/*
	$(info [DEL] $(BIN_DIR)/*)
	-@rm -r $(BIN_DIR)/*

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
