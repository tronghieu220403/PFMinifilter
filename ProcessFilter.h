#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

EXTERN_C_START

NTSTATUS ProcessFilterRegister();

void ProcessFilterCreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create);

void ProcessFilterUnload();

EXTERN_C_END
