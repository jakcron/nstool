# Sources
SRC_DIR = source
OBJS = $(foreach dir,$(SRC_DIR),$(subst .cpp,.o,$(wildcard $(dir)/*.cpp))) $(foreach dir,$(SRC_DIR),$(subst .c,.o,$(wildcard $(dir)/*.c)))

# External dependencies
DEPENDS = fnd crypto
LIB_DIR = ..
INCS = -I"include" $(foreach dep,$(DEPENDS), -I"$(LIB_DIR)/lib$(dep)/include")


# Compiler Settings
CXXFLAGS = -std=c++11 $(INCS) -D__STDC_FORMAT_MACROS -Wall -Wno-unused-but-set-variable -Wno-unused-value
CFLAGS = -std=c11 $(INCS) -Wall -Wno-unused-but-set-variable -Wno-unused-value
ARFLAGS = cr -o
ifeq ($(OS),Windows_NT)
	# Windows Only Flags/Libs
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	CFLAGS += 
	CXXFLAGS += 
else
	UNAME = $(shell uname -s)
	ifeq ($(UNAME), Darwin)
		# MacOS Only Flags/Libs
		CFLAGS += 
		CXXFLAGS += 
		ARFLAGS = rc
	else
		# *nix Only Flags/Libs
		CFLAGS += 
		CXXFLAGS += 
	endif

endif

# Output
OUTPUT = $(shell basename $(CURDIR)).a

main: build

rebuild: clean build

build: $(OBJS)
	ar $(ARFLAGS) $(OUTPUT) $(OBJS)

clean:
	rm -rf $(OUTPUT) $(OBJS)
