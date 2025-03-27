#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct masInputTempMem
{
	uint8_t  *Begin;
	uint32_t  AllocIdx;
	uint32_t  Size;
};

static masInputTempMem* GInputTempMem = NULL;

static bool masInputTempMem_Create()
{
	uint32_t TempMemSize = 1024u * 512; // 512 KB
	uint64_t MemSize = sizeof(masInputTempMem) + TempMemSize;
	void* Mem = ::malloc(MemSize);
	if (!Mem)
		return false;
	else
		::memset(Mem, 0, MemSize);

	GInputTempMem           = (masInputTempMem*)Mem;
	GInputTempMem->Size     = TempMemSize;
	GInputTempMem->AllocIdx = 0;
	GInputTempMem->Begin    = (((uint8_t*)Mem) + sizeof(masInputTempMem));

	return true;
}

static void masInputTempMem_Destroy()
{
	::free(GInputTempMem);
	GInputTempMem = NULL;
}


static void* masInputTempMem_Alloc(uint32_t Size)
{
	if (GInputTempMem->AllocIdx + Size >= GInputTempMem->Size)
		return NULL;

	void* Temp = GInputTempMem->Begin + GInputTempMem->AllocIdx;
	GInputTempMem->AllocIdx += Size;

	::memset(Temp, 0, Size);
	return Temp;
}

static void masInputTempMem_Reset()
{
	::memset(GInputTempMem->Begin, 0, GInputTempMem->Size);
	GInputTempMem->AllocIdx = 0;
}