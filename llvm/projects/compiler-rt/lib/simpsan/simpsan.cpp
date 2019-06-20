#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_flag_parser.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "simpsan/simpsan.h" 
#include <stdint.h>
#include <iostream> 
#include <stdlib.h> 
#include <string.h> 
#include <string> 

using namespace __sanitizer; //Use sanitizer namespace for common sanitizer functions like MmapFixedNoReserve etc. 


//Define our own private namespace for our runtime implementation
namespace __simpsan {

	static struct {
		uint64_t shadow_mem_size; 
		int * shadow_mem_start; 
		int * shadow_mem_end; 

	} shadow_memory_storage;

} //end of __simpsan

//use our namespace and interface with the rest of the toolchain
using namespace __simpsan; 

//Sanitizer interface attribute sets the visability of the symbol so we can export it
SANITIZER_INTERFACE_ATTRIBUTE 
void simpsan_AllocateShadowMemory(int max_elements) { 
	if (max_elements < 0) {
		VReport(1, "Max elements < 0\n");
		//throw "Construction failed";
		//Exception handling disabled apparently
		return;
	}
	shadow_memory_storage.shadow_mem_size = max_elements * sizeof(*shadow_memory_storage.shadow_mem_start);

	//Allocate shadow memory of a certain size or die. 
	shadow_memory_storage.shadow_mem_start = (int*)MmapNoReserveOrDie(shadow_memory_storage.shadow_mem_size, "Simple Shadow Memory");
	shadow_memory_storage.shadow_mem_end = shadow_memory_storage.shadow_mem_start + shadow_memory_storage.shadow_mem_size; 
	//Init it all to 0
	memset(shadow_memory_storage.shadow_mem_start, 0, shadow_memory_storage.shadow_mem_size);
	//Optional: Report to stdout shadow region, useful for debugging. 
	VReport(1, "Shadow mem at %zx .. %zx\n", shadow_memory_storage.shadow_mem_start, shadow_memory_storage.shadow_mem_start + shadow_memory_storage.shadow_mem_size);
}

SANITIZER_INTERFACE_ATTRIBUTE 
int simpsan_StoreInteger(int value, int index) {
	if (shadow_memory_storage.shadow_mem_start + index > shadow_memory_storage.shadow_mem_end || index < 0 || value < 0) {
		VReport(2, "Store failed, invalid address, index or value\n");
		return -1;
	}	
	shadow_memory_storage.shadow_mem_start[index] = value; 
	return 0;
}

SANITIZER_INTERFACE_ATTRIBUTE	
int simpsan_FetchInteger(int index) {
	if (shadow_memory_storage.shadow_mem_start + index >  shadow_memory_storage.shadow_mem_end || index < 0) {
		VReport(2, "Store failed, invalid address or index\n"); 
		return -1;
	}
	return shadow_memory_storage.shadow_mem_start[index];
}

SANITIZER_INTERFACE_ATTRIBUTE
int simpsan_PutsWrapper(char * some_string) {
	return puts(some_string);
}

SANITIZER_INTERFACE_ATTRIBUTE
void simpsan_Callback()
{
	static int nonce = 0;
	int fetch_val = simpsan_FetchInteger(0);
  std::string temp = "This is your value: " + std::to_string(fetch_val) + "\n";	
	simpsan_PutsWrapper((char*)temp.c_str());
	int ret_val = simpsan_StoreInteger(++nonce, 0); 
	//puts("Ayy lmao\n");
	return;
}

extern "C" SANITIZER_INTERFACE_ATTRIBUTE
#if !SANITIZER_CAN_USE_PREINIT_ARRAY
// For non Linux platforms this will run this function when the RT library is loaded. 
__attribute__((constructor(0)))
#endif
	void simpsan_Init() {
		//Set sanitizer tool name, not required. 
		SanitizerToolName = "simpsan";

		//Sanitizers have a lot of flags, theres a built in to set everything to default. 
		//The flags are located in sanitizer_flags.inc
		SetCommonFlagsDefaults();

		//Try to allocate shadowmem, have it store 500 elements. 
		simpsan_AllocateShadowMemory(500);
		VReport(2, "Initialized Simpsan\n"); 
	}
#if SANITIZER_CAN_USE_PREINIT_ARRAY

// For Linux platforms we use the .preinit_array ELF magic.
extern "C" {
	__attribute__((section(".preinit_array"),
				used)) void (*simpsan_init_ptr)(void) = simpsan_Init;
}
#endif
