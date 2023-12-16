#include "PFMinifilter.h"

NTSTATUS DriverEntry (PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( RegistryPath );
    DebugMessage("PFMinifilter!DriverEntry: Entered\n");

    // DriverObject->DriverUnload = (PDRIVER_UNLOAD)&DriverUnload;

    //
    //  Register with FltMgr to tell it our callback routines
    //
    filter::FileFilter::SetDriverObjectPtr(DriverObject);
    status = filter::FileFilter::Register();

    if (!NT_SUCCESS(status))
    {
        DebugMessage("FileFilter: Register not successfull\n");
        return status;
    }

    status = filter::ProcessFilter::Register();

    if (!NT_SUCCESS(status))
    {
        DebugMessage("ProcessFilter: Register not successfull\n");
    }
    return status;
}

void FilterUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    DebugMessage("Filter Unload Called \r\n");
    filter::FileFilter::Unload(0);
    filter::ProcessFilter::Unload();

    return;
}

