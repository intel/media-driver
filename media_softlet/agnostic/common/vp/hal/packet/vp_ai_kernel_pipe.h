/*
* Copyright (c) 2025, Intel Corporation
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
#ifndef __VP_AI_KERNEL_PIPE_H__
#define __VP_AI_KERNEL_PIPE_H__

#include <map>
#include <vector>
#include "mos_defs.h"
#include "mos_resource_defs.h"
#include "vp_common_defs.h"
#include "vp_render_common.h"

namespace vp
{
class SwFilterPipe;

struct AI_SURFACE_PARAMS
{
    std::string     surfaceName  = "";
    MOS_FORMAT      format       = Format_Any;
    MOS_GFXRES_TYPE resourceType = MOS_GFXRES_2D;
    MOS_TILE_TYPE   tileType     = MOS_TILE_Y;
    uint32_t        width        = 0;
    uint32_t        height       = 0;
};
using AI_SURFACE_ALLOCATION_MAP = std::map<SurfaceType, AI_SURFACE_PARAMS>;

struct AI_SINGLE_LAYER_SETTING
{
    void        Init();
    std::string kernelName  = "";
    uint32_t    groupWidth  = 0;
    uint32_t    groupHeight = 0;
    uint32_t    localWidth  = 0;
    uint32_t    localHeight = 0;
    MOS_STATUS (*pfnSetStatefulSurface)            (uint32_t swPipeIndex, uint32_t krnArgEnum, const SwFilterPipe &executingPipe, SURFACE_PARAMS &surfaceParam, bool &bInit) = nullptr;
    MOS_STATUS (*pfnSetStatelessSurface)           (uint32_t swPipeIndex, uint32_t krnArgEnum, const SwFilterPipe &executingPipe, SURFACE_PARAMS &surfaceParam, bool &bInit) = nullptr;
    MOS_STATUS (*pfnSetKernelArg)                  (uint32_t swPipeIndex, uint32_t krnArgEnum, const SwFilterPipe &executingPipe, void *&argData, bool &bInit)               = nullptr;
    MOS_STATUS (*pfnGetIntermediateSurfaceSetting) (uint32_t swPipeIndex, const SwFilterPipe &executingPipe, AI_SURFACE_ALLOCATION_MAP &surfaceMap)                          = nullptr;
};

using AI_KERNEL_SETTING_PIPE      = std::vector<AI_SINGLE_LAYER_SETTING>;
using AI_KERNEL_SPLIT_GROUP_INDEX = std::vector<uint32_t>;

}  // namespace vp

#endif