# llvm-sanitizer-tutorial

This is a tutorial on how to build an LLVM sanitizer. 

# Build and run the testsanitizer 
 
```
#Clone the repo
git clone https://github.com/trailofbits/llvm-sanitizer-tutorial.git && cd llvm-sanitizer-tutorial 
#Make the build dir 
mkdir build && cd build 
#configure and build, there are a lot of configuration options for LLVM
cmake -DLLVM_TARGETS_TO_BUILD=x86 .. && make -j
```

