/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     hw_filter.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_obj_factories.h"
#include "vp_pipeline.h"

using namespace vp;

/****************************************************************************************************/
/*                                      HwFilter                                                    */
/****************************************************************************************************/

HwFilter::HwFilter(VpInterface &vpInterface,EngineType type) : m_vpInterface(vpInterface)
{
    m_Params.Type = type;
}

HwFilter::~HwFilter()
{
    Clean();
}

MOS_STATUS HwFilter::Initialize(HW_FILTER_PARAMS &param)
{
    VP_FUNC_CALL();

    bool bRet = true;

    Clean();

    m_swFilterPipe = param.executedFilters;
    m_vpExecuteCaps = param.vpExecuteCaps;
    m_Params.Type = param.Type;

    // Clear executedFilters, which will be destroyed during hwFilter destroying.
    param.executedFilters = nullptr;

    std::vector<HwFilterParameter *>::iterator it = param.Params.begin();
    for (; it != param.Params.end(); ++it)
    {
        VP_PUBLIC_CHK_STATUS_RETURN((*it)->ConfigParams(*this));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HwFilter::ConfigParam(HW_FILTER_PARAM& param)
{
    VP_FUNC_CALL();

    if (!param.pfnCreatePacketParam)
    {
        VP_PUBLIC_ASSERTMESSAGE("Create packet params function is Null, return invalid params");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    VpPacketParameter* p = param.pfnCreatePacketParam(param);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_Params.Params.push_back(p);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HwFilter::Clean()
{
    VP_FUNC_CALL();

    std::vector<VpPacketParameter *>::iterator it = m_Params.Params.begin();
    for (; it != m_Params.Params.end(); ++it)
    {
        VpPacketParameter *p = *it;
        VpPacketParameter::Destory(p);
    }
    m_Params.Params.clear();

    m_vpInterface.GetSwFilterPipeFactory().Destory(m_swFilterPipe);

    return MOS_STATUS_SUCCESS;
}

RenderTargetType HwFilter::GetRenderTargetType()
{
    VP_FUNC_CALL();

    if (m_swFilterPipe)
    {
        return m_swFilterPipe->GetRenderTargetType();
    }
    else
    {
        return RenderTargetTypeInvalid;
    }
}

/****************************************************************************************************/
/*                                      HwFilterVebox                                               */
/****************************************************************************************************/

HwFilterVebox::HwFilterVebox(VpInterface &vpInterface) : HwFilter(vpInterface, EngineTypeVebox)
{
}

HwFilterVebox::HwFilterVebox(VpInterface &vpInterface, EngineType type) : HwFilter(vpInterface, type)
{
}

HwFilterVebox::~HwFilterVebox()
{
}

MOS_STATUS HwFilterVebox::SetPacketParams(VpCmdPacket &packet)
{
    VP_FUNC_CALL();

    bool bRet = true;

    PVPHAL_SURFACE pSrcSurface = nullptr;
    PVPHAL_SURFACE pOutputSurface = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(m_swFilterPipe);
    VP_SURFACE *inputSurf = m_swFilterPipe->GetSurface(true, 0);
    VP_SURFACE *outputSurf = m_swFilterPipe->GetSurface(false, 0);
    // previousSurf can be nullptr;
    VP_SURFACE *previousSurf = m_swFilterPipe->GetPastSurface(0);
    auto &surfSetting = m_swFilterPipe->GetSurfacesSetting();
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_STATUS_RETURN(packet.PacketInit(inputSurf, outputSurf,
        previousSurf, surfSetting, m_vpExecuteCaps));

    for (auto handler : m_Params.Params)
    {
        if (handler)
        {
            bRet = handler->SetPacketParam(&packet) && bRet;
        }
    }
    return bRet ? MOS_STATUS_SUCCESS : MOS_STATUS_UNKNOWN;
}

/****************************************************************************************************/
/*                                      HwFilterVeboxSfc                                                 */
/****************************************************************************************************/

HwFilterVeboxSfc::HwFilterVeboxSfc(VpInterface &vpInterface) : HwFilterVebox(vpInterface, EngineTypeVeboxSfc)
{}

HwFilterVeboxSfc::~HwFilterVeboxSfc()
{}

MOS_STATUS HwFilterVeboxSfc::SetPacketParams(VpCmdPacket &packet)
{
    VP_FUNC_CALL();

    return HwFilterVebox::SetPacketParams(packet);
}

/****************************************************************************************************/
/*                                      HwFilterRender                                              */
/****************************************************************************************************/

HwFilterRender::HwFilterRender(VpInterface &vpInterface) : HwFilter(vpInterface, EngineTypeRender)
{}

HwFilterRender::~HwFilterRender()
{}

MOS_STATUS HwFilterRender::SetPacketParams(VpCmdPacket &packet)
{
    VP_FUNC_CALL();

    bool bRet = true;

    PVPHAL_SURFACE pSrcSurface    = nullptr;
    PVPHAL_SURFACE pOutputSurface = nullptr;

    // Remove dependence on vphal surface later.
    VP_PUBLIC_CHK_NULL_RETURN(m_swFilterPipe);
    VP_SURFACE* inputSurf  = m_swFilterPipe->GetSurface(true, 0);
    VP_SURFACE* outputSurf = m_swFilterPipe->GetSurface(false, 0);
    // previousSurf can be nullptr;
    VP_SURFACE* previousSurf = m_swFilterPipe->GetPastSurface(0);
    auto& surfSetting        = m_swFilterPipe->GetSurfacesSetting();

    // There exist some features without input surface.
    if (inputSurf == nullptr)
    {
        VP_PUBLIC_NORMALMESSAGE("No render input!");
    }
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_STATUS_RETURN(packet.PacketInit(inputSurf, outputSurf,
        previousSurf, surfSetting, m_vpExecuteCaps));

    for (auto handler : m_Params.Params)
    {
        if (handler)
        {
            bRet = handler->SetPacketParam(&packet) && bRet;
        }
    }
    return bRet ? MOS_STATUS_SUCCESS : MOS_STATUS_UNKNOWN;
}
