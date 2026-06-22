/*===================== begin_copyright_notice ==================================
Copyright (c) 2026 Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/

//!
//! \file     mos_bypass_hw_defs.h
//! \brief    Shared NullHW type definitions for BypassHW (code-gen codecs) and BypassHwLegacy (legacy codecs)
//! \details  Contains DummyVdboxInfo, RepeatCountEntry, and constants shared across
//!           OsContextNext, BypassHW, and BypassHwLegacy without cross-layer dependencies
//!

#ifndef __MOS_BYPASS_HW_DEFS_H__
#define __MOS_BYPASS_HW_DEFS_H__

#include "mos_os.h"
#include <string>

#define DUMMY_VDBOX_NUM_MAX 3

//!
//! \struct   DummyVdboxInfo
//! \brief    Structure to store dummy vdbox engine information
//!
struct DummyVdboxInfo
{
    MOS_GPU_NODE m_node;         //!< GPU node type (VIDEO, VIDEO2, or VE)
    bool         m_sfcEnabled; //!< SFC support flag for this engine
    bool         m_isSlimVd;     //!< SlimVD flag for this engine
};

//!
//! \struct   RepeatCountEntry
//! \brief    Config file entry mapping pipeline characteristics to repeat count
//!
struct RepeatCountEntry
{
    std::string codec;        //!< Codec name (e.g. "av1", "avc", "hevc")
    std::string direction;    //!< "decode" or "encode"
    std::string subsampling;  //!< "420", "422", or "444"
    uint32_t    width;        //!< Frame width in pixels (e.g. 3840)
    uint32_t    height;       //!< Frame height in pixels (e.g. 2160)
    uint32_t    bitDepth;     //!< 8, 10, or 12
    uint32_t    tu;           //!< Target usage (1-7 for encode, 0 for decode)
    uint32_t    repeatCount;  //!< Repeat count value
    uint32_t    scalability;  //!< 1 = scalability-aware slot assignment, 0 = standard
};


//! \name     NullHW / MockAdaptor engine ID constants
//! \brief    CS_ENGINE_ID MMIO address and per-engine instance values written by the
//!           NullHW profiler path. MockAdaptor does not populate CS_ENGINE_ID from
//!           physical context state, so the driver writes these values directly.
// @{
#define NULLHW_CS_ENGINE_ID_MMIO  0x1C008Cu  //!< CS_ENGINE_ID MMIO register address
#define NULLHW_ENGINE_ID_VD0      0x01u       //!< MockAdaptor engine ID: VDBox0 (video)
#define NULLHW_ENGINE_ID_VD1      0x21u       //!< MockAdaptor engine ID: VDBox1 (video2)
#define NULLHW_ENGINE_ID_VE       0x02u       //!< MockAdaptor engine ID: VEBOX
// @}

#endif // __MOS_BYPASS_HW_DEFS_H__
