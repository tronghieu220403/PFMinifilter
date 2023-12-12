#include "ProcessFilter.h"

#pragma warning( disable : 4302 4311 )

NTSTATUS filter::ProcessFilter::Register()
{
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
	PEPROCESS process = NULL;
	PUNICODE_STRING parentProcessName = NULL, processName = NULL;

	PsLookupProcessByProcessId(ppid, &process);
	SeLocateProcessImageName(process, &parentProcessName);

	PsLookupProcessByProcessId(pid, &process);
	SeLocateProcessImageName(process, &processName);

	if (create)
	{
		DbgPrint("%d %wZ\n\t\t%d %wZ", (int)ppid, parentProcessName, (int)pid, processName);
	}
	else
	{
		DbgPrint("Process %d lost child %wZ, pid: %d", (int)ppid, processName, (int)pid);
	}
}

void filter::ProcessFilter::Unload()
{
    PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&filter::ProcessFilter::CreateOperation, TRUE);
}
