main: build

rebuild: clean build

build:
	cd lib && $(MAKE) && cd ..
	cd programs && $(MAKE) && cd ..

clean:
	cd lib && $(MAKE) clean && cd ..
	cd programs && $(MAKE) clean && cd ..