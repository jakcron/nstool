# C++/C Recursive Project Makefile 
# (c) Jack
# Version 2

# Project Defines
PROJECT_NAME = nstool
PROJECT_VER_MAJOR = 1
PROJECT_VER_MINOR = 0
PROJECT_VER_PATCH = 5

# Project Relative Paths
PROJECT_PATH = $(CURDIR)
PROJECT_SRC_PATH = src
PROJECT_SRC_SUBDIRS = $(PROJECT_SRC_PATH)
#PROJECT_INCLUDE_PATH = include
#PROJECT_TESTSRC_PATH = test
#PROJECT_TESTSRC_SUBDIRS = $(PROJECT_TESTSRC_PATH)
PROJECT_BIN_PATH = bin
#PROJECT_DOCS_PATH = docs                                                                                                                                    
#PROJECT_DOXYFILE_PATH = Doxyfile

# Determine if the root makefile has been established, and if not establish this makefile as the root makefile
ifeq ($(ROOT_PROJECT_NAME),)
	export ROOT_PROJECT_NAME = $(PROJECT_NAME)
	export ROOT_PROJECT_PATH = $(PROJECT_PATH)
	export ROOT_PROJECT_DEPENDENCY_PATH = $(ROOT_PROJECT_PATH)/deps
endif

# Shared Library Definitions
PROJECT_SONAME = $(PROJECT_NAME).so.$(PROJECT_VER_MAJOR)
PROJECT_SO_FILENAME = $(PROJECT_SONAME).$(PROJECT_VER_MINOR).$(PROJECT_VER_PATCH)

# Project Dependencies
PROJECT_DEPEND_LOCAL = nintendo-hac-hb nintendo-hac nintendo-es nintendo-pki fnd polarssl lz4 
PROJECT_DEPEND_EXTERNAL =

# Generate compiler flags for including project include path
ifneq ($(PROJECT_INCLUDE_PATH),)
	INC += -I"$(PROJECT_INCLUDE_PATH)"
endif

# Generate compiler flags for local included dependencies
ifneq ($(PROJECT_DEPEND_LOCAL),)
	LIB += $(foreach dep,$(PROJECT_DEPEND_LOCAL), -L"$(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(dep)/bin" -l$(dep))
	INC += $(foreach dep,$(PROJECT_DEPEND_LOCAL), -I"$(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(dep)/include")
endif

# Generate compiler flags for external dependencies
ifneq ($(PROJECT_DEPEND_EXTERNAL),)
	LIB +=  $(foreach dep,$(PROJECT_DEPEND_EXTERNAL), -l$(dep))
endif

# Generate compiler flags for program/library version
PROJECT_VER_DEF = -DPROJECT_VER_MAJOR=$(PROJECT_VER_MAJOR) -DPROJECT_VER_MINOR=$(PROJECT_VER_MINOR) -DPROJECT_VER_PATCH=$(PROJECT_VER_PATCH)

# Detect Platform
ifeq ($(PROJECT_PLATFORM),)
	ifeq ($(OS), Windows_NT)
		export PROJECT_PLATFORM = WIN32
	else
		UNAME = $(shell uname -s)
		ifeq ($(UNAME), Darwin)
			export PROJECT_PLATFORM = MACOS
		else
			export PROJECT_PLATFORM = GNU
		endif
	endif
endif

# Generate platform specific compiler flags
ifeq ($(PROJECT_PLATFORM), WIN32)
	# Windows Flags/Libs
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	WARNFLAGS = -Wall -Wno-unused-value -Wno-unused-but-set-variable
	INC +=
	LIB += -static
	ARFLAGS = cr -o
else ifeq ($(PROJECT_PLATFORM), GNU)
	# GNU/Linux Flags/Libs
	#CC = 
	#CXX =
	WARNFLAGS = -Wall -Wno-unused-value -Wno-unused-but-set-variable
	INC +=
	LIB +=
	ARFLAGS = cr -o
else ifeq ($(PROJECT_PLATFORM), MACOS)
	# MacOS Flags/Libs
	#CC = 
	#CXX =
	WARNFLAGS = -Wall -Wno-unused-value -Wno-unused-private-field
	INC +=
	LIB +=
	ARFLAGS = rc	
endif

