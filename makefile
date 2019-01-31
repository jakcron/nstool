# Project Defines
PROJECT_NAME = nstool
PROJECT_PATH = $(CURDIR)
PROJECT_SRC_PATH = ./src
PROJECT_INCLUDE_PATH =
PROJECT_TEST_PATH = 
PROJECT_BIN_PATH = ./bin
PROJECT_DOCS_PATH = 
PROJECT_DOXYFILE_PATH = 

# Project Library Defines
PROJECT_LIB_VER_MAJOR = 0
PROJECT_LIB_VER_MINOR = 1
PROJECT_LIB_VER_PATCH = 0
PROJECT_SONAME = $(PROJECT_NAME).so.$(PROJECT_LIB_VER_MAJOR)
PROJECT_SO_FILENAME = $(PROJECT_SONAME).$(PROJECT_LIB_VER_MINOR).$(PROJECT_LIB_VER_PATCH)

# Project Dependencies
PROJECT_DEPEND = nintendo-hac-hb nintendo-hac nintendo-es nintendo-pki fnd polarssl lz4 

# Check if this is the root makefile
ifeq ($(ROOT_PROJECT_NAME),)
	export ROOT_PROJECT_NAME = $(PROJECT_NAME)
	export ROOT_PROJECT_PATH = $(PROJECT_PATH)
	export ROOT_PROJECT_DEPENDENCY_PATH = $(ROOT_PROJECT_PATH)/deps
endif

# Compiler Flags
ifneq ($(PROJECT_INCLUDE_PATH),)
	INC += -I"$(PROJECT_INCLUDE_PATH)"
endif
ifneq ($(PROJECT_DEPEND),)
	LIB += $(foreach dep,$(PROJECT_DEPEND), -L"$(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(dep)/bin" -l$(dep))
	INC += $(foreach dep,$(PROJECT_DEPEND), -I"$(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(dep)/include")
endif
CXXFLAGS = -std=c++11 $(INC) -fPIC -D__STDC_FORMAT_MACROS -Wall -Wno-unused-value
CFLAGS = -std=c11 $(INC) -fPIC -Wall -Wno-unused-value
ARFLAGS = cr -o
ifeq ($(OS),Windows_NT)
	# Windows Only Flags/Libs
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	CFLAGS += -Wno-unused-but-set-variable
	CXXFLAGS += -Wno-unused-but-set-variable
	LIB += -static
else
	UNAME = $(shell uname -s)
	ifeq ($(UNAME), Darwin)
		# MacOS Only Flags/Libs
		CFLAGS += -Wno-unused-private-field
		CXXFLAGS += -Wno-unused-private-field
		LIB +=
		ARFLAGS = rc
	else
		# *nix Only Flags/Libs
		CFLAGS += -Wno-unused-but-set-variable
		CXXFLAGS += -Wno-unused-but-set-variable
		LIB +=
	endif
endif
ifeq ($(OS),Windows_NT)
	# Windows Only Flags/Libs
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	CFLAGS += -Wno-unused-but-set-variable
	CXXFLAGS += -Wno-unused-but-set-variable
	LIBS += -static
else
	UNAME = $(shell uname -s)
	ifeq ($(UNAME), Darwin)
		# MacOS Only Flags/Libs
		CFLAGS += -Wno-unused-private-field
		CXXFLAGS += -Wno-unused-private-field
		LIBS +=
	else
		# *nix Only Flags/Libs
		CFLAGS += -Wno-unused-but-set-variable
		CXXFLAGS += -Wno-unused-but-set-variable
		LIBS +=
	endif
endif

# Object Files
SRC_DIR = $(PROJECT_SRC_PATH)
SRC_OBJ = $(foreach dir,$(SRC_DIR),$(subst .cpp,.o,$(wildcard $(dir)/*.cpp)))
TEST_DIR = $(PROJECT_TEST_PATH)
TEST_OBJ = $(foreach dir,$(TEST_DIR),$(subst .cpp,.o,$(wildcard $(dir)/*.cpp)))

# all is the default, user should specify what the default should do
#	- 'static_lib' for building static library
#	- 'shared_lib' for building shared library
#	- 'program' for building the program
#	- 'test_program' for building the test program
# These can typically be used together however *_lib and program should not be used together
all: program
	
clean: clean_objs clean_bin_dir

# Binary Directory
.PHONY: bin_dir
bin_dir:
	@mkdir -p "$(PROJECT_BIN_PATH)"

.PHONY: clean_bin_dir
clean_bin_dir:
	@rm -rf "$(PROJECT_BIN_PATH)"

.PHONY: clean_objs
clean_objs:
	@rm -f $(SRC_OBJ) $(TEST_OBJ) 

# Build Library
static_lib: $(SRC_OBJ) bin_dir
	ar $(ARFLAGS) "$(PROJECT_BIN_PATH)/$(PROJECT_NAME).a" $(SRC_OBJ)

shared_lib: $(SRC_OBJ) bin_dir
	gcc -shared -Wl,-soname,$(PROJECT_SONAME) -o "$(PROJECT_BIN_PATH)/$(PROJECT_SO_FILENAME)" $(SRC_OBJ)

# Build Program
program: $(SRC_OBJ) bin_dir
	$(CXX) $(SRC_OBJ) $(LIB) -o "$(PROJECT_BIN_PATH)/$(PROJECT_NAME)"

# Build Test Program
test_program: $(TEST_OBJ) $(SRC_OBJ) bin_dir
	$(CXX) $(TEST_OBJ) $(SRC_OBJ) $(LIB) -o "$(PROJECT_BIN_PATH)/$(PROJECT_NAME)_test"

# Documentation
docs:
	doxygen $(PROJECT_DOXYFILE_PATH)

.PHONY: clean_docs
clean_docs:
	@rm -rf $(PROJECT_DOCS_PATH)

# Dependencies
.PHONY: build_deps
build_deps:
	@$(foreach lib,$(PROJECT_DEPEND), cd $(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(lib) && $(MAKE) static_lib && cd $(PROJECT_PATH);)

.PHONY: clean_deps
clean_deps:
	@$(foreach lib,$(PROJECT_DEPEND), cd $(ROOT_PROJECT_DEPENDENCY_PATH)/lib$(lib) && $(MAKE) clean && cd $(PROJECT_PATH);)