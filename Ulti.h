#pragma once

#include <fltKernel.h>
#include <dontuse.h>
#include <Ntstrsafe.h>

namespace ulti
{
	int MergeString(PWCHAR des, int size, PWCHAR src1, PWCHAR src2);
	
	int GetWStringSize(PWCHAR str);
}