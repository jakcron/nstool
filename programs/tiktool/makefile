# Sources
SRC_DIR = .
OBJS = $(foreach dir,$(SRC_DIR),$(subst .cpp,.o,$(wildcard $(dir)/*.cpp))) $(foreach dir,$(SRC_DIR),$(subst .c,.o,$(wildcard $(dir)/*.c)))

#local dependencies
DEPENDS = es crypto fnd

LIB_DIR = ../../lib

LIBS = -L"$(LIB_DIR)" $(foreach dep,$(DEPENDS), -l"$(dep)")
INCS = -I"$(LIB_DIR)/"

OUTPUT = ../../bin/$(shell basename $(CURDIR))

# Compiler Settings
CXXFLAGS = -std=c++11 $(INCS) -D__STDC_FORMAT_MACROS -Wall -Wno-unused-but-set-variable -Wno-unused-value
ifeq ($(OS),Windows_NT)
	# Windows Only Flags/Libs
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	CFLAGS += 
	CXXFLAGS += 
	LIBS += -static
else
	# *nix Only Flags/Libs
	CFLAGS += 
	CXXFLAGS += 
	LIBS +=
endif

all: build

rebuild: clean build

build: $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -o $(OUTPUT)

clean:
	rm -rf $(OBJS) $(OUTPUT)