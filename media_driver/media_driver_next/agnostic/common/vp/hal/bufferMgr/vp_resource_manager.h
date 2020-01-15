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
//! \file     vp_resource_manager.h
//! \brief    The header file of the base class of vp resource manager
//! \details  all the vp resources will be traced here for usages using intermeida 
//!           surfaces.
//!
#ifndef _VP_RESOURCE_MANAGER_H__
#define _VP_RESOURCE_MANAGER_H__

#include <map>
#include "vp_allocator.h"
#include "vp_pipeline_common.h"
#include "vp_utils.h"

#define VP_MAX_NUM_VEBOX_SURFACES     4                                       //!< Vebox output surface creation, also can be reuse for DI usage:
                                                                              //!< for DI: 2 for ADI plus additional 2 for parallel execution
#define VP_NUM_DN_SURFACES           2                                       //!< Number of DN output surfaces
#define VP_NUM_STMM_SURFACES         2                                       //!< Number of STMM statistics surfaces
#define VP_DNDI_BUFFERS_MAX          4                                       //!< Max DNDI buffers
#define VP_NUM_KERNEL_VEBOX          8                                       //!< Max kernels called at Adv stage

#define VP_VEBOX_PER_BLOCK_STATISTICS_SIZE   16
#define VP_VEBOX_FMD_HISTORY_SIZE            (144 * sizeof(uint32_t))

#define VP_NUM_ACE_STATISTICS_HISTOGRAM      256
#define VP_NUM_STD_STATISTICS                2

#ifndef VEBOX_AUTO_DENOISE_SUPPORTED
#define VEBOX_AUTO_DENOISE_SUPPORTED    1
#endif


#define IS_VP_VEBOX_DN_ONLY(_a) (_a.bDN &&          \
                               !(_a.bDI) &&   \
                               !(_a.bQueryVariance) && \
                               !(_a.bIECP))

namespace vp {
    struct VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION
    {
        // DWORD 0
        union
        {
            // RangeThrStart0
            struct
            {
                uint32_t       RangeThrStart0;
            };

            uint32_t       Value;
        } DW00;

        // DWORD 1
        union
        {
            // RangeThrStart1
            struct
            {
                uint32_t       RangeThrStart1;
            };

            uint32_t       Value;
        } DW01;

        // DWORD 2
        union
        {
            // RangeThrStart2
            struct
            {
                uint32_t       RangeThrStart2;
            };

            uint32_t   Value;
        } DW02;

        // DWORD 3
        union
        {
            // RangeThrStart3
            struct
            {
                uint32_t       RangeThrStart3;
            };

            uint32_t   Value;
        } DW03;

        // DWORD 4
        union
        {
            // RangeThrStart4
            struct
            {
                uint32_t       RangeThrStart4;
            };

            uint32_t   Value;
        } DW04;

        // DWORD 5
        union
        {
            // RangeThrStart5
            struct
            {
                uint32_t       RangeThrStart5;
            };

            uint32_t   Value;
        } DW05;

        // DWORD 6
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW06;

        // DWORD 7
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW07;

        // DWORD 8
        union
        {
            // RangeWgt0
            struct
            {
                uint32_t       RangeWgt0;
            };

            uint32_t   Value;
        } DW08;

        // DWORD 9
        union
        {
            // RangeWgt1
            struct
            {
                uint32_t       RangeWgt1;
            };

            uint32_t   Value;
        } DW09;

        // DWORD 10
        union
        {
            // RangeWgt2
            struct
            {
                uint32_t       RangeWgt2;
            };

            uint32_t   Value;
        } DW10;

        // DWORD 11
        union
        {
            // RangeWgt3
            struct
            {
                uint32_t       RangeWgt3;
            };

            uint32_t   Value;
        } DW11;

        // DWORD 12
        union
        {
            // RangeWgt4
            struct
            {
                uint32_t       RangeWgt4;
            };

            uint32_t   Value;
        } DW12;

        // DWORD 13
        union
        {
            // RangeWgt5
            struct
            {
                uint32_t       RangeWgt5;
            };

            uint32_t   Value;
        } DW13;

        // DWORD 14
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW14;

        // DWORD 15
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW15;

        // DWORD 16 - 41: DistWgt[5][5]
        uint32_t DistWgt[5][5];

        // Padding for 32-byte alignment, VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION_G9 is 7 uint32_ts
        uint32_t dwPad[7];
    };

class VpResourceManager
{
public:
    VpResourceManager(MOS_INTERFACE &osInterface, VpAllocator &allocator);
    virtual ~VpResourceManager();

protected:

    virtual MOS_STATUS AllocateVeboxResource(VP_EXECUTE_CAPS& caps, VP_SURFACE inputSurface, VP_SURFACE outputSurface);

    MOS_STATUS InitVeboxSpatialAttributesConfiguration();

protected:
    MOS_INTERFACE                &m_osInterface;
    VpAllocator                  &m_allocator;

    // Vebox Resource
    VP_SURFACE* veboxDenoiseOutput[VP_NUM_DN_SURFACES] = {};            //!< Vebox Denoise output surface
    VP_SURFACE* veboxOutput[VP_MAX_NUM_VEBOX_SURFACES] = {};            //!< Vebox output surface, can be reuse be DI usages
    VP_SURFACE* veboxSTMMSurface[VP_NUM_STMM_SURFACES] = {};            //!< Vebox STMM input/output surface
    VP_SURFACE *veboxStatisticsSurface                 = nullptr;       //!< Statistics Surface for VEBOX
    VP_SURFACE *veboxRgbHistogram                      = nullptr;       //!< RGB Histogram surface for Vebox
    VP_SURFACE *veboxDNTempSurface                     = nullptr;       //!< Vebox DN Update kernels temp surface
    VP_SURFACE *veboxDNSpatialConfigSurface            = nullptr;       //!< Spatial Attributes Configuration Surface for DN kernel
    VP_SURFACE *vebox3DLookUpTables                    = nullptr;       //!< VEBOX 3D LUT surface for Vebox Gen12
};
}
#endif // _VP_RESOURCE_MANAGER_H__
