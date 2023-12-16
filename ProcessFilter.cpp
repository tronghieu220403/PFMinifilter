#include "ProcessFilter.h"

#pragma warning( disable : 4302 4311 )

NTSTATUS filter::ProcessFilter::Register()
{
	DebugMessage("ProcessFilter registering\n");
    NTSTATUS status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&filter::ProcessFilter::CreateOperation, FALSE);

    FLT_ASSERT(NT_SUCCESS(status));

    if (!NT_SUCCESS(status)) 
    {
        PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&filter::ProcessFilter::CreateOperation, TRUE);
    }
    return status;
}

void filter::ProcessFilter::CreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create)
{
	UNREFERENCED_PARAMETER(ppid);

	if (create)
	{
		PEPROCESS process = NULL;
		// PUNICODE_STRING parentProcessName = NULL;
		PUNICODE_STRING processName = NULL;

		PsLookupProcessByProcessId(pid, &process);
		SeLocateProcessImageName(process, &processName);

		DebugMessage("Process %d is created with name: %wZ", (int)pid, processName);
	}
	else
	{
		DebugMessage("Process %d is terminated.", (int)pid);
	}
}

void filter::ProcessFilter::Unload()
{
    PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&filter::ProcessFilter::CreateOperation, TRUE);
}
