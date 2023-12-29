#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "Ulti.h"
#include "ComPort.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

namespace filter
{
    class ProcessFilter
    {
    public:
        static NTSTATUS Register();

        static void CreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create);

        static void Unload();
    };
}
