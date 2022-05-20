/*
* Copyright (c) 2020, Intel Corporation
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

#include "bitstream_writer.h"
#include <assert.h>

BitstreamWriter::BitstreamWriter(mfxU8 *bs, mfxU32 size, mfxU8 bitOffset)
    : m_bsStart(bs), m_bsEnd(bs + size), m_bs(bs), m_bitStart(bitOffset & 7), m_bitOffset(bitOffset & 7), m_codILow(0)  // cabac variables
      ,
      m_codIRange(510),
      m_bitsOutstanding(0),
      m_BinCountsInNALunits(0),
      m_firstBitFlag(true)
{
    assert(bitOffset < 8);
    *m_bs &= 0xFF << (8 - m_bitOffset);
}

BitstreamWriter::~BitstreamWriter()
{
}

void BitstreamWriter::Reset(mfxU8 *bs, mfxU32 size, mfxU8 bitOffset)
{
    if (bs)
    {
        m_bsStart           = bs;
        m_bsEnd             = bs + size;
        unsigned char *curr = m_bsStart;
        /*
        while (curr<m_bsEnd)//Not Necessary
        {
            *curr = 0;
            curr++;
        }
        */
        m_bs        = bs;
        m_bitOffset = (bitOffset & 7);
        m_bitStart  = (bitOffset & 7);
    }
    else
    {
        m_bs        = m_bsStart;
        m_bitOffset = m_bitStart;
    }
}

void BitstreamWriter::PutBitsBuffer(mfxU32 n, void *bb, mfxU32 o)
{}

void BitstreamWriter::PutBits(mfxU32 n, mfxU32 b)
{
    assert(n <= sizeof(b) * 8);
    while (n > 24)
    {
        n -= 16;
        PutBits(16, (b >> n));
    }

    b <<= (32 - n);

    if (!m_bitOffset)
    {
        m_bs[0] = (mfxU8)(b >> 24);
        m_bs[1] = (mfxU8)(b >> 16);
    }
    else
    {
        b >>= m_bitOffset;
        n += m_bitOffset;

        m_bs[0] |= (mfxU8)(b >> 24);
        m_bs[1] = (mfxU8)(b >> 16);
    }

    if (n > 16)
    {
        m_bs[2] = (mfxU8)(b >> 8);
        m_bs[3] = (mfxU8)b;
    }

    m_bs += (n >> 3);
    m_bitOffset = (n & 7);
}

void BitstreamWriter::PutBit(mfxU32 b)
{
    switch (m_bitOffset)
    {
    case 0:
        m_bs[0]     = (mfxU8)(b << 7);
        m_bitOffset = 1;
        break;
    case 7:
        m_bs[0] |= (mfxU8)(b & 1);
        m_bs++;
        m_bitOffset = 0;
        break;
    default:
        if (b & 1)
            m_bs[0] |= (mfxU8)(1 << (7 - m_bitOffset));
        m_bitOffset++;
        break;
    }
}

void BitstreamWriter::PutGolomb(mfxU32 b)
{
    if (!b)
    {
        PutBit(1);
    }
    else
    {
        mfxU32 n = 1;

        b++;

        while (b >> n)
            n++;

        PutBits(n - 1, 0);
        PutBits(n, b);
    }
}

void BitstreamWriter::PutTrailingBits(bool bCheckAligened)
{
    if ((!bCheckAligened) || m_bitOffset)
        PutBit(1);

    if (m_bitOffset)
    {
        *(++m_bs)   = 0;
        m_bitOffset = 0;
    }
}

void BitstreamWriter::PutBitC(mfxU32 B)
{
    if (m_firstBitFlag)
        m_firstBitFlag = false;
    else
        PutBit(B);

    while (m_bitsOutstanding > 0)
    {
        PutBit(1 - B);
        m_bitsOutstanding--;
    }
}
void BitstreamWriter::RenormE()
{
    while (m_codIRange < 256)
    {
        if (m_codILow < 256)
        {
            PutBitC(0);
        }
        else if (m_codILow >= 512)
        {
            m_codILow -= 512;
            PutBitC(1);
        }
        else
        {
            m_codILow -= 256;
            m_bitsOutstanding++;
        }
        m_codIRange <<= 1;
        m_codILow <<= 1;
    }
}

void BitstreamWriter::EncodeBin(mfxU8 &ctx, mfxU8 binVal)
{
    mfxU8  pStateIdx     = (ctx >> 1);
    mfxU8  valMPS        = (ctx & 1);
    mfxU32 qCodIRangeIdx = (m_codIRange >> 6) & 3;
    mfxU32 codIRangeLPS  = tab_cabacRangeTabLps[pStateIdx][qCodIRangeIdx];

    m_codIRange -= codIRangeLPS;

    if (binVal != valMPS)
    {
        m_codILow += m_codIRange;
        m_codIRange = codIRangeLPS;

        if (pStateIdx == 0)
            valMPS = 1 - valMPS;

        pStateIdx = tab_cabacTransTbl[1][pStateIdx];  //transIdxLPS[pStateIdx];
    }
    else
    {
        pStateIdx = tab_cabacTransTbl[0][pStateIdx];  //transIdxMPS[pStateIdx];
    }

    ctx = (pStateIdx << 1) | valMPS;

    RenormE();
    m_BinCountsInNALunits++;
}

void BitstreamWriter::EncodeBinEP(mfxU8 binVal)
{
    m_codILow += m_codILow + m_codIRange * (binVal == 1);
    RenormE();
    m_BinCountsInNALunits++;
}

void BitstreamWriter::SliceFinish()
{
    m_codIRange -= 2;
    m_codILow += m_codIRange;
    m_codIRange = 2;

    RenormE();
    PutBitC((m_codILow >> 9) & 1);
    PutBit(m_codILow >> 8);
    PutTrailingBits();

    m_BinCountsInNALunits++;
}

void BitstreamWriter::cabacInit()
{
    m_codILow             = 0;
    m_codIRange           = 510;
    m_bitsOutstanding     = 0;
    m_BinCountsInNALunits = 0;
    m_firstBitFlag        = true;
}