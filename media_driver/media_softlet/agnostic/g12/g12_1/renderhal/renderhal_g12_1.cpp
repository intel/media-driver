/*
* Copyright (c) 2011-2021, Intel Corporation
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
//! \file       renderhal_g12_1.cpp
//! \brief      implementation of Gen12 DG1 hardware functions
//! \details    Render functions
//!

#include "renderhal_legacy.h"
#include "renderhal_g12_1.h"
#include "mhw_render.h"
#include "mhw_render_g12_X.h"

//!
//! \brief    Enables L3 cacheing flag and sets related registers/values
//! \param    PRENDERHAL_INTERFACE    pRenderHal
//!           [in]  Pointer to Hardware Interface
//! \param    pCacheSettings
//!           [in] L3 Cache Configurations
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS XRenderHal_Interface_G12_1::EnableL3Caching(
    PRENDERHAL_INTERFACE         pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings)
{
    MOS_STATUS                           eStatus;
    MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12  mHwL3CacheConfig = {};
    PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS pCacheConfig;
    MhwRenderInterface                   *pMhwRender;
    PRENDERHAL_INTERFACE_LEGACY          pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;

    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    pMhwRender = pRenderHalLegacy->pMhwRenderInterface;
    MHW_RENDERHAL_CHK_NULL(pMhwRender);

    if (nullptr == pCacheSettings)
    {
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->EnableL3Caching(nullptr));
        goto finish;
    }

    // customize the cache config for renderhal and let mhw_render overwrite it
    pCacheConfig = &mHwL3CacheConfig;

    pCacheConfig->dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12LP_DG1_RENDERHAL;

    // Override L3 cache configuration
    if (pCacheSettings->bOverride)
    {
        if (pCacheSettings->bCntlRegOverride)
        {
            pCacheConfig->dwCntlReg = pCacheSettings->dwCntlReg;
        }
    }
    MHW_RENDERHAL_CHK_STATUS(pMhwRender->EnableL3Caching(pCacheConfig));

finish:
    return eStatus;
}

//! \brief      Set L3 cache override config parameters
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in,out] pCacheSettings
//!             Pointer to pCacheSettings
//! \param      [in] bEnableSLM
//!             Flag to enable SLM
//! \return     MOS_STATUS
//!             MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS XRenderHal_Interface_G12_1::SetCacheOverrideParams(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
    bool                            bEnableSLM)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pCacheSettings);
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    pCacheSettings->dwCntlReg        = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12LP_DG1_RENDERHAL;
    pCacheSettings->bCntlRegOverride = true;

finish:
    return eStatus;
}