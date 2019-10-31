/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_csc_ds_g12.h
//! \brief    Gen12 class for CSC and Downscaling.
//!

#ifndef __CodechalEncodeCscDsG12__
#define __CodechalEncodeCscDsG12__

#include "codechal_encode_csc_ds.h"

class CodechalEncodeCscDsG12 : public CodechalEncodeCscDs
{
public:
    //!
    //! \brief    Extension params used only by HEVC
    //!
    struct HevcExtKernelParams
    {
        bool                bHevcEncHistorySum = false;
        bool                bUseLCU32 = false;
        PMOS_RESOURCE       presHistoryBuffer = nullptr;
        uint32_t            dwSizeHistoryBuffer = 0;
        uint32_t            dwOffsetHistoryBuffer = 0;
        PMOS_RESOURCE       presHistorySumBuffer = nullptr;
        uint32_t            dwSizeHistorySumBuffer = 0;
        uint32_t            dwOffsetHistorySumBuffer = 0;
        PMOS_RESOURCE       presMultiThreadTaskBuffer = nullptr;
        uint32_t            dwSizeMultiThreadTaskBuffer = 0;
        uint32_t            dwOffsetMultiThreadTaskBuffer = 0;
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
            DW8 = ds4xDstMbVProc;
            DW9_MBVProcStatsBTIBottomField = 0;
            DW10_Reserved =
            DW11_Reserved =
            DW12_Reserved =
            DW13_Reserved =
            DW14_Reserved =
            DW15_Reserved = 0;
        }

        // DW0
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
            uint32_t: MOS_BITFIELD_RANGE(4, 31);
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
                uint32_t   DW8_MBVProcStatsBTIFrame;
            };
            struct
            {
                uint32_t   DW8_MBVProcStatsBTITopField;
            };
            uint32_t DW8;
        };

        // DW9
        uint32_t DW9_MBVProcStatsBTIBottomField;

        // DW10
        uint32_t DW10_Reserved;

        // DW11
        uint32_t DW11_Reserved;

        //DW12
        uint32_t DW12_Reserved;

        //DW13
        uint32_t DW13_Reserved;

        //DW14
        uint32_t DW14_Reserved;

        //DW15
        uint32_t DW15_Reserved;

    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(Ds4xKernelCurbeData)) == 16);

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeCscDsG12(CodechalEncoderState* encoder);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeCscDsG12();

    virtual uint8_t GetBTCount() const override;

protected:
    virtual MOS_STATUS AllocateSurfaceCsc() override;
    virtual MOS_STATUS InitKernelStateDS() override;
    virtual MOS_STATUS SetCurbeDS4x() override;
    virtual MOS_STATUS SetKernelParamsCsc(KernelParams* params) override;
    virtual MOS_STATUS InitKernelStateCsc() override;   

