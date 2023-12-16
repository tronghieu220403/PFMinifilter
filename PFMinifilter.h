#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "ProcessFilter.h"

#pragma warning( disable : 4083 4024 4047 )

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")


PDRIVER_OBJECT gDriverObject = { 0 };
PFLT_FILTER gFilterHandle = { 0 };

/*************************************************************************
    Prototypes
*************************************************************************/

EXTERN_C_START

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

<<<<<<< HEAD
void FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
=======
void DriverUnload(PDRIVER_OBJECT DriverObject);
>>>>>>> parent of b207d43 (Update)

EXTERN_C_END

//
//  Assign text sections for each routine.
//

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
<<<<<<< HEAD
#pragma alloc_text(PAGE, FilterUnload)
#endif

NTSTATUS FileFilterRegister();

FLT_PREOP_CALLBACK_STATUS FileFilterPreCreateOperation(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
);

FLT_PREOP_CALLBACK_STATUS FileFilterPreWriteOperation(
    PFLT_CALLBACK_DATA Data,
    PCFLT_RELATED_OBJECTS FltObjects,
    PVOID* CompletionContext
);

NTSTATUS FileFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

CONST FLT_OPERATION_REGISTRATION gCallbacks[] =
{
    { IRP_MJ_CREATE,
        0,
        (PFLT_PRE_OPERATION_CALLBACK)&FileFilterPreCreateOperation,
        NULL },

    { IRP_MJ_WRITE,
        0,
        (PFLT_PRE_OPERATION_CALLBACK)&FileFilterPreWriteOperation,
        NULL },

    { IRP_MJ_OPERATION_END }
};

CONST FLT_REGISTRATION gFilterRegistration =
{
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    0,                                  //  Flags

    NULL,                               //  Context
    &gCallbacks,                         //  Operation callbacks

    (PFLT_FILTER_UNLOAD_CALLBACK)&FilterUnload, //  MiniFilterUnload

    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};
=======
#pragma alloc_text(PAGE, DriverUnload)
#endif
>>>>>>> parent of b207d43 (Update)
