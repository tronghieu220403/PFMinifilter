#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

EXTERN_C_START

namespace filter
{
    class ProcessFilter
    {
    private:

    public:

        static NTSTATUS Register();

        static void CreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create);

        static void Unload();

    };
}

EXTERN_C_END
