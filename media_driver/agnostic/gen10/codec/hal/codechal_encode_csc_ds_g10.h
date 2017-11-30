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
        // DW0
        union
        {
            struct
            {
                uint32_t   DW0_InputPictureWidth : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   DW0_InputPictureHeight : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW0 = 0;
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
            uint32_t DW1 = ds4xSrcYPlane;
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
            uint32_t DW2 = ds4xDstYPlane;
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
            uint32_t DW6 = 0;
        };

        // DW7
        uint32_t DW7_Reserved = 0;

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
            uint32_t DW8 = ds4xDstMbVProc;
        };

        // DW9
        uint32_t DW9_MBVProcStatsBTIBottomField = 0;

        // DW10
        uint32_t DW10_Reserved = 0;

        // DW11
        uint32_t DW11_Reserved = 0;

        //DW12
        uint32_t DW12_Reserved = 0;

        //DW13
        uint32_t DW13_Reserved = 0;

        //DW14
        uint32_t DW14_Reserved = 0;

        //DW15
        uint32_t DW15_Reserved = 0;

    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(Ds4xKernelCurbeData)) == 16);

    //!
    //! \brief    Constructor
    //!            
    CodechalEncodeCscDsG10(PCODECHAL_ENCODER pEncoder);
    CodechalEncodeCscDsG10(CodechalEncoderState* encoder);

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeCscDsG10() {}

protected:
    virtual MOS_STATUS InitKernelStateDS() override;
    virtual MOS_STATUS SetCurbeDS4x() override;

private:
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
