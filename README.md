# llvm-sanitizer-tutorial

This is a tutorial on how to build an LLVM sanitizer. 
Related blogpost: <URL HERE> 

# Quickstart: Building the toolchain & running a sanitizer 
 
```
#Clone the repo
git clone https://github.com/trailofbits/llvm-sanitizer-tutorial.git && cd llvm-sanitizer-tutorial/llvm 
#Make the build dir 
mkdir build && cd build 
#configure and build, there are a lot of configuration options for LLVM
cmake -DLLVM_TARGETS_TO_BUILD="X86" .. && make -j
```

# Building an out of source pass 

# Building a runtime component 

# Integrating an out of source pass 

# Integrating a runtime component 

# Helpful resources 

