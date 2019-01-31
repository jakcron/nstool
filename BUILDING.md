# Building
## Linux (incl. Windows Subsystem for Linux) & MacOS - Makefile
### Requirements
* `make`
* Terminal access
* Typical GNU compatible development tools (e.g. `clang`, `g++`, `c++`, `ar` etc) with __C++11__ support

### Using Makefile
* `make` (default) - Compile program
* `make clean` - Remove executable and object files

## Native Win32 - Visual Studio
### Requirements
* [Visual Studio Community](https://visualstudio.microsoft.com/vs/community/) 2015 or 2017

### Compiling NSTool
* Open `build/visualstudio/nstool.sln` in Visual Studio
* Select Target (e.g `Debug`|`Release` & `x64`)
* Navigate to `Build`->`Build Solution`
