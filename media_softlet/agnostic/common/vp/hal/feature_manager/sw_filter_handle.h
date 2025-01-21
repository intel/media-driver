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
//! \file     sw_filter_handle.h
//! \brief    Factories for vp sw filter handle creation.
//!
#ifndef __SW_FILTER_HANDLE_H__
#define __SW_FILTER_HANDLE_H__

#include "vp_pipeline.h"
#include "vp_obj_factories.h"

namespace vp {
class SwFilterFeatureHandler
{
public:
    SwFilterFeatureHandler(VpInterface& vpInterface, FeatureType type);
    virtual ~SwFilterFeatureHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual MOS_STATUS CreateSwFilter(SwFilter*& swFilter, VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual bool IsFeatureEnabled(VEBOX_SFC_PARAMS& params);
    virtual MOS_STATUS CreateSwFilter(SwFilter*& swFilter, VEBOX_SFC_PARAMS& params);
    virtual SwFilter* CreateSwFilter()
    {
        return nullptr;
    }

    virtual void Destory(SwFilter*& swFilter)
    {
        return;
    }

    virtual int GetPipeCountForProcessing(VP_PIPELINE_PARAMS& params)
    {
        return 1;
    }
    virtual MOS_STATUS UpdateParamsForProcessing(VP_PIPELINE_PARAMS& params, int index)
    {
        return MOS_STATUS_SUCCESS;
    }
protected:
    VpInterface& m_vpInterface;
    FeatureType     m_type;

MEDIA_CLASS_DEFINE_END(vp__SwFilterFeatureHandler)
};

class SwFilterCscHandler : public SwFilterFeatureHandler
{
public:
    SwFilterCscHandler(VpInterface& vpInterface);
    virtual ~SwFilterCscHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
    virtual bool IsFeatureEnabled(VEBOX_SFC_PARAMS& params);
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterCsc> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterCscHandler)
};

class SwFilterRotMirHandler : public SwFilterFeatureHandler
{
public:
    SwFilterRotMirHandler(VpInterface& vpInterface);
    virtual ~SwFilterRotMirHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
    virtual bool IsFeatureEnabled(VEBOX_SFC_PARAMS& params);
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterRotMir> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterRotMirHandler)
};

class SwFilterScalingHandler : public SwFilterFeatureHandler
{
public:
    SwFilterScalingHandler(VpInterface& vpInterface);
    virtual ~SwFilterScalingHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
    virtual bool IsFeatureEnabled(VEBOX_SFC_PARAMS& params);
    virtual int GetPipeCountForProcessing(VP_PIPELINE_PARAMS& params);
    virtual MOS_STATUS UpdateParamsForProcessing(VP_PIPELINE_PARAMS& params, int index);
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterScaling> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterScalingHandler)
};

class SwFilterDnHandler : public SwFilterFeatureHandler
{
public:
    SwFilterDnHandler(VpInterface& vpInterface);
    virtual ~SwFilterDnHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterDenoise> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterDnHandler)
};

class SwFilterDiHandler : public SwFilterFeatureHandler
{
public:
    SwFilterDiHandler(VpInterface& vpInterface);
    virtual ~SwFilterDiHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterDeinterlace> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterDiHandler)
};

class SwFilterSteHandler : public SwFilterFeatureHandler
{
public:
    SwFilterSteHandler(VpInterface& vpInterface);
    virtual ~SwFilterSteHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterSte> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterSteHandler)
};

class SwFilterTccHandler : public SwFilterFeatureHandler
{
public:
    SwFilterTccHandler(VpInterface& vpInterface);
    virtual ~SwFilterTccHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterTcc> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterTccHandler)
};

class SwFilterProcampHandler : public SwFilterFeatureHandler
{
public:
    SwFilterProcampHandler(VpInterface& vpInterface);
    virtual ~SwFilterProcampHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterProcamp> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterProcampHandler)
};

class SwFilterHdrHandler : public SwFilterFeatureHandler
{
public:
    SwFilterHdrHandler(VpInterface &vpInterface);
    virtual ~SwFilterHdrHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter *CreateSwFilter();

protected:
    virtual void Destory(SwFilter *&swFilter);

protected:
    SwFilterFactory<SwFilterHdr> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterHdrHandler)
};

class SwFilterLumakeyHandler : public SwFilterFeatureHandler
{
public:
    SwFilterLumakeyHandler(VpInterface &vpInterface, FeatureType featureType);
    virtual ~SwFilterLumakeyHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter *CreateSwFilter();

protected:
    virtual void Destory(SwFilter *&swFilter);

protected:
    SwFilterFactory<SwFilterLumakey>    m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterLumakeyHandler)
};

