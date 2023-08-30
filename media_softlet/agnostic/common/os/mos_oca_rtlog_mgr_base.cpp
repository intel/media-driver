/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     mos_oca_rtlog_mgr_base.cpp
//! \brief    OCA buffer manager class
//!

#include "mos_oca_rtlog_mgr_base.h"
#include "oca_rtlog_section_mgr.h"
#include "mos_context_specific_next.h"

bool MosOcaRTLogMgrBase::s_enableOcaRTLog = true;
MosMutex MosOcaRTLogMgrBase::s_ocaMutex;
bool MosOcaRTLogMgrBase::s_isOcaRtlogMgrDestoryed = false;

/****************************************************************************************************/
/*                                      MosOcaRTLogMgrBase                                          */
/****************************************************************************************************/

int32_t MosOcaRTLogMgrBase::GetGlobleIndex()
{
    return MosUtilities::MosAtomicIncrement(&m_globleIndex);
}