# Building
## Git Submodules
This project makes use of git submodules to import dependencies into the source tree.
After cloning this repository using git, prior to building NSTool the dependencies need to be downloaded.
Run these two commands to initialise and download the dependencies:
```
git submodule init
git submodule update
```

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

## Native Windows - Visual Studio
### Requirements
* [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) 2015 / 2017 / 2019

### Compiling NSTool
* Open `build/visualstudio/nstool.sln` in Visual Studio
* Select Target (e.g `Debug`|`Release` & `x86`|`x64`)
* Navigate to `Build`->`Build Solution`