/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \brief    Common OS Debug and Print utilities across different platform
//! \details  Provides assert and print to debug console functionality
//!           All MOS debug and print utilities will only function in debug or
//!           release internal drivers, in a release driver they will be NOPs.
//!
#ifndef __MOS_UTIL_DEBUG_H__
#define __MOS_UTIL_DEBUG_H__
#include <memory>
#include "mos_defs.h"
#include "mos_os_trace_event.h"
#include "media_class_trace.h"
#include "mos_oca_util_debug.h"

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
//! \brief Define MOS Sub-Component IDs
//!
typedef enum
{
    MOS_SUBCOMP_SELF               = 0,
    MOS_SUBCOMP_HLT                = 1,
    MOS_SUBCOMP_CODEC              = 2,
    MOS_SUBCOMP_VP                 = 3,
    MOS_SUBCOMP_CP                 = 4,
    MOS_SUBCOMP_EXT                = 5,
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
    MOS_CP_SUBCOMP_TEE_HAL          = 15,            // CP TEE HAL class
    MOS_CP_SUBCOMP_CAPS             = 16,            // CP CAPS clas
    MOS_CP_SUBCOMP_CPLIB            = 17,            // CP CPLIB interacting
    MOS_CP_SUBCOMP_CENC             = 18,            // CP cenc class
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
//! \brief Define media copy Sub-Component IDs
//!
typedef enum
{
    MOS_MCPY_SUBCOMP_SELF   = 0,
    MOS_MCPY_SUBCOMP_BLT,
    MOS_MCPY_SUBCOMP_VEBOX,
    MOS_MCPY_SUBCOMP_RENDER,
    MOS_MCPY_SUBCOMP_COUNT
} MOS_MCPY_SUBCOMP_ID;

//!
//! \brief Define messaging levels here in the order of importance
//!
typedef enum
{
    MOS_MESSAGE_LVL_DISABLED                  = 0,
    MOS_MESSAGE_LVL_CRITICAL                  = 1,
    MOS_MESSAGE_LVL_NORMAL                    = 2,
    MOS_MESSAGE_LVL_VERBOSE                   = 3,
    MOS_MESSAGE_LVL_FUNCTION_ENTRY            = 4,
    MOS_MESSAGE_LVL_FUNCTION_EXIT             = 5,
    MOS_MESSAGE_LVL_FUNCTION_ENTRY_VERBOSE    = 6,
    MOS_MESSAGE_LVL_MEMNINJA                  = 7,
    MOS_MESSAGE_LVL_COUNT
} MOS_MESSAGE_LEVEL;

//!
//! \brief Define Component IDs
//! When adding a component, need to update
//!   MOS_COMPONENT_ID,
//!   MOS_ComponentName,
//!   pcComponentUserFeatureKeys,
//!   subComponentCount
//!   and MOS_MESSAGE_COMPONENT_TAG.
//!
typedef enum
{
    MOS_COMPONENT_OS,
    MOS_COMPONENT_HW,
    MOS_COMPONENT_CODEC,
    MOS_COMPONENT_VP,
    MOS_COMPONENT_CP,
    MOS_COMPONENT_DDI,
    MOS_COMPONENT_CM,
    MOS_COMPONENT_CPLIB,
    MOS_COMPONENT_SCALABILITY,
    MOS_COMPONENT_MMC,
    MOS_COMPONENT_MCPY,
    MOS_COMPONENT_COUNT
} MOS_COMPONENT_ID;

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
//!        Initialized by MosUtilDebug::MosMessageInit() and cleared by MosUtilDebug::MosMessageClose()
//!
typedef struct _MOS_MESSAGE_PARAMS
{
    PFILE                       pLogFile;
    PFILE                       pTraceFile;
    uint32_t                    uiCounter;
    int32_t                     bUseHybridLogTrace;                             //!< Log debug messages and trace dumps to a file or not
    int32_t                     bUseOutputDebugString;                          //!< Onscreen debug message prints enabled or not
    uint32_t                    bEnableMaps;                                    //!< Dump mapped memory regions to trace file
    uint32_t                    bDisableAssert;                                 //!< Disable assert
    uint32_t                    bEnableFlush;                                   //!< Enable flush
    uint32_t                    bEnableMemoryFootPrint;                        //!< Disable Memory Foot Print
    MOS_COMPONENT_DEBUG_PARAMS  components[MOS_COMPONENT_COUNT];
    char                        g_MosMsgBuffer[MOS_MAX_MSG_BUF_SIZE];           //!< Array for debug message
} MOS_MESSAGE_PARAMS;

//!
//! When printing from a C++ class, we'd like the class and function to be printed.
//! With our current Linux compiler, __FUNCTION__ does not include the class name.
//! So we use __PRETTY_FUNCTION__ and MOS_Message will remove extra data.
//! This is not needed for prints from C files so they will usually use __FUNCTION__.
//!

#if USE_PRETTY_FUNCTION
#define MOS_FUNCTION __PRETTY_FUNCTION__
#else
#define MOS_FUNCTION __FUNCTION__
#endif // USE_PRETTY_FUNCTION
#endif

class MosUtilDebug
{
public:
    MosUtilDebug()          = delete;
    ~MosUtilDebug()         = delete;

#if MOS_MESSAGES_ENABLED
    //!
    //! \brief    Initialize the MOS message params structure and HLT.
    //! \details  Initialize the MOS message params structure and HLT,
    //!           to be called during device creation
    //! \param    [in] mosCtx
    //!           os device ctx handle
    //! \return   void
    //!
    static void MosMessageInit(MediaUserSettingSharedPtr userSettingPtr);

    //!
    //! \brief    Frees the MOS message buffer and MOS message parameters structure
    //! \details  Frees the MOS message buffer and MOS message parameters structure,
    //!           to be called during device destruction
    //! \return   void
    //!
    static void MosMessageClose();

    //!
    //! \brief    Close file handles and frees resources
    //! \details  Close file handles and frees resources,
    //!           and reopen file handles.To be called before workload submission
    //! \return   void
    //!
    static void MosHLTFlush();

    //!
    //! \brief    Disable Memory Foot Print
    //! \details  Disable Memory Foot Print
    //! \return   bool
    //!
    static bool EnableMemoryFootPrint();

    //!
    //! \brief    Form a string that will prefix MOS's log file name
    //! \details  Form a string that will prefix MOS's log file name
    //!           The default log file location will be under 
    //!           %ProgramData%\Intel\Logs or %DriverData%\Intel\Logs 
    //!           depending on OS version
    //! \param    PCHAR fileNamePrefix
    //!           [out] Pointer to the string where the prefix is returned
    //! \param    [in] userSettingPtr
    //!           MediaUserSettingSharedPtr
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS MosLogFileNamePrefix(char *fileNamePrefix, MediaUserSettingSharedPtr userSettingPtr);

    //!
    //! \brief    Enable or disable asserts of a particular component, it is used by ULT also
    //! \details  Enable or disable asserts of a particular component
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    int32_t bEnable
    //!           [in] Enable/disable flag
    //! \return   void
    //!
    static void MosCompAssertEnableDisable(MOS_COMPONENT_ID compID, int32_t bEnable);

    //!
    //! \brief    Prints debug messages in debug mode when enabled
    //! \details  Prints debug messages if the level of the comp and sub-comp is
    //!           set to less than the message level. Nop in release version.
    //! \param    MOS_MESSAGE_LEVEL level
    //!           [in] Level of the message
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    uint8_t subCompID
    //!           [in] Indicates which sub-component
    //! \param    const PCHAR functionName
    //!           [in] pointer to the function name
    //! \param    int32_t lineNum
    //!           [in] Indicates which line the message locate, -1 for no line output
    //! \param    const PCHAR message
    //!           [in] pointer to the message format string
    //! \param    var_args
    //!           [in] variable list of arguments for the message
    //! \return   VOID
    //!
    static void MosMessage(
        MOS_MESSAGE_LEVEL level,
        MOS_COMPONENT_ID  compID,
        uint8_t           subCompID,
        const PCCHAR      functionName,
        int32_t           lineNum,
        const PCCHAR      message,
        ...);

    //!
    //! \brief    Prints debug messages in debug mode when enabled
    //! \details  Prints debug messages if the level of the comp and sub-comp is
    //!           set to less than the message level. Nop in release version.
    //! \param    MOS_MESSAGE_LEVEL level
    //!           [in] Level of the message
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    uint8_t subCompID
    //!           [in] Indicates which sub-component
    //! \param    const PCHAR functionName
    //!           [in] pointer to the function name
    //! \param    int32_t lineNum
    //!           [in] Indicates which line the message locate, -1 for no line output
    //! \param    const PCHAR message
    //!           [in] pointer to the message format string
    //! \param    var_args
    //!           [in] variable list of arguments for the message
    //! \return   VOID
    //!
    static void MosMessageInternal(
        MOS_MESSAGE_LEVEL level,
        MOS_COMPONENT_ID  compID,
        uint8_t           subCompID,
        const PCCHAR      functionName,
        int32_t           lineNum,
        const PCCHAR      message,
        va_list           var_args);

    //!
    //! \brief    Checks whether debug messages should be printed.
    //! \details  Determines by the print level, component and sub-component IDs
    //!           whether the debug message should be printed.
    //! \param    MOS_MESSAGE_LEVEL level
    //!           [in] Level of the message
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    uint8_t subCompID
    //!           [in] Indicates which sub-component
    //! \param    const char  *message
    //!           [in] pointer to the message format string
    //! \return   int32_t
    //!
    static int32_t MosShouldPrintMessage(
        MOS_MESSAGE_LEVEL level,
        MOS_COMPONENT_ID  compID,
        uint8_t           subCompID,
        const char *const message);
#endif

#if MOS_ASSERT_ENABLED
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
    static void MosAssert(
        MOS_COMPONENT_ID compID,
        uint8_t          subCompID);

#endif // MOS_ASSERT_ENABLED

private:

#if MOS_MESSAGES_ENABLED
    //!
    //! \brief    Add preface information to the HLT log when initialized
    //! \details  Add preface information to the HLT log when initialized
    //!           Used internally by MOS_HLTInit().
    //! \param    PFILE pFile
    //!           [out] Pointer to the log file
    //! \return   void
    //!
    static void MosHltpPreface(
        PFILE            pFile);

    /*----------------------------------------------------------------------------
    | Name      : MOS_HltpCopyFile
    | Purpose   : Copy all file content from the source file to the target file.
    | Arguments : szFileName - source file name to copy from
    |             pFile - target file
    | Returns   : Returns one of the MOS_STATUS error codes if failed,
    |             else MOS_STATUS_SUCCESS
    | Comments  :
    \---------------------------------------------------------------------------*/
    static MOS_STATUS MosHltpCopyFile(PFILE pFile, const PCCHAR szFileName);

    //!
    //! \brief    Set debug message level for a sub-component within a component
    //! \details  Set debug message level for a sub-component within a component
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    uint8_t subCompID
    //!           [in] Indicates which sub-component
    //! \param    MOS_MESSAGE_LEVEL msgLevel
    //!           [in] Message level that the sub component allows
    //! \return   void
    //!
    static void MosSetSubCompMessageLevel(MOS_COMPONENT_ID compID, uint8_t subCompID, MOS_MESSAGE_LEVEL msgLevel);

    //!
    //! \brief    Set debug message level for a particular component
    //! \details  Set debug message level for a particular component
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    MOS_MESSAGE_LEVEL msgLevel
    //!           [in] Message level that the component allows
    //! \return   void
    //!
    static void MosSetCompMessageLevel(MOS_COMPONENT_ID compID, MOS_MESSAGE_LEVEL msgLevel);

    //!
    //! \brief    Set debug message level for all components
    //! \details  Set all component to the same msg level
    //! \param    MOS_MESSAGE_LEVEL msgLevel
    //!           [in] Message level that all components allow
    //! \return   void
    //!
    static void MosSetCompMessageLevelAll(MOS_MESSAGE_LEVEL msgLevel);

    //!
    //! \brief    Enable/disable asserts for a sub-component within a component
    //! \details  Enable/disable asserts for a sub-component within a component
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    uint8_t subCompID
    //!           [in] Indicates which sub-component
    //! \param    int32_t iFlag
    //!           [in] Enable/disable flag
    //! \return   void
    //!
    static void MosSubCompAssertEnableDisable(MOS_COMPONENT_ID compID, uint8_t subCompID, int32_t bEnable);

    //!
    //! \brief    Set debug message level and asserts for a component and its sub-components.
    //! \details  Set debug message level and asserts for a component and its sub-components.
    //!              This includes registering all sub-components.
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    [in] mosCtx
    //!           os device ctx handle
    //! \return   void
    //!
    static void MosMessageInitComponent(MOS_COMPONENT_ID compID, MediaUserSettingSharedPtr userSettingPtr);

    //!
    //! \brief    Initialize or refresh the Hybrid Log and Trace facility
    //! \details  Initialize or refresh the Hybrid Log and Trace facility
    //!           Called during MOS init
    //! \param    [in] mosCtx
    //!           os device ctx handle
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS MosHLTInit( MediaUserSettingSharedPtr userSettingPtr);

    //!
    //! \brief    Close file handles and frees resources
    //! \details  Close file handles and frees resources
    //!           Called during MOS close
    //! \return   void
    //!
    static void MosHLTClose();

#if USE_PRETTY_FUNCTION
    //!
    //! \brief    Converts a __PRETTY_FUNCTION__ into Class::Method
    //! \details  Converts a __PRETTY_FUNCTION__ into Class::Method to allow prettier debug output
    //! \param    PCCHAR pcPrettyFunction
    //!           [in] in the form of "TYPE [CLASS::]FUNCTION(INPUT LIST)"
    //! \return   PCCHAR in the form of [CLASS::]FUNCTION
    //!
    static PCCHAR MosGetClassMethod(PCCHAR pcPrettyFunction);
#endif

#endif // MOS_MESSAGES_ENABLED

#if MOS_ASSERT_ENABLED
    //!
    //! \brief    Checks whether assert should be hit.
    //! \details  Determines by the component and sub-component IDs
    //!           whether an assert should be hit.
    //! \param    MOS_COMPONENT_ID compID
    //!           [in] Indicates which component
    //! \param    uint8_t subCompID
    //!           [in] Indicates which sub-component
    //! \return   int32_t
    //!
    static int32_t MosShouldAssert(MOS_COMPONENT_ID compID, uint8_t subCompID);
#endif
public:
#if MOS_MESSAGES_ENABLED

    static const char * const m_mosLogLevelName[MOS_MESSAGE_LVL_COUNT];
    static const char * const m_mosComponentName[MOS_COMPONENT_COUNT];

    //Temporarily defined as the reference to compatible with the cases using uf key to enable/disable APG.
    static MOS_MESSAGE_PARAMS m_mosMsgParams;
    static const char* m_pcComponentUserFeatureKeys[MOS_COMPONENT_COUNT][3];
    static const uint8_t m_subComponentCount[MOS_COMPONENT_COUNT];
#endif
MEDIA_CLASS_DEFINE_END(MosUtilDebug)
};

#if MOS_ASSERT_ENABLED

//!
//! \def MOS_ASSERT(_compID, _subCompID, _expr)
//!  If \a _expr is not true, asserts with \a _compID and \a _subCompID info
//!
#define MOS_ASSERT(_compID, _subCompID, _expr)                   \
    if(!(_expr))                                                 \
    {                                                            \
        MosUtilDebug::MosAssert(_compID, _subCompID);            \
    }

#else // MOS_ASSERT_ENABLED

#define MOS_ASSERT(_compID, _subCompID, _expr)

#endif // MOS_ASSERT_ENABLED

#if MOS_MESSAGES_ENABLED

// flush hlt message before workload submission
#define MOS_FLUSH_HLT_MESSAGE MosUtilDebug::MosHLTFlush();

#define MOS_IS_MEMORY_FOOT_PRINT_ENABLED() MosUtilDebug::EnableMemoryFootPrint()


//!
//! \def MOS_DEBUGMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_DEBUGMESSAGE(_level, _compID, _subCompID, _message, ...)                        \
    MosUtilDebug::MosMessage(_level, _compID, _subCompID, MOS_FUNCTION, __LINE__, _message, ##__VA_ARGS__)

//!
//! \def MOS_DEBUGMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_DEBUGMESSAGE_NOLINE(_level, _compID, _subCompID, _message, ...)                 \
    MosUtilDebug::MosMessage(_level, _compID, _subCompID, MOS_FUNCTION, -1, _message, ##__VA_ARGS__)

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
//! \def MOS_CRITICALMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_CRITICALMESSAGE(_compID, _subCompID, _message, ...)                             \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_CRITICAL, _compID, _subCompID, _message, ##__VA_ARGS__)

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
//! \def MOS_MEMNINJAMESSAGE(_compID, _subCompID, _message, ...)
//!  Output DEBUG message \a _message with \_a _compID and \_a _subCompID info
//!
#define MOS_MEMNINJAMESSAGE(_compID, _subCompID, _message, ...)                             \
    MOS_DEBUGMESSAGE(MOS_MESSAGE_LVL_MEMNINJA, _compID, _subCompID, _message, ##__VA_ARGS__)

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
#define MOS_TraceEventExt MOS_TraceEvent
#define MOS_TraceDumpExt  MOS_TraceDataDump

#define MOS_TraceDataExt0 MOS_TraceData0
#define MOS_TraceDataExt  MOS_TraceData

//!
//! \def New trace interface with keyword filter, need have at least 1 param in trace data
//!
#define MOS_TraceDecodePicParam(usId, usType, ...)                                 \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_PICPARAM))                     \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeSliceParam(usId, usType, ...)                               \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_SLICEPARAM))                   \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeTileParam(usId, usType, ...)                                \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_TILEPARAM))                    \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeQMatrix(usId, usType, ...)                                  \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_QMATRIX))                      \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeBitStreamInfo(usId, usType, ...)                                  \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_BITSTREAM_INFO))                     \
    {                                                                                    \
        TR_FILL_PARAM(__VA_ARGS__);                                                      \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                       \
    }
#define MOS_TraceDecodeBitStream(usId, usType, ...)                                \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_BITSTREAM))                    \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeInternal(usId, usType, ...)                                 \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_INTERNAL))                     \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeCommand(usId, usType, ...)                                  \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_COMMAND))                      \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeDstYuv(usId, usType, ...)                                   \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_DSTYUV))                       \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }
#define MOS_TraceDecodeRefYuv(usId, usType, ...)                                   \
    if (MosUtilities::TraceKeyEnabled(TR_KEY_DECODE_REFYUV))                       \
    {                                                                              \
        TR_FILL_PARAM(__VA_ARGS__);                                                \
        TR_WRITE_PARAM(MosUtilities::MosTraceEvent, usId, usType);                 \
    }

#define MT_LOG(id, lvl)                                                     \
    {                                                                       \
        int32_t _head[] = {id, lvl};                                         \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 0, _head, sizeof(_head), nullptr, 0); \
    }

#define MT_LOG1(id, lvl, p1, v1)                                                      \
    {                                                                                 \
        int32_t   _head[] = {id, lvl};                                                 \
        MT_PARAM  _param[] = {p1, v1};                                                  \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 1, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_LOG2(id, lvl, p1, v1, p2, v2)                                              \
    {                                                                                 \
        int32_t   _head[] = {id, lvl};                                                 \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}};                                      \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 2, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_LOG3(id, lvl, p1, v1, p2, v2, p3, v3)                                      \
    {                                                                                 \
        int32_t   _head[] = {id, lvl};                                                 \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}, {p3, v3}};                            \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 3, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_LOG4(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4)                              \
    {                                                                                 \
        int32_t   _head[] = {id, lvl};                                                 \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}, {p3, v3}, {p4, v4}};                  \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 4, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_LOG5(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4, p5, v5)                                   \
    {                                                                                              \
        int32_t   _head[] = {id, lvl};                                                              \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}, {p3, v3}, {p4, v4}, {p5, v5}};                     \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 5, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_LOG6(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4, p5, v5, p6, v6)                           \
    {                                                                                              \
        int32_t   _head[] = {id, lvl};                                                              \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}, {p3, v3}, {p4, v4}, {p5, v5}, {p6, v6}};           \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 6, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_LOG7(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4, p5, v5, p6, v6, p7, v7)                   \
    {                                                                                              \
        int32_t   _head[] = {id, lvl};                                                              \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}, {p3, v3}, {p4, v4}, {p5, v5}, {p6, v6}, {p7, v7}}; \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_LOG, 7, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_ERR(id)                                                          \
    {                                                                       \
        int32_t _head[] = {id};                                              \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_ERR, 0, _head, sizeof(_head), nullptr, 0); \
    }

