/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     sw_filter_pipe_factory.cpp
//! \brief    Defines the common interface for sw filter pipe factory.
//!
#ifndef __SW_FILTER_FACTORY_H__
#define __SW_FILTER_FACTORY_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include "sw_filter_pipe.h"

namespace vp
{

class SwFilterPipeFactory
{
public:
    SwFilterPipeFactory();
    MOS_STATUS CreateSwFilterPipe(VP_PIPELINE_PARAMS &params, SwFilterPipe *pSwFilterPipe);
    virtual ~SwFilterPipeFactory();
    void ReturnSwFilterPipe(SwFilterPipe *&pSwfilterPipe);

private:
    SwFilterFactory m_SwFilterFactroy;
};

}
#endif // !__SW_FILTER_FACTORY_H__
