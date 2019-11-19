/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_mmc.h
//! \brief    Defines the public interface for CodecHal Media Memory Compression
//!
#ifndef __CODECHAL_MMC_H__
#define __CODECHAL_MMC_H__

#include "codechal_utilities.h"

//!
//! \brief Forward declarations
//!
typedef struct _MHW_PIPE_CONTROL_PARAMS        *PMHW_PIPE_CONTROL_PARAMS;
typedef struct _MHW_VDBOX_SURFACE_PARAMS       *PMHW_VDBOX_SURFACE_PARAMS;
using PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS = MHW_VDBOX_PIPE_BUF_ADDR_PARAMS * ;
//! \class CodecHalMmcState
//! \brief Media memory compression state. This class defines the member fields
//!        functions etc used by memory compression. 
//!
class CodecHalMmcState
{
public:
    //!
    //! \brief    Constructor
    //!
    CodecHalMmcState(CodechalHwInterface  *hwInterface);

    //!
    //! \brief    Destructor
    //!
    virtual ~CodecHalMmcState() {};

    //!
    //! \brief    Check if MMC is enabled 
    //!
    //! \return   bool
    //!           true if mmc is enabled, else false
    //!
    static bool IsMmcEnabled();

    //!
    //! \brief    Disable MMC state 
    //!
    //! \return   void 
    //!
    void SetMmcDisabled();

    //!
    //! \brief    Get surface memory compression state
    //! \param    [out] surface
    //!           Pointer to PMOS_SURFACE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetSurfaceMmcState(PMOS_SURFACE surface);

    //!
    //! \brief    Disable surface memory compression state
    //! \param    [in,out] surface
    //!           Pointer to PMOS_SURFACE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DisableSurfaceMmcState(PMOS_SURFACE surface);

    //!
    //! \brief    Set destinate surface memory compression state by source surface
    //! \param    [out] dstSurface
    //!           Pointer to PMOS_SURFACE
    //! \param    [in] srcSurface
    //!           Pointer to PMOS_SURFACE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSurfaceMmcMode(
        PMOS_SURFACE dstSurface,
        PMOS_SURFACE srcSurface);

    //!
    //! \brief    Set surface paramter
    //! \param    [in,out] surfaceParams
    //!           Pointer to PCODECHAL_SURFACE_CODEC_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSurfaceParams(
        PCODECHAL_SURFACE_CODEC_PARAMS surfaceParams);

    //!
    //! \brief    Set pipe buffer address parameter
    //! \details  Set pipe buffer address parameter in MMC case 
    //! 
    //! \param    [in,out] pipeBufAddrParams
    //!           Pointer to PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetPipeBufAddr(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams,
        PMOS_COMMAND_BUFFER cmdBuffer = nullptr)
    {
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief    Set Surface State MMC state parameter
    //! \details  Set MMC state for speficied SurfaceState cmd parameters
    //! 
    //! \param    [in,out] surfaceStateParams
    //!           Pointer to PMHW_VDBOX_SURFACE_PARAMS
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS command buffer
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSurfaceState(
        PMHW_VDBOX_SURFACE_PARAMS surfaceStateParams,
        PMOS_COMMAND_BUFFER cmdBuffer = nullptr)
    {
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief    Send prolog MI cmds used to control MMC state
    //! \details  Send H/W MMIO cmds used to initialze MMC related states
    //!
    //! \param    [in] miInterface
    //!           Pointer to MhwMiInterface
    //! \param    [in] cmdBuffer
    //!           Command buffer pointer
    //! \param    [in] gpuContext
    //!           Current pipe of the cmd buffer
    //!
    //! \return   MOS_STATUS
    //!           Return status of sending register MMIOs
    virtual MOS_STATUS SendPrologCmd(
        MhwMiInterface      *miInterface,
        MOS_COMMAND_BUFFER  *cmdBuffer,
        MOS_GPU_CONTEXT     gpuContext)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set reference sync
    //! \details  Set reference sync, check if reference surface needs to be synchronized in MMC case 
    //!
    //! \param    [in] disableDecodeSyncLock
    //!           Indicates if decode sync lock is disabled
    //! \param    [in] disableLockForTranscode
    //!           Indicates if transcoe lock is disabled
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetRefrenceSync(
        bool disableDecodeSyncLock,
        bool disableLockForTranscode)
    {
        MOS_UNUSED(disableDecodeSyncLock);
        MOS_UNUSED(disableLockForTranscode);
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief    Check reference list
    //! \details  Check reference list, including self-reference detection and mmc state consistence detection 
    //! 
    //! \param    [in,out] pipeBufAddrParams
    //!           Pointer to PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckReferenceList(
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pipeBufAddrParams)
    {
        return MOS_STATUS_SUCCESS;
    };

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    Update mmc user feature key 
    //! \details  Report out the memory compression state for render target surface
    //! 
    //! \param    [in] surface
    //!           Pointer to PMOS_SURFACE
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateUserFeatureKey(PMOS_SURFACE surface);
#endif

    //!
    //! \brief    Is extension MMC
    //! \details  Report if is extension MMC
    //!
    //! \return   bool
    //!
    bool IsMmcExtensionEnabled()
    {
        return m_mmcExtensionEnabled;
    }

protected:

    static bool             m_mmcEnabled;                           //!< Indicate if media memory compression is enabled
    PMOS_INTERFACE          m_osInterface = nullptr;                //!< Os Inteface
    CodechalHwInterface     *m_hwInterface = nullptr;               //!< Pointer to HW Interface
    bool                    m_hcpMmcEnabled = false;                //!< Inidate if hcp mmc is enabled
    bool                    m_10bitMmcEnabled = false;              //!< Inidate if 10bit mmc is enabled
    bool                    m_gpuMmuPageFaultEnabled = false;       //!< Inidate if page fault is enabled
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                    m_userFeatureUpdated = false;              //!< Inidate if mmc user feature key is updated
    uint32_t                m_compressibleId  = 0;
    uint32_t                m_compressModeId  = 0;
#endif
    bool                    m_mmcExtensionEnabled = false;          //!< Indicate if extension MMC
};

#endif  // __CODECHAL_MMC_H__
