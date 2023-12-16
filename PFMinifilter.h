#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "FileFilter.h"
#include "ProcessFilter.h"


EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

void DriverUnload(PDRIVER_OBJECT DriverObject);

EXTERN_C_END

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DriverUnload)
#endif