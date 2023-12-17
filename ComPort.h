#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

namespace communication
{
	class ComPort
	{
	private:
		PFLT_FILTER g_filter_handle_ = { 0 };

	public:

		NTSTATUS Create();
		NTSTATUS Send();
		NTSTATUS Revc();
		NTSTATUS Close();

	};
}