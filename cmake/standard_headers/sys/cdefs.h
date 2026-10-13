#pragma once

#include_next <sys/cdefs.h>

// glibc's Clang variadic fortify overloads are static, so libc++ cannot export
// them from std. Give only the C++ overloads hidden inline linkage; retain their
// pass_object_size attributes and checked bodies. C compilation is unchanged.
#if defined(__cplusplus) && defined(__GLIBC__) && defined(__fortify_use_clang) && __fortify_use_clang
#undef __fortify_function_error_function
#define __fortify_function_error_function inline __attribute__((__unused__, __visibility__("hidden")))
#endif
