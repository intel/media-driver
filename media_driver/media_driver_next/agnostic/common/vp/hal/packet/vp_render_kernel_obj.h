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
#ifndef __VP_RENDER_KERNEL_OBJ_H__
#define __VP_RENDER_KERNEL_OBJ_H__

#include "vp_pipeline_common.h"
#include "sw_filter.h"
#include "vp_kernelset.h"
#include <vector>
#include <map>

class RenderCmdPacket;
namespace vp {

//!
//! \brief Secure Block Copy kernel inline data size
//!
#define SECURE_BLOCK_COPY_KERNEL_INLINE_SIZE    (1 * sizeof(uint32_t))
//!
//! \brief Secure Block Copy kernel width
//!
#define SECURE_BLOCK_COPY_KERNEL_SURF_WIDTH     64

//!
//! \brief Secure Block Copy kernel block height
//!
#define SECURE_BLOCK_COPY_KERNEL_BLOCK_HEIGHT   24

struct MEDIA_OBJECT_KA2_INLINE_DATA
{
    // DWORD 0 - GRF R7.0
    union
    {
        // All
        struct
        {
            uint32_t       DestinationBlockHorizontalOrigin : 16;
            uint32_t       DestinationBlockVerticalOrigin : 16;
        };

        // Secure Block Copy
        struct
        {
            uint32_t       BlockHeight : 16;
            uint32_t       BufferOffset : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartRowOffset;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1 - GRF R7.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer0 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer0 : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalRows;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2 - GRF R7.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer1 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer1 : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       StartColumnOffset;
        };

        uint32_t       Value;
    } DW02;

    // DWORD 3 - GRF R7.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer2 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer2 : 16;
        };

        // FMD Summation
        struct
        {
            uint32_t       TotalColumns;
        };

        uint32_t       Value;
    } DW03;

    // DWORD 4 - GRF R7.4
    union
    {
        // Sampler Load
        struct
        {
            float       VideoXScalingStep;
        };

        uint32_t       Value;
    } DW04;

    // DWORD 5 - GRF R7.5
    union
    {
        // NLAS
        struct
        {
            float       VideoStepDelta;
        };

        uint32_t       Value;
    } DW05;

    // DWORD 6 - GRF R7.6
    union
    {
        // AVScaling
        struct
        {
            uint32_t       VerticalBlockNumber : 17;
            uint32_t       AreaOfInterest : 1;
            uint32_t : 14;
        };

        uint32_t       Value;
    } DW06;

    // DWORD 7 - GRF R7.7
    union
    {
        // AVScaling
        struct
        {
            uint32_t       GroupIDNumber;
        };

        uint32_t       Value;
    } DW07;

    // DWORD 8 - GRF R8.0
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer3 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer3 : 16;
        };

        uint32_t       Value;
    } DW08;

    // DWORD 9 - GRF R8.1
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer4 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer4 : 16;
        };

        uint32_t       Value;
    } DW09;

    // DWORD 10 - GRF R8.2
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer5 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer5 : 16;
        };

        uint32_t       Value;
    } DW10;

    // DWORD 11 - GRF R8.3
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer6 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer6 : 16;
        };

        uint32_t       Value;
    } DW11;

    // DWORD 12 - GRF R8.4
    union
    {
        // Composite
        struct
        {
            uint32_t       HorizontalBlockCompositeMaskLayer7 : 16;
            uint32_t       VerticalBlockCompositeMaskLayer7 : 16;
        };

        uint32_t       Value;
    } DW12;

    // DWORD 13 - GRF R8.5
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW13;

    // DWORD 14 - GRF R8.6
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW14;

    // DWORD 15 - GRF R8.7
    union
    {
        struct
        {
            uint32_t       Reserved;
        };

        uint32_t       Value;
    } DW15;
};

extern const MEDIA_OBJECT_KA2_INLINE_DATA g_cInit_MEDIA_VP_OBJECT_KA2_INLINE_DATA;

typedef struct _KERNEL_SURFACE2D_STATE_PARAM
{
    MOS_FORMAT format; //[IN] MOS_FORMAT
    uint32_t   width;
    uint32_t   height;
    uint32_t   depth;
    uint32_t   pitch;
    uint16_t   memory_object_control;
    uint32_t   surface_x_offset;   // Horizontal offset to the origin of the surface, in columns of pixels.
    uint32_t   surface_y_offset;   // Vertical offset to the origin of the surface, in rows of pixels.
    uint32_t   surface_offset;     // Offset to the origin of the surface, in bytes.
    MOS_TILE_TYPE tileType;
    uint32_t   reserved[3]; // for future usage
} KERNEL_SURFACE2D_STATE_PARAM;

class VpRenderKernelObj
{
public:
    VpRenderKernelObj(PVP_MHWINTERFACE hwInterface);
    virtual ~VpRenderKernelObj() {};

    virtual void SetKernelParams(RENDER_KERNEL_PARAMS& kernelParams)
    {
        m_kernelParams = &kernelParams;
    }

    virtual MOS_STATUS SetupSurfaceState() = 0;

    virtual MOS_STATUS GetCurbeState(void * curbe, uint32_t curbeLength) = 0;

    virtual MOS_STATUS GetMediaWalkerSettings() = 0;

    virtual MOS_STATUS GetInlineState(void* inlineData, uint32_t inlineLength) = 0;

    virtual MOS_STATUS GetKernelID(int32_t& kuid) = 0;

    MOS_STATUS SetProcessSurface(std::map<SurfaceType, VP_SURFACE*>& surface)
    {
        m_surfaceGroup = &surface;
        return MOS_STATUS_SUCCESS;
    }

protected:
    RENDER_KERNEL_PARAMS                                *m_kernelParams = nullptr;   // kernel input for processing params include kernel ID and process surface group
    std::map<SurfaceType, VP_SURFACE*>                  *m_surfaceGroup = nullptr;   // input surface process surface groups
    PVP_MHWINTERFACE                                     m_hwInterface = nullptr;
    std::vector<SurfaceType>                             m_surfaces;                 // vector for processed surfaces, the order should match with Curbe surface order
    std::map<SurfaceType, KERNEL_SURFACE2D_STATE_PARAM>  m_surfacePool;              // surfaces processed pool where the surface state will generated here
    std::map<SurfaceType, uint32_t*>                     m_surfaceIndex;             // store the binding index for processed surface
};
}
#endif // __VP_RENDER_KERNEL_OBJ_H__
