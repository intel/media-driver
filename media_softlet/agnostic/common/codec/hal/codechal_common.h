/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     codechal_common.h
//! \brief    Defines the public interface for CodecHal.
//!
#ifndef __CODECHAL_COMMON_H__
#define __CODECHAL_COMMON_H__

#include "mos_os.h"
#include "mos_util_debug.h"

class CodechalDebugInterface;
class CodechalHwInterface;
class CodechalSetting;

//------------------------------------------------------------------------------
// Simplified macros for debug message, Assert, Null check and MOS eStatus check
// within Codechal without the need to explicitly pass comp and sub-comp name
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Macros specific to MOS_CODEC_SUBCOMP_PUBLIC sub-comp
//------------------------------------------------------------------------------
#define CODECHAL_PUBLIC_ASSERT(_expr)                                                   \
    MOS_ASSERT(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _expr)

#define CODECHAL_PUBLIC_ASSERTMESSAGE(_message, ...)                                    \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define CODECHAL_PUBLIC_NORMALMESSAGE(_message, ...)                                    \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define CODECHAL_PUBLIC_VERBOSEMESSAGE(_message, ...)                                   \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _message, ##__VA_ARGS__)

#define CODECHAL_PUBLIC_FUNCTION_ENTER                                                  \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC)

#define CODECHAL_PUBLIC_CHK_STATUS_RETURN(_stmt)                                        \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _stmt)

#define CODECHAL_PUBLIC_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _stmt, _message, ##__VA_ARGS__)

#define CODECHAL_PUBLIC_CHK_NULL_RETURN(_ptr)                                           \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _ptr)

#define CODECHAL_PUBLIC_CHK_NULL_NO_STATUS_RETURN(_ptr)                                 \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_PUBLIC, _ptr)

//!
//! \class Codechal
//! \brief This class defines the common member fields, functions etc as Codechal base class.
//!
class Codechal
{
public:
    //!
    //! \brief    Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //!
    Codechal(
        CodechalHwInterface    *hwInterface,
        CodechalDebugInterface *debugInterface);

    //!
    //! \brief    Copy constructor
    //!
    Codechal(const Codechal&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    Codechal& operator=(const Codechal&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~Codechal();

    //!
    //! \brief    Allocate and intialize the Codechal.
    //! \param    [in] codecHalSettings
    //!           Settings used to finalize the creation of the CodecHal device
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS Allocate(CodechalSetting *codecHalSettings);

    //!
    //! \brief    Signals the beginning of a picture.
    //! \details  Initializes necessary parameters to perform the requested operation.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS BeginFrame();

    //!
    //! \brief    Signals the end of a picture.
    //! \details  This function closes out the picture which was started by BeginFrame().
    //!           All Execute() calls for a particular picture must be complete before 
    //!           EndFrame() is called. Resets all current picture parameters in 
    //!           preparation for the next BeginFrame(). For decode, in the case 
    //!           of incomplete frames, if the picture is still incomplete at EndFrame(), 
    //!           CodecHal conceals the error internally and submits the codec workload.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS EndFrame();

    //!
    //! \brief    Performs the operation requested by the codec function.
    //! \param    [in] params
    //!           Parameters need to perform the requested function. The parameter structure
    //!           changes based on codec function.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS Execute(void *params);

    //!
    //! \brief    Gets available statuses for executed pictures.
    //! \details  All pictures for which EndFrame() has been called are eligable
    //!           for status reporting. Once a successful or error status is reported out by 
    //!           CodecHal, it is discarded.
    //! \param    [out] status
    //!           Array to store statuses up to a maximum of wNumStatus, valid pointer 
    //!           must be passed in to GetStatusReport()
    //! \param    [in] numStatus
    //!           The size of the pCodecStatus array
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS GetStatusReport(
        void                *status,
        uint16_t            numStatus);

    //!
    //! \brief  Destroy codechl state
    //!
    //! \return void
    //!
    virtual void Destroy();

    //!
    //! \brief    Resolve MetaData.
    //! \details  Resolve MetaData from Input to Output.
    //! \param    [out] pOutput
    //!           Resolved Metadata resource. 
    //! \param    [in] pInput
    //!           Metadata resource to be resolve.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS ResolveMetaData(PMOS_RESOURCE pInput, PMOS_RESOURCE pOutput);

    //!
    //! \brief    Gets hardware interface.
    //! \return   CodechalHwInterface
    //!           return hardware interface
    //!
    CodechalHwInterface *GetHwInterface() { return m_hwInterface; }

    //!
    //! \brief    Gets OS interface.
    //! \return   PMOS_INTERFACE
    //!           return OS interface
    //!
    PMOS_INTERFACE GetOsInterface() { return m_osInterface; }

    //!
    //! \brief    Gets debug interface.
    //! \return   CodechalDebugInterface
    //!           return debug interface
    //!
    CodechalDebugInterface *GetDebugInterface() { return m_debugInterface; }

    //!
    //! \brief    Check if Apogeios enabled.
    //! \return   bool
    //!           return m_apogeiosEnable
    //!
    bool IsApogeiosEnabled() { return m_apogeiosEnable; }
protected:
    //! \brief    HW Inteface
    //! \details  Responsible for constructing all defined states and commands. 
    //!           Each HAL has a separate OS interface.
    CodechalHwInterface     *m_hwInterface      = nullptr;

    //! \brief    Os Inteface
    //! \details  Used to abstract all OS and KMD interactions such that CodecHal may be 
    //!           OS agnostic. Each HAL has a separate OS interface.
    PMOS_INTERFACE          m_osInterface       = nullptr;

    //! \brief    Interface used for debug dumps.
    //! \details  This interface is only valid for release internal and debug builds.
    CodechalDebugInterface  *m_debugInterface   = nullptr;

    //! \brief    Interface used for debug dumps in GetStatusReport.
    //! \details  This interface is only valid for release internal and debug builds.
    CodechalDebugInterface  *m_statusReportDebugInterface   = nullptr;

    //! \brief    Indicates whether or not using null hardware
    bool                    m_useNullHw[MOS_GPU_CONTEXT_MAX] = { false };

    //! \brief    Apogeios Enable Flag
    bool                    m_apogeiosEnable = false;
};

#endif
