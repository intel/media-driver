/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     decode_vdbox_mfx_common.h
//! \brief    vdbox packet which used in by mediapipline.
//! \details  vdbox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

//!

#ifndef __DECODE_VDBOX_MFX_COMMON_H__
#define __DECODE_VDBOX_MFX_COMMON_H__

namespace decode {

class DecodeVdboxMfxData
{
public:
    bool m_streamOutEnabled = false;
    bool m_deblockingEnabled = false;
    bool m_shortFormatInUse = false;
    uint32_t      m_mode = CODECHAL_UNSUPPORTED_MODE;//! \brief Decode mode
    MOS_SURFACE   m_destSurface = {};    //!< Dest data surface
    PMOS_RESOURCE m_streamOutBuffer = nullptr;//!< Stream Out Buffer

MEDIA_CLASS_DEFINE_END(decode__DecodeVdboxMfxData)
};
}
#endif // !__DECODE_VDBOX_MFX_COMMON_H__