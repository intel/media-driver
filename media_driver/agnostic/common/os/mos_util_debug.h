/*
* Copyright (c) 2013-2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
//!
//! \file        mos_util_debug.h 
//! \brief 
//!
//!
//! \file     mos_util_debug.h
//! \brief    Common OS Debug and Print utilities across different platform
//! \details  Provides assert and print to debug console functionality
//!           All MOS debug and print utilities will only function in debug or
//!           release internal drivers, in a release driver they will be NOPs.
//!
#ifndef __MOS_UTIL_DEBUG_H__
#define __MOS_UTIL_DEBUG_H__

#include "mos_defs.h"
#include "mos_util_debug_specific.h"

#ifdef __cplusplus
extern "C" {
#endif

#if MOS_MESSAGES_ENABLED

//!
//! \brief Max number or sub-components per debug component
//!
#define MOS_MAX_SUBCOMPONENT_COUNT  20

//!
//! \brief Mos message max buffer size
//!
#define MOS_MAX_MSG_BUF_SIZE        1024

//!
//! \brief Max length of HLT file name
//!
#define MOS_MAX_HLT_FILENAME_LEN    260

//!
//! \brief Debug message prefix string to identify component
//!
extern const PCCHAR MOS_ComponentName[MOS_COMPONENT_COUNT];

//!
//! \brief Debug message string to identify log level
//!
extern const PCCHAR MOS_LogLevelName[MOS_MESSAGE_LVL_COUNT];

//!
//! \brief Define MOS Sub-Component IDs
//!
typedef enum
{
    MOS_SUBCOMP_SELF               = 0,
    MOS_SUBCOMP_HLT                = 1,
    MOS_SUBCOMP_COUNT
} MOS_SELF_SUBCOMP_ID;

//!
//! \brief Define MHW Sub-Component IDs
//!
typedef enum
{
    MOS_HW_SUBCOMP_ALL               = 0,
    MOS_HW_SUBCOMP_COUNT
} MOS_HW_SUBCOMP_ID;

// Define CodecHal Sub-Component IDs
typedef enum
{
    MOS_CODEC_SUBCOMP_DDI          = 0,        // DDI files.
    MOS_CODEC_SUBCOMP_DECODE       = 1,        // Decoders
    MOS_CODEC_SUBCOMP_ENCODE       = 2,        // Encoders
    MOS_CODEC_SUBCOMP_HW           = 3,        // HW interface
    MOS_CODEC_SUBCOMP_PUBLIC       = 4,        // Public interface
    MOS_CODEC_SUBCOMP_DEBUG        = 5,        // Debug interface
    MOS_CODEC_SUBCOMP_CENC         = 6,        // CencDecoders
    MOS_CODEC_SUBCOMP_COUNT                    // Must be last in the list
} MOS_CODEC_SUBCOMP_ID;

// Define VpHal Sub-Component IDs
typedef enum
{
    MOS_VP_SUBCOMP_DDI             = 0,        // DDI files.
    MOS_VP_SUBCOMP_HW              = 1,        // HW interface
    MOS_VP_SUBCOMP_PUBLIC          = 2,        // Public interface
    MOS_VP_SUBCOMP_DEBUG           = 3,        // Debug interface
    MOS_VP_SUBCOMP_RENDER          = 4,        // Render interface
    MOS_VP_SUBCOMP_COUNT                       // Must be last in the list
} MOS_VP_SUBCOMP_ID;

// Define CP Sub-Component IDs
// Mote: please update the diagram above __MOS_USER_FEATURE_KEY_SUB_COMPONENT_CP_TAG
//       in mos_util_user_feature_keys.h if you change this enum.
typedef enum
{
    MOS_CP_SUBCOMP_DDI              = 0,             // CP-related DDIs
    MOS_CP_SUBCOMP_DEVICE           = 1,             // The CP device class
    MOS_CP_SUBCOMP_OS               = 2,             // The CP OS services classes
    MOS_CP_SUBCOMP_PCH_HAL          = 3,             // The CP PCH HAL class
    MOS_CP_SUBCOMP_GPU_HAL          = 4,             // The CP GPU HAL classes
    MOS_CP_SUBCOMP_CODEC            = 5,             // Content Protection portions of the Codec UMD
    MOS_CP_SUBCOMP_UMD_CONTEXT      = 6,             // Content Protection portions of UMD device context
    MOS_CP_SUBCOMP_CMD_BUFFER       = 7,             // Content Protection Command buffer class
    MOS_CP_SUBCOMP_SECURESESSION    = 8,             // The secure session classes
    MOS_CP_SUBCOMP_AUTHCHANNEL      = 9,             // The AuthChannel classes
    MOS_CP_SUBCOMP_DLL              = 10,            // CP DLL classes
    MOS_CP_SUBCOMP_LIB              = 11,            // Lib classes
    MOS_CP_SUBCOMP_MHW              = 12,            // CP MHW classes
    MOS_CP_SUBCOMP_PROTECTEDSESSION = 13,            // Protected session class
    MOS_CP_SUBCOMP_PROTECTED_RESOURCE_SESSION = 14,  // Protected Resource session class
    MOS_CP_SUBCOMP_RTE_HAL          = 15,            // CP RTE HAL class
    MOS_CP_SUBCOMP_CAPS             = 16,            // CP CAPS clas
    MOS_CP_SUBCOMP_CPLIB            = 17,            // CP CPLIB interacting
    MOS_CP_SUBCOMP_COUNT                             // Must be last in the list
} MOS_CP_SUBCOMP_ID;

//!
//! \brief Define DDI Sub-Component IDs
//!
typedef enum
{
    MOS_DDI_SUBCOMP_SELF           = 0,
    MOS_DDI_SUBCOMP_COUNT
} MOS_DDI_SUBCOMP_ID;

//!
//! \brief Define CM Sub-Component IDs
//!
typedef enum
{
    MOS_CM_SUBCOMP_DDI             = 0,   // DDI files.
    MOS_CM_SUBCOMP_SELF            = 1,
    MOS_CM_SUBCOMP_PUBLIC          = 2,   // Public interface
    MOS_CM_SUBCOMP_RENDERHAL       = 3,
    MOS_CM_SUBCOMP_COUNT
} MOS_CM_SUBCOMP_ID;

//!
//! \brief Define Scalability Sub-Component IDs
//!
typedef enum
{
    MOS_SCALABILITY_SUBCOMP_SELF   = 0,
    MOS_SCALABILITY_SUBCOMP_COUNT
} MOS_SCALABILITY_SUBCOMP_ID;

//!
//! \brief Define MMC Sub-Component IDs
//!
typedef enum
{
    MOS_MMC_SUBCOMP_SELF   = 0,
    MOS_MMC_SUBCOMP_COUNT
} MOS_MMC_SUBCOMP_ID;

//!
//! \brief Define BLT Sub-Component IDs
//!
typedef enum
{
    MOS_BLT_SUBCOMP_SELF   = 0,
    MOS_BLT_SUBCOMP_COUNT
} MOS_BLT_SUBCOMP_ID;

//!
//! \brief MOS debug params structure, includes debug level and asserts enabled.
//!
typedef struct _MOS_DEBUG_PARAMS
{
    int32_t             bAssertEnabled;
    MOS_MESSAGE_LEVEL   uiMessageLevel;
} MOS_DEBUG_PARAMS;

//!
//! \brief MOS component debug params structure,
//!        Includes a component's MOS_DEBUG_PARAMS and an array of sub-component params.
//!
typedef struct _MOS_COMPONENT_DEBUG_PARAMS
{
    MOS_DEBUG_PARAMS  component;
    int32_t           bBySubComponent;
    MOS_DEBUG_PARAMS  subComponents[MOS_MAX_SUBCOMPONENT_COUNT];    //!< currently only 16 subcomponent for each component
} MOS_COMPONENT_DEBUG_PARAMS;

//!
//! \brief MOS message params structure
//!        Initialized by MOS_MessageInit() and cleared by  MOS_MessageClose()
//!
typedef struct _MOS_MESSAGE_PARAMS
{
    PFILE                       pLogFile;
    PFILE                       pTraceFile;
    uint32_t                    uiCounter;
    int32_t                     bUseHybridLogTrace;                             //!< Log debug messages and trace dumps to a file or not
    int32_t                     bUseOutputDebugString;                          //!< Onscreen debug message prints enabled or not
    uint32_t                    bEnableMaps;                                    //!< Dump mapped memory regions to trace file
    MOS_COMPONENT_DEBUG_PARAMS  components[MOS_COMPONENT_COUNT];
    char                        g_MosMsgBuffer[MOS_MAX_MSG_BUF_SIZE];           //!< Array for debug message
} MOS_MESSAGE_PARAMS;

//!
//! \brief    Initialize the MOS message params structure and HLT.
//! \details  Initialize the MOS message params structure and HLT,
//!           to be called during device creation
//! \return   void
//!
void MOS_MessageInit();

//!
//! \brief    Frees the MOS message buffer and MOS message parameters structure
//! \details  Frees the MOS message buffer and MOS message parameters structure,
//!           to be called during device destruction
//! \return   void
//!
void MOS_MessageClose();

//!
//! \brief    Add preface information to the HLT log when initialized
//! \details  Add preface information to the HLT log when initialized
//!           Used internally by MOS_HLTInit().
//! \param    PFILE pFile
//!           [out] Pointer to the log file
//! \return   void
//!
void MOS_HltpPreface(
    PFILE            pFile);

//!
//! \brief    Form a string that will prefix MOS's log file name
//! \param    char  *fileNamePrefix
//!           [out] Pointer to the string where the prefix is returned
//! \return   MOS_STATUS
//!           Returns one of the MOS_STATUS error codes if failed,
//!           else MOS_STATUS_SUCCESS
//!
MOS_STATUS MOS_LogFileNamePrefix(char  *fileNamePrefix);

//!
//! \def MOS_FUNCTION_ENTER(_compID, _subCompID)
//!  Output ENTRY message with \_a _compID and \_a _subCompID info
//!
#define MOS_FUNCTION_ENTER(_compID, _subCompID)                                             \
    MOS_DEBUGMESSAGE_NOLINE(MOS_MESSAGE_LVL_FUNCTION_ENTRY, _compID, _subCompID, "")

//!
//! \def MOS_FUNCTION_EXIT(_compID, _subCompID, hr)
//!  Output EXIT message with \_a _compID and \_a _subCompID info and the result hr.
//!
#define MOS_FUNCTION_EXIT(_compID, _subCompID, eStatus)                                           \
    MOS_DEBUGMESSAGE_NOLINE(MOS_MESSAGE_LVL_FUNCTION_EXIT, _compID, _subCompID, ": eStatus = 0x%x", eStatus)

//!
//! \def MOS_FUNCTION_ENTER_VERBOSE(_compID, _subCompID)
//!  Output VERBOSE ENTRY message with \_a _compID and \_a _subCompID info
//!
#define MOS_FUNCTION_ENTER_VERBOSE(_compID, _subCompID)                                     \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_ENTRY_VERBOSE, _compID, _subCompID, "")

//!
//! \def MOS_FUNCTION_EXIT_VERBOSE(_compID, _subCompID)
//!  Output VERBOSE EXIT message with \_a _compID and \_a _subCompID info
//!
#define MOS_FUNCTION_EXIT_VERBOSE(_compID, _subCompID)                                      \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_FUNCTION_EXIT_VERBOSE, _compID, _subCompID, "")

//!
//! \def MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ...)
//!  Output CRITICAL message \a _message with \_a _compID and \_a _subCompID info
//!  and triggers an assert.
//!
#define MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ...)                               \
    MOS_CRITICALMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);                      \
    MOS_ASSERT(_compID, _subCompID, false);

//!
//! \def MOS_NORMALMESSAGE(_compID, _subCompID, _message, ...)
//!  Output NORMAL message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_NORMALMESSAGE(_compID, _subCompID, _message, ...)                               \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_NORMAL, _compID, _subCompID, _message, ##__VA_ARGS__)

//!
//! \def MOS_VERBOSEMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_VERBOSEMESSAGE(_compID, _subCompID, _message, ...)                              \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_VERBOSE, _compID, _subCompID, _message, ##__VA_ARGS__)

//!
//! \def MOS_CRITICALMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_CRITICALMESSAGE(_compID, _subCompID, _message, ...)                             \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, _compID, _subCompID, _message, ##__VA_ARGS__)

//!
//! \def MOS_DEBUGMESSAGE_IF(_cond, _level, _compID, _subCompID, _message, ...)
//!  If \a _cond is true, output message \a _message of level \a _level with
//!  \_a _compID and \_a _subCompID info
//!
#define MOS_DEBUGMESSAGE_IF(_cond, _level, _compID, _subCompID, _message, ...)              \
    if (_cond)                                                                              \
    {                                                                                       \
        MOS_DEBUGMESSAGE(_level, _compID, _subCompID, _message, ##__VA_ARGS__);             \
    }

//!
//! \def MOS_TraceEventExt
//!  this is trace event interface extension, only for debug purpose.
//!
#define MOS_TraceEventExt  MOS_TraceEvent
#define MOS_TraceDump MOS_TraceDataDump

#else // !MOS_MESSAGES_ENABLED

//!
//! \brief   The two methods below are used only for debug or release internal drivers
//!            but are called in release drivers too.
//!
#define MOS_TraceEventExt(usId, usType, pArg1, dwSize1, pArg2, dwSize2)
#define MOS_TraceDump(name, flags, pBuf, dwSize)

#define MOS_FUNCTION_ENTER(_compID, _subCompID)
#define MOS_FUNCTION_EXIT(_compID, _subCompID, hr)
#define MOS_FUNCTION_ENTER_VERBOSE(_compID, _subCompID)
#define MOS_FUNCTION_EXIT_VERBOSE(_compID, _subCompID)
#define MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_NORMALMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_VERBOSEMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_DEBUGMESSAGE_IF(_cond, _level, _compID, _subCompID, _message, ...)
#define MOS_DEBUGMESSAGE(_compID, _subCompID, _message, ...)

#endif // MOS_MESSAGES_ENABLED

#if MOS_ASSERT_ENABLED

//!
//! \def MOS_ASSERT(_compID, _subCompID, _expr)
//!  If \a _expr is not true, asserts with \a _compID and \a _subCompID info
//!
#define MOS_ASSERT(_compID, _subCompID, _expr)                   \
    if(!(_expr))                                                 \
    {                                                            \
        _MOS_Assert(_compID, _subCompID);                        \
    }

//!
//! \brief    MOS assert function for MOS internal use
//! \details  Halts the cpu in debug mode when expression resolves to zero
//!           and only if assert enabled for both comp and sub-comp.
//!           Nop in release version
//!           Called by MOS_ASSERT macro only
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \return   void
//!
void _MOS_Assert(
    MOS_COMPONENT_ID compID,
    uint8_t          subCompID);

#else // MOS_ASSERT_ENABLED

#define MOS_ASSERT(_compID, _subCompID, _expr)

#endif // MOS_ASSERT_ENABLED

//------------------------------------------------------------------------------
// Generic Macros for use by all components.
//------------------------------------------------------------------------------

//!
//! \def MOS_CHK_STATUS_RETURN(_compID, _subCompID, _stmt)
//!  Check MOS_STATUS \a _stmt, assert and return an error for failure
//!
#define MOS_CHK_STATUS_RETURN(_compID, _subCompID, _stmt)                                           \
{                                                                                                   \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);                                                    \
    if (stmtStatus != MOS_STATUS_SUCCESS)                                                           \
    {                                                                                               \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "MOS returned error, eStatus = 0x%x", stmtStatus);   \
        return stmtStatus;                                                                          \
    }                                                                                               \
}

//!
//! \def MOS_CHK_STATUS_BREAK(_compID, _subCompID, _stmt)
//!  Check MOS_STATUS \a _stmt, assert and break out of current loop
//!
#define MOS_CHK_STATUS_BREAK(_compID, _subCompID, _stmt)                                       \
{                                                                                              \
    eStatus = (MOS_STATUS)(_stmt);                                                             \
    if (eStatus != MOS_STATUS_SUCCESS)                                                         \
    {                                                                                          \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "MOS returned error, eStatus = 0x%x", eStatus); \
        break;                                                                                 \
    }                                                                                          \
}

//!
//! \def MOS_CHK_STATUS_MESSAGE_RETURN(_compID, _subCompID, _stmt, _message, ...)
//!  Check MOS_STATUS \a _stmt, assert and return an error for failure, and print message
//!
#define MOS_CHK_STATUS_MESSAGE_RETURN(_compID, _subCompID, _stmt, _message, ...)\
{                                                                               \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);                                \
    if (stmtStatus != MOS_STATUS_SUCCESS)                                       \
    {                                                                           \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);        \
        return stmtStatus;                                                      \
    }                                                                           \
}

//!
//! \def MOS_CHK_NULL_RETURN(_compID, _subCompID, _ptr)
//!  Check if \a _ptr == nullptr, if so assert and return an error
//!
#define MOS_CHK_NULL_RETURN(_compID, _subCompID, _ptr)                      \
{                                                                           \
    if ((_ptr) == nullptr)                                                  \
    {                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");  \
        return MOS_STATUS_NULL_POINTER;                                     \
    }                                                                       \
}

//!
//! \def MOS_CHK_STATUS(_compID, _subCompID, _stmt)
//!  Check MOS_STATUS \a _stmt, assert and return an error for failure
//!
#define MOS_CHK_STATUS(_compID, _subCompID, _stmt)                                             \
{                                                                                              \
    eStatus = (MOS_STATUS)(_stmt);                                                             \
    if (eStatus != MOS_STATUS_SUCCESS)                                                         \
    {                                                                                          \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "MOS returned error, eStatus = 0x%x", eStatus); \
        goto finish;                                                                           \
    }                                                                                          \
}

//!
//! \def MOS_CHK_STATUS_MESSAGE(_compID, _subCompID, _stmt, _message, ...)
//!  Check MOS_STATUS \a _stmt, assert and return an error for failure, and print message
//!
#define MOS_CHK_STATUS_MESSAGE(_compID, _subCompID, _stmt, _message, ...)                   \
{                                                                                           \
    eStatus = (MOS_STATUS)(_stmt);                                                          \
    if (eStatus != MOS_STATUS_SUCCESS)                                                      \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);                    \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_STATUS_SAFE(_stmt)
//!  Check MOS_STATUS \a _stmt, return for failure
//!
#define MOS_CHK_STATUS_SAFE(_stmt)                                                          \
{                                                                                           \
    eStatus = (MOS_STATUS)(_stmt);                                                          \
    if (eStatus != MOS_STATUS_SUCCESS)                                                      \
    {                                                                                       \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_NULL(_compID, _subCompID, _ptr)
//!  Check if \a _ptr == nullptr, if so assert and return an error
//!
#define MOS_CHK_NULL(_compID, _subCompID, _ptr)                                             \
{                                                                                           \
    if ((_ptr) == nullptr)                                                                     \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");                  \
        eStatus = MOS_STATUS_NULL_POINTER;                                                  \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_NULL_NO_STATUS(_compID, _subCompID, _ptr)
//!  Assert and print a message if \a _ptr == nullptr, but not set an error
//!
#define MOS_CHK_NULL_NO_STATUS(_compID, _subCompID, _ptr)                                   \
{                                                                                           \
    if ((_ptr) == nullptr)                                                                     \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");                  \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_NULL_NO_STATUS_RETURN(_compID, _subCompID, _ptr)
//!  Assert and print a message if \a _ptr == nullptr, but not set an error
//!
#define MOS_CHK_NULL_NO_STATUS_RETURN(_compID, _subCompID, _ptr)                            \
{                                                                                           \
    if ((_ptr) == nullptr)                                                                     \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");                  \
        return;                                                                             \
    }                                                                                       \
}

//!
//! \def MOS_CHK_COND(_compID, _subCompID, _condition, _str)
//!  Check if \a _condition is true, if so assert and return an error
//!
#define MOS_CHK_COND(_compID, _subCompID, _condition,  _message, ...)                       \
{                                                                                           \
    if (_condition)                                                                         \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID,  _message, ##__VA_ARGS__);                   \
        eStatus = MOS_STATUS_INVALID_PARAMETER;                                             \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_COND_RETURN(_compID, _subCompID, _condition, _str)
//!  Check if \a _condition is true, if so assert and return an error
//!
#define MOS_CHK_COND_RETURN(_compID, _subCompID, _condition,  _message, ...)                \
{                                                                                           \
    if (_condition)                                                                         \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID,  _message, ##__VA_ARGS__);                   \
        return MOS_STATUS_INVALID_PARAMETER;                                                \
    }                                                                                       \
}

//!
//! \def MOS_CHK_COND_RETURN_VALUE(_compID, _subCompID, _condition, retVal, _str)
//!  Check if \a _condition is true, if so assert and return \a retVal
//!
#define MOS_CHK_COND_RETURN_VALUE(_compID, _subCompID, _condition, retVal,  _message, ...)  \
{                                                                                           \
    if (_condition)                                                                         \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID,  _message, ##__VA_ARGS__);                   \
        return retVal;                                                                      \
    }                                                                                       \
}

//!
//! The following HR macros are temporary until MOS switches to MOS_STATUS. When that happens,
//! and therefore these macros will be moved to an OS specific file.
//!

//!
//! \def MOS_CHK_HR(_compID, _subCompID, _stmt)
//!  Check _stmt, assert and return an error for failure
//!
#define MOS_CHK_HR(_compID, _subCompID, _stmt)                                              \
{                                                                                           \
    hr = (_stmt);                                                                           \
    if (hr != MOS_STATUS_SUCCESS)                                                                         \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "hr check failed.");                         \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_STATUS_MESSAGE(_compID, _subCompID, _stmt, _message, ...)
//!  Check MOS_STATUS \a _stmt, assert and return an error for failure, and print message
//!
#define MOS_CHK_HR_MESSAGE(_compID, _subCompID, _stmt, _message, ...)                       \
{                                                                                           \
    hr = (_stmt);                                                                           \
    if (hr != MOS_STATUS_SUCCESS)                                                                         \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);                    \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHECK_CONDITION(_compID, _subCompID, _condition, _str, _ret)
//!  Check if \a _condition is true, if so assert and print the error message _str
//!  and then return the specified value _ret
//!
#define MOS_CHECK_CONDITION(_compID, _subCompID, _condition, _str, _ret)                    \
{                                                                                           \
    if (_condition)                                                                         \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _str);                                       \
        return _ret;                                                                        \
    }                                                                                       \
}

//------------------------------------------------------------------------------
// Macros for debug message and Assert defined for ease of use within MOS files.
//------------------------------------------------------------------------------

//!
//! \def MOS_OS_CHK_STATUS(_stmt)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_STATUS(_stmt)                                                            \
    MOS_CHK_STATUS(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _stmt)

//!
//! \def MOS_OS_CHK_NULL(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL(_ptr)                                                               \
    MOS_CHK_NULL(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHK_NULL_NO_STATUS(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info without returning a status
//!
#define MOS_OS_CHK_NULL_NO_STATUS(_ptr)                                                               \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHK_NULL_NO_STATUS_RETURN(_ptr)
//!  MOS_ASSERTMESSAGE \a _ptr with MOS utility comp/subcomp info without returning a status
//!
#define MOS_OS_CHK_NULL_NO_STATUS_RETURN(_ptr)                                                               \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHK_STATUS_MESSAGE(_ptr)
//!  MOS_CHK_STATUS_MESSAGE \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_STATUS_MESSAGE(_ptr, _message, ...)                                          \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr, _message, ##__VA_ARGS__)

//!
//! \def MOS_OS_CHK_STATUS_RETURN(_stmt)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_STATUS_RETURN(_stmt)                                                            \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _stmt)

//!
//! \def MOS_OS_CHK_NULL_RETURN(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL_RETURN(_ptr)                                                               \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHK_HR(_ptr)
//!  MOS_CHK_HR \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_HR(_ptr)                                                                 \
    MOS_CHK_HR(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHK_HR_MESSAGE(_ptr)
//!  MOS_CHK_HR_MESSAGE \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_HR_MESSAGE(_ptr, _message, ...)                                          \
    MOS_CHK_HR_MESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr, _message, ##__VA_ARGS__)

//!
//! \def MOS_OS_CHK_NULL_WITH_HR(_ptr)
//!  MOS_CHK_NULL_WITH_HR \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL_WITH_HR(_ptr)                                                       \
    MOS_CHK_NULL_WITH_HR(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHECK_CONDITION(_condition, _str, _ret)
//!  MOS_CHECK_CONDITION \a _condition with MOS utility comp/subcomp info
//!
#define MOS_OS_CHECK_CONDITION(_condition, _str, _ret)                                      \
   MOS_CHECK_CONDITION(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _condition, _str, _ret)

//!
//! \def MOS_OS_ASSERT(_expr)
//!  MOS_ASSERT \a _expr with MOS Utility comp/subcomp info
//!
#define MOS_OS_ASSERT(_expr)                                                                \
    MOS_ASSERT(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _expr)

//!
//! \def MOS_OS_CHK_NULL_RETURN(_ptr)
//!  MOS_CHK_NULL_RETURN \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL_RETURN(_ptr)                                                               \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHK_STATUS(_stmt)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_STATUS_RETURN(_stmt)                                                            \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _stmt)

//!
//! \def MOS_OS_ASSERTMESSAGE(_message, ...)
//!  MOS_ASSERTMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_OS_ASSERTMESSAGE(_message, ...)                                                 \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

//!
//! \def MOS_OS_NORMALMESSAGE(_message, ...)
//!  MOS_UTIL_NORMALMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_OS_NORMALMESSAGE(_message, ...)                                                 \
    MOS_NORMALMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

//!
//! \def MOS_OS_VERBOSEMESSAGE(_message, ...)
//!  MOS_VERBOSEMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_OS_VERBOSEMESSAGE(_message, ...)                                                \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

//!
//! \def MOS_OS_FUNCTION_ENTER
//!  Output ENTRY message with MOS Utility comp/subcomp info
//!
#define MOS_OS_FUNCTION_ENTER                                                               \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF)

#ifdef __cplusplus
}
#endif

#endif // __MOS_UTIL_DEBUG_H__
