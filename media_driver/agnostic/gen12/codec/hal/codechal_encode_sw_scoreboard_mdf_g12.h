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
//! \file     codechal_encode_sw_scoreboard_g12.h
//! \brief    Gen12 class for SW socreboard init kernel.
//!

#ifndef __CODECHAL_ENCODE_SW_SCOREBOARD_MDF_G12_H__
#define __CODECHAL_ENCODE_SW_SCOREBOARD_MDF_G12_H__

#include "codechal_encode_sw_scoreboard_g12.h"

class CodechalEncodeSwScoreboardMdfG12 : public CodechalEncodeSwScoreboardG12
{
public:
    //!
    //! \brief    Initialize SW scoreboard init kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelState()override;

    //!
    //! \brief    SW scoreboard init kernel function
    //!
    //! \param    [in] params
    //!           Pointer to KernelParams
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Execute(KernelParams *params)override;

    //!
    //! \brief    Constructor
    //!            
    CodechalEncodeSwScoreboardMdfG12(CodechalEncoderState* encoder) :
        CodechalEncodeSwScoreboardG12(encoder) {}

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeSwScoreboardMdfG12() {};

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
                uint32_t   scoreboardWidth    : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   scoreboardHeight   : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   isHevc                   : MOS_BITFIELD_BIT  (     0);
                uint32_t   reserved                 : MOS_BITFIELD_RANGE( 1, 31);
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
                uint32_t   numberOfWaveFrontsSplits : MOS_BITFIELD_RANGE( 0,  7);
                uint32_t   numberofChildThreads     : MOS_BITFIELD_RANGE( 8, 15);
                uint32_t   reserved                 : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   dependencyPattern  : MOS_BITFIELD_RANGE( 0, 15);
                uint32_t   reserved           : MOS_BITFIELD_RANGE(16, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW15;
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CurbeData)) == 16);

    CmThreadSpace          *m_threadSpace = nullptr;
    CmKernel               *m_cmKrn = nullptr;
    CmProgram              *m_cmProgram = nullptr;
    CmSurface2D            *m_scoreboardSurface = nullptr;
    CmSurface2D            *m_lcuInfoSurface = nullptr;

    //!
    //! \brief    Set Curbe (depends on Generation/implementation)
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbe(CurbeData& curbe);
 
    MOS_STATUS SetupSurfaces();
    MOS_STATUS SetupKernelArgs();
    MOS_STATUS ReleaseResources();
};

#endif  // __CODECHAL_ENCODE_SW_SCOREBOARD_G12_H__
