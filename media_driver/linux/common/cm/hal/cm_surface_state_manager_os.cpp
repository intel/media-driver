/*
* Copyright (c) 2018-2019, Intel Corporation
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
//! \file      cm_surface_state_manager_os.cpp
//! \brief     Contains Class CmSurfaceState2Dor3DMgr  definitions 
//!

#include "cm_surface_state_manager.h"

CmSurfaceState3DMgr::CmSurfaceState3DMgr(CM_HAL_STATE *cmhal, MOS_RESOURCE *resource):
    CmSurfaceState2Dor3DMgr(cmhal, resource)
{
    CM_CHK_NULL_RETURN_VOID(resource);
    MOS_ZeroMemory(&m_resourceFor3d, sizeof(m_resourceFor3d));
    m_resourceFor3d = *resource;
    m_resource = &m_resourceFor3d;
}

