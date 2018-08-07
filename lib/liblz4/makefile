# Sources
SRC_DIR = source
OBJS = $(foreach dir,$(SRC_DIR), $(foreach dir,$(SRC_DIR),$(subst .c,.o,$(wildcard $(dir)/*.c)))

# External dependencies
INCS = -I"include"


# Compiler Settings
CFLAGS = -std=c11 $(INCS) -Wall -Wno-unused-value
ARFLAGS = cr -o
ifeq ($(OS),Windows_NT)
	# Windows Only Flags/Libs
	CC = x86_64-w64-mingw32-gcc
	CXX = x86_64-w64-mingw32-g++
	CFLAGS += -Wno-unused-but-set-variable
	CXXFLAGS += -Wno-unused-but-set-variable
else
	UNAME = $(shell uname -s)
	ifeq ($(UNAME), Darwin)
		# MacOS Only Flags/Libs
		CFLAGS += -arch x86_64 -Wno-unused-private-field
		CXXFLAGS += -arch x86_64 -Wno-unused-private-field
		ARFLAGS = rc
	else
		# *nix Only Flags/Libs
		CFLAGS += -Wno-unused-but-set-variable
		CXXFLAGS += -Wno-unused-but-set-variable
	endif
endif

# Output
OUTPUT = $(shell basename $(CURDIR)).a

main: build

rebuild: clean build

build: $(OBJS)
	ar cr -o $(OUTPUT) $(OBJS)

clean:
	rm -rf $(OUTPUT) $(OBJS)