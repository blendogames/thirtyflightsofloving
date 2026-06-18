#include "g_functable.h"

/* We don't care about the exact function signature, we just need the function pointer */
#define FUNTAB(name) void name(void);
#include "functions.inl"
#undef FUNTAB(name)

static const void* g_gamefunctions[] =
{
	#define FUNTAB(name) name,
	#include "functions.inl"
	#undef FUNTAB(name)
};
static const int g_gamefunctiontablesize = sizeof(g_gamefunctions) / sizeof(g_gamefunctions[0]);

void* g_getgamefunction(int index)
{
	return g_gamefunctions[index];
}

int g_getgamefunctionindex(void* ptr)
{
	int index;
	for (index = 0; index < g_gamefunctiontablesize; index += 1)
	{
		if (ptr == g_gamefunctions[index])
		{
			return index;
		}
	}
	return -1;
}

const char* g_getgamefunctionname(int index)
{
	static const char* g_functionnames[] =
	{
		#define FUNTAB(name) #name,
		#include "functions.inl"
		#undef FUNTAB(name)
	};

	if (index == -1)
	{
		return "NONE";
	}
	return g_functionnames[index];
}
