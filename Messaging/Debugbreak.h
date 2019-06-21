#pragma once

#if defined(_MSC_VER)
	#define GPG_DEBUGBREAK() do{__debugbreak(); } while(false)
#elif (!defined(__NACL__) && defined(__GNUC__) && (defined(__i386__) || defined(__x86_64__)))
#define GPG_DEBUGBREAK() do{__asm__ __volatile__ ( "int $3\n\t" );}while(false)
#elif defined(__EMSCRIPTEN__)
#define GPG_DEBUGBREAK() EM_ASM({debugger;});
#elif defined(__clang__)
#define GPG_DEBUGBREAK()  do{ __builtin_trap(); ) while(false)
#elif __has_include(<signal.h>)
#define GPG_DEBUGBREAK() do{raise(SIGTRAP);}while(false)
#else
#pragma message("Couldnt find a suitable definition for GPG_DEBUGBREAK() it is evaluated to nothing!!")
#define GPG_DEBUGBREAK() do{}while(false)
#endif