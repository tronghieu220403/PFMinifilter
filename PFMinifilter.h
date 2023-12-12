#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "FileFilter.h"
#include "ProcessFilter.h"

EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

void DriverUnload(PDRIVER_OBJECT DriverObject);

EXTERN_C_END


