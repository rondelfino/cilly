EXEC = cilly

# dirs
SRC_DIR = src
BUILD_DIR_ROOT = build
BIN_DIR_ROOT = bin
INSTALL_DIR = .
INCLUDE_DIR = include
INCLUDES := $(INCLUDE_DIR)

arch =

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
	# TODO: figure out how to do this
	ifeq ($(shell echo $(UNAME) | grep -c "MINGW"),1)
		CC = gcc
		ENV = mingw
	else
		ENV = win
		CC = cl
	endif
else
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


# Define shell commands and detect target architecture
ifeq ($(ENV),win)
	RM = rmdir /s /q
else
	arch := $(strip $(shell uname -m))
	RM = rm -rf
endif

# Detect target architecture
ifeq ($(ENV),win)
	# TODO: this is annoying fix this
	ifeq ($(arch),32-bit)
		arch = 32
	else ifeq ($(arch),64-bit)
		arch = 64
	endif
else 
	ifeq ($(arch),x86)
		arch = 32
	else ifeq ($(arch),x86_64)
		arch = 64
	endif
endif

# Set OS-dependent flags
# List all C source files
ifeq ($(ENV),win)
	EXEC := $(EXEC).exe
	SRCS := $(wildcard $(SRC_DIR)/*.c)
	
	CPPFLAGS = /I$(INCLUDES)
	# WARNINGS = /W4
	
	LDFLAGS = /link
else
	SRCS := $(sort $(shell find $(SRC_DIR) -name '*.c'))
	
	WARNINGS = -Wall -Wextra -Wpedantic
	CPPFLAGS = -I$(INCLUDES)
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

# OS-specific settings
ifeq ($(OS),windows)
	ifeq ($(CC),cl)
		# Windows 32- and 64-bit common settings
		# Required by SDL
		LDFLAGS += /SUBSYSTEM:WINDOWS
		LIBS = SDL2.lib SDL2main.lib shell32.lib
	else
		LDFLAGS += -mwindows
		LIBS = -lSDL2 -lmingw32 -lSDL2main 
	endif
		
	# Target architecture settings
	ifeq ($(arch),32)
		ifeq ($(CC),gcc)
			INCLUDES +=
			LDFLAGS += -Llibs/gcc32/
			LIBS +=
		else ifeq ($(CC),clang)
			INCLUDES +=
			LDFLAGS += -Llibs/clang32/
			LIBS +=
		else
			LDFLAGS += /LIBPATH:libs\cl\x86 
		endif
	else
		ifeq ($(CC),gcc)
			INCLUDES +=
			LDFLAGS += -Llibs/gcc/
			LIBS +=
		else ifeq ($(CC),clang)
			INCLUDES +=
			LDFLAGS += -Llibs/clang/
			LIBS +=
		else
			LDFLAGS += /LIBPATH:libs\cl\x64 
		endif
	endif

else ifeq ($(OS),macos)
	# macOS-specific settings
	INCLUDES +=
	LDFLAGS += -mwindows
	LIBS += -lSDL2
else ifeq ($(OS),linux)
	# Linux-specific settings
	INCLUDES +=
	LDFLAGS += -mwindows
	LIBS += -lSDL2
endif

BUILD_DIR := $(BUILD_DIR_ROOT)/$(OS)
BIN_DIR := $(BIN_DIR_ROOT)/$(OS)
ifeq ($(OS),windows)
	# Windows 32-bit
	ifeq ($(arch),32)
		BUILD_DIR := $(BUILD_DIR)$(arch)
		BIN_DIR := $(BIN_DIR)$(arch)
	# Windows 64-bit
	else
		BUILD_DIR := $(BUILD_DIR)64
		BIN_DIR := $(BIN_DIR)64
	endif
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
		CFLAGS += /Zi
	endif
endif

# Generate a list of corresponding object files
ifeq ($(ENV),win)
	OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.obj)
else
	OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
endif

.PHONY: all
all: $(BIN_DIR)/$(EXEC)

$(BIN_DIR):
ifeq ($(ENV),win)
	if not exist "$(BIN_DIR)" mkdir "$(BIN_DIR)"
else
	mkdir -p $(BIN_DIR)
endif

$(BUILD_DIR):
ifeq ($(ENV),win)
	if not exist "$(BUILD_DIR)" mkdir "$(BUILD_DIR)"
else
	mkdir -p $(BUILD_DIR)
endif
	
# Compile source files
$(BUILD_DIR)/%.obj: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) /c $< /Fo$@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@
	
# Link object files
$(BIN_DIR)/$(EXEC): $(OBJS) | $(BIN_DIR)
ifeq ($(CC),cl)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) /out:$@
else
	$(CC) $(OBJS) $(LDFLAGS) $(LIBS) -o $@
endif

# Packaged program
$(INSTALL_DIR):
ifeq ($(ENV),win)
	if not exist "$(INSTSALL_DIR)" mkdir "$(INSTALL_DIR)"
else
	mkdir -p $(INSTALL_DIR)
endif

# .PHONY: install
# install: all

.PHONY: clean
clean:
ifeq ($(ENV),win)
	if exist $(BUILD_DIR_ROOT) $(RM) $(BUILD_DIR_ROOT)
	if exist $(BIN_DIR_ROOT) $(RM) $(BIN_DIR_ROOT)
else
	$(RM) $(BUILD_DIR_ROOT)
	$(RM) $(BIN_DIR_ROOT)
endif