#define MT_ERR1(id, p1, v1)                                                           \
    {                                                                                 \
        int32_t   _head[] = {id};                                                      \
        MT_PARAM  _param[] = {p1, v1};                                                  \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_ERR, 1, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_ERR2(id, p1, v1, p2, v2)                                                   \
    {                                                                                 \
        int32_t   _head[] = {id};                                                      \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}};                                      \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_ERR, 2, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#define MT_ERR3(id, p1, v1, p2, v2, p3, v3)                                           \
    {                                                                                 \
        int32_t   _head[] = {id};                                                      \
        MT_PARAM  _param[] = {{p1, v1}, {p2, v2}, {p3, v3}};                            \
        MosUtilities::MosTraceEvent(EVENT_MEDIA_ERR, 3, _head, sizeof(_head), _param, sizeof(_param)); \
    }

#else // !MOS_MESSAGES_ENABLED

#define MOS_FLUSH_HLT_MESSAGE
#define MOS_IS_MEMORY_FOOT_PRINT_ENABLED() 0

//!
//! \brief   The two methods below are used only for debug or release internal drivers
//!            but are called in release drivers too.
//!
#define MOS_TraceEventExt(...)
#define MOS_TraceDumpExt(...)
#define MOS_TraceDataExt0(usId, usType)
#define MOS_TraceDataExt(usId, usType, ...)
#define MOS_TraceDecodePicParam(usId, usType, ...)
#define MOS_TraceDecodeSliceParam(usId, usType, ...)
#define MOS_TraceDecodeTileParam(usId, usType, ...)
#define MOS_TraceDecodeQMatrix(usId, usType, ...)
#define MOS_TraceDecodeBitStreamInfo(usId, usType, ...)
#define MOS_TraceDecodeBitStream(usId, usType, ...)
#define MOS_TraceDecodeInternal(usId, usType, ...)
#define MOS_TraceDecodeCommand(usId, usType, ...)
#define MOS_TraceDecodeDstYuv(usId, usType, ...)
#define MOS_TraceDecodeRefYuv(usId, usType, ...)
#define MT_LOG(id, lvl)
#define MT_LOG1(id, lvl, p1, v1)
#define MT_LOG2(id, lvl, p1, v1, p2, v2)
#define MT_LOG3(id, lvl, p1, v1, p2, v2, p3, v3)
#define MT_LOG4(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4)
#define MT_LOG5(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4, p5, v5)
#define MT_LOG6(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4, p5, v5, p6, v6)
#define MT_LOG7(id, lvl, p1, v1, p2, v2, p3, v3, p4, v4, p5, v5, p6, v6, p7, v7)
#define MT_ERR(id)
#define MT_ERR1(id, p1, v1)
#define MT_ERR2(id, p1, v1, p2, v2)
#define MT_ERR3(id, p1, v1, p2, v2, p3, v3)

