#include "FileFilter.h"

namespace filter 
{
    NTSTATUS FileFilter::Register()
    {
        // DebugMessage("FileFilter registering hihi\n");

        NTSTATUS status = FltRegisterFilter(p_driver_object_,
            &filter_registration_,
            &g_filter_handle_);

        if (!NT_SUCCESS(status))
        {
            switch (status)
            {
            case STATUS_INSUFFICIENT_RESOURCES:
                // DebugMessage("STATUS_INSUFFICIENT_RESOURCES\n");
                break;
            case STATUS_INVALID_PARAMETER:
                // DebugMessage("STATUS_INVALID_PARAMETER\n");
                break;
            case STATUS_FLT_NOT_INITIALIZED:
                // DebugMessage("STATUS_FLT_NOT_INITIALIZED\n");
                break;
            case STATUS_OBJECT_NAME_NOT_FOUND:
                // DebugMessage("STATUS_OBJECT_NAME_NOT_FOUND\n");
                break;
            default:
                break;
            }
        }
        else
        {
            // DebugMessage("Reg Oke");
        }

        FLT_ASSERT(NT_SUCCESS(status));

        if (NT_SUCCESS(status)) {

            //
            //  Start filtering i/o
            //

            status = FltStartFiltering(g_filter_handle_);

            if (!NT_SUCCESS(status)) {

                FileFilter::Unload();
            }
            else
            {
                // DebugMessage("Start filtering\n");
            }
        }

        return status;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID* completion_context)
    {
        PDF_STREAM_CONTEXT stream_context;
        NTSTATUS status;

        UNREFERENCED_PARAMETER(flt_objects);

        PAGED_CODE();

        // below is for file deletion
        if (FlagOn(data->Iopb->Parameters.Create.Options, FILE_DELETE_ON_CLOSE)) {

            status = AllocateContext(FLT_STREAM_CONTEXT,
                (PFLT_CONTEXT * )&stream_context);

            if (NT_SUCCESS(status)) {

                *completion_context = (PVOID)stream_context;

                return FLT_PREOP_SYNCHRONIZE;

            }
        }

        *completion_context = NULL;

        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    FLT_POSTOP_CALLBACK_STATUS FileFilter::PostCreateOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID completion_context, FLT_POST_OPERATION_FLAGS flags)
    {
        WCHAR name[260] = { 0 };
        WCHAR msg[500] = { 0 };
        NTSTATUS status = STATUS_SUCCESS;
        PDF_STREAM_CONTEXT stream_context = NULL;

        UNREFERENCED_PARAMETER(completion_context);
        UNREFERENCED_PARAMETER(flags);

        PAGED_CODE();

        ASSERT(!FlagOn(flags, FLTFL_POST_OPERATION_DRAINING));

        // below is for file creation

        if (data->IoStatus.Information == FILE_CREATED)
        {
            if (FileFilter::GetFileName(data, name, 260) == false)
            {
                return FLT_POSTOP_FINISHED_PROCESSING;
            }
            // DebugMessage("File is created: %ws \r\n", name);

            if (RtlStringCchPrintfW(msg, 500, L"File is created: %ws \r\n", name) == STATUS_SUCCESS);
            {
                // DebugMessage("Sending msg");
                com::ComPort::Send(msg, 500 * sizeof(WCHAR));
                // DebugMessage("Sent msg");
            }
        }

        if (!NT_SUCCESS(data->IoStatus.Status) ||
            (STATUS_REPARSE == data->IoStatus.Status))
        {
            return FLT_POSTOP_FINISHED_PROCESSING;
        }

        // below is for file deletion
        status = GetOrSetContext(flt_objects,
            data->Iopb->TargetFileObject,
            (PFLT_CONTEXT *)&stream_context,
            FLT_STREAM_CONTEXT);

        if (NT_SUCCESS(status)) {
            stream_context->DeleteOnClose = BooleanFlagOn(data->Iopb->Parameters.Create.Options,
                FILE_DELETE_ON_CLOSE);
            FltReleaseContext(stream_context); // FltReleaseContext decrements the reference count on the given context. When the reference count reaches zero, if the caller is running at IRQL DISPATCH_LEVEL, a work item is scheduled to free the context.
        }

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreWriteOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID* completion_context)
    {
        // Write here
        UNREFERENCED_PARAMETER(data);
        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(completion_context);

        return FLT_PREOP_SUCCESS_WITH_CALLBACK;
    }

