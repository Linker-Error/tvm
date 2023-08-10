# TVM
TVM, or Tiny Virtual Machine is an extremely small 64-bit VM with it's own unique (imaginary) architecture.
------------------------------------------------------------------------------------
## Features
TVM features console i/o, subroutines, 64 binary flags, basic arithmetic/bitwise operations, complex conditionals based on boolean operations on the 64 flags, roughly 110 opcodes, and 5 registers. It is a single c file and requires no external libraries.

## Usage
The VM takes input as any binary file and loads it into iternal program space which can be defined by the user and begins execution immediately. The VM should be run with 3 arguments in this format:
```
tvm.exe [Program] [Program space to allocate] [RAM to allocate]
```
You may use TVM however you want for personal or commercial use. It is intended to aid in projects such as JIT such as Java compiled languages and it (probably) compiles on multiple platforms.

## Plans
I plan on possibly adding graphics with SDL or Raylib, but since those are not standard C libraries I will keep the original source in this repository as well. I may also convert the VM to a single-file (not including the header file) C library for an even easier use in personal projects. I may also release 8-bit and 16-bit versions of TVM for simpler retro-style projects. If so, I will only release the binaries as the source can easily be modified to host an 8-bit machine.
