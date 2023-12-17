#include "PFMinifilter.h"

NTSTATUS DriverEntry (PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER( RegistryPath );
    DebugMessage("PFMinifilter!DriverEntry: Entered\n");

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

NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags)
{
    UNREFERENCED_PARAMETER(Flags);

    DebugMessage("Driver Unload Called \r\n");
    filter::FileFilter::Unload();
    filter::ProcessFilter::Unload();
    return STATUS_SUCCESS;
}

