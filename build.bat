@echo off
REM this is used instead of CMake, MakeGNU, Ninja, etc. Build tools are used to optimize the compiling of programmes. Large projects can take long to compile.
REM  %comspec% /k “C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat”
mkdir build
pushd build
cl /arch:AVX2 -Zi ..\easeq.cpp
popd