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

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID* completion_context)
    {
        UNREFERENCED_PARAMETER(data);
        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(completion_context);

        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    FLT_POSTOP_CALLBACK_STATUS FileFilter::PostCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID completion_context, FLT_POST_OPERATION_FLAGS Flags)
    {

        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(completion_context);
        UNREFERENCED_PARAMETER(Flags);

        if (data && data->Iopb && data->Iopb->MajorFunction == IRP_MJ_CREATE)
        {
            WCHAR name[260] = { 0 };

            if (FileFilter::GetFileName(data, name, 260) == false)
            {
                return FLT_POSTOP_FINISHED_PROCESSING;
            }
            UINT64 flag = data->IoStatus.Information;

            if (flag == FILE_CREATED)
            {
                DebugMessage("File is created: %ws \r\n", name);
            }
        }

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreWriteOperationNoPostOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID* completion_context)
    {
        // Write here
        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(completion_context);

        if (data && data->Iopb && data->Iopb->MajorFunction == IRP_MJ_WRITE)
        {
            WCHAR name[260] = { 0 };

            if (FileFilter::GetFileName(data, name, 260) == false)
            {
                return FLT_PREOP_SUCCESS_NO_CALLBACK;
            }
            ULONG written = data->Iopb->Parameters.Write.Length;

            DebugMessage("File %ws is written %x bytes,  \r\n", name, written);
        }

        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    NTSTATUS FileFilter::Unload(FLT_FILTER_UNLOAD_FLAGS Flags)
    {
        UNREFERENCED_PARAMETER(Flags);
        DebugMessage("Unload File Filter");
        FltUnregisterFilter(FileFilter::GetFilterHandle());
        return STATUS_SUCCESS;
    }

    bool FileFilter::GetFileName(PFLT_CALLBACK_DATA data, PWCHAR name, DWORD32 size)
    {
        PFLT_FILE_NAME_INFORMATION file_name_info;
        NTSTATUS status;
        bool ret;

        if (data == nullptr || name == nullptr)
        {
            return false;
        }
        if (data->Iopb != NULL && data->Iopb->TargetFileObject != NULL)
        {
            PWCH buffer = data->Iopb->TargetFileObject->FileName.Buffer;
            USHORT max_length = data->Iopb->TargetFileObject->FileName.MaximumLength;
            USHORT length = data->Iopb->TargetFileObject->FileName.Length;
            if (buffer != NULL && max_length < size && length > 0)
            {
                RtlCopyMemory(name, buffer, max_length);
                return true;
            }
        }

        status = FltGetFileNameInformation(data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &file_name_info);

        if (NT_SUCCESS(status))
        {
            status = FltParseFileNameInformation(file_name_info);

            if (NT_SUCCESS(status))
            {
                ret = true;
                if (file_name_info->Name.MaximumLength < size)
                {
                    RtlCopyMemory(name, file_name_info->Name.Buffer, file_name_info->Name.MaximumLength);
                }
                else
                {
                    ret = false;
                }
            }
            else 
            {
                ret = false;
            }
            FltReleaseFileNameInformation(file_name_info);
        }
        else
        {
            ret = false;
        }

        return ret;
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
          FileFilter::PreCreateOperation,
          FileFilter::PostCreateOperation },

        { IRP_MJ_WRITE,
          0,
          FileFilter::PreWriteOperationNoPostOperation,
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

        (PFLT_FILTER_UNLOAD_CALLBACK)::FilterUnload, //  MiniFilterUnload

        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };
}