private:
    //!
    //! \brief    CSC kernel binding table
    //!
    enum CscKernelBTI
    {
        cscSrcYPlane = 0,
        cscSrcUVPlane = 1,
        cscDstConvYPlane = 2,
        cscDstConvUVlane = 3,
        cscDst4xDs = 4,
        cscDstMbStats = 5,
        cscDst2xDs = 6,
        cscDstHistBuffer = 7,
        cscDstHistSum = 8,
        cscDstMultiTask = 9,
        cscNumSurfaces = 10,
    };

    //!
    //! \brief    Csc kernel Curbe data
    //!
    struct CscKernelCurbeData
    {
        CscKernelCurbeData()
        {
            DW0 =
            DW1 =
            DW2 =
            DW3_MBFlatnessThreshold =
            DW4_CSC_Coefficient_C0 =
            DW4_CSC_Coefficient_C1 =
            DW5_CSC_Coefficient_C2 =
            DW5_CSC_Coefficient_C3 =
            DW6_CSC_Coefficient_C4 =
            DW6_CSC_Coefficient_C5 =
            DW7_CSC_Coefficient_C6 =
            DW7_CSC_Coefficient_C7 =
            DW8_CSC_Coefficient_C8 =
            DW8_CSC_Coefficient_C9 =
            DW9_CSC_Coefficient_C10 =
            DW9_CSC_Coefficient_C11 = 0;
            DW10_BTI_InputSurface = cscSrcYPlane;
            DW11_BTI_Enc8BitSurface = cscDstConvYPlane;
            DW12_BTI_4xDsSurface = cscDst4xDs;
            DW13_BTI_MbStatsSurface = cscDstMbStats;
            DW14_BTI_2xDsSurface = cscDst2xDs;
            DW15_BTI_HistoryBuffer = cscDstHistBuffer;
            DW16_BTI_HistorySumBuffer = cscDstHistSum;
            DW17_BTI_MultiTaskBuffer = cscDstMultiTask;
        }

        union
        {
            struct
            {
                // DWORD 0
                uint32_t    DW0_Reserved_0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t    DW0_Reserved_1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t    DW0_OutputBitDepthForChroma : MOS_BITFIELD_RANGE(16, 23);
                uint32_t    DW0_OutputBitDepthForLuma : MOS_BITFIELD_RANGE(24, 30);
                uint32_t    DW0_RoundingEnable : MOS_BITFIELD_BIT(31);
            };
            uint32_t DW0;
        };

        union
        {
            struct
            {
                // DWORD 1
                uint32_t    DW1_PictureFormat : MOS_BITFIELD_RANGE(0, 7);
                uint32_t    DW1_ConvertFlag : MOS_BITFIELD_BIT(8);
                uint32_t    DW1_DownscaleStage : MOS_BITFIELD_RANGE(9, 11);
                uint32_t    DW1_MbStatisticsDumpFlag : MOS_BITFIELD_BIT(12);
                uint32_t    DW1_YUY2ConversionFlag : MOS_BITFIELD_BIT(13);
                uint32_t    DW1_HevcEncHistorySum : MOS_BITFIELD_BIT(14);
                uint32_t    DW1_LCUSize : MOS_BITFIELD_BIT(15);
                uint32_t    DW1_ChromaSitingLocation : MOS_BITFIELD_RANGE(16, 23);
                uint32_t    DW1_Reserved_0 : MOS_BITFIELD_RANGE(24, 31);
            };
            uint32_t DW1;
        };

        union
        {
            struct
            {
                // DWORD 2
                uint32_t    DW2_OriginalPicWidthInSamples : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW2_OriginalPicHeightInSamples : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW2;
        };

        // DWORD 3
        uint32_t    DW3_MBFlatnessThreshold;

        // DWORD 4
        uint32_t    DW4_CSC_Coefficient_C0 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW4_CSC_Coefficient_C1 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 5
        uint32_t    DW5_CSC_Coefficient_C2 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW5_CSC_Coefficient_C3 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 6
        uint32_t    DW6_CSC_Coefficient_C4 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW6_CSC_Coefficient_C5 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 7
        uint32_t    DW7_CSC_Coefficient_C6 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW7_CSC_Coefficient_C7 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 8
        uint32_t    DW8_CSC_Coefficient_C8 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW8_CSC_Coefficient_C9 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 9
        uint32_t    DW9_CSC_Coefficient_C10 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW9_CSC_Coefficient_C11 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 10
        uint32_t    DW10_BTI_InputSurface;

        // DWORD 11
        uint32_t    DW11_BTI_Enc8BitSurface;

        // DWORD 12
        uint32_t    DW12_BTI_4xDsSurface;

        // DWORD 13
        uint32_t    DW13_BTI_MbStatsSurface;

        // DWORD 14
        uint32_t    DW14_BTI_2xDsSurface;

        // DWORD 15
        uint32_t    DW15_BTI_HistoryBuffer;

        // DWORD 16
        uint32_t    DW16_BTI_HistorySumBuffer;

        // DWORD 17
        uint32_t    DW17_BTI_MultiTaskBuffer;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CscKernelCurbeData)) == 18);

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
        ds4xDstMbVProc = 4,
        ds4xDstMbVProcTopField = 4,
        ds4xDstMbVProcBtmField = 5,
        ds4xNumSurfaces = 6
    };

    virtual MOS_STATUS CheckRawColorFormat(MOS_FORMAT format) override;
    virtual MOS_STATUS SetCurbeCsc() override;
    virtual MOS_STATUS SendSurfaceCsc(PMOS_COMMAND_BUFFER cmdBuffer) override;
    virtual MOS_STATUS InitSfcState() override;
};

#endif  // __CodechalEncodeCscDsG12__
