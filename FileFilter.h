#pragma once

#include <fltKernel.h>
#include <dontuse.h>

#include "Debug.h"
#include "ComPort.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

EXTERN_C_START

extern NTSTATUS FilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

EXTERN_C_END

namespace filter
{

    #define FF_CONTEXT_POOL_TYPE            PagedPool
    #define FF_STREAM_CONTEXT_POOL_TAG      'xSfD'

    typedef union _DF_FILE_REFERENCE {

        struct {
            ULONGLONG   Value;          //  The 64-bit file ID lives here.
            ULONGLONG   UpperZeroes;    //  In a 64-bit file ID this will be 0.
        } FileId64;
        UCHAR           FileId128[16];  //  The 128-bit file ID lives here.

    } DF_FILE_REFERENCE, * PDF_FILE_REFERENCE;

    #define DfSizeofFileId(FID) (               \
                ((FID).FileId64.UpperZeroes == 0ll) ?   \
                    sizeof((FID).FileId64.Value)    :   \
                    sizeof((FID).FileId128)             \
                )

    typedef struct _DF_STREAM_CONTEXT {
        PFLT_FILE_NAME_INFORMATION  NameInfo;
        DF_FILE_REFERENCE           FileId;
        volatile LONG               NumOps; //  Number of SetDisp operations in flight.
        volatile LONG               IsNotified; //  IsNotified == 1 means a file/stream deletion was already notified.
        BOOLEAN                     FileIdSet; //  Whether or not we've already queried the file ID.
        BOOLEAN                     SetDisp; //  Delete Disposition for this stream.
        BOOLEAN                     DeleteOnClose; //  Delete-on-Close state for this stream.
    } DF_STREAM_CONTEXT, * PDF_STREAM_CONTEXT;

    class FileFilter
    {
    private:
        inline static PDRIVER_OBJECT p_driver_object_ = { 0 };
        inline static PFLT_FILTER g_filter_handle_ = { 0 };
        static const FLT_OPERATION_REGISTRATION callbacks_[]; //  operation registration
        static const FLT_CONTEXT_REGISTRATION contexts_[]; //  operation registration
        static const FLT_REGISTRATION filter_registration_; //  This defines what we want to filter with FltMgr

    public:

        static NTSTATUS Register();

        static FLT_PREOP_CALLBACK_STATUS PreCreateOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID* completion_context
        );

        static FLT_POSTOP_CALLBACK_STATUS PostCreateOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID completion_context,
            FLT_POST_OPERATION_FLAGS flags
        );

        static FLT_PREOP_CALLBACK_STATUS PreWriteOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID* completion_context
        );

        static FLT_POSTOP_CALLBACK_STATUS PostWriteOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID completion_context,
            FLT_POST_OPERATION_FLAGS flags
        );

        static FLT_PREOP_CALLBACK_STATUS PreSetInfoOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID* completion_context
        );

        static FLT_POSTOP_CALLBACK_STATUS PostSetInfoOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID completion_context,
            FLT_POST_OPERATION_FLAGS flags
        );

        static FLT_PREOP_CALLBACK_STATUS PreCleanupOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID* completion_context
        );

        static FLT_POSTOP_CALLBACK_STATUS PostCleanupOperation(
            PFLT_CALLBACK_DATA data,
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID completion_context,
            FLT_POST_OPERATION_FLAGS flags
        );

        static void StreamContextCleanupOperation(
            PDF_STREAM_CONTEXT stream_context,
            FLT_CONTEXT_TYPE context_type
        );

        static NTSTATUS Unload();




        static NTSTATUS GetFileNameInformation(
            PFLT_CALLBACK_DATA data,
            PDF_STREAM_CONTEXT stream_context
        );

        static NTSTATUS AllocateContext(
            FLT_CONTEXT_TYPE context_type,
            PFLT_CONTEXT* context
        );

        static NTSTATUS SetContext(
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID target,
            FLT_CONTEXT_TYPE context_type,
            PFLT_CONTEXT new_context,
            PFLT_CONTEXT* old_context
        );

        static NTSTATUS GetContext(
            PCFLT_RELATED_OBJECTS flt_objects, 
            PVOID target, 
            FLT_CONTEXT_TYPE context_type, 
            PFLT_CONTEXT* context
        );


        static NTSTATUS GetOrSetContext(
            PCFLT_RELATED_OBJECTS flt_objects,
            PVOID target,
            PFLT_CONTEXT* context,
            FLT_CONTEXT_TYPE context_type
        );


        static bool GetFileName(PFLT_CALLBACK_DATA data, PWCHAR name, DWORD32 size);

        static void SetDriverObjectPtr(const PDRIVER_OBJECT p_driver_object);
        static PDRIVER_OBJECT GetDriverObjectPtr();

        static void SetFilterHandle(const PFLT_FILTER g_filter_handle);
        static PFLT_FILTER GetFilterHandle();

    };


}

