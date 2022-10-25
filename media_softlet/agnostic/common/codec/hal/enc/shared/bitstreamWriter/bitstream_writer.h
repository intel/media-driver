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
//!
//! \file     bitstream_writer.h
//! \brief    Defines the common interface for hevc packer
//!

#ifndef __BITSTREAM_WRITER_H__
#define __BITSTREAM_WRITER_H__

#include "media_class_trace.h"
#include <map>

typedef unsigned char  mfxU8;
typedef char           mfxI8;
typedef short          mfxI16;
typedef unsigned short mfxU16;
typedef unsigned int   mfxU32;
typedef int            mfxI32;
typedef unsigned long mfxUL32;
typedef long          mfxL32;
typedef float  mfxF32;
typedef double mfxF64;
//typedef __UINT64            mfxU64;
//typedef __INT64             mfxI64;
typedef void * mfxHDL;
typedef mfxHDL mfxMemId;
typedef void * mfxThreadTask;
typedef char   mfxChar;

constexpr mfxU8  MAX_DPB_SIZE               = 15;
constexpr mfxU8  HW_SURF_ALIGN_W            = 16;
constexpr mfxU8  HW_SURF_ALIGN_H            = 16;
constexpr mfxU16 MAX_SLICES                 = 600;  // conforms to level 6 limits
constexpr mfxU8  DEFAULT_LTR_INTERVAL       = 16;
constexpr mfxU8  DEFAULT_PPYR_INTERVAL      = 3;
constexpr mfxU16 GOP_INFINITE               = 0xFFFF;
constexpr mfxU8  MAX_NUM_TILE_COLUMNS       = 20;
constexpr mfxU8  MAX_NUM_TILE_ROWS          = 22;
constexpr mfxU8  MAX_NUM_LONG_TERM_PICS     = 8;
constexpr mfxU16 MIN_TILE_WIDTH_IN_SAMPLES  = 256;
constexpr mfxU16 MIN_TILE_HEIGHT_IN_SAMPLES = 64;

const mfxU8 tab_cabacRangeTabLps[128][4] =
{
    { 128, 176, 208, 240 }, { 128, 167, 197, 227 }, { 128, 158, 187, 216 }, { 123, 150, 178, 205 },
    { 116, 142, 169, 195 }, { 111, 135, 160, 185 }, { 105, 128, 152, 175 }, { 100, 122, 144, 166 },
    {  95, 116, 137, 158 }, {  90, 110, 130, 150 }, {  85, 104, 123, 142 }, {  81,  99, 117, 135 },
    {  77,  94, 111, 128 }, {  73,  89, 105, 122 }, {  69,  85, 100, 116 }, {  66,  80,  95, 110 },
    {  62,  76,  90, 104 }, {  59,  72,  86,  99 }, {  56,  69,  81,  94 }, {  53,  65,  77,  89 },
    {  51,  62,  73,  85 }, {  48,  59,  69,  80 }, {  46,  56,  66,  76 }, {  43,  53,  63,  72 },
    {  41,  50,  59,  69 }, {  39,  48,  56,  65 }, {  37,  45,  54,  62 }, {  35,  43,  51,  59 },
    {  33,  41,  48,  56 }, {  32,  39,  46,  53 }, {  30,  37,  43,  50 }, {  29,  35,  41,  48 },
    {  27,  33,  39,  45 }, {  26,  31,  37,  43 }, {  24,  30,  35,  41 }, {  23,  28,  33,  39 },
    {  22,  27,  32,  37 }, {  21,  26,  30,  35 }, {  20,  24,  29,  33 }, {  19,  23,  27,  31 },
    {  18,  22,  26,  30 }, {  17,  21,  25,  28 }, {  16,  20,  23,  27 }, {  15,  19,  22,  25 },
    {  14,  18,  21,  24 }, {  14,  17,  20,  23 }, {  13,  16,  19,  22 }, {  12,  15,  18,  21 },
    {  12,  14,  17,  20 }, {  11,  14,  16,  19 }, {  11,  13,  15,  18 }, {  10,  12,  15,  17 },
    {  10,  12,  14,  16 }, {   9,  11,  13,  15 }, {   9,  11,  12,  14 }, {   8,  10,  12,  14 },
    {   8,   9,  11,  13 }, {   7,   9,  11,  12 }, {   7,   9,  10,  12 }, {   7,   8,  10,  11 },
    {   6,   8,   9,  11 }, {   6,   7,   9,  10 }, {   6,   7,   8,   9 }, {   2,   2,   2,   2 },
    //The same for valMPS=1
    { 128, 176, 208, 240 }, { 128, 167, 197, 227 }, { 128, 158, 187, 216 }, { 123, 150, 178, 205 },
    { 116, 142, 169, 195 }, { 111, 135, 160, 185 }, { 105, 128, 152, 175 }, { 100, 122, 144, 166 },
    {  95, 116, 137, 158 }, {  90, 110, 130, 150 }, {  85, 104, 123, 142 }, {  81,  99, 117, 135 },
    {  77,  94, 111, 128 }, {  73,  89, 105, 122 }, {  69,  85, 100, 116 }, {  66,  80,  95, 110 },
    {  62,  76,  90, 104 }, {  59,  72,  86,  99 }, {  56,  69,  81,  94 }, {  53,  65,  77,  89 },
    {  51,  62,  73,  85 }, {  48,  59,  69,  80 }, {  46,  56,  66,  76 }, {  43,  53,  63,  72 },
    {  41,  50,  59,  69 }, {  39,  48,  56,  65 }, {  37,  45,  54,  62 }, {  35,  43,  51,  59 },
    {  33,  41,  48,  56 }, {  32,  39,  46,  53 }, {  30,  37,  43,  50 }, {  29,  35,  41,  48 },
    {  27,  33,  39,  45 }, {  26,  31,  37,  43 }, {  24,  30,  35,  41 }, {  23,  28,  33,  39 },
    {  22,  27,  32,  37 }, {  21,  26,  30,  35 }, {  20,  24,  29,  33 }, {  19,  23,  27,  31 },
    {  18,  22,  26,  30 }, {  17,  21,  25,  28 }, {  16,  20,  23,  27 }, {  15,  19,  22,  25 },
    {  14,  18,  21,  24 }, {  14,  17,  20,  23 }, {  13,  16,  19,  22 }, {  12,  15,  18,  21 },
    {  12,  14,  17,  20 }, {  11,  14,  16,  19 }, {  11,  13,  15,  18 }, {  10,  12,  15,  17 },
    {  10,  12,  14,  16 }, {   9,  11,  13,  15 }, {   9,  11,  12,  14 }, {   8,  10,  12,  14 },
    {   8,   9,  11,  13 }, {   7,   9,  11,  12 }, {   7,   9,  10,  12 }, {   7,   8,  10,  11 },
    {   6,   8,   9,  11 }, {   6,   7,   9,  10 }, {   6,   7,   8,   9 }, {   2,   2,   2,   2 }
};

