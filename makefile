PROJECT_DIR = $(CURDIR)
LIB_DIR = $(PROJECT_DIR)/lib
PROGRAM_DIR = $(PROJECT_DIR)/programs
BIN_DIR = $(PROJECT_DIR)/bin

LIBS = libpolarssl liblz4 libfnd libes libpki libhac libhac-hb
PROGS = nstool

main: build

rebuild: clean build

build:
	mkdir -p $(BIN_DIR)
	@$(foreach lib,$(LIBS), cd $(LIB_DIR)/$(lib) && $(MAKE) && cd $(PROJECT_DIR);)
	@$(foreach prog,$(PROGS), cd $(PROGRAM_DIR)/$(prog) && $(MAKE) && cd $(PROJECT_DIR) && cp -r $(PROGRAM_DIR)/$(prog)/bin/* $(BIN_DIR);)
	

clean:
	rm -rf $(BIN_DIR)
	@$(foreach lib,$(LIBS), cd $(LIB_DIR)/$(lib) && $(MAKE) clean && cd $(PROJECT_DIR);)
	@$(foreach prog,$(PROGS), cd $(PROGRAM_DIR)/$(prog) && $(MAKE) clean && cd $(PROJECT_DIR);)