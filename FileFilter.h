#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

EXTERN_C_START

namespace filter
{
    class FileFilter
    {
    private:
        static PDRIVER_OBJECT p_driver_object_;
        static PFLT_FILTER g_filter_handle_;
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

EXTERN_C_END
