cmake_minimum_required(VERSION 3.29)
project(VMM)

set(CMAKE_CXX_STANDARD 20)

add_executable(VMM main.cpp
        Hypervisor.cpp
        Hypervisor.h
        VM.cpp
        VM.h
        Util.cpp
        Util.h
        CPU.cpp
        CPU.h
        Instruction.h
        VMM.cc
)
