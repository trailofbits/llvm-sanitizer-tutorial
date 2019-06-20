//Types/Headers
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/InstrTypes.h"

//Instrumentation 
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Instrumentation.h"

//Pass registration
#include "llvm/IR/PassManager.h"

#include <vector>
#include <string>

using namespace llvm;

#define DEBUG_TYPE "testsan"

//Defining our classes
namespace { 
	class TestSanitizerModule : public ModulePass {
		public: 
			static char ID; 
			TestSanitizerModule() : ModulePass(ID) {} 
			//Creates a simple hello world call at entry 
			bool runOnModule(Module &M) override; 
	};
	class TestSanitizerFunction : public FunctionPass {
		public:
			static char ID; 
			TestSanitizerFunction() : FunctionPass(ID) {}
			//Creation and insertion of calls at entry
			bool runOnFunction(Function &F) override;
	};
  class TestSanitizerBlock : public BasicBlockPass {
		public: 
			static char ID; 
			TestSanitizerBlock() : BasicBlockPass(ID) {}
			//Inserts, modifies and replaces instructions 
			bool runOnBasicBlock(BasicBlock &B) override; 
	}; 
}//End anon namespace

bool TestSanitizerModule::runOnModule(Module &module) {
	errs() << "This is the LLVM Module for source file:  " << module.getSourceFileName() << "\n"; 	

	//This is the LLVM context, this contains important data and is used to make most important things 
	LLVMContext& context = module.getContext();
	IRBuilder<> builder(context);

	//The LLVM module class has a list of all functions in the module, I just picked the first one in the list. 
	Function& single_function = *(module.begin()); 
	builder.SetInsertPoint(&(single_function.getEntryBlock().front()));

	//Create a string
	Value *hello_world_str = builder.CreateGlobalStringPtr("Hello World!\n");

	//Create the prototype for puts function 
	std::vector<llvm::Type *> puts_args;
	puts_args.push_back(builder.getInt8Ty()->getPointerTo());
	ArrayRef<Type*> puts_args_ref(puts_args);

	//Create function and insert call into IR
	FunctionType *puts_type =
		FunctionType::get(builder.getInt32Ty(), puts_args_ref, false);
	Constant *putsFunc = module.getOrInsertFunction("puts", puts_type);
	builder.CreateCall(putsFunc, hello_world_str);

	return true;
}


bool TestSanitizerFunction::runOnFunction(Function &func) {
	errs() << "Instrumenting function entry!\n"; 

	//Create the IR Builder
	LLVMContext& context = func.getContext();
	IRBuilder<> builder(context);

	//Set the Insertion point 
	builder.SetInsertPoint(&(func.getEntryBlock().front()));

	//Create a function prototype. 
	std::vector<llvm::Type *> name_set_func_args;
	name_set_func_args.push_back(llvm::Type::getInt8Ty(context)->getPointerTo());
	
	ArrayRef<Type*> name_set_args_ref(name_set_func_args);
	
	FunctionType * name_set_func_type = 
		FunctionType::get(llvm::Type::getVoidTy(context), name_set_args_ref, false);

	//Create the function and use the IR Builder to create a function call
	Constant * name_set_func = func.getParent()->getOrInsertFunction("LeakCheck_SetFunctionName", name_set_func_type);
	Value * name_val = builder.CreateGlobalStringPtr(func.getName());
 	Value * args[] = {name_val};
	builder.CreateCall(name_set_func, args);
	
	//If its the main function locate all ret instructions and place a hook to summary at the end
	if (func.getName().compare("main") == 0) {
		for (auto& block : func) {
			for (auto& inst : block) {
				if (auto * potential_ret = dyn_cast<ReturnInst>(&inst)) {
					builder.SetInsertPoint(potential_ret);

					std::vector<llvm::Type *> summary_func_args;
					ArrayRef<Type*> summary_args_ref(summary_func_args);
	
					FunctionType * summary_func_type = 
							FunctionType::get(llvm::Type::getVoidTy(context), summary_args_ref, false);

					Constant * summary_func = func.getParent()->getOrInsertFunction("LeakCheck_PrintLeakSummary", summary_func_type);
					builder.CreateCall(summary_func, {});
				}
			}
		}
	}
	return true;
}

//check for object allocation instruction and grab the pointer after it, print it. 
bool TestSanitizerBlock::runOnBasicBlock(BasicBlock &BB) {
	
	LLVMContext& context = BB.getContext();
	Module* current_module = BB.getModule(); 	
	
	PointerType * byte_ptr_type = llvm::Type::getInt8Ty(context)->getPointerTo(); 	

	errs() << "got void ty\n"; 

	std::vector<llvm::Type *> log_allocation_args;
	log_allocation_args.push_back(byte_ptr_type);
	errs() << "got ptr 2\n"; 
	ArrayRef<Type*> log_allocation_ref(log_allocation_args);

	FunctionType *log_allocation_type = FunctionType::get(byte_ptr_type, log_allocation_ref, false);
	
	Constant *log_allocation_func = current_module->getOrInsertFunction("LeakCheck_LogAllocation", log_allocation_type);
	
	errs() << "Checking instructions\n"; 	
	for (auto& Inst : BB) {
		//THe LLVM Programmers manual states that dyn_cast is how you should check for types of instructions, functions, etc
		//Returns a pointer on success 
		if (auto* potential_malloc  = dyn_cast<CallInst>(&Inst)) {
			errs() << "Call instruction\n";
			//CallInst inherits from CallBase meaning we can get the called function
			Function * called_func = potential_malloc->getCalledFunction();
			std::string func_name = called_func->getName(); 	
			if (func_name.compare("malloc") == 0) {
				errs() << "Malloc found!\n";
				//Add instrumentation code with the address returned from malloc
				//You can use the IR Builder to pass in the result of other operations/instructions to calls
				IRBuilder<> builder(potential_malloc); 
				builder.SetInsertPoint(potential_malloc->getNextNode());
				builder.CreateCall(log_allocation_func, potential_malloc);
			}
		}
	}
	return true; 
}

//Initializers 
char TestSanitizerModule::ID = 0;  
char TestSanitizerFunction::ID = 0;
char TestSanitizerBlock::ID = 0;  

//For registering with opt for debugging/testing
static RegisterPass<TestSanitizerModule> TestMod("testmod", "testing module pass"); 
static RegisterPass<TestSanitizerFunction> TestFunc("testfunc", "testing function pass");  
static RegisterPass<TestSanitizerBlock> TestBlock("testblock", "testing block pass");  
