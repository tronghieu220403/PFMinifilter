#include "FileFilter.h"

namespace filter 
{
    NTSTATUS FileFilter::Register()
    {
        DebugMessage("FileFilter registering hihi\n");

        NTSTATUS status = FltRegisterFilter(p_driver_object_,
            &filter_registration_,
            &g_filter_handle_);

        if (!NT_SUCCESS(status))
        {
            switch (status)
            {
            case STATUS_INSUFFICIENT_RESOURCES:
                DebugMessage("STATUS_INSUFFICIENT_RESOURCES\n");
                break;
            case STATUS_INVALID_PARAMETER:
                DebugMessage("STATUS_INVALID_PARAMETER\n");
                break;
            case STATUS_FLT_NOT_INITIALIZED:
                DebugMessage("STATUS_FLT_NOT_INITIALIZED\n");
                break;
            case STATUS_OBJECT_NAME_NOT_FOUND:
                DebugMessage("STATUS_OBJECT_NAME_NOT_FOUND\n");
                break;
            default:
                break;
            }
        }
        else
        {
            DebugMessage("Reg Oke");
        }

        FLT_ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status)) {

            //
            //  Start filtering i/o
            //

            status = FltStartFiltering(g_filter_handle_);

            if (!NT_SUCCESS(status)) {

                FltUnregisterFilter(g_filter_handle_);
            }
            else
            {
                DebugMessage("Start filtering\n");
            }
        }

        return status;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreCreateOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
    {
        UNREFERENCED_PARAMETER(Data);
        UNREFERENCED_PARAMETER(FltObjects);
        UNREFERENCED_PARAMETER(CompletionContext);
        DebugMessage("File Create");
        /*
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
                    DebugMessage(("create file: %ws \r\n", Name));
                }
            }
            FltReleaseFileNameInformation(FileNameInfo);
        }
        */
        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreWriteOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
    {
        // Write here
        UNREFERENCED_PARAMETER(Data);
        UNREFERENCED_PARAMETER(FltObjects);
        UNREFERENCED_PARAMETER(CompletionContext);

        DebugMessage("File Write");

        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
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

    NTSTATUS FileFilter::Unload(FLT_FILTER_UNLOAD_FLAGS Flags)
    {
        UNREFERENCED_PARAMETER(Flags);
        DebugMessage("Unload");
        FltUnregisterFilter(filter::FileFilter::GetFilterHandle());
        return STATUS_SUCCESS;
    }


    const FLT_OPERATION_REGISTRATION FileFilter::callbacks_[] = 
    {
        { IRP_MJ_CREATE,
          0,
          (PFLT_PRE_OPERATION_CALLBACK)FileFilter::PreCreateOperation,
          NULL },

        { IRP_MJ_WRITE,
          0,
          (PFLT_PRE_OPERATION_CALLBACK)FileFilter::PreWriteOperation,
          NULL },

        { IRP_MJ_OPERATION_END }
    };

    const FLT_REGISTRATION FileFilter::filter_registration_ = 
    {
        sizeof(FLT_REGISTRATION),
        FLT_REGISTRATION_VERSION,
        0,                                  //  Flags

        NULL,                               //  Context
        FileFilter::callbacks_,                         //  Operation callbacks

        (PFLT_FILTER_UNLOAD_CALLBACK)FilterUnload, //  MiniFilterUnload

        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };
}