    FLT_POSTOP_CALLBACK_STATUS FileFilter::PostWriteOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID completion_context, FLT_POST_OPERATION_FLAGS flags)
    {
        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(completion_context);
        UNREFERENCED_PARAMETER(flags);

        WCHAR name[260] = { 0 };
        WCHAR msg[500] = { 0 };

        if (!NT_SUCCESS(data->IoStatus.Status))
        {
            // DebugMessage("WriteOperation: before get file name, must have after");
            // if this doesn't work, we might need to save "completion_context" in pre-operation 
            if (FileFilter::GetFileName(data, name, 260) == false)
            {
                return FLT_POSTOP_FINISHED_PROCESSING;
            }
            
            ULONG written = data->Iopb->Parameters.Write.Length;

            // DebugMessage("File %ws is written %x bytes,  \r\n", name, written);
            
            if (RtlStringCchPrintfW(msg, 500, L"File %ws is written %x bytes,  \r\n", name, written) == STATUS_SUCCESS);
            {
                // DebugMessage("Sending msg");
                com::ComPort::Send(msg, 500 * sizeof(WCHAR));
                // DebugMessage("Sent msg");
            }

        }

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreSetInfoOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID* completion_context)
    {
        NTSTATUS status;
        PDF_STREAM_CONTEXT streamContext = NULL;
        BOOLEAN race;

        PAGED_CODE();

        // below is for file deletion
        switch (data->Iopb->Parameters.SetFileInformation.FileInformationClass) {

        case FileDispositionInformation:
        case FileDispositionInformationEx:

            status = GetOrSetContext(flt_objects,
                data->Iopb->TargetFileObject,
                (PFLT_CONTEXT *)&streamContext,
                FLT_STREAM_CONTEXT);

            if (!NT_SUCCESS(status)) {

                return FLT_PREOP_SUCCESS_NO_CALLBACK;
            }

            race = (InterlockedIncrement(&streamContext->NumOps) > 1);
            //
            //  Race detection logic. The NumOps field in the StreamContext
            //  counts the number of in-flight changes to delete disposition
            //  on the stream.
            //
            //  If there's already some operations in flight, don't bother
            //  doing postop. Since there will be no postop, this value won't
            //  be decremented, staying forever 2 or more, which is one of
            //  the conditions for checking deletion at post-cleanup.
            //  
            //  It means that we just log only one FileDispositionInformation or FileDispositionInformationEx
            if (!race) {
                *completion_context = (PVOID)streamContext;

                return FLT_PREOP_SYNCHRONIZE;
            }
            else {

                FltReleaseContext(streamContext);
            }

            // FALL_THROUGH

        default:

            return FLT_PREOP_SUCCESS_NO_CALLBACK;

            break;
        }
    }


    FLT_POSTOP_CALLBACK_STATUS FileFilter::PostSetInfoOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID completion_context, FLT_POST_OPERATION_FLAGS flags)
    {
        PDF_STREAM_CONTEXT stream_context;

        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(flags);

        PAGED_CODE();

        // below is for file deletion
        
        ASSERT((data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation) ||
            (data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformationEx));

        ASSERT(NULL != completion_context);
        stream_context = (PDF_STREAM_CONTEXT)completion_context;

        if (NT_SUCCESS(data->IoStatus.Status)) {

            if (data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformationEx) {

                ULONG file_flags = ((PFILE_DISPOSITION_INFORMATION_EX)data->Iopb->Parameters.SetFileInformation.InfoBuffer)->Flags;

                if (FlagOn(file_flags, FILE_DISPOSITION_ON_CLOSE)) {

                    stream_context->DeleteOnClose = BooleanFlagOn(file_flags, FILE_DISPOSITION_DELETE);

                }
                else {

                    stream_context->SetDisp = BooleanFlagOn(file_flags, FILE_DISPOSITION_DELETE);
                }

            }
            else {

                stream_context->SetDisp = ((PFILE_DISPOSITION_INFORMATION)data->Iopb->Parameters.SetFileInformation.InfoBuffer)->DeleteFile;
            }
        }

        InterlockedDecrement(&stream_context->NumOps);

        FltReleaseContext(stream_context);

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    FLT_PREOP_CALLBACK_STATUS FileFilter::PreCleanupOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID* completion_context)
    {
        PDF_STREAM_CONTEXT stream_context;
        NTSTATUS status;

        UNREFERENCED_PARAMETER(flt_objects);

        PAGED_CODE();

        status = FltGetStreamContext(data->Iopb->TargetInstance,
            data->Iopb->TargetFileObject,
            (PFLT_CONTEXT *)&stream_context);

        if (NT_SUCCESS(status)) {

            status = GetFileNameInformation(data, stream_context);

            if (NT_SUCCESS(status)) {

                // pass from pre-callback to post-callback
                *completion_context = (PVOID)stream_context;
                return FLT_PREOP_SYNCHRONIZE;

            }
            else {

                FltReleaseContext(stream_context);
            }
        }

        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    }

    FLT_POSTOP_CALLBACK_STATUS FileFilter::PostCleanupOperation(PFLT_CALLBACK_DATA data, PCFLT_RELATED_OBJECTS flt_objects, PVOID completion_context, FLT_POST_OPERATION_FLAGS flags)
    {
        UNREFERENCED_PARAMETER(flt_objects);
        UNREFERENCED_PARAMETER(flags);

        FILE_STANDARD_INFORMATION file_info;
        PDF_STREAM_CONTEXT stream_context = NULL;
        NTSTATUS status;
        WCHAR msg[500] = { 0 };

        UNREFERENCED_PARAMETER(completion_context);

        UNREFERENCED_PARAMETER(flags);

        PAGED_CODE();

        // assert we're not draining.
        ASSERT(!FlagOn(flags, FLTFL_POST_OPERATION_DRAINING));
        // below is for file deletion
        
        // pass from pre-callback to post-callback
        ASSERT(NULL != completion_context);
        stream_context = (PDF_STREAM_CONTEXT)completion_context;

        if (NT_SUCCESS(data->IoStatus.Status)) {
            if (((stream_context->NumOps > 0) ||
                (stream_context->SetDisp) ||
                (stream_context->DeleteOnClose)) &&
                (0 == stream_context->IsNotified)) {

                status = FltQueryInformationFile(data->Iopb->TargetInstance,
                    data->Iopb->TargetFileObject,
                    &file_info,
                    sizeof(file_info),
                    FileStandardInformation,
                    NULL);
                
                if (STATUS_FILE_DELETED == status) {
                    // Cons:
                    // If there are additional hard links (possible on NTFS & UDF) then the file system
                    // just only removes this name from the namespace and decrements the link count, but
                    // retains the file data. But in here we will count that also a file deletion and do 
                    // not handle that case.

                    if (stream_context->NameInfo->Name.MaximumLength > 0 && 
                        stream_context->NameInfo->Name.Length > 0 && 
                        stream_context->NameInfo->Name.Buffer != NULL)
                    {
                        // DebugMessage("A file is deleted with context (%p), name %ws \r\n", stream_context, stream_context->NameInfo->Name.Buffer);

                        if (RtlStringCchPrintfW(msg, 500, L"A file is deleted with context (%p), name %ws \r\n", stream_context, stream_context->NameInfo->Name.Buffer) == STATUS_SUCCESS);
                        {
                            // DebugMessage("Sending msg");
                            com::ComPort::Send(msg, 500 * sizeof(WCHAR));
                            // DebugMessage("Sent msg");
                        }

                    }
                    else
                    {
                        // DebugMessage("A file is deleted with context (%p), without name \r\n", stream_context);
                        if (RtlStringCchPrintfW(msg, 500, L"A file is deleted with context (%p), without name \r\n", stream_context) == STATUS_SUCCESS);
                        {
                            // DebugMessage("Sending msg");
                            com::ComPort::Send(msg, 500 * sizeof(WCHAR));
                            // DebugMessage("Sent msg");
                        }

                    }
                    InterlockedIncrement((volatile LONG *)&(stream_context->IsNotified));
                }
            }
        }
        // _postcleanupexit:
        FltReleaseContext(stream_context);

        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    void FileFilter::StreamContextCleanupOperation(PDF_STREAM_CONTEXT stream_context, FLT_CONTEXT_TYPE context_type)
    {
        UNREFERENCED_PARAMETER(context_type);

        PAGED_CODE();

        ASSERT(context_type == FLT_STREAM_CONTEXT);

        if (stream_context->NameInfo != NULL) {
            FltReleaseFileNameInformation(stream_context->NameInfo);
            stream_context->NameInfo = NULL;
        }
    }

    NTSTATUS FileFilter::Unload()
    {
        // DebugMessage("FileFilter Unload: Entered");
        if (g_filter_handle_ != NULL)
        {
            FltUnregisterFilter(g_filter_handle_);
            g_filter_handle_ = NULL;
        }
        return STATUS_SUCCESS;
    }

    NTSTATUS FileFilter::GetFileNameInformation(PFLT_CALLBACK_DATA data, PDF_STREAM_CONTEXT stream_context)
    {
        NTSTATUS status;
        PFLT_FILE_NAME_INFORMATION old_name_info;
        PFLT_FILE_NAME_INFORMATION new_name_info;

        PAGED_CODE();

        status = FltGetFileNameInformation(data,
            (FLT_FILE_NAME_OPENED |
                FLT_FILE_NAME_QUERY_DEFAULT),
            &new_name_info);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        status = FltParseFileNameInformation(new_name_info);

        if (!NT_SUCCESS(status)) {
            return status;
        }

        old_name_info = (PFLT_FILE_NAME_INFORMATION)InterlockedExchangePointer((volatile PVOID*)(&stream_context->NameInfo),
            (PVOID)new_name_info);

        if (NULL != old_name_info) {

            FltReleaseFileNameInformation(old_name_info);
        }

        return status;
    }

    NTSTATUS FileFilter::AllocateContext(FLT_CONTEXT_TYPE context_type, PFLT_CONTEXT* context)
    {
        NTSTATUS status;

        PAGED_CODE();

        switch (context_type) {

        case FLT_STREAM_CONTEXT:

            status = FltAllocateContext(FileFilter::g_filter_handle_,
                FLT_STREAM_CONTEXT,
                sizeof(DF_STREAM_CONTEXT),
                FF_CONTEXT_POOL_TYPE,
                context);

            if (NT_SUCCESS(status)) {
                RtlZeroMemory(*context, sizeof(DF_STREAM_CONTEXT));
            }

            return status;

        default:

            return STATUS_INVALID_PARAMETER;
        }
    }

    NTSTATUS FileFilter::SetContext(PCFLT_RELATED_OBJECTS flt_objects, PVOID target, FLT_CONTEXT_TYPE context_type, PFLT_CONTEXT new_context, PFLT_CONTEXT* old_context)
    {
        PAGED_CODE();

        switch (context_type) {

        case FLT_STREAM_CONTEXT:

            return FltSetStreamContext(flt_objects->Instance,
                (PFILE_OBJECT)target,
                FLT_SET_CONTEXT_KEEP_IF_EXISTS,
                new_context,
                old_context);

        default:

            ASSERT(!"Unexpected context type!\n");

            return STATUS_INVALID_PARAMETER;
        }
    }

    NTSTATUS FileFilter::GetContext(PCFLT_RELATED_OBJECTS flt_objects, PVOID target, FLT_CONTEXT_TYPE context_type, PFLT_CONTEXT* context)
    {
        PAGED_CODE();

        switch (context_type) {

        case FLT_STREAM_CONTEXT:

            return FltGetStreamContext(flt_objects->Instance,
                (PFILE_OBJECT)target,
                context);
            
        default:
            break;
        }

        return STATUS_INVALID_PARAMETER;
    }

    NTSTATUS FileFilter::GetOrSetContext(PCFLT_RELATED_OBJECTS flt_objects, PVOID target, PFLT_CONTEXT* context, FLT_CONTEXT_TYPE context_type)
    {
        NTSTATUS status;
        PFLT_CONTEXT new_context;
        PFLT_CONTEXT old_context;

        PAGED_CODE();

        ASSERT(NULL != context);

        new_context = *context;

        status = GetContext(flt_objects,
            target,
            context_type,
            &old_context);

        if (STATUS_NOT_FOUND == status) {
            if (NULL == new_context) {
                status = AllocateContext(context_type, &new_context);
                if (!NT_SUCCESS(status)) {
                    return status;
                }
            }
        }
        else if (!NT_SUCCESS(status)) {
            return status;
        }
        else {
            ASSERT(new_context != old_context);

            if (NULL != new_context) {

                FltReleaseContext(new_context);
            }

            *context = old_context;
            return status;
        }

        status = SetContext(flt_objects,
            target,
            context_type,
            new_context,
            &old_context);

        if (!NT_SUCCESS(status)) {
            FltReleaseContext(new_context);

            if (STATUS_FLT_CONTEXT_ALREADY_DEFINED == status) {
                *context = old_context;
                return STATUS_SUCCESS;
            }
            else {
                *context = NULL;
                return status;
            }
        }

        *context = new_context;
        return status;
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
          FileFilter::PreWriteOperation,
          FileFilter::PostWriteOperation },

        { IRP_MJ_SET_INFORMATION,
          FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO,
          FileFilter::PreSetInfoOperation,
          FileFilter::PostSetInfoOperation },
        
        { IRP_MJ_CLEANUP,
          0,
          FileFilter::PreCleanupOperation,
          FileFilter::PostCleanupOperation },
        
        { IRP_MJ_OPERATION_END }
    };

    const FLT_CONTEXT_REGISTRATION FileFilter::contexts_[] = {

        { FLT_STREAM_CONTEXT,
          0,
          (PFLT_CONTEXT_CLEANUP_CALLBACK)FileFilter::StreamContextCleanupOperation,
          sizeof(DF_STREAM_CONTEXT),
          FF_STREAM_CONTEXT_POOL_TAG,
          NULL,
          NULL,
          NULL },

        { FLT_CONTEXT_END }

    };

    const FLT_REGISTRATION FileFilter::filter_registration_ =
    {
        sizeof(FLT_REGISTRATION),
        FLT_REGISTRATION_VERSION,
        0,                                  //  flags

        FileFilter::contexts_,                          //  Context
        FileFilter::callbacks_,                         //  Operation callbacks

        (PFLT_FILTER_UNLOAD_CALLBACK)::FilterUnload,    //  MiniFilterUnload

        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL,
        NULL
    };

}

