# C++ Web Game Template
Basic game web template using raylib

### Dependencies
CMake, Ninja

### How to use
If you do not have `emsdk` installed globally, run `./emsdk-init.sh`

Then, if building for web, run `./build-init.sh -w`
else simply run `./build-init.sh`

Then `./build.sh -w` for web build, or `./build.sh` for native

### Running the web build
A script `./run-web.sh` can be used to start a basic HTTP server using Python.

This can be run once and kept in the background - simply refresh the web page after each `./build.sh -w` to see changes
