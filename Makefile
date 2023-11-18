EXEC = cilly

# Build, bin, assets, and install directories (bin and build root directories are kept for clean)
BUILD_DIR_ROOT = build
BIN_DIR_ROOT = bin
INSTALL_DIR := ~/Desktop/$(EXEC)

# Sources (searches recursively inside the source directory)
SRC_DIR = src
SRCS := $(sort $(shell find $(SRC_DIR) -name '*.c'))

# Includes
INCLUDE_DIR = include
INCLUDES := -I$(INCLUDE_DIR)

# C preprocessor settings
CPPFLAGS = $(INCLUDES) -MMD -MP

# C compiler settings
CC = gcc
CFLAGS = 
WARNINGS = -Wall -Wpedantic -Wextra

# Linker flags
LDFLAGS =

# Libraries to link
LDLIBS = -lX11

# Target OS detection
ifeq ($(OS),Windows_NT) # OS is a preexisting environment variable on Windows
	OS = windows
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Darwin)
		OS = macos
	else ifeq ($(UNAME),Linux)
		OS = linux
	else
    	$(error OS not supported by this Makefile)
	endif
endif

# OS-specific settings
ifeq ($(OS),windows)
	# Link libgcc and libstdc++ statically on Windows
	LDFLAGS += -static-libgcc -static-libstdc++

	# Windows 32- and 64-bit common settings
	INCLUDES +=
	LDFLAGS +=
	LDLIBS +=

	ifeq ($(win32),1)
		# Windows 32-bit settings
		INCLUDES +=
		LDFLAGS +=
		LDLIBS +=
	else
		# Windows 64-bit settings
		INCLUDES +=
		LDFLAGS +=
		LDLIBS +=
	endif
else ifeq ($(OS),macos)
	# macOS-specific settings
	INCLUDES +=
	LDFLAGS +=
	LDLIBS +=
else ifeq ($(OS),linux)
	# Linux-specific settings
	INCLUDES +=
	LDFLAGS +=
	LDLIBS +=
endif


# Windows-specific default settings
ifeq ($(OS),windows)
	# Add .exe extension to executable
	EXEC := $(EXEC).exe

	ifeq ($(win32),1)
		# Compile for 32-bit
		CFLAGS += -m32
	else
		# Compile for 64-bit
		CFLAGS += -m64
	endif
endif

# OS-specific build, bin, and assets directories
BUILD_DIR := $(BUILD_DIR_ROOT)/$(OS)
BIN_DIR := $(BIN_DIR_ROOT)/$(OS)
ifeq ($(OS),windows)
	# Windows 32-bit
	ifeq ($(win32),1)
		BUILD_DIR := $(BUILD_DIR)32
		BIN_DIR := $(BIN_DIR)32
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
	CFLAGS += -O3
	CPPFLAGS += -DNDEBUG
else
	BUILD_DIR := $(BUILD_DIR)/debug
	BIN_DIR := $(BIN_DIR)/debug
	CFLAGS += -O0 -g
endif

OBJS := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
COMPDBS := $(OBJS:.o=.json)

FILES := $(shell find $(SRC_DIR) $(INCLUDE_DIR) -name '*.c' -o -name '*.h' -o -name '*.hpp' -o -name '*.inl')


.PHONY: all
all: $(BIN_DIR)/$(EXEC)

# Build executable
$(BIN_DIR)/$(EXEC): $(OBJS)
	@echo "Building executable: $@"
	@mkdir -p $(@D)
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

# Compile C++ source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "Compiling: $<"
	@mkdir -p $(@D)
	@$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -c $< -o $@

# Include automatically generated dependencies
-include $(DEPS)

# Install packaged program
.PHONY: install
install: all
	@echo "Installing $(EXEC) to $(INSTALL_DIR)"
	@mkdir -p $(INSTALL_DIR) && cp -r $(BIN_DIR)/. $(INSTALL_DIR)

# Build and run
.PHONY: run
run: all
	@echo "Starting program: $(BIN_DIR)/$(EXEC)"
	@cd $(BIN_DIR) && ./$(EXEC)

# Clean build and bin directories for all platforms
.PHONY: clean
clean:
	@echo "Cleaning $(BUILD_DIR_ROOT) and $(BIN_DIR_ROOT) directories"
	@$(RM) -r $(BUILD_DIR_ROOT)
	@$(RM) -r $(BIN_DIR_ROOT)

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
