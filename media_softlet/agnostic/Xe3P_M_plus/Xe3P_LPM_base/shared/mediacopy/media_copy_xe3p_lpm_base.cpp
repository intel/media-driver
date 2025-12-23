/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     media_copy_xe3p_lpm_base.cpp
//! \brief    Common interface and structure used in media copy
//! \details  Common interface and structure used in media copy which are platform independent
//!

#include "media_copy_xe3p_lpm_base.h"
#include "media_debug_dumper.h"
#include "media_copy_common.h"

MediaCopyStateXe3P_Lpm_Base::MediaCopyStateXe3P_Lpm_Base():
    MediaCopyBaseState()
{

}

MOS_STATUS MediaCopyStateXe3P_Lpm_Base::Initialize(PMOS_INTERFACE  osInterface, MhwInterfacesNext *mhwInterfaces)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MCPY_CHK_NULL_RETURN(osInterface);
    MCPY_CHK_NULL_RETURN(mhwInterfaces);
    MEDIA_FEATURE_TABLE* pSkuTable;

    m_osInterface   = osInterface;
    m_mhwInterfaces = mhwInterfaces;
    pSkuTable       = osInterface->pfnGetSkuTable(osInterface);

    MCPY_CHK_STATUS_RETURN(MediaCopyBaseState::Initialize(osInterface));

    if (!MEDIA_IS_SKU(pSkuTable, FtrMainCopyRemoved))
    {
        // blt copy init
        if (nullptr == m_bltCopy)
        {
            m_bltCopy = MOS_New(BltStateXe3P_Lpm_Base, m_osInterface, m_mhwInterfaces);
            MCPY_CHK_NULL_RETURN(m_bltCopy);
            MCPY_CHK_STATUS_RETURN(m_bltCopy->Initialize());
        }
    }
    else
    {
        MCPY_NORMALMESSAGE(" Blt copy don't support due to no Main Copy Engine ");
    }

    // vebox init
    if (nullptr == m_veboxCopyState)
    {
        m_veboxCopyState = MOS_New(VeboxCopyStateXe3P_Lpm_Base, m_osInterface, m_mhwInterfaces);
        MCPY_CHK_NULL_RETURN(m_veboxCopyState);
        MCPY_CHK_STATUS_RETURN(m_veboxCopyState->Initialize());
    }

    return eStatus;
}

MediaCopyStateXe3P_Lpm_Base::~MediaCopyStateXe3P_Lpm_Base()
{
    MOS_Delete(m_bltCopy);
    MOS_Delete(m_veboxCopyState);
    if (m_mhwInterfaces != nullptr)
    {
        m_mhwInterfaces->Destroy();
        MOS_Delete(m_mhwInterfaces);
    }
}

MOS_STATUS MediaCopyStateXe3P_Lpm_Base::FeatureSupport(PMOS_RESOURCE src,
                                                    PMOS_RESOURCE dst,
                                                    MCPY_STATE_PARAMS &mcpy_src,
                                                    MCPY_STATE_PARAMS &mcpy_dst,
                                                    MCPY_ENGINE_CAPS  &caps)
{
    caps.engineVebox  = true;
    caps.engineBlt    = true;
    caps.engineRender = false;

    if (m_osInterface &&
        MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrMainCopyRemoved))
    {
        caps.engineBlt = false;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateXe3P_Lpm_Base::CopyEnigneSelect(MCPY_METHOD& preferMethod, MCPY_ENGINE& mcpyEngine, MCPY_ENGINE_CAPS& caps)
{
    switch (preferMethod)
    {
        case MCPY_METHOD_BALANCE:
            mcpyEngine = caps.engineVebox ? MCPY_ENGINE_VEBOX : MCPY_ENGINE_BLT;
            break;
        case MCPY_METHOD_POWERSAVING:
        case MCPY_METHOD_DEFAULT:
            mcpyEngine = caps.engineBlt ? MCPY_ENGINE_BLT : MCPY_ENGINE_VEBOX;
            break;
        default:
            break;
    }
#if (_DEBUG || _RELEASE_INTERNAL)
    if (MCPY_METHOD_POWERSAVING == m_MCPYForceMode)
    {
        mcpyEngine = MCPY_ENGINE_BLT;
    }
    else if (MCPY_METHOD_BALANCE == m_MCPYForceMode)
    {
        mcpyEngine = MCPY_ENGINE_VEBOX;
    }
    else if (MCPY_METHOD_DEFAULT != m_MCPYForceMode)
    {
        return MOS_STATUS_INVALID_PARAMETER; // For wrong value, bypass copy engine, just let APP handle it.
    }
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaCopyStateXe3P_Lpm_Base::MediaBltCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    if (m_bltCopy != nullptr)
    {
         return m_bltCopy->CopyMainSurface(src, dst);
    }
    else
    {
         return MOS_STATUS_UNIMPLEMENTED;
    }
}

MOS_STATUS MediaCopyStateXe3P_Lpm_Base::MediaVeboxCopy(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    // implementation
    if (m_veboxCopyState != nullptr)
    {
        return m_veboxCopyState->CopyMainSurface(src, dst);
    }
    else
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }
}

bool MediaCopyStateXe3P_Lpm_Base::IsVeboxCopySupported(PMOS_RESOURCE src, PMOS_RESOURCE dst)
{
    bool supported = false;

    if (m_osInterface &&
        !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrVERing))
    {
        return false;
    }

    if (m_veboxCopyState)
    {
        supported = m_veboxCopyState->IsSurfaceSupported(src) && m_veboxCopyState->IsSurfaceSupported(dst);
    }

    if (src->TileType == MOS_TILE_LINEAR &&
        dst->TileType == MOS_TILE_LINEAR)
    {
        supported = false;
    }

    return supported;
}