/* CABAC trans tables: state (MPS and LPS ) + valMPS in 6th bit */
const mfxU8 tab_cabacTransTbl[2][128] =
{
    {
          1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,
         17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,
         33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
         49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  62,  63,
          0,  64,  65,  66,  66,  68,  68,  69,  70,  71,  72,  73,  73,  75,  75,  76,
         77,  77,  79,  79,  80,  80,  82,  82,  83,  83,  85,  85,  86,  86,  87,  88,
         88,  89,  90,  90,  91,  91,  92,  93,  93,  94,  94,  94,  95,  96,  96,  97,
         97,  97,  98,  98,  99,  99,  99, 100, 100, 100, 101, 101, 101, 102, 102, 127
    },
    {
           0,   0,   1,   2,   2,   4,   4,   5,   6,   7,   8,   9,   9,  11,  11,  12,
          13,  13,  15,  15,  16,  16,  18,  18,  19,  19,  21,  21,  22,  22,  23,  24,
          24,  25,  26,  26,  27,  27,  28,  29,  29,  30,  30,  30,  31,  32,  32,  33,
          33,  33,  34,  34,  35,  35,  35,  36,  36,  36,  37,  37,  37,  38,  38,  63,
          65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,  80,
          81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,  96,
          97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112,
         113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 126, 127
    }
};

class IBsWriter
{
public:
    virtual ~IBsWriter() {}
    virtual void PutBits(mfxU32 n, mfxU32 b) = 0;
    virtual void PutBit(mfxU32 b)            = 0;
    virtual void PutUE(mfxU32 b)             = 0;
    virtual void PutSE(mfxI32 b)             = 0;

MEDIA_CLASS_DEFINE_END(IBsWriter)
};

class BitstreamWriter
    : public IBsWriter
{
public:
    BitstreamWriter(mfxU8 *bs, mfxU32 size, mfxU8 bitOffset = 0);
    ~BitstreamWriter();

    virtual void PutBits(mfxU32 n, mfxU32 b) override;
    void         PutBitsBuffer(mfxU32 n, void *b, mfxU32 offset = 0);
    virtual void PutBit(mfxU32 b) override;
    void         PutGolomb(mfxU32 b);
    void         PutTrailingBits(bool bCheckAligned = false);

    virtual void PutUE(mfxU32 b) override { PutGolomb(b); }
    virtual void PutSE(mfxI32 b) override { (b > 0) ? PutGolomb((b << 1) - 1) : PutGolomb((-b) << 1); }

    mfxU32 GetOffset()
    {
        return mfxU32(m_bs - m_bsStart) * 8 + m_bitOffset - m_bitStart;
    }
    mfxU8 *GetStart() { return m_bsStart; }
    mfxU8 *GetEnd() { return m_bsEnd; }

    void Reset(mfxU8 *bs = 0, mfxU32 size = 0, mfxU8 bitOffset = 0);
    void cabacInit();
    void EncodeBin(mfxU8 &ctx, mfxU8 binVal);
    void EncodeBinEP(mfxU8 binVal);
    void SliceFinish();
    void PutBitC(mfxU32 B);

    void AddInfo(mfxU32 key, mfxU32 value)
    {
        if (m_pInfo)
            m_pInfo[0][key] = value;
    }
    void SetInfo(std::map<mfxU32, mfxU32> *pInfo)
    {
        m_pInfo = pInfo;
    }

private:
    void   RenormE();
    mfxU8 *m_bsStart;
    mfxU8 *m_bsEnd;
    mfxU8 *m_bs;
    mfxU8  m_bitStart;
    mfxU8  m_bitOffset;

    mfxU32                    m_codILow;
    mfxU32                    m_codIRange;
    mfxU32                    m_bitsOutstanding;
    mfxU32                    m_BinCountsInNALunits;
    bool                      m_firstBitFlag;
    std::map<mfxU32, mfxU32> *m_pInfo = nullptr;

MEDIA_CLASS_DEFINE_END(BitstreamWriter)
};

#endif
