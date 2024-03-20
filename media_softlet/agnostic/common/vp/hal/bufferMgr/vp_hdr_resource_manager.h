/* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_hdr_resource_manager.h
//! \brief    The header file of the extention class of vp hdr resource manager
//! \details  all the vp hdr resources will be traced here for usages using intermeida
//!           surfaces.
//!
#ifndef _VP_HDR_RESOURCE_MANAGER_H__
#define _VP_HDR_RESOURCE_MANAGER_H__
#include "mos_defs.h"
#include "vp_allocator.h"
#include "sw_filter_pipe.h"
#include "vp_utils.h"

#define VP_MAX_HDR_INPUT_LAYER        8

typedef unsigned char uchar;

namespace vp {
class VphdrResourceManager
{
public:
    VphdrResourceManager(VpAllocator &allocator);
    ~VphdrResourceManager();
    MOS_STATUS AssignRenderResource(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe &executedFilters, MOS_INTERFACE &osInterface, VphalFeatureReport &reporting, bool deferredDestroyed);

private:
    MOS_STATUS FreeHdrRenderResource();

protected:
    VpAllocator &m_allocator;
    VP_SURFACE  *m_hdrCoeff                                    = nullptr;
    VP_SURFACE  *m_hdrAutoModeCoeffSurface                     = nullptr;
    VP_SURFACE  *m_hdrAutoModeIirTempSurface                   = nullptr;
    VP_SURFACE  *m_hdrOETF1DLUTSurface[VP_MAX_HDR_INPUT_LAYER] = {};
    VP_SURFACE  *m_hdrCri3DLUTSurface[VP_MAX_HDR_INPUT_LAYER]  = {};

MEDIA_CLASS_DEFINE_END(vp__VphdrResourceManager)
};
}  // namespace vp
#endif  // _VP_HDR_RESOURCE_MANAGER_H__
