#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "FileFilter.h"
#include "ProcessFilter.h"

#pragma warning( disable : 4083 4024 4047 )

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

PDRIVER_OBJECT gDriverObject = { 0 };
PFLT_FILTER gFilterHandle = { 0 };

EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, FilterUnload)
#endif
