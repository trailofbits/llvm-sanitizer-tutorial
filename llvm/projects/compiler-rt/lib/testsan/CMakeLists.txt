include_directories(..)

# Runtime library sources and build flags.
set(TESTSAN_RTL_SOURCES
	testsan.cpp
	)

set(TESTSAN_RTL_HEADERS
  testsan.h
	)
#No special cflags
set(TESTSAN_COMMON_CFLAGS ${SANITIZER_COMMON_CFLAGS})
# Static runtime library.
add_compiler_rt_component(testsan)

foreach(arch ${TESTSAN_SUPPORTED_ARCH})
	set(TESTSAN_CFLAGS ${TESTSAN_COMMON_CFLAGS})
	append_list_if(COMPILER_RT_HAS_FPIE_FLAG -fPIE TESTSAN_CFLAGS)
	add_compiler_rt_runtime(clang_rt.testsan
		STATIC
		ARCHS ${arch}
		SOURCES ${TESTSAN_RTL_SOURCES}
		$<TARGET_OBJECTS:RTInterception.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommon.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommonLibc.${arch}>
		CFLAGS ${TESTSAN_CFLAGS}
		PARENT_TARGET testsan)
	add_sanitizer_rt_symbols(clang_rt.testsan
		ARCHS ${arch}
		 )
	add_dependencies(testsan
		clang_rt.testsan-${arch}-symbols)
endforeach()
