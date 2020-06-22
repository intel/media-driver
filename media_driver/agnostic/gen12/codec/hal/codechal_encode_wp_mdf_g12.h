#pragma once
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
//! \file     codechal_encode_wp_mdf_g12.h
//! \brief    Gen12 class for weighted prediction.
//!

#ifndef __CODECHAL_ENCODE_WP_MDF_G12_H__
#define __CODECHAL_ENCODE_WP_MDF_G12_H__

#include "codechal_encode_wp.h"

class CodechalEncodeWPMdfG12 : public CodechalEncodeWP
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeWPMdfG12(CodechalEncoderState* encoder):
    CodechalEncodeWP(encoder) {}

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeWPMdfG12() { ReleaseResources(); };

    //!
    //! \brief    Initialize weighted prediction kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateIsa(void *kernelIsa, uint32_t kernelIsaSize);

    //!
    //! \brief    SW scoreboard init kernel function
    //!
    //! \param    [in] params
    //!           Pointer to KernelParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute(KernelParams *params)override;

protected:
    //!
    //! \brief    SW scoreboard init kernel Curbe data
    //!
    struct CurbeData
    {
        // DW0
        union
        {
            struct
            {
                uint32_t   defaultWeight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   defaultOffset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t   roi0XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi0YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t   roi0XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi0YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t   roi0Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi0Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t   roi1XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi1YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t   roi1XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi1YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t   roi1Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi1Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t   roi2XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi2YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW7;

        // DW8
        union
        {
            struct
            {
                uint32_t   roi2XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi2YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW8;

        // DW9
        union
        {
            struct
            {
                uint32_t   roi2Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi2Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW9;

        // DW10
        union
        {
            struct
            {
                uint32_t   roi3XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi3YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW10;

        // DW11
        union
        {
            struct
            {
                uint32_t   roi3XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi3YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW11;

        // DW12
        union
        {
            struct
            {
                uint32_t   roi3Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi3Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW12;

        // DW13
        union
        {
            struct
            {
                uint32_t   roi4XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi4YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW13;

        // DW14
        union
        {
            struct
            {
                uint32_t   roi4XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi4YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW14;

        // DW15
        union
        {
            struct
            {
                uint32_t   roi4Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi4Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW15;

        // DW16
        union
        {
            struct
            {
                uint32_t   roi5XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi5YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW16;

        // DW17
        union
        {
            struct
            {
                uint32_t   roi5XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi5YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW17;

        // DW18
        union
        {
            struct
            {
                uint32_t   roi5Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi5Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW18;

        // DW19
        union
        {
            struct
            {
                uint32_t   roi6XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi6YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW19;

        // DW20
        union
        {
            struct
            {
                uint32_t   roi6XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi6YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW20;

        // DW21
        union
        {
            struct
            {
                uint32_t   roi6Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi6Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW21;

        // DW22
        union
        {
            struct
            {
                uint32_t   roi7XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi7YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW22;

        // DW23
        union
        {
            struct
            {
                uint32_t   roi7XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi7YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW23;

        // DW24
        union
        {
            struct
            {
                uint32_t   roi7Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi7Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW24;

        // DW25
        union
        {
            struct
            {
                uint32_t   roi8XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi8YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW25;

        // DW26
        union
        {
            struct
            {
                uint32_t   roi8XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi8YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW26;

        // DW27
        union
        {
            struct
            {
                uint32_t   roi8Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi8Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW27;

        // DW28
        union
        {
            struct
            {
                uint32_t   roi9XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi9YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW28;

        // DW29
        union
        {
            struct
            {
                uint32_t   roi9XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi9YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW29;

        // DW30
        union
        {
            struct
            {
                uint32_t   roi9Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi9Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW30;

        // DW31
        union
        {
            struct
            {
                uint32_t   roi10XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi10YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW31;

        // DW32
        union
        {
            struct
            {
                uint32_t   roi10XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi10YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t   roi10Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi10Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t   roi11XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi11YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t   roi11XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi11YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t   roi11Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi11Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t   roi12XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi12YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t   roi12XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi12YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW38;

        // DW39
        union
        {
            struct
            {
                uint32_t   roi12Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi12Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW39;

        // DW40
        union
        {
            struct
            {
                uint32_t   roi13XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi13YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW40;

        // DW41
        union
        {
            struct
            {
                uint32_t   roi13XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi13YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW41;

        // DW42
        union
        {
            struct
            {
                uint32_t   roi13Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi13Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW42;

        // DW43
        union
        {
            struct
            {
                uint32_t   roi14XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi14YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW43;

        // DW44
        union
        {
            struct
            {
                uint32_t   roi14XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi14YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW44;

        // DW45
        union
        {
            struct
            {
                uint32_t   roi14Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi14Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW45;

        // DW46
        union
        {
            struct
            {
                uint32_t   roi15XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi15YTop : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW46;

        // DW47
        union
        {
            struct
            {
                uint32_t   roi15XRight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi15YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW47;

        // DW48
        union
        {
            struct
            {
                uint32_t   roi15Weight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t   roi15Offset : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW48;

        // DW49
        union
        {
            struct
            {
                uint32_t lumaLog2WeightDenom : MOS_BITFIELD_RANGE(0, 2);
                uint32_t reserved1 : MOS_BITFIELD_RANGE(3, 7);
                uint32_t roiEnableFlag : MOS_BITFIELD_RANGE(8, 8);
                uint32_t reserved2 : MOS_BITFIELD_RANGE(9, 31);
            };
            struct
            {
                uint32_t value;
            };
        } DW49;

    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CurbeData)) == 50);

    CmThreadSpace          *m_threadSpace = nullptr;
    CmProgram              *m_cmProgram = nullptr;
    // every reference frame has own CmKernel and input/output surfaces
    // for L0 references - [0, CODEC_WP_OUTPUT_L1_START - 1], for L1 references - [CODEC_WP_OUTPUT_L1_START, CODEC_NUM_WP_FRAME - 1]
    CmKernel               *m_cmKrn[CODEC_NUM_WP_FRAME] = { nullptr };
    CmSurface2D            *m_wpInputSurface[CODEC_NUM_WP_FRAME] = { nullptr };
    CmSurface2D            *m_wpOutputSurface[CODEC_NUM_WP_FRAME] = { nullptr };

    //!
    //! \brief    Set Curbe
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCurbe(CurbeData& curbe);

    MOS_STATUS SetupSurfaces(uint8_t wpKrnIdx);
    virtual MOS_STATUS SetupKernelArgs(uint8_t wpKrnIdx);
    MOS_STATUS ReleaseResources();

};

#endif  // __CODECHAL_ENCODE_WP_MDF_G12_H__
