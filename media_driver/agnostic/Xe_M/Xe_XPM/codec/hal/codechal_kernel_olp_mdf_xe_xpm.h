/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     codechal_kernel_olp_mdf_xe_xpm.h
//! \brief    Implements the MDF OLP kernel for Xe_XPM VC1.
//! \details  Implements the MDF OLP kernel for Xe_XPM VC1.
//!

#ifndef __CODECHAL_KERNEL_OLP_MDF_XE_XPM_H__
#define __CODECHAL_KERNEL_OLP_MDF_XE_XPM_H__

#include "codechal_decoder.h"
#include "codechal_kernel_base.h"

union OLPFlags
{
    struct
    {
        unsigned short Profile              :MOS_BITFIELD_BIT(0);
        unsigned short RangeExpansion       :MOS_BITFIELD_BIT(1);
        unsigned short HorizontalUpscaling  :MOS_BITFIELD_BIT(2);
        unsigned short VerticalUpscaling    :MOS_BITFIELD_BIT(3);
        unsigned short Unused1              :MOS_BITFIELD_BIT(4);
        unsigned short Interlace            :MOS_BITFIELD_BIT(5);
        unsigned short Unused2              :MOS_BITFIELD_RANGE(6, 7);
        unsigned short RangeMapUV           :MOS_BITFIELD_RANGE(8, 10);
        unsigned short RangeMapUVFlag       :MOS_BITFIELD_BIT(11);
        unsigned short RangeMapY            :MOS_BITFIELD_RANGE(12, 14);
        unsigned short RangeMapYFlag        :MOS_BITFIELD_BIT(15);
    };
    unsigned short value;
};

struct OLPCurbe
{
    unsigned int   rsvd;
    unsigned short width;
    unsigned short height;
    OLPFlags       olpflags;
    unsigned short cmp_flag;
    unsigned short rsvd1;
};


//!
//! \class CodechalKernelOlpMdf
//! \brief This class defines the member fields, functions etc used by MDF OLP kernel.
//!
class CodechalKernelOlpMdf
{
public:
    CodechalKernelOlpMdf() {};
    virtual ~CodechalKernelOlpMdf() {}
    virtual MOS_STATUS Init(PMOS_INTERFACE osInterface);
    MOS_STATUS UnInit();

    MOS_STATUS Execute(PMOS_SURFACE src, uint16_t *srcMemory_object_control, PMOS_SURFACE dst, uint16_t *dstMemory_object_control, uint16_t flags);

protected:
    MOS_STATUS SetupSurfaces(PMOS_SURFACE src, uint16_t *srcMemory_object_control, PMOS_SURFACE dst, uint16_t *dstMemory_object_control);
    MOS_STATUS SetKernelArgs(uint16_t flags, bool uv);

protected:
    PMOS_INTERFACE      m_osInterface = nullptr;
    CmDevice            *m_cmDevice = nullptr;
    CmQueue             *m_cmQueue = nullptr;
    CmTask              *m_cmTask = nullptr;
    CmProgram           *m_cmProgram = nullptr;
    CmKernel            *m_cmKernels[2] = { nullptr, };
    CmThreadGroupSpace  *m_threadGroupSpaces[2] = { nullptr, };

    CmSurface2D         *m_cmSurfSrc = nullptr;
    CmSurface2D         *m_cmSurfDst = nullptr;
    SurfaceIndex        *m_srcYIndex = nullptr;
    SurfaceIndex        *m_srcUVIndex = nullptr;
    SurfaceIndex        *m_dstYIndex = nullptr;
    SurfaceIndex        *m_dstUVIndex = nullptr;

    bool                 m_SingleTaskPhase = false;
};

#endif // __CODECHAL_KERNEL_OLP_MDF_XE_XPM_H__