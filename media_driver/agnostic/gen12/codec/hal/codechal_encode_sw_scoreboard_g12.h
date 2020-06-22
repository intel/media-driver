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

#ifndef __CODECHAL_ENCODE_SW_SCOREBOARD_G12_H__
#define __CODECHAL_ENCODE_SW_SCOREBOARD_G12_H__

#include "codechal_encode_sw_scoreboard.h"

class CodechalEncodeSwScoreboardG12 : public CodechalEncodeSwScoreboard
{
public:
    //!
    //! \brief    SW scoreboard init kernel binding table
    //!
    enum KernelBTI
    {
        swScoreboardInitSurface = 0,
        swScoreboardNumSurfaces = 1,
    };

    //!
    //! \brief    Get SW scoreboard init BT count
    //!
    //! \return   Number of BTI
    //!
    virtual uint8_t GetBTCount() override;

    //!
    //! \brief    Initialize SW scoreboard init kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitKernelState() override;

    //!
    //! \brief    Constructor
    //!
    CodechalEncodeSwScoreboardG12(CodechalEncoderState* encoder) :
        CodechalEncodeSwScoreboard(encoder) {}

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeSwScoreboardG12() {};

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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
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

        // DW16
        union
        {
            struct
            {
                uint32_t   softwareScoreboard : MOS_BITFIELD_RANGE( 0, 31);
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
                uint32_t   reserved           : MOS_BITFIELD_RANGE( 0, 31);
            };
            struct
            {
                uint32_t   value;
            };
        } DW17;

    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CurbeData)) == 18);

    //!
    //! \brief    Setup Curbe
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbe() override;
};

#endif  // __CODECHAL_ENCODE_SW_SCOREBOARD_G12_H__
