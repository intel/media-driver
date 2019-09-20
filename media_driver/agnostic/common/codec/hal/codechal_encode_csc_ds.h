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
//! \file     codechal_encode_csc_ds.h
//! \brief    Defines base class for CSC and Downscaling
//!

#ifndef __CODECHAL_ENCODE_CSC_DS_H__
#define __CODECHAL_ENCODE_CSC_DS_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_encode_sfc.h"
#include "codechal_utilities.h"

//!
//! \enum     DsStage
//! \brief    Ds stage
//!
enum DsStage
{
    dsDisabled = 0,
    dsStage2x = 1,
    dsStage4x = 2,
    dsStage16x = 3,
    dsStage2x4x = 4,
    dsStage32x = 5,
    // below used by HEVC Gen10 DsConv kernel
    dsConvUnknown = 0,
    ds2xFromOrig = 1,
    ds4xFromOrig = 2,
    ds2x4xFromOrig = 3,
    ds16xFromOrig = 4,
    convFromOrig = 8,
    convDs2xFromOrig = 9,
    convDs4xFromOrig = 10,
    convDs2x4xFromOrig = 11,
};

//!
//! \class    ScalingBindingTable
//! \brief    Scaling binding table
//!
struct ScalingBindingTable
{
    uint32_t   dwScalingFrameSrcY;
    uint32_t   dwScalingFrameDstY;
    uint32_t   dwScalingFieldTopSrcY;
    uint32_t   dwScalingFieldBotSrcY;
    uint32_t   dwScalingFieldTopDstY;
    uint32_t   dwScalingFieldBotDstY;
    uint32_t   dwScalingFrameFlatnessChkDst;
    uint32_t   dwScalingFieldFlatnessChkTopDst;
    uint32_t   dwScalingFieldFlatnessChkBotDst;
    uint32_t   dwScalingFrameMbVprocStatsDst;
    uint32_t   dwScalingFieldMbVprocStatsTopDst;
    uint32_t   dwScalingFieldMbVprocStatsBotDst;
    uint32_t   dwBindingTableStartOffset;
    uint32_t   dwNumBindingTableEntries;
};

//!
//! \class    CodechalEncodeCscDs
//! \details  CSC and Downscaling base class
//!           Entry point to create CSC Downscaling class instance
//! This class defines the base class for CSC and Downscaling feature, it includes
//! common member fields, functions, interfaces etc shared by all Gens.
//!
class CodechalEncodeCscDs
{
public:
    //!< \cond SKIP_DOXYGEN
    //!
    //! \brief    Curbe params for CSC kernel
    //!
    struct CurbeParams
    {
        PMHW_KERNEL_STATE       pKernelState = nullptr;
        uint32_t                dwInputPictureWidth = 0;
        uint32_t                dwInputPictureHeight = 0;
        bool                    b16xScalingInUse = false;
        bool                    b32xScalingInUse = false;
        bool                    bFieldPicture = false;
        bool                    bCscOrCopyOnly = false;
        bool                    bFlatnessCheckEnabled = false;
        bool                    bMBVarianceOutputEnabled = false;
        bool                    bMBPixelAverageOutputEnabled = false;
        bool                    bBlock8x8StatisticsEnabled = false;
        ENCODE_INPUT_COLORSPACE inputColorSpace = ECOLORSPACE_P709;
        DsStage                 downscaleStage = dsDisabled;
        uint8_t                 ucEncBitDepthLuma = 8;
        uint8_t                 ucEncBitDepthChroma = 8;
        bool                    bConvertFlag = false;
        bool                    bHevcEncHistorySum = false;
        bool                    bUseLCU32 = false;
    };

