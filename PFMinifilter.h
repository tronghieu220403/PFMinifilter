#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
// #include "ProcessFilter.h"

#pragma warning( disable : 4083 4024 4047 )

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


PDRIVER_OBJECT gDriverObject = { 0 };
PFLT_FILTER gFilterHandle = { 0 };

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

NTSTATUS FileFilterRegister();

NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

NTSTATUS FileFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

FLT_PREOP_CALLBACK_STATUS FileFilterPreCreateOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);
FLT_PREOP_CALLBACK_STATUS FileFilterPreWriteOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, FileFilterRegister)

#pragma alloc_text(PAGE, FileFilterUnload)
#pragma alloc_text(PAGE, FilterUnload)
#endif

CONST FLT_OPERATION_REGISTRATION gCallbacks[3] =
{
    { IRP_MJ_CREATE,
        0,
        FileFilterPreCreateOperation,
        NULL },

    { IRP_MJ_WRITE,
        0,
        FileFilterPreWriteOperation,
        NULL },

    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION gFilterRegistration =
{
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    0,                                  //  Flags

    NULL,                               //  Context
    gCallbacks,                         //  Operation callbacks

    FilterUnload,                   //  MiniFilterUnload

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(RegistryPath);
    DebugMessage("PFMinifilter!DriverEntry: Entered\n");

    gDriverObject = DriverObject;

    status = FileFilterRegister();

    if (!NT_SUCCESS(status))
    {
        DebugMessage("FileFilter: Register not successfull\n");
        return status;
    }

    // status = ProcessRegister();

    if (!NT_SUCCESS(status))
    {
        DebugMessage("ProcessFilter: Register not successfull\n");
    }
    return status;
}

NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Flags);
    DebugMessage("FilterUnload Called \r\n");
    FileFilterUnload(0);
    // ProcessUnload();

    return STATUS_SUCCESS;
}

NTSTATUS FileFilterRegister()
{
    DebugMessage("FileFilter registering hihi\n");

    NTSTATUS status = FltRegisterFilter(gDriverObject,
        &gFilterRegistration,
        &gFilterHandle);

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

    // FLT_ASSERT(NT_SUCCESS(status));

    if (NT_SUCCESS(status)) {

        //
        //  Start filtering i/o
        //

        status = FltStartFiltering(gFilterHandle);

        if (!NT_SUCCESS(status)) {

            FltUnregisterFilter(gFilterHandle);
        }
        else
        {
            DebugMessage("Start filtering\n");
        }
    }
    return status;
}

FLT_PREOP_CALLBACK_STATUS FileFilterPreCreateOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
{
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    // DebugMessage("File Created");
    /*
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    NTSTATUS status;
    const int fize_sz = 520;
    WCHAR Name[fize_sz] = { 0 };

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status))
    {
        status = FltParseFileNameInformation(FileNameInfo);

        if (NT_SUCCESS(status))
        {
            if (FileNameInfo->Name.MaximumLength < fize_sz)
            {
                RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
                DebugMessage("Create file: %ws \r\n", Name);
            }
        }
        FltReleaseFileNameInformation(FileNameInfo);
    }
    */
    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS FileFilterPreWriteOperation(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext)
{
    // Write here
    UNREFERENCED_PARAMETER(Data);
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    // DebugMessage("File Write");

    return FLT_PREOP_SUCCESS_WITH_CALLBACK;
}

NTSTATUS FileFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Flags);
    DebugMessage("FileFilter Unload: Entered");
    FltUnregisterFilter(gFilterHandle);
    return STATUS_SUCCESS;
}
