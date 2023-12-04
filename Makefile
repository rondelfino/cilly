EXEC = cilly

# dirs
SRC_DIR = src
SRCS := $(wildcard $(SRC_DIR)/*.c)

INCLUDE_DIR = include

BUILD_DIR_ROOT = build
BIN_DIR_ROOT = bin

EXTERNAL_DIR_ROOT = external
INSTALL_DIR := $(EXEC)

# external
SDL_DIR := $(EXTERNAL_DIR_ROOT)/SDL2
SDL_INCLUDE_DIR := $(SDL_DIR)/include
SDL_LIB_DIR := $(SDL_DIR)/lib

# compiler
CC =
CFLAGS =
CPPFLAGS =
WARNINGS = 

# linker
LDFLAGS =
LIBS =

# target arch
arch =

UNAME := $(shell uname -s)
# Target OS detection
ifeq ($(OS),Windows_NT) # OS is a preexisting environment variable on Windows
	OS = windows
	
	# Detect target architecture
	arch := $(strip $(shell wmic os get osarchitecture | FINDSTR bit))
	
	# Detect environment
	# TODO: figure out how to do this without using ENV
	ifneq (,$(findstring MINGW,$(UNAME)))
		CC = gcc
		ENV = mingw
	else
		ENV = win
		CC = cl
	endif
else
	arch := $(strip $(shell uname -m | grep 64))
	ifeq ($(UNAME),Darwin)
		OS = macos
		CC = clang
	else ifeq ($(UNAME),Linux)
		OS = linux
		CC = gcc
	else
    	$(error OS not supported by this Makefile)
	endif
endif

# Define shell commands
ifeq ($(ENV),win)
	RM = rmdir /s /q
else
	RM = rm -rf
endif

# Set arch
ifneq (,$(findstring 64,$(arch)))
	arch = 64
else
	arch = 32
endif

# Set OS-dependent flags
# List all C source files
ifeq ($(ENV),win)
	EXEC := $(EXEC).exe
	INCLUDES := /I$(INCLUDE_DIR)
	CPPFLAGS = $(INCLUDES)
	LDFLAGS = /link
else
	WARNINGS = -Wall -Wextra -Wpedantic
	INCLUDES := -I$(INCLUDE_DIR)
	CPPFLAGS = $(INCLUDES) -MMD -MP
endif

# Set target arch compilation
ifeq ($(arch),32)
	ifeq ($(ENV),win)
		LDFLAGS += /MACHINE:x86
	else
		LDFLAGS += -m32
	endif
else ifeq ($(arch),64)
	ifeq ($(ENV),win)
		LDFLAGS += /MACHINE:x64
	else
		LDFLAGS += -m64
	endif
endif


SDL_LIB := $(SDL_LIB_DIR)/$(OS)/$(CC)/$(arch)
# OS-specific settings
ifeq ($(OS),windows)
	ifeq ($(ENV),win)
		# Windows 32- and 64-bit common settings
		# Required by SDL
		INCLUDES += /I$(SDL_INCLUDE_DIR)
		LDFLAGS += /SUBSYSTEM:WINDOWS
		LIBS = SDL2.lib SDL2main.lib shell32.lib
		LDFLAGS += /LIBPATH:$(SDL_LIB)
	else
		LDFLAGS += -mwindows
		LIBS = -lSDL2 -lmingw32 -lSDL2main 
		LDFLAGS += -L$(SDL_LIB)
	endif
else ifeq ($(OS),macos)
	# macOS-specific settings
	INCLUDES +=
	LDFLAGS += 
	LIBS += -lSDL2
else ifeq ($(OS),linux)
	# Linux-specific settings
	INCLUDES += -I/usr/include/SDL2
	CFLAGS += -D_REENTRANT
	LDFLAGS += -L/usr/lib
	LIBS += -lSDL2
endif

BUILD_DIR := $(BUILD_DIR_ROOT)/$(OS)
BIN_DIR := $(BIN_DIR_ROOT)/$(OS)
ifeq ($(OS),windows)
	# Windows 32-bit
	BUILD_DIR := $(BUILD_DIR)$(arch)
	BIN_DIR := $(BIN_DIR)$(arch)
endif

# Debug (default) and release modes settings
ifeq ($(release),1)
	BUILD_DIR := $(BUILD_DIR)/release
	BIN_DIR := $(BIN_DIR)/release
	ifneq ($(CC),cl)
		CFLAGS += -O3
		CPPFLAGS += -DNDEBUG
	else
		CFLAGS += /O2
	endif
else
	BUILD_DIR := $(BUILD_DIR)/debug
	BIN_DIR := $(BIN_DIR)/debug
	ifneq ($(CC),cl)
		CFLAGS += -O0 -g
	else
		CFLAGS += /Zi /Od
	endif
endif

# Objects and dependencies
ifeq ($(ENV),win)
	OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.obj)
	DEPS := $(OBJS:.obj=.d)	
	COMPDBS := $(OBJS:.obj=.json)
else
	OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
	DEPS := $(OBJS:.o=.d)	
	COMPDBS := $(OBJS:.obj=.json)
endif

.PHONY: all
all: $(BIN_DIR)/$(EXEC)

# Compile source files
# Move msvc debug info to build
$(BUILD_DIR)/%.obj: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $(CPPFLAGS) /c $< /Fo$@
	if not exist "$(@D)" mkdir "$(@D)"
	if exist *.pdb move *.pdb $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	
# Link object files
$(BIN_DIR)/$(EXEC): $(OBJS)
ifeq ($(CC),cl)
	if not exist "$(@D)" mkdir "$(@D)"
	$(CC) $^ $(LDFLAGS) $(LIBS) /out:$@
else
	mkdir -p $(@D)
	$(CC) $^ $(LDFLAGS) $(LIBS) -o $@
endif

# Include automatically generated dependencies
-include $(DEPS)

# Packages executable to with dependencies to install directory
.PHONY: install
install: all copyassets
	@echo "Packaging program to $(INSTALL_DIR)"
ifeq ($(ENV),win)
	if not exist "$(INSTALL_DIR)" mkdir "$(INSTALL_DIR) 
	copy "$(BIN_DIR)" "$(INSTALL_DIR)"
else
	mkdir -p $(INSTALL_DIR) && cp -r $(BIN_DIR)/. $(INSTALL_DIR)
endif

# Copies SDL dependencies and roms to the executable's directory
.PHONY: copyassets
copyassets:
ifeq ($(ENV),win)
	copy "$(SDL_LIB)\*.dll" "$(BIN_DIR)"
	xcopy /e /y /i roms $(INSTALL_DIR)\roms
else ifeq ($(ENV),mingw)
	cp -r $(SDL_LIB)\*.dll $(BIN_DIR)
	cp -r roms $(BIN_DIR)
else
	cp -r roms $(BIN_DIR)
endif

.PHONY: clean
clean:
ifeq ($(ENV),win)
	if exist $(BUILD_DIR_ROOT) $(RM) $(BUILD_DIR_ROOT)
	if exist $(BIN_DIR_ROOT) $(RM) $(BIN_DIR_ROOT)
	if exist *.pdb del *.pdb
else
	$(RM) $(BUILD_DIR_ROOT)
	$(RM) $(BIN_DIR_ROOT)
endif

# TODO: support cmd/powershell
.PHONY: compdb
compdb: $(BUILD_DIR_ROOT)/compile_commands.json

# Generate JSON compilation database (compile_commands.json) by merging fragments
$(BUILD_DIR_ROOT)/compile_commands.json: $(COMPDBS)
	@echo "Generating: $@"
	@mkdir -p $(@D)
	@printf "[\n" > $@
	@sed -e '$$s/$$/,/' -s $(COMPDBS) | sed -e '$$s/,$$//' -e 's/^/    /' >> $@
	@printf "]\n" >> $@

# Generate JSON compilation database fragments from source files
$(BUILD_DIR)/%.json: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	@printf "\
	{\n\
	    \"directory\": \"$(CURDIR)\",\n\
	    \"command\": \"$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -c $< -o $(basename $@).o\",\n\
	    \"file\": \"$<\"\n\
	}\n" > $@

# TODO: support cmd/powershell
.PHONY: help
help:
	@printf "\
	Usage: make target... [options]...\n\
	\n\
	Targets:\n\
	  all             Build executable (debug mode by default) (default target)\n\
	  install         Install packaged program to desktop (debug mode by default)\n\
	  copyassets      Copy assets to executable directory for selected platform and configuration\n\
	  clean           Clean build and bin directories (all platforms)\n\
	  compdb          Generate JSON compilation database (compile_commands.json)\n\
	  help            Print this information\n\
	\n\
	Options:\n\
	  release=1       Run target using release configuration rather than debug\n\
	  arch=32/64      Build in 32-bit or 64-bit mode\n\
	\n\
	Note: the above options affect the all, install, run, copyassets, compdb, and printvars targets\n"
