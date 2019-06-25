add_llvm_library(LLVMTestPass MODULE BUILDTREE_ONLY
	TestPass.cpp

	DEPENDS
	PLUGIN_TOOL
	opt
	)