class SwFilterBlendingHandler : public SwFilterFeatureHandler
{
public:
    SwFilterBlendingHandler(VpInterface &vpInterface, FeatureType featureType);
    virtual ~SwFilterBlendingHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter *CreateSwFilter();

protected:
    virtual void Destory(SwFilter *&swFilter);

protected:
    SwFilterFactory<SwFilterBlending>   m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterBlendingHandler)
};

class SwFilterColorFillHandler : public SwFilterFeatureHandler
{
public:
    SwFilterColorFillHandler(VpInterface &vpInterface, FeatureType featureType);
    virtual ~SwFilterColorFillHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter *CreateSwFilter();

protected:
    virtual void Destory(SwFilter *&swFilter);

protected:
    SwFilterFactory<SwFilterColorFill>  m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterColorFillHandler)
};

class SwFilterAlphaHandler : public SwFilterFeatureHandler
{
public:
    SwFilterAlphaHandler(VpInterface &vpInterface, FeatureType featureType);
    virtual ~SwFilterAlphaHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter *CreateSwFilter();

protected:
    virtual void Destory(SwFilter *&swFilter);

protected:
    SwFilterFactory<SwFilterAlpha>      m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterAlphaHandler)
};

class SwFilterCgcHandler : public SwFilterFeatureHandler
{
public:
    SwFilterCgcHandler(VpInterface& vpInterface);
    virtual ~SwFilterCgcHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual SwFilter* CreateSwFilter();
protected:
    virtual void Destory(SwFilter*& swFilter);
protected:
    SwFilterFactory<SwFilterCgc> m_swFilterFactory;

MEDIA_CLASS_DEFINE_END(vp__SwFilterCgcHandler)
};

template <class Type>
class SwFilterAiBaseHandler : public SwFilterFeatureHandler
{
public:
    SwFilterAiBaseHandler(VpInterface &vpInterface, FeatureType type) : SwFilterFeatureHandler(vpInterface, type), m_swFilterFactory(vpInterface) {}
    virtual ~SwFilterAiBaseHandler()
    {
        for (auto vpSurf : m_pipeIntermediateVpSurfaces)
        {
            m_vpInterface.GetAllocator().DestroyVpSurface(vpSurf);
        }
        m_pipeIntermediateVpSurfaces.clear();
    }
    virtual SwFilter *CreateSwFilter(FeatureType type)
    {
        VP_FUNC_CALL();
        SwFilter *swFilter = nullptr;
        swFilter           = m_swFilterFactory.Create();
        if (swFilter)
        {
            swFilter->SetFeatureType(type);
        }

        return swFilter;
    }
    virtual MOS_STATUS InitializePipeIntermediateSurface(PVPHAL_SURFACE vphalSurf)
    {
        VP_PUBLIC_CHK_NULL_RETURN(vphalSurf);
        if (!Mos_ResourceIsNull(&vphalSurf->OsResource))
        {
            //this is not an empty vphal surface or the pipeline intermediate surface has already been initialized
            return MOS_STATUS_SUCCESS;
        }

        bool        allocated       = false;
        PVP_SURFACE originVpSurface = vphalSurf->pPipeIntermediateSurface;

        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface.GetAllocator().ReAllocateVpSurfaceWithSameConfigOfVphalSurface(
            vphalSurf->pPipeIntermediateSurface,
            vphalSurf,
            "VpPipelineIntermediateSurface",
            allocated));

        if (allocated)
        {
            m_pipeIntermediateVpSurfaces.erase(originVpSurface);
        }
        m_pipeIntermediateVpSurfaces.insert(vphalSurf->pPipeIntermediateSurface);

        return MOS_STATUS_SUCCESS;
    }
    virtual SwFilter *CreateSwFilter() = 0;
    // This need to be implemented by derived class
    virtual bool       IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType) = 0;
    // If the feature needs pre/post processing, then return 3, otherwise return 1
    virtual int        GetPipeCountForProcessing(VP_PIPELINE_PARAMS &params) override                                           = 0;
    // If the feature needs pre/post processing, then override this function, ottherwise just return MOS_STATUS_SUCCESS
    virtual MOS_STATUS UpdateParamsForProcessing(VP_PIPELINE_PARAMS &params, int index) override                                = 0;

protected:
    virtual void Destory(SwFilter*& swFilter)
    {
        VP_FUNC_CALL();
        Type *filter = dynamic_cast<Type *>(swFilter);
        m_swFilterFactory.Destory(filter);
    }

protected:
    SwFilterFactory<Type>  m_swFilterFactory;
    std::set<VP_SURFACE *> m_pipeIntermediateVpSurfaces;

MEDIA_CLASS_DEFINE_END(vp__SwFilterAiBaseHandler)
};
}

#endif //__SW_FILTER_HANDLE_H__
