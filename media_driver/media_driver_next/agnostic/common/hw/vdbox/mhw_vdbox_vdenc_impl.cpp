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
//! \file     mhw_vdbox_vdenc_impl.cpp
//! \brief    MHW VDBOX VDENC interface common base
//! \details
//!

#include "mhw_vdbox_vdenc_impl.h"

using namespace mhw::vdbox::vdenc;

Impl::Impl(PMOS_INTERFACE osItf)
{
    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL_NO_STATUS_RETURN(osItf);

    m_osItf = osItf;
    if (m_osItf->bUsesGfxAddress)
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }

    InitRowstoreUserFeatureSettings();
}

MOS_STATUS Impl::EnableVdencRowstoreCacheIfSupported(uint32_t address)
{
    MHW_FUNCTION_ENTER;

    if (this->m_vdencRowStoreCache.supported)
    {
        this->m_vdencRowStoreCache.enabled   = true;
        this->m_vdencRowStoreCache.dwAddress = address;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Impl::EnableVdencRowIpdlstoreCacheIfSupported(uint32_t address)
{
    MHW_FUNCTION_ENTER;

    if (this->m_vdencIpdlRowstoreCache.supported)
    {
        this->m_vdencIpdlRowstoreCache.enabled   = true;
        this->m_vdencIpdlRowstoreCache.dwAddress = address;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Impl::SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
{
    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL_RETURN(settings);

    size_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

    return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
}

MOS_STATUS Impl::InitRowstoreUserFeatureSettings()
{
    MHW_FUNCTION_ENTER;

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MEDIA_FEATURE_TABLE *       skuTable = this->m_osItf->pfnGetSkuTable(this->m_osItf);

    MHW_MI_CHK_NULL(skuTable);

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    if (this->m_osItf->bSimIsActive)
    {
        // Disable RowStore Cache on simulation by default
        userFeatureData.u32Data = 1;
    }
    else
    {
        userFeatureData.u32Data = 0;
    }

    userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ROWSTORE_CACHE_DISABLE_ID,
        &userFeatureData,
        m_osItf->pOsContext);
#endif  // _DEBUG || _RELEASE_INTERNAL
    bool rowstoreCachingSupported = userFeatureData.i32Data ? false : true;

    if (!rowstoreCachingSupported)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_VDENCROWSTORECACHE_DISABLE_ID,
        &userFeatureData,
        m_osItf->pOsContext);
#endif  // _DEBUG || _RELEASE_INTERNAL
    this->m_vdencRowStoreCache.supported = userFeatureData.i32Data ? false : true;

    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_INTRAROWSTORECACHE_DISABLE_ID,
        &userFeatureData,
        m_osItf->pOsContext);
#endif  // _DEBUG || _RELEASE_INTERNAL
    this->m_vdencIpdlRowstoreCache.supported = userFeatureData.i32Data ? false : true;

    return MOS_STATUS_SUCCESS;
}
