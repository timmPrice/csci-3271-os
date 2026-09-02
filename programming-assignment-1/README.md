# Queue Implementation Project

In this project you will implement a simple data structure, a queue that contains pointers.
Although the code is very simple, it is structured as a CMake project to help familiarize you with using CMake to compile and run C programs.

## Basic Prerequisites

Before you can compile and run any C programs, you'll need to have the C compiler and CMake installed on your computer.
On Linux (including WSL in Windows), this means installing the `gcc` , `make`, and `cmake` packages.
On Windows, this means selecting the "C++ Desktop Development" role for Visual Studio in the Visual Studio installer (despite the name, this bundle of tools also includes the C compiler).

## Compiling Manually

Since there are only two source-code files (and one header), it's easy enough to compile this project without using CMake: simply invoke the compiler on these two files.
On Linux, navigate to the `src` directory and run the command:

```console
~/queue-cmake-project/src/$ gcc queue.c test_queue.c -o test_queue
```

This will create an executable named `test_queue` in the current directory.

On Windows, you'll need to open a "Visual Studio Developer Command Prompt" to ensure the terminal can find the compiler.
Then navigate to the `src` directory and run the command:

```console
C:\Users\...\queue-cmake-project\src>cl.exe queue.c test_queue.c /Fe: test_queue.exe
```

This will create an executable named `test_queue.exe` in the current directory.

## Compiling with CMake

CMake uses two specially-named files, CMakeLists.txt and CMakePresets.json, to store its configuration settings.
CMakeLists.txt identifies which source code files need to be compiled and what type of output should be produced (libraries, executables, or objects), while CMakePresets.json specifies options for invoking the compiler such as whether to use "debug" flags.
These files have already been written for you in this project, so you don't need to know how to write them, but you're welcome to read them if you want.

To use CMake to compile the project on Linux, navigate to the root queue-cmake-project folder, i.e. the one that contains `src`, and run the command:

```console
~/queue-cmake-project$ cmake --preset linux-debug
```

This will create a directory named build-Debug, which you can then instruct CMake to use for building the project:

```console
~/queue-cmake-project$ cmake --build build-Debug
```

CMake will place the compiled executable, which will be named `test_queue`, inside the directory `build-Debug/src`.
Note that this keeps the compiled output separate from the source code: The code is in `src/test_queue.c`, while the compiled executable is at `build-Debug/src/test_queue`.

On Windows, using CMake is even easier because Visual Studio has built-in support for it.
Simply launch Visual Studio and choose the "Open Folder" option (rather than "Open Project"), then select the queue-cmake-project folder.
You should then be able to choose the "Windows Debug" build configuration and build the project with F6.
The compiled executable will be placed in `out\build\windows-debug\src`, and will be named test_queue.exe.
