PROGS = ncatool

main: build

rebuild: clean build

build:
	mkdir -p "../bin"
	@$(foreach prog,$(PROGS), cd $(prog) && $(MAKE) && cd ..;)

clean:
	@$(foreach prog,$(PROGS), cd $(prog) && $(MAKE) clean && cd ..;)
	#rm -rf "../bin"