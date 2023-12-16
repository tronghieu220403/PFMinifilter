#include "ProcessFilter.h"

#pragma warning( disable : 4302 4311 )

namespace filter
{

	NTSTATUS ProcessFilter::Register()
	{
		DebugMessage("ProcessFilter registering\n");
		NTSTATUS status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&ProcessFilter::CreateOperation, FALSE);

		if (!NT_SUCCESS(status)) 
		{
			PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&ProcessFilterCreateOperation, TRUE);
		}
		return status;
	}

	void ProcessFilter::CreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create)
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

	void ProcessFilter::Unload()
	{
		DebugMessage("ProcessFilter Unload: Entered");

		PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&ProcessFilterCreateOperation, TRUE);
	}

}
