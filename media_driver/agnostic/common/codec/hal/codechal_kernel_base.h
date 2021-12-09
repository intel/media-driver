/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_kernel_base.h
//! \brief    Defines base class for all kernels
//! \details  Kernel base class abstracts all common functions and definitions
//!           for all kernels, each kernel class should inherit from kernel base
//!

#ifndef __CODECHAL_KERNEL_BASE_H__
#define __CODECHAL_KERNEL_BASE_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_encoder_base.h"
#include <map>

//!
//! \class    CodechalKernelBase
//! \brief    Codechal kernel base
//!
class CodechalKernelBase
{
public:
    using KernelBinaryCallback = MOS_STATUS (*)(void *,
                                                EncOperation,
                                                uint32_t,
                                                void *,
                                                uint32_t *);

    //!
    //! \brief Constructor
    //!
    CodechalKernelBase(CodechalEncoderState *encoder);

    //!
    //! \brief Destructor
    //!
    virtual ~CodechalKernelBase();

    //!
    //! \brief  Intialize kernel state object
    //!
    //! \param  [in] callback
    //!         callback function which parse kernel header from kernel structure
    //! \param  [in] binaryBase
    //!         pointer to the base address of kernel binary array
    //! \param  [in] kernelUID
    //!         kernel UID 
    //!
    virtual MOS_STATUS Initialize(
        KernelBinaryCallback callback,
        uint8_t             *binaryBase,
        uint32_t             kernelUID);

    //!
    //! \brief  Allocate kernel required resources
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AllocateResources() = 0;

    //!
    //! \brief  Get binding table count
    //!
    //! \return uint32_t
    //!         Binding table count
    //!
    virtual uint32_t GetBTCount() = 0;

    //!
    //! \brief  Get input/output surfaces allocated by kernel
    //!
    //! \param  [in] surfaceId
    //!         uint32_t, surface index id
    //! \return PMOS_SURFACE
    //!         Pointer to MOS_SURFACE
    //!
    PMOS_SURFACE GetSurface(uint32_t surfaceId);

    //!
    //! \brief  Get input/output surfaces allocated for HME kernel when using MDF RT
    //!
    //! \param  [in] surfaceId
    //!         uint32_t, surface index id
    //!         Pointer to CM 2D Surface
    //!
    virtual CmSurface2D* GetCmSurface(uint32_t surfaceId) { return nullptr; };

#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS DumpKernelOutput() { return MOS_STATUS_SUCCESS; }
#endif

protected:
    //!
    //! \brief  get kernel bianry address and size
    //!
    //! \param  [in] kernelBase
    //!         base address of kernel binary
    //! \param  [in] kernelUID
    //!         kernel UID
    //! \param  [out] kernelBinary
    //!         kernel binary address for kernel UID
    //! \param  [out] size
    //!         kernel binary size
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    static MOS_STATUS GetKernelBinaryAndSize(
        uint8_t * kernelBase,
        uint32_t  kernelUID,
        uint8_t **kernelBinary,
        uint32_t *size);
    //!
    //! \brief  create kernel states
    //!
    //! \param  [out] kernelState
    //!         pointer to MHW_KERNEL_STATE*
    //! \param  [in] kernelIndex
    //!         the sub kernel index in kernel state map
    //! \param  [in] operation
    //!         EncOperation,index used to get kernel header
    //! \param  [in] kernelOffset
    //!         kernel offset index
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS CreateKernelState(
        MHW_KERNEL_STATE **           kernelState,
        uint32_t                      kernelIndex,
        EncOperation                  operation,
        uint32_t                      kernelOffset);

    //!
    //! \brief  Release kernel required resources
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS ReleaseResources() = 0;

    //!
    //! \brief  Define common pipeline to run the kernel
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS Run();

    //!
    //! \brief  Get curbe buffer size
    //!
    //! \return uint32_t
    //!         Curbe buffer size
    //!
    virtual uint32_t GetCurbeSize() = 0;

    //!
    //! \brief  Get DSH inline data length
    //!
    //! \return uint32_t
    //!         Inline data length
    //!
    virtual uint32_t GetInlineDataLength() { return 0; }

    //!
    //! \brief  Add perf tag for each kernel
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS AddPerfTag() = 0;

    //!
    //! \brief  Get the pointer to current active kernel state
    //!
    //! \return MHW_KERNEL_STATE*
    //!         Pointer to MHW_KERNEL_STATE
    //!
    virtual MHW_KERNEL_STATE * GetActiveKernelState() = 0;

