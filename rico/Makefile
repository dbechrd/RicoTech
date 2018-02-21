################################################################################
#|  Project: RicoTech
#|     Date: 2016-11-03
#|   Author: Dan Bechard
################################################################################

# Directories
SRC_DIR := src
INC_DIR := include include/SDL include/GL include/AL ../dlb/include
OBJ_DIR := obj
LIB_DIR := lib

SRC_FILE := rico_main.c
OUT_FILE := $(LIB_DIR)/rico.lib

INCLUDE_DIRS := $(INC_DIR:%=-I%)

#SOURCES := $(wildcard $(SRC_DIR)/*.c)
#SOURCES := $(filter-out $(SRC_DIR)/notes.c, $(SOURCES))
#OBJECTS := $(patsubst $(SRC_DIR)/%,$(OBJ_DIR)/%,$(SOURCES:.c=.o))
SOURCES := $(SRC_FILE:%=$(SRC_DIR)/%)
OBJECTS := $(subst $(SRC_DIR)/,$(OBJ_DIR)/,$(SOURCES:.c=.o))
DEPS := $(OBJECTS:.o=.d)

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

default: prebuild build

copyright:
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

prebuild: copyright make-build-dirs
build: $(OUT_FILE)

################################################################################
# Link library
$(OUT_FILE): $(OBJECTS)
	$(info [LIB] $@)
	$(foreach O,$^,$(info +  [OBJ] ${O}))
	@ar rcs $@ $^

# Compile C files into OBJ files and generate dependencies
$(OBJECTS): $(SOURCES)
	$(info [OBJ] $@)
	$(foreach S,$^,$(info +  [SRC] ${S}))
	@$(CC) $(CFLAGS) $(INCLUDE_DIRS) -o $@ -c $<

make-build-dirs:
	@mkdir -p $(LIB_DIR) $(OBJ_DIR)

# Include generated dependency files
-include $(DEPS)

# Delete all generated files
.PHONY: clean
clean:
	$(info )
	$(info ---- Clean [clean] -------------------------------------------------)
	$(info [DEL] $(OUT_FILE))
	-@rm $(OUT_FILE)
	$(info [DEL] $(OBJ_DIR)/*)
	-@rm -r $(OBJ_DIR)
	$(info [DEL] $(LIB_DIR)/*)
	-@rm -r $(LIB_DIR)

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
