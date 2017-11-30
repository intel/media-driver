/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file     renderhal_common.cpp
//! \brief    The file of common utilities definitions shared between VPHAL and RenderHal
//! \details  Common utilities for writing sync tag between VPHal and RenderHal
//!
#include "mos_os.h"
#include "mos_util_debug.h"

//!
//! \brief    Write Sync Tag to VP and RenderHal GSH at the same time
//! \details  Write Sync Tag to VP and RenderHal GSH at the same time
//! \param    uint32_t *pdwVpSyncTag
//!           [in] Pointer to legacy VPHal pHwInterface GSH sync tag(last sync tag completed)
//! \param    uint32_t *pdwRenderHalSyncTag
//!           [in] Pointer to RenderHal GSH sync tag(last sync tag completed)
//! \param    uint32_t dwSyncTag
//!           [in] Value of Sync tag to be updated
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS RenderCommon_WriteSyncTag(
                                        uint32_t *pdwVpSyncTag,
                                        uint32_t *pdwRenderHalSyncTag,
                                        uint32_t dwSyncTag)
{
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pdwVpSyncTag);
    MOS_OS_CHK_NULL(pdwRenderHalSyncTag);

    *pdwVpSyncTag           = dwSyncTag;
    *pdwRenderHalSyncTag    = dwSyncTag;

finish:
    return eStatus;
}

//!
//! \brief    Write Next Sync Tag to VP and RenderHal GSH at the same time
//! \details  Write Next Sync Tag to VP and RenderHal GSH at the same time
//! \param    uint32_t *pdwVpNextSyncTag
//!           [in] Pointer to legacy VPHal pHwInterface GSH next sync tag(Next sync tag value to use)
//! \param    uint32_t *pdwRenderHalNextSyncTag
//!           [in] Pointer to RenderHal GSH next sync tag(Next sync tag value to use)
//! \param    uint32_t dwSyncTag
//!           [in] Value of Sync tag to be updated
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS RenderCommon_WriteNextSyncTag(
                                        uint32_t *pdwVpNextSyncTag,
                                        uint32_t *pdwRenderHalNextSyncTag,
                                        uint32_t dwNextSyncTag)
{
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;

    MOS_OS_CHK_NULL(pdwVpNextSyncTag);
    MOS_OS_CHK_NULL(pdwRenderHalNextSyncTag);

    *pdwVpNextSyncTag           = dwNextSyncTag;
    *pdwRenderHalNextSyncTag    = dwNextSyncTag;

finish:
    return eStatus;
}

