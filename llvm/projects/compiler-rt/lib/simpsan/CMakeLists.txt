include_directories(..)

# Runtime library sources and build flags.
set(TESTSAN_RTL_SOURCES
	test_san.cpp
	)

set(TESTSAN_RTL_HEADERS
	  test_san.h
		)

set(TESTSAN_COMMON_CFLAGS ${SANITIZER_COMMON_CFLAGS})
append_rtti_flag(OFF TESTSAN_COMMON_CFLAGS)
# Prevent clang from generating libc calls.
append_list_if(COMPILER_RT_HAS_FFREESTANDING_FLAG -ffreestanding TESTSAN_COMMON_CFLAGS)

# Static runtime library.
add_compiler_rt_component(test_san)

foreach(arch ${TESTSAN_SUPPORTED_ARCH})
	set(TESTSAN_CFLAGS ${TESTSAN_COMMON_CFLAGS})
	append_list_if(COMPILER_RT_HAS_FPIE_FLAG -fPIE TESTSAN_CFLAGS)
	add_compiler_rt_runtime(clang_rt.test_san
		STATIC
		ARCHS ${arch}
		SOURCES ${TESTSAN_RTL_SOURCES}
		$<TARGET_OBJECTS:RTInterception.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommon.${arch}>
		$<TARGET_OBJECTS:RTSanitizerCommonLibc.${arch}>
		CFLAGS ${TESTSAN_CFLAGS}
		PARENT_TARGET test_san)
	add_sanitizer_rt_symbols(clang_rt.test_san
		ARCHS ${arch}
		EXTRA test_san.syms.extra)
	add_dependencies(test_san
		clang_rt.test_san-${arch}-symbols)
endforeach()
