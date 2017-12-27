/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     codechal_decode_downsampling.h
//! \brief    Defines the decode interface extension for field downsampling.
//! \details  Downsampling in this case is supported by EU kernels.
//!

#ifndef __CODECHAL_DECODER_DOWNSAMPLING_H__
#define __CODECHAL_DECODER_DOWNSAMPLING_H__

#include "mos_os.h"
#include "mhw_state_heap.h"

using CODECHAL_DECODE_PROCESSING_PARAMS = struct _CODECHAL_DECODE_PROCESSING_PARAMS;

//!
//! \class FieldScalingInterface 
//! \brief This class defines the member fields, functions etc used by Field Downscaling interface.
//!
class FieldScalingInterface
{
public:
    //!
    //! \brief    Destructor
    //!
    ~FieldScalingInterface();

    //!
    //! \brief    Check Field Scaling Supported
    //! \details  Check parameter legitimacy for field scaling
    //! \param    [in] procParams
    //!           Pointer to decode processing paramters #CODECHAL_DECODE_PROCESSING_PARAMS
    //! \return   bool
    //!           true if support, else false
    //!
    bool IsFieldScalingSupported(
        CODECHAL_DECODE_PROCESSING_PARAMS   *procParams);

    //!
    //! \brief    Initialize Field Scaling Kernel State
    //! \details  Initialize Field Scaling Kernel State & Params
    //! \param    [in] decoder
    //!           Pointer to decode interface
    //! \param    [in] hwInterface
    //!           Pointer to hardware interface
    //! \param    [in] osInterface
    //!           Pointer to OS interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializeKernelState(
        CodechalDecode                      *decoder,
        CodechalHwInterface                 *hwInterface,
        PMOS_INTERFACE                      osInterface);

    //!
    //! \brief    Perform Field Scaling
    //! \details  Configure kernel regions to do field scaling
    //! \param    [in] procParams
    //!           Pointer to decode processing paramters #CODECHAL_DECODE_PROCESSING_PARAMS
    //! \param    [in] renderContext
    //!           The render context using for decode
    //! \param    [in] disableDecodeSyncLock
    //!           Disable decode sync lock
    //! \param    [in] disableLockForTranscode
    //!           Disable lock for transcode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DoFieldScaling(
        CODECHAL_DECODE_PROCESSING_PARAMS   *procParams,
        MOS_GPU_CONTEXT                     renderContext,
        bool                                disableDecodeSyncLock,
        bool                                disableLockForTranscode);

protected:
    //!
    //! \brief    Constructor
    //!    
    FieldScalingInterface(CodechalHwInterface *hwInterface);

    //!
    //! \enum     FieldScalingKernelStateIdx
    //! \brief    Field scaling kernel state index
    //!
    enum FieldScalingKernelStateIdx
    {
        stateNv12     = 0,                                                                              //!< Field scaling kernel index for NV12
        stateYuy2,                                                                                      //!< Field scaling kernel index for YUY2
        stateMax                                                                                        //!< Max kernel index for Field scaling
    };

    enum
    {
        fieldTopSrcY        = 0,                                                                        //!< Binding table offset for Top field input Y
        fieldTopSrcUV       = 1,                                                                        //!< Binding table offset for Top field input UV
        fieldBotSrcY        = 48,                                                                       //!< Binding table offset for Bottom field input Y
        fieldBotSrcUV       = 49,                                                                       //!< Binding table offset for Bottom field input UV
        dstY                = 24,                                                                       //!< Binding table offset for output Y
        dstUV               = 25,                                                                       //!< Binding table offset for output UV
        numSurfaces         = 50                                                                        //!< Number of BT entries for Field scaling
    };

    static const uint32_t           m_maxInputWidth             = 4096;                                 //!< Max input width supported by Field Scaling
    static const uint32_t           m_minInputWidth             = 128;                                  //!< Min input width supported by Field Scaling
    static const uint32_t           m_maxInputHeight            = 4096;                                 //!< Max input height supported by Field Scaling
    static const uint32_t           m_minInputHeight            = 128;                                  //!< Min input height supported by Field Scaling

    static const uint32_t           m_initDshSize               = MHW_PAGE_SIZE;                        //!< Init DSH size for Field Downscailng kernel
    static const uint32_t           m_samplerNum                = 4;                                    //!< Sampler count for Field Downscaling kernel
    static const uint32_t           m_numSyncTags               = 16;                                   //!< Sync tags num of state heap settings
    static const float              m_maxScaleRatio;                                                    //!< Maximum scaling ratio for both X and Y directions
    static const float              m_minScaleRatio;                                                    //!< Minimum scaling ratio for both X and Y directions

    CodechalDecode                  *m_decoder                  = nullptr;                              //!< Pointer to Decode Interface
    MOS_INTERFACE                   *m_osInterface              = nullptr;                              //!< Pointer to OS Interface
    CodechalHwInterface             *m_hwInterface              = nullptr;                              //!< Pointer to HW Interface
    MhwRenderInterface              *m_renderInterface          = nullptr;                              //!< Pointer to Render Interface
    MHW_STATE_HEAP_INTERFACE        *m_stateHeapInterface       = nullptr;                              //!< Pointer to State Heap Interface
    MhwMiInterface                  *m_miInterface              = nullptr;                              //!< Pointer to MI interface.
    uint8_t                         *m_kernelBase               = nullptr;                              //!< Pointer to kernel base address
    uint8_t                         *m_kernelBinary[stateMax];                                          //!< Kernel binary
    uint32_t                        m_kernelUID[stateMax];                                              //!< Kernel unique ID
    uint32_t                        m_kernelSize[stateMax];                                             //!< Kernel size
    MHW_KERNEL_STATE                m_kernelStates[stateMax];                                           //!< Kernel state
    uint32_t                        m_dshSize[stateMax];                                                //!< DSH size
    MOS_RESOURCE                    m_syncObject;                                                       //!< Sync Object
    
    //!
    //! \brief    Initialize state heap settings and kernel params
    //! \details  Initialize Field Scaling Kernel State heap settings & params
    //! \param    [in] hwInterface
    //!           Pointer to HW Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitInterfaceStateHeapSetting(
        CodechalHwInterface               *hwInterface);

private:
    //!
    //! \brief    Set Field Scaling Curbe
    //! \details  Set curbe for field scaling kernel
    //! \param    MHW_KERNEL_STATE *kernelState
    //!           [in] Pointer to kernel state
    //! \param    CODECHAL_DECODE_PROCESSING_PARAMS *procParams
    //!           [in] Pointer to decode processing paramters 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbeFieldScaling(
        MHW_KERNEL_STATE                    *kernelState,
        CODECHAL_DECODE_PROCESSING_PARAMS   *procParams);
};
#endif // __CODECHAL_DECODER_DOWNSAMPLING_H__
