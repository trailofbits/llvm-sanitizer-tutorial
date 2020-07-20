add_llvm_library(LLVMTestPass MODULE BUILDTREE_ONLY
	TestPass.cpp

	DEPENDS
	intrinsics_gen
	PLUGIN_TOOL
	opt
	)