#define MOS_FUNCTION_ENTER(_compID, _subCompID)
#define MOS_FUNCTION_EXIT(_compID, _subCompID, hr)
#define MOS_FUNCTION_ENTER_VERBOSE(_compID, _subCompID)
#define MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_NORMALMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_VERBOSEMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_CRITICALMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_DEBUGMESSAGE_IF(_cond, _level, _compID, _subCompID, _message, ...)
#define MOS_DEBUGMESSAGE(_compID, _subCompID, _message, ...)
#define MOS_MEMNINJAMESSAGE(_compID, _subCompID, _message, ...)

#endif // MOS_MESSAGES_ENABLED

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
        MT_ERR3(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, stmtStatus); \
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
        MT_ERR3(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, eStatus); \
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
        MT_ERR3(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, stmtStatus); \
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
        MT_ERR2(MT_ERR_NULL_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
        return MOS_STATUS_NULL_POINTER;                                     \
    }                                                                       \
}

//!
//! \def MOS_CHK_NULL_RETURN(_compID, _subCompID, _ptr)
//!  Check if \a _ptr == nullptr, if so assert and return an error
//!
#define MOS_CHK_NULL_MESSAGE_RETURN(_compID, _subCompID, _ptr, _message, ...)       \
    {                                                                               \
        if ((_ptr) == nullptr)                                                      \
        {                                                                           \
            MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);        \
            MT_ERR2(MT_ERR_NULL_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
            return MOS_STATUS_NULL_POINTER;                                         \
        }                                                                           \
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
        MT_ERR2(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
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
        MT_ERR2(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);                    \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_STATUS_NO_STATUS_RETURN(_compID, _subCompID, _stmt)
//!  Check MOS_STATUS \a _stmt, return void
//!
#define MOS_CHK_STATUS_NO_STATUS_RETURN(_compID, _subCompID, _stmt)                         \
{                                                                                           \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);                                                          \
    if (stmtStatus != MOS_STATUS_SUCCESS)                                                      \
    {                                                                                       \
        MT_ERR3(MT_ERR_MOS_STATUS_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, stmtStatus); \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "MOS returned error, eStatus = 0x%x", stmtStatus);\
        return;                                                                             \
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
    if ((_ptr) == nullptr)                                                                  \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");               \
        MT_ERR2(MT_ERR_NULL_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID);   \
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
    if ((_ptr) == nullptr)                                                                  \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");               \
        MT_ERR2(MT_ERR_NULL_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID);   \
        goto finish;                                                                        \
    }                                                                                       \
}


//!
//! \def MOS_CHK_NULL_RETURN_NULL(_compID, _subCompID, _ptr)
//!  Assert and print a message if \a _ptr == nullptr, but not set an error
//!
#define MOS_CHK_NULL_RETURN_NULL(_compID, _subCompID, _ptr)                                \
{                                                                                        \
    if ((_ptr) == nullptr)                                                               \
    {                                                                                    \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");            \
        MT_ERR2(MT_ERR_NULL_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
        return nullptr;                                                                     \
    }                                                                                    \
}

//!
//! \def MOS_CHK_NULL_NO_STATUS_RETURN(_compID, _subCompID, _ptr)
//!  Assert and print a message if \a _ptr == nullptr, but not set an error
//!
#define MOS_CHK_NULL_NO_STATUS_RETURN(_compID, _subCompID, _ptr)                            \
{                                                                                           \
    if ((_ptr) == nullptr)                                                                  \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "Invalid (nullptr) Pointer.");               \
        MT_ERR2(MT_ERR_NULL_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID);   \
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
        MT_ERR2(MT_ERR_CONDITION_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
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
        MT_ERR2(MT_ERR_CONDITION_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
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
        MT_ERR2(MT_ERR_CONDITION_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID); \
        MOS_ASSERTMESSAGE(_compID, _subCompID,  _message, ##__VA_ARGS__);                   \
        return retVal;                                                                      \
    }                                                                                       \
}

//!
//! \def MOS_CHK_COND_WITH_DESTROY_RETURN_VALUE(_compID, _subCompID, _condition, destroyFunction, retVal, _message)
//!  Check if \a _condition is true, if so assert, call destroy function and return \a retVal
//!
#define MOS_CHK_COND_WITH_DESTROY_RETURN_VALUE(_compID, _subCompID, _condition, destroyFunction, retVal, _message, ...)  \
{                                                                                                                        \
    if (_condition)                                                                                                      \
    {                                                                                                                    \
        destroyFunction();                                                                                               \
        MT_ERR2(MT_ERR_CONDITION_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID);                            \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);                                                 \
        return retVal;                                                                                                   \
    }                                                                                                                    \
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
    if (hr != MOS_STATUS_SUCCESS)                                                           \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "hr check failed.");                         \
        MT_ERR3(MT_ERR_HR_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, hr);    \
        goto finish;                                                                        \
    }                                                                                       \
}