    //!
    //! \brief    Kernel params for CSC kernel
    //!
    struct KernelParams
    {
        DsStage                     stageDsConversion = dsDisabled;
        bool                        b16xScalingInUse = false;
        bool                        b32xScalingInUse = false;
        bool                        cscOrCopyOnly = false;
        bool                        bLastTaskInPhaseCSC = false;
        bool                        bLastTaskInPhase4xDS = false;
        bool                        bLastTaskInPhase16xDS = false;
        bool                        bLastTaskInPhase32xDS = false;
        bool                        bRawInputProvided = false;
        bool                        bStatsInputProvided = false;
        bool                        bScalingforRef = false;
        MOS_SURFACE                 sInputRawSurface = {};           // for FEI to pass in raw surface
        MOS_RESOURCE                sInputStatsBuffer = {};
        MOS_RESOURCE                sInputStatsBotFieldBuffer = {};
        CODEC_PICTURE               inputPicture = {};
        ENCODE_INPUT_COLORSPACE     inputColorSpace = ECOLORSPACE_P709;
        PMOS_SURFACE                psFormatConversionOnlyInputSurface = nullptr;
        PMOS_SURFACE                psFormatConvertedSurface = nullptr;
        void*                       hevcExtParams = nullptr;
    };

    //!
    //! \brief    4xDS kernel Curbe data
    //!
    struct Ds4xKernelCurbeData
    {
        Ds4xKernelCurbeData()
        {
            DW0 = 0;
            DW1 = ds4xSrcYPlane;
            DW2 = ds4xDstYPlane;
            DW3_InputYBTIBottomField =
            DW4_OutputYBTIBottomField =
            DW5_FlatnessThreshold =
            DW6 =
            DW7_Reserved = 0;
            DW8 = ds4xDstFlatness;
            DW9_FlatnessOutputBTIBottomField = 0;
            DW10 = ds4xDstMbVProc;
            DW11_MBVProcStatsBTIBottomField = 0;
        }

        // uint32_t 0 - GRF R1.0
        union
        {
            struct
            {
                uint32_t   DW0_InputPictureWidth : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DW0_InputPictureHeight : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW0;
        };

        // DW1
        union
        {
            struct
            {
                uint32_t   DW1_InputYBTIFrame;
            };
            struct
            {
                uint32_t   DW1_InputYBTITopField;
            };
            uint32_t DW1;
        };

        // DW2
        union
        {
            struct
            {
                uint32_t   DW2_OutputYBTIFrame;
            };
            struct
            {
                uint32_t   DW2_OutputYBTITopField;
            };
            uint32_t DW2;
        };

        // DW3
        uint32_t DW3_InputYBTIBottomField;

        // DW4
        uint32_t DW4_OutputYBTIBottomField;

        // DW5
        uint32_t DW5_FlatnessThreshold;

        // DW6
        union
        {
            struct
            {
                uint32_t   DW6_EnableMBFlatnessCheck : MOS_BITFIELD_BIT(0);
                uint32_t   DW6_EnableMBVarianceOutput : MOS_BITFIELD_BIT(1);
                uint32_t   DW6_EnableMBPixelAverageOutput : MOS_BITFIELD_BIT(2);
                uint32_t   DW6_EnableBlock8x8StatisticsOutput : MOS_BITFIELD_BIT(3);
                uint32_t   : MOS_BITFIELD_RANGE(4, 31);
            };
            uint32_t DW6;
        };

        // DW7
        uint32_t DW7_Reserved;

        // DW8
        union
        {
            struct
            {
                uint32_t   DW8_FlatnessOutputBTIFrame;
            };
            struct
            {
                uint32_t   DW8_FlatnessOutputBTITopField;
            };
            uint32_t DW8;
        };

        // DW9
        uint32_t DW9_FlatnessOutputBTIBottomField;

        // DW10
        union
        {
            struct
            {
                uint32_t   DW10_MBVProcStatsBTIFrame;
            };
            struct
            {
                uint32_t   DW10_MBVProcStatsBTITopField;
            };
            uint32_t DW10;
        };

        // DW11
        uint32_t DW11_MBVProcStatsBTIBottomField;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(Ds4xKernelCurbeData)) == 12);

    //!
    //! \brief    2xDS kernel Curbe data
    //!
    struct Ds2xKernelCurbeData
    {
        Ds2xKernelCurbeData()
        {
            DW0 =
            DW1_Reserved =
            DW2_Reserved =
            DW3_Reserved =
            DW4_Reserved =
            DW5_Reserved =
            DW6_Reserved =
            DW7_Reserved = 0;
            DW8 = ds2xSrcYPlane;
            DW9 = ds2xDstYPlane;
            DW10_InputYBTIBottomField =
            DW11_OutputYBTIBottomField = 0;
        }

