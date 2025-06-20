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
#include "media_class_trace.h"

namespace vp
{
class SwFilterPipe;

struct AI_SURFACE_PARAMS
{
    std::string     surfaceName     = "";
    MOS_FORMAT      format          = Format_Any;
    MOS_GFXRES_TYPE resourceType    = MOS_GFXRES_2D;
    MOS_TILE_TYPE   tileType        = MOS_TILE_Y;
    uint32_t        width           = 0;
    uint32_t        height          = 0;
    uint8_t        *fillContent     = nullptr;
    uint32_t        fillContentSize = 0;
};
using AI_SURFACE_ALLOCATION_MAP = std::map<SurfaceType, AI_SURFACE_PARAMS>;

struct AI_NPU_BUFFER_PARAMS
{
    std::string surfaceName = "";
    uint32_t    size        = 0;
};
using AI_NPU_BUFFER_ALLOCATION_MAP = std::map<SurfaceType,AI_NPU_BUFFER_PARAMS>;

enum class FEATURE_AI_ENGINE
{
    GPU = 0,
    NPU = 1
};

struct AI_SINGLE_LAYER_BASE_SETTING
{
    FEATURE_AI_ENGINE engine          = FEATURE_AI_ENGINE::GPU;
    uint32_t          stageGroupIndex = 0;
    virtual ~AI_SINGLE_LAYER_BASE_SETTING() {};
    virtual std::unique_ptr<AI_SINGLE_LAYER_BASE_SETTING> Clone() const = 0;
};

struct AI_SINGLE_GPU_LAYER_SETTING : public AI_SINGLE_LAYER_BASE_SETTING
{
    AI_SINGLE_GPU_LAYER_SETTING() { engine = FEATURE_AI_ENGINE::GPU; };
    void        Init();
    virtual     ~AI_SINGLE_GPU_LAYER_SETTING() {};
    std::string kernelName  = "";
    uint32_t    groupWidth  = 0;
    uint32_t    groupHeight = 0;
    uint32_t    groupDepth  = 0;
    uint32_t    localWidth  = 0;
    uint32_t    localHeight = 0;
    MOS_STATUS (*pfnSetStatefulSurface)            (uint32_t krnArgEnum, SwFilterPipe &executingPipe, SURFACE_PARAMS &surfaceParam, bool &bInit) = nullptr;
    MOS_STATUS (*pfnSetStatelessSurface)           (uint32_t krnArgEnum, SwFilterPipe &executingPipe, SURFACE_PARAMS &surfaceParam, bool &bInit) = nullptr;
    MOS_STATUS (*pfnSetKernelArg)                  (uint32_t krnArgEnum, SwFilterPipe &executingPipe, void *&argData, bool &bInit)               = nullptr;
    MOS_STATUS (*pfnGetIntermediateSurfaceSetting) (SwFilterPipe &executingPipe, AI_SURFACE_ALLOCATION_MAP &surfaceMap)                          = nullptr;

    std::unique_ptr<AI_SINGLE_LAYER_BASE_SETTING> Clone() const override
    {
        return std::make_unique<AI_SINGLE_GPU_LAYER_SETTING>(*this);
    }
};

struct AI_SINGLE_NPU_GRAPH_SETTING : public AI_SINGLE_LAYER_BASE_SETTING
{
    AI_SINGLE_NPU_GRAPH_SETTING() { engine = FEATURE_AI_ENGINE::NPU; };
    void      Init();
    virtual   ~AI_SINGLE_NPU_GRAPH_SETTING() {};
    VP_GRAPH_ID              id        = VP_GRAPH_ID_INVALID;
    std::vector<SurfaceType> graphArgs = {};
    MOS_STATUS (*pfnGetIntermediateSurfaceSetting) (SwFilterPipe &executingPipe, AI_NPU_BUFFER_ALLOCATION_MAP &surfaceMap) = nullptr;

    std::unique_ptr<AI_SINGLE_LAYER_BASE_SETTING> Clone() const override
    {
        return std::make_unique<AI_SINGLE_NPU_GRAPH_SETTING>(*this);
    }
};

using AI_SPLIT_GROUP_INDEX = std::vector<uint32_t>;
using AI_SETTING_PIPE = std::vector<std::unique_ptr<AI_SINGLE_LAYER_BASE_SETTING>>;


class AiGpuSettingInterface
{
public:
    AiGpuSettingInterface() {};
    virtual ~AiGpuSettingInterface() {};
    MOS_STATUS Init();
    MOS_STATUS Register(AI_SETTING_PIPE &settingPipe);

private:
    virtual MOS_STATUS InitBaseSetting()                       = 0;
    virtual MOS_STATUS InitFuncGetIntermediateSurfaceSetting() = 0;
    virtual MOS_STATUS InitFuncSetStatefulSurface()            = 0;
    virtual MOS_STATUS InitFuncSetStatelessSurface()           = 0;
    virtual MOS_STATUS InitFuncSetKernelArg()                  = 0;

protected:
    std::unique_ptr<AI_SINGLE_GPU_LAYER_SETTING> setting = std::make_unique<AI_SINGLE_GPU_LAYER_SETTING>();

MEDIA_CLASS_DEFINE_END(vp__AiGpuSettingInterface)
};

class AiNpuSettingInterface
{
public:
    AiNpuSettingInterface() {};
    virtual ~AiNpuSettingInterface() {};
    MOS_STATUS Init();
    MOS_STATUS Register(AI_SETTING_PIPE &settingPipe);
   
private:
    virtual MOS_STATUS InitBaseSetting()                       = 0;
    virtual MOS_STATUS InitFuncGetIntermediateSurfaceSetting() = 0;
    virtual MOS_STATUS InitGraphArguments()                    = 0;

protected:
    std::unique_ptr<AI_SINGLE_NPU_GRAPH_SETTING> setting = std::make_unique<AI_SINGLE_NPU_GRAPH_SETTING>();

MEDIA_CLASS_DEFINE_END(vp__AiNpuSettingInterface)
};

}  // namespace vp

#endif