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
//! \file     vp_obj_factories.h
//! \brief    Factories for vp object creation.
//!
#ifndef __VP_OBJ_FACTORIES_H__
#define __VP_OBJ_FACTORIES_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include "sw_filter_pipe.h"
#include "hw_filter_pipe.h"

namespace vp
{

template<class Type>
class VpObjAllocator
{
public:
    VpObjAllocator(VpInterface &vpInterface) : m_vpInterface(vpInterface)
    {
    }

    virtual ~VpObjAllocator()
    {
        while (!m_Pool.empty())
        {
            Type *p = m_Pool.back();
            m_Pool.pop_back();
            MOS_Delete(p);
        }
    }

    virtual Type *Create()
    {
        Type *obj = nullptr;
        if (m_Pool.empty())
        {
            obj = MOS_New(Type, m_vpInterface);
        }
        else
        {
            obj = m_Pool.back();
            if (obj)
            {
                m_Pool.pop_back();
            }
        }

        return obj;
    }

    virtual MOS_STATUS Destory(Type *&obj)
    {
        if (nullptr == obj)
        {
            return MOS_STATUS_SUCCESS;
        }
        obj->Clean();
        m_Pool.push_back(obj);
        obj = nullptr; 
        return MOS_STATUS_SUCCESS;
    }

private:
    std::vector<Type *> m_Pool;
    VpInterface      &m_vpInterface;

MEDIA_CLASS_DEFINE_END(vp__VpObjAllocator)
};

class HwFilterPipeFactory
{
public:
    HwFilterPipeFactory(VpInterface &vpInterface);

    virtual ~HwFilterPipeFactory();

    MOS_STATUS Create(SwFilterPipe &swfilterPipe,
        Policy &policy, HwFilterPipe *&pHwFilterPipe);

    MOS_STATUS Destory(HwFilterPipe *&pHwfilterPipe);

private:
    VpObjAllocator<HwFilterPipe> m_allocator;

MEDIA_CLASS_DEFINE_END(vp__HwFilterPipeFactory)
};

class HwFilterFactory
{
public:
    HwFilterFactory(VpInterface &vpInterface);
    virtual ~HwFilterFactory();
    HwFilter *Create(HW_FILTER_PARAMS &param);
    void Destory(HwFilter *&pHwFilter);

private:
    VpObjAllocator<HwFilterVebox> m_allocatorVebox;
    VpObjAllocator<HwFilterVeboxSfc> m_allocatorVeboxSfc;
    VpObjAllocator<HwFilterRender> m_allocatorRender;

MEDIA_CLASS_DEFINE_END(vp__HwFilterFactory)
};

class SwFilterPipeFactory
{
public:
    SwFilterPipeFactory(VpInterface &vpInterface);
    virtual ~SwFilterPipeFactory();

    MOS_STATUS Create(PVP_PIPELINE_PARAMS params, std::vector<SwFilterPipe *> &swFilterPipe, VpPipelineParamFactory *pipelineParamFactory);
    MOS_STATUS Create(VEBOX_SFC_PARAMS *params, std::vector<SwFilterPipe*> &swFilterPipe);

    // Create empty swFilter
    MOS_STATUS Create(SwFilterPipe *&swFilterPipe);
    void Destory(SwFilterPipe *&swfilterPipe);

private:
    int GetPipeCountForProcessing(VP_PIPELINE_PARAMS &params);
    MOS_STATUS Update(VP_PIPELINE_PARAMS &params, int index);
    VpObjAllocator<SwFilterPipe> m_allocator;
    VpInterface &m_vpInterface;

MEDIA_CLASS_DEFINE_END(vp__SwFilterPipeFactory)
};

template <class _T>
class SwFilterFactory
{
public:
    SwFilterFactory(VpInterface& vpInterface) : m_allocator(vpInterface)
    {
    }
    virtual ~SwFilterFactory()
    {
    }
    SwFilter* Create()
    {
        SwFilter* swFilter = nullptr;
        swFilter = m_allocator.Create();

        if (swFilter)
        {
            return swFilter;
        }
        else
        {
            return nullptr;
        }
    }
    void Destory(_T*& swFilter)
    {
        m_allocator.Destory(swFilter);
        return;
    }

private:
    VpObjAllocator<_T> m_allocator;

MEDIA_CLASS_DEFINE_END(vp__SwFilterFactory)
};
}
#endif // !__VP_OBJ_FACTORIES_H__
