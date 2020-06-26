## How to build
### Requirements
- CMake 3.13+
- Compiler with C++17 support ( tested with gcc-8 / Visual Studio 15 2017)

### Linux (Ubuntu)
The easiest way to build is using *CMake*. Make sure that CMake uses *g++-8* (or later) as a 
C++ compiler.
Just open terminal in the project directory and run:

`cmake`  

Followed by:

`make`

Be sure to have all system dependencies installed. To be precise you should check *dockerfile* that is used to setup CI environment. It can be found in 
root project directory in file `Dockerfile`. Roughly you need to have installed following packages.  

`apt-get install -y libx11-dev libfreetype6 libfreetype6-dev libxft-dev libxcursor-dev`

### Windows
Again the easiest way is to use *CMake*. Command below is for *Visual Studio 15 2017* but you can use any version that supports *C++17*.

`cmake -G "Visual Studio 15 2017 Win64"`

Then you can work directly with generated Visual Studio solution.

### Info when building for the first time
First time CMake is run the dependencies are going to be downloaded. 
This may take some time depending on your connection. The first time you build 
the project dependencies are also going to get build so it might also take quite 
a while to finish. Consecutive builds should be much quicker.

If you have any problems you should post an issue.