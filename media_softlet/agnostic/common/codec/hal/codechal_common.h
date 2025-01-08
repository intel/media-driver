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
#include "codec_def_common.h"

class CodechalDebugInterface;
class CodechalHwInterfaceNext;
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

#define CODECHAL_PUBLIC_CHK_STATUS_WITH_DESTROY_RETURN(_stmt, destroyFunction)                                                                                    \
    {                                                                                                                                                             \
        MOS_STATUS sts = (MOS_STATUS)(_stmt);                                                                                                                     \
        MOS_CHK_COND_WITH_DESTROY_RETURN_VALUE(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_ENCODE, (MOS_STATUS_SUCCESS != sts), destroyFunction, sts, "error status!") \
    }

typedef struct CODECHAL_SSEU_SETTING
{
    uint8_t ui8NumSlices;
    uint8_t ui8NumSubSlices;
    uint8_t ui8NumEUs;
    uint8_t reserved;  // Place holder for frequency setting
} CODECHAL_SSEU_SETTING, *PCODECHAL_SSEU_SETTING;

//!
//! \class CodechalResLock
//! \brief Help function to lock the resource, the resource will be unlock automatically when this class destroy.
//!
class CodechalResLock
{
public:
    //! \brief lock flag
    enum LockFlag
    {
        readOnly     = 1,       //!< lock with read only
        writeOnly    = 1 << 1,  //!< lock with write only
        tiledAsTiled = 1 << 2,  //!< lock tile surface as tile format
        noOverWrite  = 1 << 3,  //!< lock with no overwrite
        noDecompress = 1 << 4,  //!< lock with no decompress
        uncached     = 1 << 5,  //!< lock with uncached
        forceCached  = 1 << 6   //!< lock with cache by force
    };

    //!
    //! \brief    Constructor
    //!
    CodechalResLock(PMOS_INTERFACE osInterface, PMOS_RESOURCE resource) : m_os(osInterface),
                                                                          m_res(resource),
                                                                          m_lockedPtr(nullptr)
    {
    }

    //!
    //! \brief    Destructor
    //!
    ~CodechalResLock()
    {
        if (m_os != nullptr && m_res != nullptr && m_lockedPtr != nullptr)
        {
            m_os->pfnUnlockResource(m_os, m_res);
        }
    }

    //!
    //! \brief    Lock the resource
    //! \details  Lock the resource, will be unlock automatically when this class destroy
    //! \param  [in] lockFlags
    //!         Combined lock flags, reference to enum LockFlag.
    //!
    //! \return   void pointer
    //!           locked pointer if success, else return null pointer
    //!
    void *Lock(uint32_t lockFlags)
    {
        if (m_os == nullptr || m_res == nullptr)
        {
            return nullptr;
        }

        if (m_lockedPtr == nullptr)
        {
            MOS_LOCK_PARAMS mosLockFlags;
            MOS_ZeroMemory(&mosLockFlags, sizeof(MOS_LOCK_PARAMS));

            if ((lockFlags & readOnly) == readOnly)
            {
                mosLockFlags.ReadOnly = 1;
            }

            if ((lockFlags & writeOnly) == writeOnly)
            {
                mosLockFlags.WriteOnly = 1;
            }

            if ((lockFlags & tiledAsTiled) == tiledAsTiled)
            {
                mosLockFlags.TiledAsTiled = 1;
            }

            if ((lockFlags & noOverWrite) == noOverWrite)
            {
                mosLockFlags.NoOverWrite = 1;
            }

            if ((lockFlags & noDecompress) == noDecompress)
            {
                mosLockFlags.NoDecompress = 1;
            }

            if ((lockFlags & uncached) == uncached)
            {
                mosLockFlags.Uncached = 1;
            }

            if ((lockFlags & forceCached) == forceCached)
            {
                mosLockFlags.ForceCached = 1;
            }

            m_lockedPtr = m_os->pfnLockResource(
                m_os,
                m_res,
                &mosLockFlags);
        }

        return m_lockedPtr;
    }

private:
    PMOS_INTERFACE m_os;
    PMOS_RESOURCE  m_res;
    void          *m_lockedPtr;

MEDIA_CLASS_DEFINE_END(CodechalResLock)
};

/*! \brief Settings used to create the CodecHal device.
*/
typedef struct _CODECHAL_STANDARD_INFO
{
    CODECHAL_FUNCTION CodecFunction;  //!< High level codec functionality requested.
    uint32_t          Mode;           //!< Mode requested (high level combination between Standard and CodecFunction).
    /*! \brief Applies to decode only, hybrid decoding requested.
    *
    *   Hybrid decoding uses EU kernels when FF HW is not available.
    */
    bool bIsHybridCodec;
} CODECHAL_STANDARD_INFO, *PCODECHAL_STANDARD_INFO;

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
        CodechalHwInterfaceNext *hwInterface,
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

    virtual MOS_STATUS Reformat();

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
    //! \brief    Get Encode Status for Vulkan Query Pool
    //! \details  Retrieve the necessary status and results for the Vulkan query pool
    //!           from the status buffer, such as bitstream size, hardware encoding status, bitstream offset
    //! \param    [in] queryFrameIndex
    //!               The specified frame index
    //!           [in] pData
    //!               Points to the buffer passed by Vulkan for storing the results.
    //!           [in] dataOffset
    //!               Starting position for writing to pData
    //!           [in] is64bit
    //!               Whether each result is 64-bit, otherwise 32-bit.
    //!           [in] reportStatus
    //!               Whether the encode status needs to be returned
    //!           [in] reportOffset
    //!               Whether the bitstream offset needs to be returned
    //!           [in] reportBitstream
    //!               Whether the bitstream size needs to be returned
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetVulkanQueryPoolResults(
        uint32_t queryFrameIndex,
        void    *pData,
        uint64_t dataOffset,
        bool     is64bit,
        uint8_t  reportStatus,
        bool     reportOffset,
        bool     reportBitstream)
    {
        return MOS_STATUS_SUCCESS;
    }


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
    //! \brief    Report Error Flag.
    //! \details  Report error flag to metadata buffer
    //! \param    [in] pMetadataBuffer
    //!           Metadata buffer.
    //! \param    [in] size
    //!           Metadata size.
    //! \param    [in] offset
    //!           Error flag offset in the metdata.
    //! \param    [in] flag
    //!           Flag to report.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS ReportErrorFlag(PMOS_RESOURCE pMetadataBuffer, uint32_t size,
                                       uint32_t offset, uint32_t flag);

    //!
    //! \brief    Gets hardware interface.
    //! \return   CodechalHwInterface
    //!           return hardware interface
    //!
    CodechalHwInterfaceNext *GetHwInterface() { return m_hwInterface; }

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
    CodechalHwInterfaceNext *m_hwInterface = nullptr;

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

    //!< usersettingInstance
    MediaUserSettingSharedPtr   m_userSettingPtr = nullptr;
MEDIA_CLASS_DEFINE_END(Codechal)
};

#endif
