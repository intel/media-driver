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
//! \file     codechal_encode_csc_ds_g10.h
//! \brief    Gen10 class for CSC and Downscaling.
//!

#ifndef __CodechalEncodeCscDsG10__
#define __CodechalEncodeCscDsG10__

#include "codechal_encode_csc_ds.h"

class CodechalEncodeCscDsG10 : public CodechalEncodeCscDs
{
public:
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
        uint32_t DW3_InputYBTIBottomField = 0;

        // DW4
        uint32_t DW4_OutputYBTIBottomField = 0;

        // DW5
        uint32_t DW5_FlatnessThreshold = 0;

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
    CodechalEncodeCscDsG10(CodechalEncoderState* encoder);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeCscDsG10() {}

protected:
    virtual MOS_STATUS InitKernelStateDS() override;
    virtual MOS_STATUS SetCurbeDS4x() override;

private:
    MOS_STATUS SetCurbeCsc() override;
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
    //! \brief    Csc kernel Curbe data
    //!
    struct CscKernelCurbeData
    {
        CscKernelCurbeData()
        {
            DW0 =
            DW1 =
            DW2_FlatnessThreshold =
            DW3_EnableMBStatSurface =
            DW10_Reserved =
            DW11_Reserved =
            DW12_Reserved =
            DW13_Reserved =
            DW14_Reserved =
            DW15_Reserved = 0;
            DW16_SrcNV12SurfYIndex = cscSrcYPlane;
            DW17_DstYSurfIndex = cscDstDsYPlane;
            DW18_MbStatDstSurfIndex = cscDstFlatOrMbStats;
            DW19_CopyDstNV12SurfIndex = cscDstCopyYPlane;
            DW20_SrcNV12SurfUVIndex = cscSrcUVPlane;
        }

        union
        {
            struct
            {
                // DWORD 0 - GRF R1.0
                uint32_t    DW0_InputPictureWidth : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW0_InputPictureHeight : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW0;
        };

        union
        {
            struct
            {
                // DWORD 1 - GRF R1.1
                uint32_t    DW1_CscDsCopyOpCode : MOS_BITFIELD_RANGE(0, 7);
                uint32_t    DW1_InputColorFormat : MOS_BITFIELD_RANGE(8, 15);
                uint32_t    DW1_ChromaSitting : MOS_BITFIELD_RANGE(16, 23);
                uint32_t    DW1_Reserved : MOS_BITFIELD_RANGE(24, 31);
            };
            uint32_t DW1;
        };

        // DWORD 2 - GRF R1.2: MBFlatnessThreshold
        uint32_t    DW2_FlatnessThreshold;

        // DWORD 3 - GRF R1.3: EnableMBStatSurface
        uint32_t    DW3_EnableMBStatSurface;

        // DWORD 4 - GRF R1.4: RGB to YUV conversion coefficients
        uint32_t    DW4_CscCoefficientC0 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW4_CscCoefficientC1 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 5 - GRF R1.5: RGB to YUV conversion coefficients
        uint32_t    DW5_CscCoefficientC2 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW5_CscCoefficientC3 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 6 - GRF R1.6: RGB to YUV conversion coefficients
        uint32_t    DW6_CscCoefficientC4 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW6_CscCoefficientC5 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 7 - GRF R1.7: RGB to YUV conversion coefficients
        uint32_t    DW7_CscCoefficientC6 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW7_CscCoefficientC7 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 8 - GRF R2.0: RGB to YUV conversion coefficients
        uint32_t    DW8_CscCoefficientC8 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW8_CscCoefficientC9 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 9 - GRF R2.1: RGB to YUV conversion coefficients
        uint32_t    DW9_CscCoefficientC10 : MOS_BITFIELD_RANGE(0, 15);
        uint32_t    DW9_CscCoefficientC11 : MOS_BITFIELD_RANGE(16, 31);

        // DWORD 10 - GRF R2.2
        uint32_t    DW10_Reserved;

        // DWORD 11 - GRF R2.3
        uint32_t    DW11_Reserved;

        // DWORD 12 - GRF R2.4
        uint32_t    DW12_Reserved;

        // DWORD 13 - GRF R2.5
        uint32_t    DW13_Reserved;

        // DWORD 14 - GRF R2.6
        uint32_t    DW14_Reserved;

        // DWORD 15 - GRF R2.7
        uint32_t    DW15_Reserved;

        // DWORD 16 - GRF R3.1: Surface index source linear NV12 Y Plane
        uint32_t    DW16_SrcNV12SurfYIndex;

        // DWORD 17 - GRF R3.2: : Surface index downscale destination Planar Y
        uint32_t    DW17_DstYSurfIndex;

        // DWORD 18 - GRF R3.3: : Surface index flatness destination
        uint32_t    DW18_MbStatDstSurfIndex;

        // DWORD 19 - GRF R3.4: Surface index copy destination NV12
        uint32_t    DW19_CopyDstNV12SurfIndex;

        // DWORD 20 - GRF R3.5: Surface index source linear NV12 UV Plane
        uint32_t    DW20_SrcNV12SurfUVIndex;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CscKernelCurbeData)) == 21);

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
};

#endif  // __CodechalEncodeCscDsG10__
