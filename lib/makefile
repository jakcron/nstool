LIBS = fnd crypto nx
main: build

rebuild: clean build

build:
	@$(foreach lib,$(LIBS), cd $(lib) && $(MAKE) && cd ..;)

clean:
	@$(foreach lib,$(LIBS), cd $(lib) && $(MAKE) clean && cd ..;)