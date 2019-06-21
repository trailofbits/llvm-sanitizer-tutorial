#include "sanitizer_common/sanitizer_common.h"
#include "sanitizer_common/sanitizer_flag_parser.h"
#include "sanitizer_common/sanitizer_flags.h"

#include "interception/interception.h"
#include "sanitizer_common/sanitizer_platform_interceptors.h"
#include "sanitizer_common/sanitizer_libc.h"
#include "sanitizer_common/sanitizer_linux.h" 



extern "C" {
void testsan_AllocateShadowMemory();
void testsan_SetFunctionName(char * func_name); 
void testsan_PrintLeakSummary(); 
void testsan_StoreAddress(char * addr); 
}
void * testsan_malloc(__sanitizer::uptr size); 
void testsan_free(void * p); 
void testsan_InitInterceptors(); 
