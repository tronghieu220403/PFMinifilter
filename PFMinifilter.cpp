#include "PFMinifilter.h"

NTSTATUS DriverEntry (PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    NTSTATUS status;

    UNREFERENCED_PARAMETER( RegistryPath );
    DbgPrint("PFMinifilter!DriverEntry: Entered\n");

    DriverObject->DriverUnload = (PDRIVER_UNLOAD)&DriverUnload;

    //
    //  Register with FltMgr to tell it our callback routines
    //
    filter::FileFilter::SetDriverObjectPtr(DriverObject);
    status = filter::FileFilter::Register();

    if (!NT_SUCCESS(status))
    {
        return status;
    }

    status = filter::ProcessFilter::Register();

    return status;
}

void DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    DbgPrint("Driver Unload Called \r\n");
    filter::FileFilter::Unload(0);
    filter::ProcessFilter::Unload();
    return;
}

