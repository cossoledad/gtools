[settings]
os=Windows
arch=x86_64
compiler=clang
compiler.version=17
compiler.libcxx=libstdc++11
compiler.cppstd=20

[conf]
tools.cmake.cmaketoolchain:generator=Ninja
tools.build:compiler_executables={"c": "clang", "cpp": "clang++"}

[buildenv]
CC=clang
CXX=clang++
