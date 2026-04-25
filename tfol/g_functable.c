#include "g_functable.h"

/* We don't care about the exact function signature, we just need the function pointer */
#define FUNTAB(name) void name(void);
#include "functions.inl"
#undef FUNTAB(name)

void* g_gamefunctions[] =
{
	#define FUNTAB(name) name,
	#include "functions.inl"
	#undef FUNTAB(name)
};

int g_gamefunctiontablesize = sizeof(g_gamefunctions) / sizeof(g_gamefunctions[0]);
