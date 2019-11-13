/*
* Copyright (c) 2018, Intel Corporation
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
//! \file    mos_auxtable_mgr.cpp
//! \brief   Container class for GMM aux table manager wrapper
//!

#include "mos_auxtable_mgr.h"

#ifndef _AUXTABLE_SUPPORTED
AuxTableMgr::AuxTableMgr(MOS_BUFMGR *bufMgr)
{
}

AuxTableMgr::~AuxTableMgr()
{
}

AuxTableMgr * AuxTableMgr::CreateAuxTableMgr(MOS_BUFMGR *bufMgr, MEDIA_FEATURE_TABLE *sku)
{
    return nullptr;
}

MOS_STATUS  AuxTableMgr::MapResource(GMM_RESOURCE_INFO *gmmResInfo, MOS_LINUX_BO * bo)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS  AuxTableMgr::UnmapResource(GMM_RESOURCE_INFO* gmmResInfo, MOS_LINUX_BO *bo)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS  AuxTableMgr::EmitAuxTableBOList(MOS_LINUX_BO *cmd_bo)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

uint64_t AuxTableMgr::GetAuxTableBase()
{
    return 0;
}
#endif
