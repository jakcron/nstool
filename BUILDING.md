# Building
## Linux (incl. Windows Subsystem for Linux) & MacOS - Makefile
### Requirements
* `make`
* Terminal access
* Typical GNU compatible development tools (e.g. `clang`, `g++`, `c++`, `ar` etc) with __C++11__ support

### Using Makefile
* `make` (default) - Compile program
	* Compiling the program requires local dependencies to be compiled via `make deps` beforehand
* `make clean` - Remove executable and object files
* `make deps` - Compile locally included dependency libraries
* `make clean_deps` - Remove compiled library binaries and object files

## Native Win32 - Visual Studio
### Requirements
* [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) 2015 or 2017

### Compiling NSTool
* Open `build/visualstudio/nstool.sln` in Visual Studio
* Select Target (e.g `Debug`|`Release` & `x86`|`x64`)
* Navigate to `Build`->`Build Solution`
