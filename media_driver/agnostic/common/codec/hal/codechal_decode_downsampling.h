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
#include "codechal_mmc.h"

using CODECHAL_DECODE_PROCESSING_PARAMS = struct _CODECHAL_DECODE_PROCESSING_PARAMS;

//!
//! \class MediaWalkerFieldScalingStaticData
//! \brief This class defines the member fields, functions etc. for Media Walker Static Data used by Field Downscaling Kernel.
//!
class MediaWalkerFieldScalingStaticData
{
public:
    //!
    //! \brief    Constructor
    //!
    MediaWalkerFieldScalingStaticData();

    //!
    //! \brief    Destructor
    //!
    ~MediaWalkerFieldScalingStaticData() {};

    //!
    //! \struct MediaWalkerData
    //! \brief Media Walker Data bit definitions
    //!
    struct MediaWalkerData
    {
        // uint32_t 0 - GRF R1.0
        union
        {
            // CSC
            struct
            {
                uint32_t       m_cscConstantC0                  : 16;
                uint32_t       m_cscConstantC1                  : 16;
            };

            uint32_t       m_value;
        } m_dword00;

        // uint32_t 1 - GRF R1.1
        union
        {
            // CSC
            struct
            {
                uint32_t       m_cscConstantC2                  : 16;
                uint32_t       m_cscConstantC3                  : 16;
            };

            uint32_t       m_value;
        } m_dword01;

        // uint32_t 2 - GRF R1.2
        union
        {
            // CSC
            struct
            {
                uint32_t       m_cscConstantC4                  : 16;
                uint32_t       m_cscConstantC5                  : 16;
            };

            uint32_t       m_value;
        } m_dword02;

        // uint32_t 3 - GRF R1.3
        union
        {
            // CSC
            struct
            {
                uint32_t       m_cscConstantC6                  : 16;
                uint32_t       m_cscConstantC7                  : 16;
            };

            uint32_t       m_value;
        } m_dword03;

        // uint32_t 4 - GRF R1.4
        union
        {
            // CSC
            struct
            {
                uint32_t       m_cscConstantC8                  : 16;
                uint32_t       m_cscConstantC9                  : 16;
            };

            uint32_t       m_value;
        } m_dword04;

        // uint32_t 5 - GRF R1.5
        union
        {
            // CSC
            struct
            {
                uint32_t       m_cscConstantC10                 : 16;
                uint32_t       m_cscConstantC11                 : 16;
            };

            uint32_t       m_value;
        } m_dword05;

        // uint32_t 6 - GRF R1.6
        uint32_t           m_padConstantBlend;

        // uint32_t 7 - GRF R1.7
        union
        {
            struct
            {
                uint32_t       m_reserved                       : 24;
                uint32_t       m_pointerToInlineParameters      : 8;
            };

            uint32_t       m_value;
        } m_dword07;

        // uint32_t 8 - GRF R2.0
        union
        {
            struct
            {
                uint32_t       m_destinationRectangleWidth      : 16;
                uint32_t       m_destinationRectangleHeight     : 16;
            };

            uint32_t       m_value;
        } m_dword08;

        // uint32_t 9 - GRF R2.1
        union
        {
            struct
            {
                uint32_t       m_reserved0                      : 27;
                uint32_t       m_iefByPassEnable                : 1;
                uint32_t       m_reserved1                      : 4;
            };

            uint32_t       m_value;
        } m_dword09;

        // uint32_t 10 - GRF R2.2
        union
        {
            struct
            {
                uint32_t       m_reserved0                      : 24;
                uint32_t       m_chromaSitingLocation           : 3;
                uint32_t       m_reserved1                      : 5;
            };

            uint32_t       m_value;
        } m_dword10;

        // uint32_t 11 - 13 - GRF R2.3 - 2.5
        uint32_t           m_pad0[3];

        // uint32_t 14 - GRF R2.6
        union
        {
            // Lumakey, NLAS
            struct
            {
                uint32_t       m_lumakeyLowThreshold            : 8;
                uint32_t       m_lumakeyHighThreshold           : 8;
                uint32_t       m_nlasEnable                     : 8;
                uint32_t       m_reserved                       : 8;
            };

            uint32_t       m_value;
        } m_dword14;

        // uint32_t 15 - GRF R2.7
        union
        {
            // Save
            struct
            {
                uint8_t        m_destinationPackedYOffset;
                uint8_t        m_destinationPackedUOffset;
                uint8_t        m_destinationPackedVOffset;
                uint8_t        m_destinationRGBFormat;
            };

            uint32_t       m_value;
        } m_dword15;

        // uint32_t 16 - GRF R3.0
        union
        {
            // Sampler Load
            struct
            {
                float          m_horizontalScalingStepRatioLayer0;
            };

            uint32_t       m_value;
        } m_dword16;

        // uint32_t 17 - 23 - GRF R3.1 - R3.7
        uint32_t           m_pad1[7];

        // uint32_t 24 - GRF R4.0
        union
        {
            // Sampler Load
            struct
            {
                float          m_verticalScalingStepRatioLayer0;
            };

            // Dataport Load
            struct
            {
                uint8_t        m_sourcePackedYOffset;
                uint8_t        m_sourcePackedUOffset;
                uint8_t        m_sourcePackedVOffset;
                uint8_t        m_reserved;
            };

