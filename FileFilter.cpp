#include "FileFilter.h"

namespace filter 
{
    NTSTATUS FileFilter::Register()
    {
        NTSTATUS status = FltRegisterFilter(p_driver_object_,
            &filter_registration_,
            &g_filter_handle_);

        FLT_ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status)) {

            //
            //  Start filtering i/o
            //

            status = FltStartFiltering(g_filter_handle_);

            if (!NT_SUCCESS(status)) {

                FltUnregisterFilter(g_filter_handle_);
            }
        }

        return status;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreCreateOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
    {
        UNREFERENCED_PARAMETER(FltObjects);
        UNREFERENCED_PARAMETER(CompletionContext);

        PFLT_FILE_NAME_INFORMATION FileNameInfo;
        NTSTATUS status;
        WCHAR Name[260] = { 0 };

        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

        if (NT_SUCCESS(status))
        {
            status = FltParseFileNameInformation(FileNameInfo);

            if (NT_SUCCESS(status))
            {
                if (FileNameInfo->Name.MaximumLength < 260)
                {
                    RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
                    KdPrint(("create file: %ws \r\n", Name));
                }
            }
            FltReleaseFileNameInformation(FileNameInfo);
        }
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreWriteOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
    {
        // Write here
        UNREFERENCED_PARAMETER(Data);
        UNREFERENCED_PARAMETER(FltObjects);
        UNREFERENCED_PARAMETER(CompletionContext);

        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    NTSTATUS FileFilter::Unload(FLT_FILTER_UNLOAD_FLAGS Flags)
    {
        UNREFERENCED_PARAMETER(Flags);
        FltUnregisterFilter(g_filter_handle_);
        return STATUS_SUCCESS;
    }

    void FileFilter::SetDriverObjectPtr(const PDRIVER_OBJECT p_driver_object)
    {
        p_driver_object_ = p_driver_object;
    }

    PDRIVER_OBJECT FileFilter::GetDriverObjectPtr()
    {
        return p_driver_object_;
    }

    void FileFilter::SetFilterHandle(const PFLT_FILTER g_filter_handle)
    {
        g_filter_handle_ = g_filter_handle;
    }

    PFLT_FILTER FileFilter::GetFilterHandle()
    {
        return g_filter_handle_;
    }

    const FLT_OPERATION_REGISTRATION FileFilter::callbacks_[] = 
    {
        { IRP_MJ_CREATE,
          0,
          (PFLT_PRE_OPERATION_CALLBACK)&FileFilter::PreCreateOperation,
          NULL },

        { IRP_MJ_WRITE,
          0,
          (PFLT_PRE_OPERATION_CALLBACK)&FileFilter::PreWriteOperation,
          NULL },

        { IRP_MJ_OPERATION_END }
    };

    const FLT_REGISTRATION FileFilter::filter_registration_ = 
    {
        sizeof(FLT_REGISTRATION),
        FLT_REGISTRATION_VERSION,
        0,                                  //  Flags

        NULL,                               //  Context
        callbacks_,                         //  Operation callbacks

        (PFLT_FILTER_UNLOAD_CALLBACK)&filter::FileFilter::Unload, //  MiniFilterUnload

        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };
}


