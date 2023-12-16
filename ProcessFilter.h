#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

namespace filter
{
    class ProcessFilter
    {
    public:
        NTSTATUS Register();

        void CreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create);

        void Unload();
    }
}
