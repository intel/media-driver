/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decode_av1_tile_packet_g12.cpp
//! \brief    Defines the interface for av1 decode tile packet for GEN12
//!
#include "codechal_utilities.h"
#include "decode_av1_tile_packet_g12.h"
#include "mhw_vdbox_avp_g12_X.h"

namespace decode
{

Av1DecodeTilePktG12::~Av1DecodeTilePktG12()
{

}

MOS_STATUS Av1DecodeTilePktG12::Execute(MOS_COMMAND_BUFFER& cmdBuffer, int16_t tileIdx)
{
    //AV1 Tile Level Commands
    DECODE_CHK_STATUS(AddAvpTileState(cmdBuffer, tileIdx));
    DECODE_CHK_STATUS(AddBsdObj(cmdBuffer, tileIdx));

    return MOS_STATUS_SUCCESS;
}

}
