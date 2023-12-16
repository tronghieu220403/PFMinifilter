#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "FileFilter.h"
#include "ProcessFilter.h"

EXTERN_C_START

DRIVER_INITIALIZE DriverEntry;

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DriverUnload)
#endif

NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

void DriverUnload(
    _In_ PDRIVER_OBJECT  DriverObject
);