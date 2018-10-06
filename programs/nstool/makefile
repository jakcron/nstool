# Sources
SRC_DIR = source
OBJS = $(foreach dir,$(SRC_DIR),$(subst .cpp,.o,$(wildcard $(dir)/*.cpp))) $(foreach dir,$(SRC_DIR),$(subst .c,.o,$(wildcard $(dir)/*.c)))

# External dependencies
DEPENDS = hac-hb hac es pki fnd polarssl lz4
LIB_DIR = ../../lib
LIBS = $(foreach dep,$(DEPENDS), -L"$(LIB_DIR)/lib$(dep)" -l$(dep))
INCS = $(foreach dep,$(DEPENDS), -I"$(LIB_DIR)/lib$(dep)/include")

# Compiler Settings
CXXFLAGS = -std=c++11 $(INCS) -D__STDC_FORMAT_MACROS -Wall -Wno-unused-value
CFLAGS = -std=c11 $(INCS) -Wall -Wno-unused-value
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

BIN_DIR = bin
OUTPUT = $(BIN_DIR)/$(shell basename "$(CURDIR)")

all: build

rebuild: clean build

build: $(OBJS)
	mkdir -p $(BIN_DIR)
	$(CXX) $(OBJS) $(LIBS) -o $(OUTPUT)

clean:
	rm -rf $(OBJS) $(OUTPUT) $(BIN_DIR)