            uint32_t       m_value;
        } m_dword24;

        // uint32_t 25 - 31 - GRF R4.1 - R4.7
        uint32_t           m_pad2[7];

        // uint32_t 32 - GRF R5.0
        union
        {
            // Sampler Load
            struct
            {
                float          m_verticalFrameOriginLayer0;
            };

            uint32_t       m_value;
        } m_dword32;

        // uint32_t 33 - 39 - GRF R5.1 - R5.7
        uint32_t           m_pad3[7];

        // uint32_t 40 - GRF R6.0
        union
        {
            // Sampler Load
            struct
            {
                float          m_horizontalFrameOriginLayer0;
            };

            uint32_t       m_value;
        } m_dword40;

        // uint32_t 41 - 47 - GRF R6.1 - R6.7
        uint32_t           m_pad4[7];

        // uint32_t 48  - GRF R7.0
        union
        {
            struct
            {
                uint32_t       m_destXTopLeftLayer0             : 16;
                uint32_t       m_destYTopLeftLayer0             : 16;
            };

            uint32_t       m_value;
        } m_dword48;

        // uint32_t 49 - 55 - GRF R7.1 - R7.7
        uint32_t           m_pad5[7];

        // uint32_t 56  - GRF R8.0
        union
        {
            struct
            {
                uint32_t       m_destXBottomRightLayer0         : 16;
                uint32_t       m_destYBottomRightLayer0         : 16;
            };

            uint32_t       m_value;
        } m_dword56;

        // uint32_t 57 - 63 - GRF R8.1
        uint32_t           m_pad6[7];

        // uint32_t 64  - GRF R9.0
        union
        {
            struct
            {
                float          m_mainVideoXScalingStepLeft;
            };

            uint32_t       m_value;
        } m_dword64;

        // DWORD65  - GRF R9.1
        union
        {
            struct
            {
                float          m_videoStepDeltaForNonLinearRegion;
            };

            uint32_t       m_value;
        } m_dword65;

        // DWORD66  - GRF R9.2
        union
        {
            struct
            {
                uint32_t       m_startofLinearScalingInPixelPositionC0          : 16;
                uint32_t       m_startofRHSNonLinearScalingInPixelPositionC1    : 16;
            };

            uint32_t       m_value;
        } m_dword66;

        // uint32_t 67 - GRF R9.3
        union
        {
            // Sampler Load
            struct
            {
                float          m_mainVideoXScalingStepCenter;
            };

            uint32_t       m_value;
        } m_dword67;

        // uint32_t 68 - GRF R9.4
        union
        {
            // Sampler Load
            struct
            {
                float          m_mainVideoXScalingStepRight;
            };

            uint32_t       m_value;
        } m_dword68;

        // uint32_t 69-71 - Padding is needed as we program ConstantURBEntryReadLength  = iCurbeLength >> 5
        uint32_t           m_pad[3];
    } m_mediaWalkerData;                                              //!< media walker data

    static const size_t m_byteSize = sizeof(MediaWalkerData);         //!< byte size of media walker data
};

//!
//! \class FieldScalingInterface 
//! \brief This class defines the member fields, functions etc used by Field Downscaling interface.
//!
class FieldScalingInterface
{
public:
    //!
    //! \brief    Copy constructor
    //!
    FieldScalingInterface(const FieldScalingInterface&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    FieldScalingInterface& operator=(const FieldScalingInterface&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~FieldScalingInterface();

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
    virtual MOS_STATUS InitializeKernelState(
        CodechalDecode                      *decoder,
        CodechalHwInterface                 *hwInterface,
        PMOS_INTERFACE                      osInterface);

    //!
    //! \brief    Send Media VFE cmds
    //! \details  Send Media VFE cmds to setup VFE for media kernel
    //! \param    [in] cmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] kernelState
    //!           Pointer to MHW kernel state
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetupMediaVfe(
        PMOS_COMMAND_BUFFER  cmdBuffer,
        MHW_KERNEL_STATE     *kernelState);

    //!
    //! \brief  Initialize MMC state for specified downsampling device
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitMmcState();

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
    virtual MOS_STATUS DoFieldScaling(
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
    CodecHalMmcState                *m_mmcState                 = nullptr;                              //!< Pointer to MMC state
    uint8_t                         *m_kernelBinary[stateMax];                                          //!< Kernel binary
    uint32_t                        m_kernelUID[stateMax];                                              //!< Kernel unique ID
    uint32_t                        m_kernelSize[stateMax];                                             //!< Kernel size
    MHW_KERNEL_STATE                m_kernelStates[stateMax];                                           //!< Kernel state
    uint32_t                        m_dshSize[stateMax];                                                //!< DSH size
    MOS_RESOURCE                    m_syncObject;                                                       //!< Sync Object
    uint32_t                        m_curbeLength;                                                      //!< Media Data struct Length

    //!
    //! \brief    Initialize state heap settings and kernel params
    //! \details  Initialize Field Scaling Kernel State heap settings & params
    //! \param    [in] hwInterface
    //!           Pointer to HW Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitInterfaceStateHeapSetting(
        CodechalHwInterface               *hwInterface);

protected:
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
