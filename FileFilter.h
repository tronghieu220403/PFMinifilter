#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "ComPort.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

EXTERN_C_START

extern NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

EXTERN_C_END

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
                PFLT_CALLBACK_DATA data,
                PCFLT_RELATED_OBJECTS flt_objects,
                PVOID* completion_context
            );

        static FLT_POSTOP_CALLBACK_STATUS PostCreateOperation(
                PFLT_CALLBACK_DATA data,
                PCFLT_RELATED_OBJECTS flt_objects,
                PVOID completion_context,
                FLT_POST_OPERATION_FLAGS flags
        );

        static FLT_PREOP_CALLBACK_STATUS PreWriteOperationNoPostOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID* completion_context
        );

        static NTSTATUS Unload();

        static bool GetFileName(PFLT_CALLBACK_DATA data, PWCHAR name, DWORD32 size);

        static void SetDriverObjectPtr(const PDRIVER_OBJECT p_driver_object);
        static PDRIVER_OBJECT GetDriverObjectPtr();

        static void SetFilterHandle(const PFLT_FILTER g_filter_handle);
        static PFLT_FILTER GetFilterHandle();

    };
}

