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
//! \file        mos_util_debug_next.h 
//! \brief 
//! \brief    Common OS Debug and Print utilities across different platform
//! \details  Provides assert and print to debug console functionality
//!           All MOS debug and print utilities will only function in debug or
//!           release internal drivers, in a release driver they will be NOPs.
//!
#ifndef __MOS_UTIL_DEBUG_NEXT_H__
#define __MOS_UTIL_DEBUG_NEXT_H__

#include "mos_defs.h"
#include "mos_util_debug_specific.h"
#include "media_user_setting.h"

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
#endif // MOS_MESSAGES_ENABLED

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

#if MOS_MESSAGES_ENABLED
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
#endif

#if MOS_MESSAGES_ENABLED

    static const char* const m_mosLogPathTemplate;
    static const char* const m_DdiLogPathTemplate;
    static const char* m_pcComponentUserFeatureKeys[MOS_COMPONENT_COUNT][3];
    static const uint8_t m_subComponentCount[MOS_COMPONENT_COUNT];
    static const PCCHAR m_mosUltLogPathPrefix;
    static const PCCHAR m_mosLogPathPrefix;
#endif
MEDIA_CLASS_DEFINE_END(MosUtilDebug)
};

#if MOS_MESSAGES_ENABLED

class FunctionTrace
{
public:
    FunctionTrace(MOS_COMPONENT_ID compID, uint8_t subCompID, const char* name) :
        m_compID(compID),
        m_subCompID(subCompID),
        m_name(name)
    {
        MOS_VERBOSEMESSAGE(m_compID, m_subCompID, "Enter Function: %s\r\n", m_name);
    }

    virtual ~FunctionTrace()
    {
        MOS_VERBOSEMESSAGE(m_compID, m_subCompID, "Exit Function: %s\r\n", m_name);
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
#endif // __MOS_UTIL_DEBUG_NEXT_H__