    //!
    //! \brief  Get media state type of each kernel
    //!
    //! \return CODECHAL_MEDIA_STATE_TYPE
    //!         media state type
    //!
    virtual CODECHAL_MEDIA_STATE_TYPE GetMediaStateType() = 0;

    //!
    //! \brief  Set curbe data for current kernel state
    //!
    //! \param  [in] kernelState
    //!         pointer to current active kernel state
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SetCurbe(MHW_KERNEL_STATE *kernelState) { return MOS_STATUS_UNIMPLEMENTED; };

    //!
    //! \brief  Send input and output surfaces for current kernel
    //!
    //! \param  [in] cmd
    //!         pointer to MOS_COMMAND_BUFFER
    //! \param  [in] kernelState
    //!         pointer to the MHW_KERNEL_STATE
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS SendSurfaces(PMOS_COMMAND_BUFFER cmd, MHW_KERNEL_STATE *kernelState) = 0;

    //!
    //! \brief  Initialize media walker parameters
    //!
    //! \param  [in] walkerParam
    //!         Reference to CODECHAL_WALKEr_CODEC_PARAMS
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS InitWalkerCodecParams(CODECHAL_WALKER_CODEC_PARAMS &walkerParam) = 0;

    //!
    //! \brief  Utility function to clean up resource
    //!
    //! \param  [in] resource
    //!         Pointer to MOS_RESOURCE
    //! \param  [in] allocParam
    //!         Pointer to MOS_ALLOC_GFXRES_PARAMS
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS CleanUpResource(PMOS_RESOURCE resource, PMOS_ALLOC_GFXRES_PARAMS allocParam);

    //!
    //! \brief  Allocate 2D surface and store surface pointer in the pool
    //!
    //! \param  [in] param
    //!         Pointer to MOS_ALLOC_GFXRES_PARAMS
    //! \param  [in] surface
    //!         Pointer to MOS_SURFACE
    //! \param  [in] surfaceId
    //!         uint32_t, index to surface
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS AllocateSurface(PMOS_ALLOC_GFXRES_PARAMS param, PMOS_SURFACE surface, uint32_t surfaceId);

protected:
    CodechalEncoderState      *m_encoder            = nullptr;  //!< Pointer to ENCODER base class
    MOS_INTERFACE             *m_osInterface        = nullptr;  //!< OS interface
    CodechalHwInterface       *m_hwInterface        = nullptr;  //!< HW interface
    CodechalDebugInterface    *m_debugInterface     = nullptr;  //!< Debug interface
    MhwMiInterface            *m_miInterface        = nullptr;  //!< Common MI interface
    MhwRenderInterface        *m_renderInterface    = nullptr;  //!< Render engine interface
    XMHW_STATE_HEAP_INTERFACE *m_stateHeapInterface = nullptr;  //!< State heap class interface

    KernelBinaryCallback       m_callback           = nullptr;  //!< kernel binary structure callback
    uint8_t                   *m_kernelBinary       = nullptr;  //!< kernel bianry base address

    std::map<uint32_t, MHW_KERNEL_STATE *> m_kernelStatePool = {};  //!< pool to store kernel state with index
    std::map<uint32_t, PMOS_SURFACE>       m_surfacePool     = {};  //!< pool to store surface with index

    bool& m_firstTaskInPhase;
    bool& m_lastTaskInPhase;
    bool& m_singleTaskPhaseSupported;
    bool& m_renderContextUsesNullHw;
    bool& m_groupIdSelectSupported;
    bool& m_fieldScalingOutputInterleaved;
    bool& m_vdencEnabled;
    uint8_t&  m_groupId;
    uint32_t& m_maxBtCount;
    uint32_t& m_vmeStatesSize;
    uint32_t& m_storeData;
    uint32_t& m_verticalLineStride;
    uint32_t& m_downscaledWidthInMb4x;
    uint32_t& m_downscaledHeightInMb4x;
    uint32_t& m_downscaledWidthInMb16x;
    uint32_t& m_downscaledHeightInMb16x;
    uint32_t& m_downscaledWidthInMb32x;
    uint32_t& m_downscaledHeightInMb32x;
    uint32_t& m_mode;
    uint16_t& m_pictureCodingType;
    uint32_t& m_frameWidth;
    uint32_t& m_frameHeight;
    uint32_t& m_frameFieldHeight;
    uint32_t& m_standard;
    MHW_WALKER_MODE& m_walkerMode;

};

#endif /* __CODECHAL_KERNEL_BASE_H__ */
