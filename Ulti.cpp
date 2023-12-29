#include "Ulti.h"

int ulti::MergeString(PWCHAR des, int size , PWCHAR src1, PWCHAR src2)
{
	int size1 = GetWStringSize(src1);
	int size2 = GetWStringSize(src2);
	if (size1 + size2 > size)
	{
		return 0;
	}
	RtlCopyMemory(&des[0], src1, size1 * sizeof(WCHAR));
	RtlCopyMemory(&des[size1], src2, size2 * sizeof(WCHAR));
	return size1 + size2;
}

int ulti::GetWStringSize(PWCHAR str)
{
	for (int i = 0; ; i++)
	{
		if (str[i] == 0)
		{
			return i;
		}
	}
}