# Compiler Flags
CXXFLAGS = -std=c++11 $(INC) $(PROJECT_VER_DEF) $(WARNFLAGS) -fPIC
CFLAGS = -std=c11 $(INC) $(PROJECT_VER_DEF) $(WARNFLAGS) -fPIC

# Object Files
SRC_OBJ = $(foreach dir,$(PROJECT_SRC_SUBDIRS),$(subst .cpp,.o,$(wildcard $(dir)/*.cpp))) $(foreach dir,$(PROJECT_SRC_SUBDIRS),$(subst .c,.o,$(wildcard $(dir)/*.c)))
TESTSRC_OBJ = $(foreach dir,$(PROJECT_TESTSRC_SUBDIRS),$(subst .cpp,.o,$(wildcard $(dir)/*.cpp))) $(foreach dir,$(PROJECT_TESTSRC_SUBDIRS),$(subst .c,.o,$(wildcard $(dir)/*.c)))

# all is the default, user should specify what the default should do
#	- 'static_lib' for building static library
#	- 'shared_lib' for building shared library
#	- 'program' for building the program
#	- 'test_program' for building the test program
# These can typically be used together however *_lib and program should not be used together
all: program
	
clean: clean_object_files remove_binary_dir

# Object Compile Rules
%.o: %.c
	@echo CC $<
	@$(CC) $(CFLAGS) -c $< -o $@ 

%.o: %.cpp
	@echo CXX $<
	@$(CXX) $(CXXFLAGS) -c $< -o $@ 

# Binary Directory
.PHONY: create_binary_dir
create_binary_dir:
	@mkdir -p "$(PROJECT_BIN_PATH)"

.PHONY: remove_binary_dir
remove_binary_dir:
ifneq ($(PROJECT_BIN_PATH),)
	@rm -rf "$(PROJECT_BIN_PATH)"
endif

.PHONY: clean_object_files
clean_object_files:
	@rm -f $(SRC_OBJ) $(TESTSRC_OBJ)

# Build Library
static_lib: $(SRC_OBJ) create_binary_dir
	@echo LINK $(PROJECT_BIN_PATH)/$(PROJECT_NAME).a
	@ar $(ARFLAGS) "$(PROJECT_BIN_PATH)/$(PROJECT_NAME).a" $(SRC_OBJ)

shared_lib: $(SRC_OBJ) create_binary_dir
	@echo LINK $(PROJECT_BIN_PATH)/$(PROJECT_SO_FILENAME)
	@gcc -shared -Wl,-soname,$(PROJECT_SONAME) -o "$(PROJECT_BIN_PATH)/$(PROJECT_SO_FILENAME)" $(SRC_OBJ)

# Build Program
program: $(SRC_OBJ) create_binary_dir
	@echo LINK $(PROJECT_BIN_PATH)/$(PROJECT_NAME)
	@$(CXX) $(SRC_OBJ) $(LIB) -o "$(PROJECT_BIN_PATH)/$(PROJECT_NAME)"

# Build Test Program
test_program: $(TESTSRC_OBJ) $(SRC_OBJ) create_binary_dir
ifneq ($(PROJECT_TESTSRC_PATH),)
	@echo LINK $(PROJECT_BIN_PATH)/$(PROJECT_NAME)_test
	@$(CXX) $(TESTSRC_OBJ) $(SRC_OBJ) $(LIB) -o "$(PROJECT_BIN_PATH)/$(PROJECT_NAME)_test"
endif

# Documentation
.PHONY: docs
docs:
ifneq ($(PROJECT_DOCS_PATH),)
	doxygen "$(PROJECT_DOXYFILE_PATH)"
endif

.PHONY: clean_docs
clean_docs:
ifneq ($(PROJECT_DOCS_PATH),)
	@rm -rf "$(PROJECT_DOCS_PATH)"
endif

# Dependencies
.PHONY: deps
deps:
	@$(foreach lib,$(PROJECT_DEPEND_LOCAL), cd "$(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(lib)" && $(MAKE) static_lib && cd "$(PROJECT_PATH)";)

.PHONY: clean_deps
clean_deps:
	@$(foreach lib,$(PROJECT_DEPEND_LOCAL), cd "$(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(lib)" && $(MAKE) clean && cd "$(PROJECT_PATH)";)