        // uint32_t 0 - GRF R1.0
        union
        {
            struct
            {
                uint32_t   DW0_InputPictureWidth : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DW0_InputPictureHeight : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW0;
        };

        // DW1
        uint32_t DW1_Reserved;

        // DW2
        uint32_t DW2_Reserved;

        // DW3
        uint32_t DW3_Reserved;

        // DW4
        uint32_t DW4_Reserved;

        // DW5
        uint32_t DW5_Reserved;

        // DW6
        uint32_t DW6_Reserved;

        // DW7
        uint32_t DW7_Reserved;

        // DW8
        union
        {
            struct
            {
                uint32_t   DW8_InputYBTIFrame : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t   DW8_InputYBTITopField : MOS_BITFIELD_RANGE(0, 31);
            };
            uint32_t DW8;
        };

        // DW9
        union
        {
            struct
            {
                uint32_t   DW9_OutputYBTIFrame : MOS_BITFIELD_RANGE(0, 31);
            };
            struct
            {
                uint32_t   DW9_OutputYBTITopField : MOS_BITFIELD_RANGE(0, 31);
            };
            uint32_t DW9;
        };

        // DW10
        uint32_t DW10_InputYBTIBottomField;

        // DW11
        uint32_t DW11_OutputYBTIBottomField;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(Ds2xKernelCurbeData)) == 12);

    uint8_t IsEnabled() const { return m_cscDsConvEnable; }
    uint32_t GetRawSurfWidth() const { return m_cscRawSurfWidth; }
    uint32_t GetRawSurfHeight() const { return m_cscRawSurfHeight; }
    bool RequireCsc() const { return m_cscFlag != 0; }
    bool UseSfc() const { return m_cscUsingSfc != 0; }
    bool IsSfcEnabled() const { return m_cscEnableSfc != 0; }
    bool RenderConsumesCscSurface() const { return m_cscRequireCopy || m_cscRequireColor || m_cscRequireConvTo8bPlanar; }
    bool VdboxConsumesCscSurface() const { return m_cscRequireCopy || m_cscRequireColor || m_cscRequireMmc; }
    // 0 for native HW support; 1 for CSC kernel; 2 for VEBOX
    uint8_t CscMethod() const { return (m_cscRequireColor ? (m_cscUsingSfc ? 2 : 1) : 0); }

