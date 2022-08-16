/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_debug.h
//! \brief    Definition of structures and functions for debugging VPHAL
//! \details  This file contains the definition of structures and functions for
//!           surface dumper, hw state dumper, perf counter dumper, and render
//!           parameter dumper
//!
#ifndef __VP_DEBUG_H__
#define __VP_DEBUG_H__

#include "vp_common.h"
#include "vp_oca_defs.h"       // Common interfaces and structures

#define VPHAL_DBG_OCA_DUMPER_CREATE(pRenderHal)                                       \
    if (pRenderHal)                                                                   \
        pRenderHal->pVphalOcaDumper = MOS_New(VphalOcaDumper);

#define VPHAL_DBG_OCA_DUMPER_DESTORY(pRenderHal)                                      \
    if (pRenderHal && pRenderHal->pVphalOcaDumper)                                    \
        VphalOcaDumper::Delete(pRenderHal->pVphalOcaDumper);

#define VPHAL_DBG_OCA_DUMPER_SET_RENDER_PARAM(pRenderHal, pRenderParams)              \
    if (pRenderHal && pRenderHal->pVphalOcaDumper)                                    \
        ((VphalOcaDumper*)pRenderHal->pVphalOcaDumper)->SetRenderParam(pRenderParams);

class VphalOcaDumper
{
public:
    VphalOcaDumper();
    virtual ~VphalOcaDumper();
    void SetRenderParam(VPHAL_RENDER_PARAMS *pRenderParams);

    VPHAL_OCA_RENDER_PARAM *GetRenderParam()
    {
        return m_pOcaRenderParam;
    }

    static void Delete(void *&p);

    void InitSurfInfo(VPHAL_OCA_SURFACE_INFO &surfInfo, VPHAL_SURFACE &surf);
    void InitSourceInfo(VPHAL_OCA_SOURCE_INFO &sourceInfo, VPHAL_SURFACE &source);
    void InitTargetInfo(VPHAL_OCA_TARGET_INFO &targetInfo, VPHAL_SURFACE &target);

    VPHAL_OCA_RENDER_PARAM *m_pOcaRenderParam = nullptr;

    MEDIA_CLASS_DEFINE_END(VphalOcaDumper)
};
#endif  // __VP_DEBUG_H__
