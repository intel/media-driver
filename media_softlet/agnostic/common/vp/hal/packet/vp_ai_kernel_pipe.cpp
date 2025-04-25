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
//!
//! \file     vp_ai_kernel_pipe.cpp
//! \brief    Defines the common interface for ai
//!           this file is for the base interface which is shared by all ai filter in driver.
//!

#include "vp_ai_kernel_pipe.h"
#include "vp_utils.h"

using namespace vp;

void AI_SINGLE_GPU_LAYER_SETTING::Init()
{
    engine                           = FEATURE_AI_ENGINE::GPU;
    kernelName                       = "";
    groupWidth                       = 0;
    groupHeight                      = 0;
    localWidth                       = 0;
    localHeight                      = 0;
    pfnSetStatefulSurface            = nullptr;
    pfnSetKernelArg                  = nullptr;
    pfnGetIntermediateSurfaceSetting = nullptr;
}

void AI_SINGLE_NPU_GRAPH_SETTING::Init()
{
    engine                           = FEATURE_AI_ENGINE::NPU;
    id                               = VP_GRAPH_ID_INVALID;
    pfnGetIntermediateSurfaceSetting = nullptr;
}

MOS_STATUS AiGpuSettingInterface::Init()
{
    VP_PUBLIC_CHK_NULL_RETURN(setting);
    VP_PUBLIC_CHK_STATUS_RETURN(InitBaseSetting());
    VP_PUBLIC_CHK_STATUS_RETURN(InitFuncGetIntermediateSurfaceSetting());
    VP_PUBLIC_CHK_STATUS_RETURN(InitFuncSetStatefulSurface());
    VP_PUBLIC_CHK_STATUS_RETURN(InitFuncSetStatelessSurface());
    VP_PUBLIC_CHK_STATUS_RETURN(InitFuncSetKernelArg());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AiGpuSettingInterface::Register(AI_SETTING_PIPE &settingPipe)
{
    VP_PUBLIC_CHK_NULL_RETURN(setting);
    settingPipe.push_back(setting->Clone());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AiNpuSettingInterface::Init()
{
    VP_PUBLIC_CHK_NULL_RETURN(setting);
    VP_PUBLIC_CHK_STATUS_RETURN(InitBaseSetting());
    VP_PUBLIC_CHK_STATUS_RETURN(InitFuncGetIntermediateSurfaceSetting());
    VP_PUBLIC_CHK_STATUS_RETURN(InitGraphArguments());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AiNpuSettingInterface::Register(AI_SETTING_PIPE& settingPipe)
{
    VP_PUBLIC_CHK_NULL_RETURN(setting);
    settingPipe.push_back(setting->Clone());
    return MOS_STATUS_SUCCESS;
}