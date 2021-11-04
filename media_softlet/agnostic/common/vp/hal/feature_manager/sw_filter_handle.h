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
};
}

#endif //__SW_FILTER_HANDLE_H__