    void DisableCsc() { m_cscDsConvEnable = 0; }
    void EnableCopy() { m_cscEnableCopy = 1; }
    void EnableColor() { m_cscEnableColor = 1; }
    void EnableMmc() { m_cscEnableMmc = 1; }
    void EnableSfc() { m_cscEnableSfc = 1; }
    void DisableSfc() { m_cscEnableSfc = 0; }
    void ResetCscFlag() { m_cscFlag = 0; }
    //! \endcond

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncodeCscDs(const CodechalEncodeCscDs&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncodeCscDs& operator=(const CodechalEncodeCscDs&) = delete;

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeCscDs();

    //!
    //! \brief    Get CSC kernel BT count
    //!
    //! \return   Number of BTI
    //!
    virtual uint8_t GetBTCount() const;

    //!
    //! \brief    Get CSC surface allocation width/height/format
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void GetCscAllocation(uint32_t &width, uint32_t &height, MOS_FORMAT &format);

    //!
    //! \brief    Initialize the class
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize();

    //!
    //! \brief    Check if need to do CSC/DS/Conversion
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckCondition();

    //!
    //! \brief    Check if recon surface's alignment meet HW requirement
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckReconSurfaceAlignment(PMOS_SURFACE surface);

    //!
    //! \brief    Check if raw surface's alignment meet HW requirement
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckRawSurfaceAlignment(PMOS_SURFACE surface);

    //!
    //! \brief    Set HCP recon surface alignment
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    void SetHcpReconAlignment(uint8_t alignment);

    //!
    //! \brief    On-demand sync for the CSC output surface before use
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS WaitCscSurface(MOS_GPU_CONTEXT gpuContext, bool readOnly);

    //!
    //! \brief    Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS KernelFunctions(KernelParams* params);

    //!
    //! \brief    CSC using SFC
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CscUsingSfc(ENCODE_INPUT_COLORSPACE colorSpace);

    //!
    //! \brief    CSC kernel function
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CscKernel(KernelParams* params);

    //!
    //! \brief    DS kernel function
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DsKernel(KernelParams* params);

protected:
    //!
    //! \brief    CSC kernel supported color format
    //!
    enum CscColor
    {
        cscColorNv12TileY = 0,      // NV12 Tile-Y
        cscColorP010 = 1,           // P010
        cscColorP210 = 2,           // P210 (not supported yet)
        cscColorYUY2 = 3,           // YUY2
        cscColorY210 = 4,           // Y210
        cscColorARGB = 5,           // ARGB
        cscColorNv12Linear = 6,     // NV12 linear
        cscColorABGR = 7,           // ABGR
        cscColorEnumNumber = 8
    };

    //!
    //! \brief    CSC kernel Curbe data
    //!
    struct CscKernelCurbeData
    {
        CscKernelCurbeData()
        {
            DW0 = 0;
            DW1_SrcNV12SurfYIndex = cscSrcYPlane;
            DW2_DstYSurfIndex = cscDstDsYPlane;
            DW3_FlatDstSurfIndex = cscDstFlatOrMbStats;
            DW4_CopyDstNV12SurfIndex = cscDstCopyYPlane;
            DW5 = 0;
            DW6_FlatnessThreshold = 0;
            DW7 = 0;
            DW8_SrcNV12SurfUVIndex = cscSrcUVPlane;
        }

        union
        {
            struct
            {
                // uint32_t 0 - GRF R1.0
                uint32_t    DW0_InputPictureWidth : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW0_InputPictureHeight : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW0;
        };

        // DW1: Surface index source linear NV12 Y Plane
        uint32_t    DW1_SrcNV12SurfYIndex;

        // DW2: Surface index downscale destination Planar Y
        uint32_t    DW2_DstYSurfIndex;

        // DW3: Surface index flatness destination
        uint32_t    DW3_FlatDstSurfIndex;

        // DW4: Surface index copy destination NV12
        uint32_t    DW4_CopyDstNV12SurfIndex;

        union
        {
            struct
            {
                // dw5: operation mode
                uint32_t    DW5_CscDsCopyOpCode : MOS_BITFIELD_RANGE(0, 7);
                uint32_t    DW5_InputColorFormat : MOS_BITFIELD_RANGE(8, 15);
                uint32_t    DW5_Reserved : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW5;
        };

        // DW6: MBFlatnessThreshold
        uint32_t    DW6_FlatnessThreshold;

        union
        {
            struct
            {
                // dw7: EnableMBFlatnessCheck
                uint32_t    DW7_EnableMBFlatnessCheck : MOS_BITFIELD_BIT(0);
                uint32_t    DW7_Reserved : MOS_BITFIELD_RANGE(1, 31);
            };
            uint32_t DW7;
        };

        // DW8: Surface index source linear NV12 UV Plane
        uint32_t    DW8_SrcNV12SurfUVIndex;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CscKernelCurbeData)) == 9);

    //!
    //! \brief    Surface params for CSC kernel
    //!
    struct SurfaceParamsCsc
    {
        bool                        bFlatnessCheckEnabled = false;
        bool                        bMBVProcStatsEnabled = false;
        bool                        bScalingInUses16UnormSurfFmt = false;
        bool                        bScalingInUses32UnormSurfFmt = false;
        PMOS_SURFACE                psInputSurface = nullptr;
        PMOS_SURFACE                psOutput4xDsSurface = nullptr;
        PMOS_SURFACE                psOutput2xDsSurface = nullptr;
        PMOS_SURFACE                psOutputCopiedSurface = nullptr;
        PMOS_SURFACE                psFlatnessCheckSurface = nullptr;
        PMOS_RESOURCE               presMBVProcStatsBuffer = nullptr;
        void*                       hevcExtParams = nullptr;
    };

    //!
    //! \brief    Surface params for DS kernel
    //!
    struct SurfaceParamsDS
    {
        PMOS_SURFACE            psInputSurface = nullptr;
        uint32_t                dwInputFrameWidth = 0;
        uint32_t                dwInputFrameHeight = 0;
        uint32_t                dwInputBottomFieldOffset = 0;
        PMOS_SURFACE            psOutputSurface = nullptr;
        uint32_t                dwOutputFrameWidth = 0;
        uint32_t                dwOutputFrameHeight = 0;
        uint32_t                dwOutputBottomFieldOffset = 0;
        bool                    bScalingOutUses16UnormSurfFmt = false;
        bool                    bScalingOutUses32UnormSurfFmt = false;
        bool                    bFlatnessCheckEnabled = false;
        PMOS_SURFACE            psFlatnessCheckSurface = nullptr;
        uint32_t                dwFlatnessCheckBottomFieldOffset = 0;
        bool                    bMBVProcStatsEnabled = false;
        PMOS_RESOURCE           presMBVProcStatsBuffer = nullptr;
        PMOS_RESOURCE           presMBVProcStatsBotFieldBuffer = nullptr;
        uint32_t                dwMBVProcStatsBottomFieldOffset = 0;
        bool                    bCurrPicIsFrame = false;
        bool                    bPreEncInUse = false;
        bool                    bEnable8x8Statistics = false;
    };

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeCscDs(CodechalEncoderState* encoder);

    //!
    //! \brief    Allocate CSC surface or pick an existing one from the pool
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateSurfaceCsc();

    //!
    //! \brief    Initialize DS kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateDS();

    //!
    //! \brief    Set SurfaceParamsDS for DS kernel
    //!
    //! \param    params
    //!           KernelParams pointer from caller
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSurfaceParamsDS(KernelParams* params);

    //!
    //! \brief    Setup Curbe for DS kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeDS4x();

    //!
    //! \brief    Set-up surface sent to ENC/PAK
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSurfacesToEncPak();


    CodechalEncoderState*                   m_encoder = nullptr;                            //!< Pointer to ENCODER base class
    MOS_INTERFACE*                          m_osInterface = nullptr;                        //!< OS interface
    CodechalHwInterface*                    m_hwInterface = nullptr;                        //!< HW interface
    CodechalDebugInterface*                 m_debugInterface = nullptr;                     //!< Debug interface
    MhwMiInterface*                         m_miInterface = nullptr;                        //!< Common Mi Interface
    MhwRenderInterface*                     m_renderInterface = nullptr;                    //!< Render engine interface
    XMHW_STATE_HEAP_INTERFACE*              m_stateHeapInterface = nullptr;                 //!< State heap class interface
    CodecHalEncodeSfc*                      m_sfcState = nullptr;                           //!< SFC interface
    MHW_KERNEL_STATE*                       m_cscKernelState = nullptr;                     //!< CSC kernel state
    MHW_KERNEL_STATE*                       m_dsKernelState = nullptr;                      //!< DS kernel state

    union
    {
        struct
        {
            uint8_t             m_cscRequireCopy : 1;                                       //!< bit 0 = 1: raw surface is non-aligned, requires copy
            uint8_t             m_cscRequireColor : 1;                                      //!< bit 1 = 1: raw surface is not NV12 Tile-Y, requires CSC
            uint8_t             m_cscRequireMmc : 1;                                        //!< bit 2 = 1: raw surface is MMC compressed, requires decompression
            uint8_t             m_cscRequireConvTo8bPlanar : 1;                             //!< bit 3 = 1: raw surface requires 10/8-bit conversion
            uint8_t             m_cscUsingSfc : 1;                                          //!< bit 4 = 1: use SFC to do CSC, only applies to RGB
            uint8_t             reserved1 : 3;
        };
        uint8_t                 m_cscFlag;                                                  //!< the actual CSC/Copy operation to be performed for raw surface
    };

    uint8_t                     m_rawSurfAlignment = 4;                                     //!< Raw surface alignment
    uint8_t                     m_mfxReconSurfAlignment = 16;                               //!< Recon surface alignment for MFX engine
    uint8_t                     m_hcpReconSurfAlignment = 8;                                //!< Recon surface alignment for HCP engine
    uint32_t                    m_cscRawSurfWidth = 0;                                      //!< Raw surface allocation width
    uint32_t                    m_cscRawSurfHeight = 0;                                     //!< Raw surface allocation height
    uint32_t                    m_walkerResolutionX = 0;                                    //!< Media walker resolution X
    uint32_t                    m_walkerResolutionY = 0;                                    //!< Media walker resolution Y
    uint32_t                    m_cscCurbeLength = 0;                                       //!< CSC kernel Curbe length
    uint32_t                    m_cscKernelUID = 0;                                         //!< CSC kernel UID
    uint32_t                    m_dsBTCount[2] = { 0 };                                     //!< DS kernel BTI count
    uint32_t                    m_dsCurbeLength[2] = { 0 };                                 //!< DS kernel Curbe length
    uint32_t                    m_dsInlineDataLength = 0;                                   //!< DS kernel inline data length
    uint32_t                    m_dsBTISrcY = ds4xSrcYPlane;                                //!< DS kernel BTI: source Y
    uint32_t                    m_dsBTIDstY = ds4xDstYPlane;                                //!< DS kernel BTI: destination Y
    uint32_t                    m_dsBTISrcYTopField = ds4xSrcYPlaneTopField;                //!< DS kernel BTI: source Y top filed
    uint32_t                    m_dsBTIDstYTopField = ds4xDstYPlaneTopField;                //!< DS kernel BTI: destination Y top filed
    uint32_t                    m_dsBTISrcYBtmField = ds4xSrcYPlaneBtmField;                //!< DS kernel BTI: source Y bottom filed
    uint32_t                    m_dsBTIDstYBtmField = ds4xDstYPlaneBtmField;                //!< DS kernel BTI: destination Y bottom filed
    uint32_t                    m_dsBTIDstFlatness = ds4xDstFlatness;                       //!< DS kernel BTI: destination flatness
    uint32_t                    m_dsBTIDstFlatnessTopField = ds4xDstFlatnessTopField;       //!< DS kernel BTI: destination flatness top field
    uint32_t                    m_dsBTIDstFlatnessBtmField = ds4xDstFlatnessBtmField;       //!< DS kernel BTI: destination flatness bottom field
    uint32_t                    m_dsBTIDstMbVProc = ds4xDstMbVProc;                         //!< DS kernel BTI: destination MbStats
    uint32_t                    m_dsBTIDstMbVProcTopField = ds4xDstMbVProcTopField;         //!< DS kernel BTI: destination MbStats top filed
    uint32_t                    m_dsBTIDstMbVProcBtmField = ds4xDstMbVProcBtmField;         //!< DS kernel BTI: destination MbStats bottom filed
    uint32_t                    m_combinedKernelSize = 0;                                   //!< Combined kernel size
    uint8_t*                    m_kernelBase = nullptr;                                     //!< kernel binary base address
    uint8_t*                    m_dsKernelBase = nullptr;                                   //!< kernel binary base address for DS kernel
    CscColor                    m_colorRawSurface = cscColorNv12TileY;                      //!< Raw surface color format
    CurbeParams                 m_curbeParams;                                              //!< Curbe param (shared by CSC and DS kernel)
    SurfaceParamsCsc            m_surfaceParamsCsc;                                         //!< CSC surface param
    SurfaceParamsDS             m_surfaceParamsDS;                                          //!< DS surface param

    //!
    //! Reference to data members in Encoder class
    //!
    bool&                       m_useRawForRef;
    bool&                       m_useCommonKernel;
    bool&                       m_useHwScoreboard;
    bool&                       m_renderContextUsesNullHw;
    bool&                       m_groupIdSelectSupported;
    bool&                       m_16xMeSupported;
    bool&                       m_32xMeSupported;
    bool&                       m_scalingEnabled;
    bool&                       m_2xScalingEnabled;
    bool&                       m_firstField;
    bool&                       m_fieldScalingOutputInterleaved;
    bool&                       m_flatnessCheckEnabled;
    bool&                       m_mbStatsEnabled;
    bool&                       m_mbStatsSupported;
    bool&                       m_singleTaskPhaseSupported;
    bool&                       m_firstTaskInPhase;
    bool&                       m_lastTaskInPhase;
    bool&                       m_pollingSyncEnabled;
    uint8_t&                    m_groupId;
    uint8_t&                    m_outputChromaFormat;
    uint32_t&                   m_standard;
    uint32_t&                   m_mode;
    uint32_t&                   m_downscaledWidth4x;
    uint32_t&                   m_downscaledHeight4x;
    uint32_t&                   m_downscaledWidth16x;
    uint32_t&                   m_downscaledHeight16x;
    uint32_t&                   m_downscaledWidth32x;
    uint32_t&                   m_downscaledHeight32x;
    uint32_t&                   m_scaledBottomFieldOffset;
    uint32_t&                   m_scaled16xBottomFieldOffset;
    uint32_t&                   m_scaled32xBottomFieldOffset;
    uint32_t&                   m_mbVProcStatsBottomFieldOffset;
    uint32_t&                   m_mbStatsBottomFieldOffset;
    uint32_t&                   m_flatnessCheckBottomFieldOffset;
    uint32_t&                   m_verticalLineStride;
    uint32_t&                   m_maxBtCount;
    uint32_t&                   m_vmeStatesSize;
    uint32_t&                   m_storeData;
    uint32_t&                   m_syncMarkerOffset;
    uint32_t &                  m_syncMarkerValue;
    MOS_GPU_CONTEXT&            m_renderContext;
    MHW_WALKER_MODE&            m_walkerMode;
    CODEC_REF_LIST*&            m_currRefList;
    MOS_RESOURCE&               m_resMbStatsBuffer;
    MOS_SURFACE*&               m_rawSurfaceToEnc;
    MOS_SURFACE*&               m_rawSurfaceToPak;

private:
    //!
    //! \brief    CSC kernel binding table
    //!
    enum CscKernelBTI
    {
        cscSrcYPlane = 0,
        cscSrcUVPlane = 1,
        cscDstDsYPlane = 2,
        cscDstDsUVPlane = 3,
        cscDstFlatOrMbStats = 4,
        cscDstCopyYPlane = 5,
        cscDstCopyUVPlane = 6,
        cscNumSurfaces = 7
    };

    //!
    //! \brief    CSC kernel header struct
    //!
    struct CscKernelHeader
    {
        int                     kernelCount = 0;
        CODECHAL_KERNEL_HEADER  header = {};
    };

    //!
    //! \brief    4xDS kernel binding table
    //!
    enum Ds4xKernelBTI
    {
        ds4xSrcYPlane = 0,
        ds4xDstYPlane = 1,
        ds4xSrcYPlaneTopField = 0,
        ds4xDstYPlaneTopField = 1,
        ds4xSrcYPlaneBtmField = 2,
        ds4xDstYPlaneBtmField = 3,
        ds4xDstFlatness = 4,
        ds4xDstFlatnessTopField = 4,
        ds4xDstFlatnessBtmField = 5,
        ds4xDstMbVProc = 6,
        ds4xDstMbVProcTopField = 6,
        ds4xDstMbVProcBtmField = 7,
        ds4xNumSurfaces = 8
    };

    //!
    //! \brief    2xDS kernel binding table
    //!
    enum Ds2xKernelBTI
    {
        ds2xSrcYPlane = 0,
        ds2xDstYPlane = 1,
        ds2xSrcYPlaneTopField = 0,
        ds2xDstYPlaneTopField = 1,
        ds2xSrcYPlaneBtmField = 2,
        ds2xDstYPlaneBtmField = 3,
        ds2xNumSurfaces = 4
    };

    //!
    //! \brief    DS kernel Curbe data
    struct DsKernelInlineData
    {
        DsKernelInlineData()
        {
            DW04_VideoXScalingStep =
            DW05_VideoStepDelta = 0.0;
            DW00 =
            DW01 =
            DW02 =
            DW03 =
            DW06 =
            DW07_GroupIDNumber =
            DW08 =
            DW09 =
            DW10 =
            DW11 =
            DW12 =
            DW13_Reserved =
            DW14_Reserved =
            DW15_Reserved = 0;
        }

        // uint32_t 0 - GRF R7.0
        union
        {
            // All
            struct
            {
                uint32_t       DestinationBlockHorizontalOrigin : 16;
                uint32_t       DestinationBlockVerticalOrigin : 16;
            };
            // Block Copy
            struct
            {
                uint32_t       BlockHeight : 16;
                uint32_t       BufferOffset : 16;
            };
            // FMD Summation
            struct
            {
                uint32_t       StartRowOffset;
            };
            uint32_t DW00;
        };

        // uint32_t 1 - GRF R7.1
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer0 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer0 : 16;
            };
            // FMD Summation
            struct
            {
                uint32_t       TotalRows;
            };
            uint32_t DW01;
        };

