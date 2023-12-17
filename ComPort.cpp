#include "ComPort.h"

namespace communication
{
	NTSTATUS ComPort::Create()
	{
		NTSTATUS status;
		UNICODE_STRING name = RTL_CONSTANT_STRING(L"\\mf");

		status = FltBuildDefaultSecurityDescriptor(&sec_des_, FLT_PORT_ALL_ACCESS);
		if (NT_SUCCESS(status)) {

			InitializeObjectAttributes(&obj_attr_, &name, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, sec_des_);
			status = FltCreateCommunicationPort(p_filter_handle_, &server_port_, &obj_attr_, NULL,
				(PFLT_CONNECT_NOTIFY)ComPort::ConnectHandler,
				(PFLT_DISCONNECT_NOTIFY)ComPort::DisonnectHandler,
				(PFLT_MESSAGE_NOTIFY)ComPort::SendRecvHandler,
				1 // MaxConnections
			);

			if (NT_SUCCESS(status))
			{
				return status;
			}
			else
			{
				ComPort::Close();
			}
		}
		return status;
	}

	NTSTATUS ComPort::Close()
	{
		if (sec_des_ != NULL)
		{
			FltFreeSecurityDescriptor(sec_des_);
			sec_des_ = NULL;
		}

		if (server_port_ == NULL)
		{
			FltCloseCommunicationPort(server_port_);
			server_port_ = NULL;
		}
		return STATUS_SUCCESS;
	}

	NTSTATUS ComPort::ConnectHandler(PFLT_PORT client_port, PVOID server_port_cookie, PVOID connection_context, ULONG size_of_context, PVOID* connection_port_cookie)
	{
		UNREFERENCED_PARAMETER(server_port_cookie);
		UNREFERENCED_PARAMETER(connection_context);
		UNREFERENCED_PARAMETER(size_of_context);
		UNREFERENCED_PARAMETER(connection_port_cookie);

		client_port_= client_port;

		DebugMessage("Connected");

		return STATUS_SUCCESS;
	}

	VOID ComPort::DisonnectHandler(PVOID connection_cookie)
	{
		UNREFERENCED_PARAMETER(connection_cookie);

		FltCloseClientPort(p_filter_handle_, &client_port_);

		DebugMessage("Disonnected");

		return;
	}

