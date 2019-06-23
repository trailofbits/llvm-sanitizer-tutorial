# llvm-sanitizer-tutorial

This is a tutorial on how to build an LLVM sanitizer. 

# Background 
An LLVM sanitizer is a powerful tool used to instrument and analyze programs. This github repo holds an example sanitizer and step by step docuemtation to integrate a sanitizer into the toolchain. This sanitizer can serve as a template towards building more complex tools. For more information on what sanitizers are, see the related blogpost: 

URL

# Quickstart: Building the toolchain & running a sanitizer 
 
```
#Clone the repo
git clone https://github.com/trailofbits/llvm-sanitizer-tutorial.git && cd llvm-sanitizer-tutorial/llvm 
#Make the build dir 
mkdir build && cd build 
#configure and build, there are a lot of configuration options for LLVM
cmake -DLLVM_TARGETS_TO_BUILD="X86" .. && make
cd bin && ./clang -fsanitize=testsan -g -o malloc_target ../../../target_programs/malloc_target.c
./malloc_target
```
You should see output from the transformation pass and additional output from the runtime component when the program is executed. Most of this readme will be from the blogpost above, but in this repo im going to list all the tedious technical details that didn't make it past editing. Note that this post only covers on how to build a sanitizer for Linux. 

# Building an out of source pass 
Why build out of source first? Building your instrumentation pass out of source is a good first step when building your sanitizer. This allows you to debug your pass and determine if it's functioning correctly. When building the LLVM tool chain, you can use the `opt` tool to run your pass on bitcode and use the `llvm-dis` tool to view the actual IR. 
ayy yo do the cmds here 

<br/>
<br/>
The first thing is to create your pass, check out `llvm/lib/Transform/TestPass/TestPass.cpp` for the full code, here is the important snippet. This shows how to create 3 types of passes and use the IR Builder, which is the most important API for llvm-based instrumenatation. <br/> 

The LLVM module is the largest unit of compilation, it essentially represents the file. The function and basic block passes operate at those respective levels. The module pass prints out every function in the module. The function pass has some more complexity... (continue tmrw im so tired) 

Save this as `TutorialSanitizer.cpp` <br/>


# Building a runtime component 
The runtime component does not take much effort to build on it's own. You 

# Integrating an out of source pass 

# Integrating a runtime component 

# Modifying the compiler driver

# Some other things I learned 
Your IR passes will be operating system agnostic but other parts of the toolchain are not. When integrating your sanitizer you will have to perform different build operations for OSX/Windows etc. For example in this tutorial we statically linked the runtime to  Fortunately compiler-rt hides a lot of the nastiness from you. I reccomend trying to use the sanitizer runtime interface as much as possible so you can run on as many operating systems without getting a headache. <br/> 

# Helpful resources 
link the dam blogpost again lol <br/>
eli's awesome stuff <br/>
adrians awesome stuff <br/>
llvm conference awesome stuff <br/>  
llvm class references   
