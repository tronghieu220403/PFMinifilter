#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "FileFilter.h"
#include "ProcessFilter.h"


EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

void FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

EXTERN_C_END

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, FilterUnload)
#endif

namespace filter
{
    class FileFilter
    {
    private:
        inline static PDRIVER_OBJECT p_driver_object_ = { 0 };
        inline static PFLT_FILTER g_filter_handle_ = { 0 };
        static const FLT_OPERATION_REGISTRATION callbacks_[]; //  operation registration
        static const FLT_REGISTRATION filter_registration_; //  This defines what we want to filter with FltMgr

    public:

        static NTSTATUS Register();

        static FLT_PREOP_CALLBACK_STATUS PreCreateOperation(
            PFLT_CALLBACK_DATA Data,
            PCFLT_RELATED_OBJECTS FltObjects,
            PVOID* CompletionContext
        );

        static FLT_PREOP_CALLBACK_STATUS PreWriteOperation(
            PFLT_CALLBACK_DATA Data,
            PCFLT_RELATED_OBJECTS FltObjects,
            PVOID* CompletionContext
        );

        static NTSTATUS Unload(FLT_FILTER_UNLOAD_FLAGS Flags);

        static void SetDriverObjectPtr(const PDRIVER_OBJECT p_driver_object);
        static PDRIVER_OBJECT GetDriverObjectPtr();

        static void SetFilterHandle(const PFLT_FILTER g_filter_handle);
        static PFLT_FILTER GetFilterHandle();

    };
}