	NTSTATUS ComPort::SendRecvHandler(PVOID port_cookie, PVOID input_buffer, ULONG input_buffer_length, PVOID output_buffer, ULONG output_buffer_length, PULONG return_output_buffer_length)
	{
        NTSTATUS status = STATUS_SUCCESS;
        AVSCAN_COMMAND command;
        HANDLE hFile = NULL;
        LONGLONG scanId = 0;
        PAV_SCAN_CONTEXT scanContext = NULL;
        AVSCAN_RESULT scanResult = AvScanResultUndetermined;
        PAV_STREAM_CONTEXT streamContext;
        HANDLE sectionHandle;

        PAGED_CODE();

        UNREFERENCED_PARAMETER(ConnectionCookie);

        AV_DBG_PRINT(AVDBG_TRACE_ROUTINES,
            ("[AV]: AvMessageNotifyCallback entered. \n"));


        if ((InputBuffer == NULL) ||
            (InputBufferSize < (FIELD_OFFSET(COMMAND_MESSAGE, Command) +
                sizeof(AVSCAN_COMMAND)))) {

            return STATUS_INVALID_PARAMETER;
        }

        __try {

            //
            //  Probe and capture input message: the message is raw user mode
            //  buffer, so need to protect with exception handler
            //

            command = ((PCOMMAND_MESSAGE)InputBuffer)->Command;
            scanId = ((PCOMMAND_MESSAGE)InputBuffer)->ScanId;

        } 
        __except(EXCEPTION_EXECUTE_HANDLER) {

            return GetExceptionCode();
        }

        //
        //  Only
        //        AvCmdCreateSectionForDataScan
        //        AvCmdCloseSectionForDataScan
        //  require the check of scanCtxId
        //
        //  We also check the output buffer size, and its alignment here.
        //

        switch (command) {

        case AvCmdCreateSectionForDataScan:

            if ((OutputBufferSize < sizeof(HANDLE)) ||
                (OutputBuffer == NULL)) {

                return STATUS_INVALID_PARAMETER;
            }

            if (!IS_ALIGNED(OutputBuffer, sizeof(HANDLE))) {

                return STATUS_DATATYPE_MISALIGNMENT;
            }

            status = AvGetScanCtxSynchronized(scanId,
                &scanContext);

            if (!NT_SUCCESS(status)) {

                return STATUS_NOT_FOUND;
            }

            status = AvHandleCmdCreateSectionForDataScan(scanContext,
                &sectionHandle);

            if (NT_SUCCESS(status)) {
                //
                //  We succesfully created a section object/handle.
                //  Try to set the handle in the OutputBuffer
                //
                try {

                    (*(PHANDLE)OutputBuffer) = sectionHandle;
                    *ReturnOutputBufferLength = sizeof(HANDLE);

                }  except(AvExceptionFilter(GetExceptionInformation(), TRUE)) {
                    //
                    //  We cannot depend on user service program to close this handle for us.
                    //  We explicitly call NtClose() instead of ZwClose() so that PreviousMode() will be User.
                    //  This prevents accidental closing of a kernel handle and also will not bugcheck the
                    //  system if the handle value is no longer valid
                    //
                    NtClose(sectionHandle);

                    //
                    // Close section and release the waiting I/O request thread
                    // We treat invalid user buffer as an exception and remove
                    // section object inside scan context. You can also design a protocol
                    // that have user program to re-try for section creation failure.
                    //
                    AvFinalizeScanAndSection(scanContext);
                    status = GetExceptionCode();
                }
            }

            //
            //  AvGetScanCtxSynchronized incremented the ref count of scan context
            //
            AvReleaseScanContext(scanContext);

            break;

        case AvCmdCloseSectionForDataScan:

            try {

                scanResult = ((PCOMMAND_MESSAGE)InputBuffer)->ScanResult;

                if (scanResult == AvScanResultInfected) {
                    AV_DBG_PRINT(AVDBG_TRACE_OPERATION_STATUS,
                        ("[AV]: *******AvCmdCreateSectionForDataScan FAILED. \n"));
                }

            } except(AvExceptionFilter(GetExceptionInformation(), TRUE)) {

                return GetExceptionCode();
            }

            status = AvGetScanCtxSynchronized(scanId,
                &scanContext);

            if (!NT_SUCCESS(status)) {

                return STATUS_NOT_FOUND;
            }

            status = AvHandleCmdCloseSectionForDataScan(scanContext, scanResult);

            if (NT_SUCCESS(status)) {
                *ReturnOutputBufferLength = 0;
            }

            //
            //  AvGetScanCtxSynchronized incremented the ref count of scan context
            //
            AvReleaseScanContext(scanContext);

            break;

        case AvIsFileModified:

            try {

                hFile = ((PCOMMAND_MESSAGE)InputBuffer)->FileHandle;

            } except(AvExceptionFilter(GetExceptionInformation(), TRUE)) {

                return GetExceptionCode();
            }

            if ((OutputBufferSize < sizeof(BOOLEAN)) ||
                (OutputBuffer == NULL)) {

                return STATUS_INVALID_PARAMETER;
            }

            if (!IS_ALIGNED(OutputBuffer, sizeof(BOOLEAN))) {

                return STATUS_DATATYPE_MISALIGNMENT;
            }

            //
            //  Get file object by file handle
            //  Get PFLT_VOLUME  by file object
            //  Get instance context by PFLT_VOLUME
            //  Get filter instance in instance context
            //  Get stream context by file object and instance context
            //  Return if the file was previously modified
            //

            status = AvGetStreamContextByHandle(hFile, &streamContext);

            if (!NT_SUCCESS(status)) {

                AV_DBG_PRINT(AVDBG_TRACE_ERROR,
                    ("[AV]: **************************AvGetStreamContextByHandle FAILED. \n"));
                break;
            }

            try {

                (*(PBOOLEAN)OutputBuffer) = (BOOLEAN)IS_FILE_MODIFIED(streamContext);
                *ReturnOutputBufferLength = (ULONG)sizeof(BOOLEAN);

            } except(AvExceptionFilter(GetExceptionInformation(), TRUE)) {

                status = GetExceptionCode();
            }

            FltReleaseContext(streamContext);

            break;

        default:
            return STATUS_INVALID_PARAMETER;
        }

        return status;

    }

	void ComPort::SetPfltFilter(PFLT_FILTER p_filter_handle)
	{
		p_filter_handle_ = p_filter_handle;
	}

	PFLT_FILTER ComPort::GetPfltFilter() const
	{
		return p_filter_handle_;
	}

	
}

