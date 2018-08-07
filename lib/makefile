LIBS = libfnd libpolarssl liblz4 libcrypto libcompress libes libpki libhac libhac-hb
main: build

rebuild: clean build

build:
	@$(foreach lib,$(LIBS), cd $(lib) && $(MAKE) && cd ..;)

clean:
	@$(foreach lib,$(LIBS), cd $(lib) && $(MAKE) clean && cd ..;)