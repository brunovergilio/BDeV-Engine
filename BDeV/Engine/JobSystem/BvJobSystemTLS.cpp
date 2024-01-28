#include "BvJobSystemTLS.h"


namespace JS
{
	thread_local i32 g_Index = -1;


	i32 GetWorkerThreadIndex()
	{
		return g_Index;
	}


	void SetWorkerThreadIndex(i32 index)
	{
		g_Index = index;
	}
}