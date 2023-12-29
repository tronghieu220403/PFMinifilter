#include "ProcessFilter.h"

#pragma warning( disable : 4302 4311 )

namespace filter
{

	NTSTATUS ProcessFilter::Register()
	{
		// DebugMessage("ProcessFilter registering\n");
		NTSTATUS status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&ProcessFilter::CreateOperation, FALSE);

		if (!NT_SUCCESS(status)) 
		{
			ProcessFilter::Unload();
		}
		return status;
	}

	void ProcessFilter::CreateOperation(HANDLE ppid, HANDLE pid, BOOLEAN create)
	{
		UNREFERENCED_PARAMETER(ppid);
		WCHAR msg[500] = { 0 };

		if (create)
		{
			PEPROCESS process = NULL;
			PUNICODE_STRING process_name = NULL;

			PsLookupProcessByProcessId(pid, &process);
			SeLocateProcessImageName(process, &process_name);

			if (RtlStringCchPrintfW(msg, 500, L"Process %d is created with name: %wZ", (int)pid, process_name) == STATUS_SUCCESS)
			{
				// DebugMessage("Sending msg");
				com::ComPort::Send(msg, 500 * sizeof(WCHAR));
				// DebugMessage("Sent msg");
			}

			// DebugMessage("Process %d is created with name: %wZ", (int)pid, process_name);
		}
		else
		{
			if (RtlStringCchPrintfW(msg, 500, L"Process %d is terminated.", (int)pid) == STATUS_SUCCESS)
			{
				// DebugMessage("Sending msg");
				com::ComPort::Send(msg, 500 * sizeof(WCHAR));
				// DebugMessage("Sent msg");
			}

			// DebugMessage("Process %d is terminated.", (int)pid);
		}
	}

	void ProcessFilter::Unload()
	{
		// DebugMessage("ProcessFilter Unload: Entered");
		PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)&CreateOperation, TRUE);
	}

}
