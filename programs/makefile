PROGS = ncatool npdmtool pfstool tiktool xcitool
BIN_DIR = "../bin"

main: build

rebuild: clean build

build:
	mkdir -p $(BIN_DIR)
	@$(foreach prog,$(PROGS), cd $(prog) && $(MAKE) && cd .. && cp -r $(prog)/bin/* $(BIN_DIR);)

clean:
	@$(foreach prog,$(PROGS), cd $(prog) && $(MAKE) clean && cd ..;)
	rm -rf $(BIN_DIR)