        // uint32_t 2 - GRF R7.2
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer1 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer1 : 16;
            };
            // FMD Summation
            struct
            {
                uint32_t       StartColumnOffset;
            };
            uint32_t DW02;
        };

        // uint32_t 3 - GRF R7.3
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer2 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer2 : 16;
            };
            // FMD Summation
            struct
            {
                uint32_t       TotalColumns;
            };
            uint32_t DW03;
        };

        // uint32_t 4 - GRF R7.4
        // Sampler Load
        float       DW04_VideoXScalingStep;

        // uint32_t 5 - GRF R7.5
        // NLAS
        float       DW05_VideoStepDelta;

        // uint32_t 6 - GRF R7.6
        union
        {
            // AVScaling
            struct
            {
                uint32_t       VerticalBlockNumber : 17;
                uint32_t       AreaOfInterest : 1;
                uint32_t : 14;
            };
            uint32_t DW06;
        };

        // uint32_t 7 - GRF R7.7
        // AVScaling
        uint32_t       DW07_GroupIDNumber;

        // uint32_t 8 - GRF R8.0
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer3 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer3 : 16;
            };
            uint32_t DW08;
        };

        // uint32_t 9 - GRF R8.1
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer4 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer4 : 16;
            };
            uint32_t DW09;
        };

        // uint32_t 10 - GRF R8.2
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer5 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer5 : 16;
            };
            uint32_t DW10;
        };

        // uint32_t 11 - GRF R8.3
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer6 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer6 : 16;
            };
            uint32_t DW11;
        };

        // uint32_t 12 - GRF R8.4
        union
        {
            // Composite
            struct
            {
                uint32_t       HorizontalBlockCompositeMaskLayer7 : 16;
                uint32_t       VerticalBlockCompositeMaskLayer7 : 16;
            };
            uint32_t DW12;
        };

        // uint32_t 13 - GRF R8.5
        uint32_t DW13_Reserved;

        // uint32_t 14 - GRF R8.6
        uint32_t DW14_Reserved;

        // uint32_t 15 - GRF R8.7
        uint32_t DW15_Reserved;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(DsKernelInlineData)) == 16);

    //!
    //! \brief    Check raw surface color format
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckRawColorFormat(MOS_FORMAT format);

    //!
    //! \brief    Initialize SFC state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitSfcState();

    //!
    //! \brief    Setup SFC params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetParamsSfc(CODECHAL_ENCODE_SFC_PARAMS* sfcParams);

    //!
    //! \brief    Initialize CSC kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateCsc();

    //!
    //! \brief    Setup Gen-specific kernel params
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetKernelParamsCsc(KernelParams* params);

    //!
    //! \brief    Setup Curbe
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeCsc();

    //!
    //! \brief    Send surface for CSC kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendSurfaceCsc(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Setup Curbe for DS kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeDS2x();

    //!
    //! \brief    Send surface for DS kernel
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendSurfaceDS(PMOS_COMMAND_BUFFER cmdBuffer);

    union
    {
        struct
        {
            uint8_t             m_cscEnableCopy : 1;                                        //!< bit 0 = 1: Ds+Copy kernel is enabled to copy non-aligned raw surface
            uint8_t             m_cscEnableColor : 1;                                       //!< bit 1 = 1: Ds+Copy kernel is enabled to perform CSC
            uint8_t             m_cscEnableMmc : 1;                                         //!< bit 2 = 1: Ds+Copy kernel is enabled to decompress MMC raw surface
            uint8_t             m_cscEnableSfc : 1;                                         //!< bit 3 = 1: VEBOX is enabled to perform ARGB CSC
            uint8_t             reserved : 4;
        };
        uint8_t                 m_cscDsConvEnable;
    };

    uint32_t                    m_threadTraverseSizeX = 5;                                  //!< target traverse thread space size in width
    uint32_t                    m_threadTraverseSizeY = 2;                                  //!< target traverse thread space size in height
};

#endif  // __CODECHAL_ENCODE_CSC_DS_H__