//!
//! \def MOS_CHK_HR_NO_STATUS_RETURN(_compID, _subCompID, _stmt)
//!  Check _stmt, assert and return void
//!
#define MOS_CHK_HR_NO_STATUS_RETURN(_compID, _subCompID, _stmt)                             \
{                                                                                           \
    hr = (_stmt);                                                                           \
    if (hr != MOS_STATUS_SUCCESS)                                                           \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, "MOS returned error, hr = 0x%x", hr);        \
        MT_ERR3(MT_ERR_HR_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, hr);    \
        return;                                                                             \
    }                                                                                       \
}

//!
//! \def MOS_CHK_STATUS_MESSAGE(_compID, _subCompID, _stmt, _message, ...)
//!  Check MOS_STATUS \a _stmt, assert and return an error for failure, and print message
//!
#define MOS_CHK_HR_MESSAGE(_compID, _subCompID, _stmt, _message, ...)                       \
{                                                                                           \
    hr = (_stmt);                                                                           \
    if (hr != MOS_STATUS_SUCCESS)                                                           \
    {                                                                                       \
        MOS_ASSERTMESSAGE(_compID, _subCompID, _message, ##__VA_ARGS__);                    \
        MT_ERR3(MT_ERR_HR_CHECK, MT_COMPONENT, _compID, MT_SUB_COMPONENT, _subCompID, MT_ERROR_CODE, hr);    \
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
//! \def MOS_OS_CHK_NULL_RETURN_NULL(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info with returning nullptr
//!
#define MOS_OS_CHK_NULL_RETURN_NULL(_ptr) \
MOS_CHK_NULL_RETURN_NULL(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

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
//! \def MOS_OS_CHK_NULL_RETURN(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL_RETURN(_ptr)                                                               \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

//!
//! \def MOS_OS_CHK_NULL_MESSAGE_RETURN(_ptr)
//!  MOS_CHK_NULL \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL_MESSAGE_RETURN(_ptr, _message, ...) \
    MOS_CHK_NULL_MESSAGE_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr, _message,  ##__VA_ARGS__)

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
//! \def MOS_OS_CHK_NULL_WITH_HR_RETURN(_ptr)
//!  MOS_CHK_NULL_WITH_HR \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL_WITH_HR_RETURN(_ptr) \
    MOS_CHK_NULL_WITH_HR_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _ptr)

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
//! \def MOS_OS_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ......)
//!  MOS_CHK_STATUS \a _stmt with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_STATUS_MESSAGE_RETURN(_stmt, _message, ...)                                  \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _stmt, _message, ##__VA_ARGS__)

//!
//! \def MOS_OS_CHK_NULL_RETURN_VALUE(_ptr, retVal)
//!  MOS_CHK_COND_RETURN_VALUE \a _ptr with MOS utility comp/subcomp info
//!
#define MOS_OS_CHK_NULL_RETURN_VALUE(_ptr, retVal)                                         \
    MOS_CHK_COND_RETURN_VALUE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, (_ptr == nullptr), retVal, "Invalid (nullptr) Pointer.")

//!
//! \def MOS_OS_ASSERTMESSAGE(_message, ...)
//!  MOS_ASSERTMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_OS_ASSERTMESSAGE(_message, ...)                                                 \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

//!
//! \def MOS_OS_CRITICALMESSAGE(_message, ...)
//!  MOS_CRITICALMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_OS_CRITICALMESSAGE(_message, ...)                                                 \
    MOS_CRITICALMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

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

//!
//! \def MOS_OS_MEMNINJAMESSAGE(_message, ...)
//!  MOS_MEMNINJAMESSAGE \a _message with MOS Utility comp/subcomp info
//!
#define MOS_OS_MEMNINJAMESSAGE(_message, ...)                                               \
    MOS_MEMNINJAMESSAGE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF, _message, ##__VA_ARGS__)

#define MOS_OS_FUNCTION_TRACE()                                                             \
    MOS_FUNCTION_TRACE(MOS_COMPONENT_OS, MOS_SUBCOMP_SELF)

#include "mos_util_debug_specific.h"

#if MOS_MESSAGES_ENABLED

class FunctionTrace
{
public:
    FunctionTrace(MOS_COMPONENT_ID compID, uint8_t subCompID, const char* name) :
        m_compID(compID),
        m_subCompID(subCompID),
        m_name(name)
    {
        MOS_VERBOSEMESSAGE(m_compID, m_subCompID, "Enter Function:%s\r\n", m_name);
    }

    virtual ~FunctionTrace()
    {
        MOS_VERBOSEMESSAGE(m_compID, m_subCompID, "Exit Function:%s\r\n", m_name);
    }

protected:
    MOS_COMPONENT_ID m_compID    = MOS_COMPONENT_COUNT;
    uint8_t          m_subCompID = 0;
    const char       *m_name     = nullptr;
};

#define MOS_FUNCTION_TRACE(_compID, _subCompID) FunctionTrace trace(_compID, _subCompID, __FUNCTION__);

#else

#define MOS_FUNCTION_TRACE(_compID, _subCompID)

#endif // #if MOS_MESSAGES_ENABLED
#endif // __MOS_UTIL_DEBUG_H__
