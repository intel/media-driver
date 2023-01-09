/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mhw_vdbox_hcp_g9_kbl.cpp
//! \brief    Defines functions for constructing Vdbox HCP commands on G9 KBL
//!

#include "mhw_vdbox_hcp_g9_kbl.h"
#include "mhw_mi_hwcmd_g9_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_kbl.h"
#include "mos_os_cp_interface_specific.h"

void MhwVdboxHcpInterfaceG9Kbl::InitRowstoreUserFeatureSettings()
{
    MhwVdboxHcpInterfaceG9::InitRowstoreUserFeatureSettings(m_osInterface->pOsContext);

    if (m_rowstoreCachingSupported && m_decodeInUse)
    {
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP9_HVDROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_vp9HvdRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_VP9_DFROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_vp9DfRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

    }
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::GetHcpStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;
    uint32_t            standard = CodecHal_GetStandardFromMode(mode);

    if (standard == CODECHAL_HEVC)
    {
        maxSize =
            mhw_vdbox_vdenc_g9_kbl::VD_PIPELINE_FLUSH_CMD::byteSize +
            mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize +
            mhw_vdbox_hcp_g9_kbl::HCP_PIPE_MODE_SELECT_CMD::byteSize +
            mhw_vdbox_hcp_g9_kbl::HCP_SURFACE_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_kbl::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw_vdbox_hcp_g9_kbl::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize;

        patchListMaxSize =
            PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD);

        if (mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            /* HCP_QM_STATE_CMD may be issued up to 20 times: 3x Colour Component plus 2x intra/inter plus 4x SizeID minus 4 for the 32x32 chroma components.
            HCP_FQP_STATE_CMD may be issued up to 8 times: 4 scaling list per intra and inter. */
            maxSize +=
                20 * mhw_vdbox_hcp_g9_kbl::HCP_QM_STATE_CMD::byteSize +
                8 * mhw_vdbox_hcp_g9_kbl::HCP_FQM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD::byteSize;

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                8 * PATCH_LIST_COMMAND(HCP_FQM_STATE_CMD);
        }
        else
        {
            maxSize +=
                20 * mhw_vdbox_hcp_g9_kbl::HCP_QM_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_TILE_STATE_CMD::byteSize;

            patchListMaxSize +=
                20 * PATCH_LIST_COMMAND(HCP_QM_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_TILE_STATE_CMD);
        }
    }
    else if (standard == CODECHAL_VP9)
    {
        if (mode == CODECHAL_ENCODE_MODE_VP9)
        {
            maxSize =
                mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize * 2 +
                mhw_vdbox_hcp_g9_kbl::HCP_PIPE_MODE_SELECT_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_SURFACE_STATE_CMD::byteSize * 5 +
                mhw_vdbox_hcp_g9_kbl::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                mhw_vdbox_hcp_g9_kbl::HCP_VP9_PIC_STATE_CMD::byteSize +
                mhw_mi_g9_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_PAK_INSERT_OBJECT_CMD::byteSize +
                mhw_mi_g9_X::MI_BATCH_BUFFER_START_CMD::byteSize * 2 +
                mhw_vdbox_vdenc_g9_kbl::VD_PIPELINE_FLUSH_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2 +
                PATCH_LIST_COMMAND(HCP_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) * 5 +
                PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
                PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                PATCH_LIST_COMMAND(HCP_PAK_INSERT_OBJECT_CMD) +
                PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) * 2 +
                PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD);
        }
        else  // VP9 Clear Decode
        {
            maxSize =
                mhw_vdbox_vdenc_g9_kbl::VD_PIPELINE_FLUSH_CMD::byteSize +
                mhw_mi_g9_X::MI_FLUSH_DW_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_PIPE_MODE_SELECT_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_SURFACE_STATE_CMD::byteSize * 4 +
                mhw_vdbox_hcp_g9_kbl::HCP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                mhw_vdbox_hcp_g9_kbl::HCP_VP9_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_BSD_OBJECT_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD) +
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
                PATCH_LIST_COMMAND(HCP_PIPE_MODE_SELECT_CMD) +
                PATCH_LIST_COMMAND(HCP_SURFACE_STATE_CMD) * 4 +
                PATCH_LIST_COMMAND(HCP_PIPE_BUF_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_IND_OBJ_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
                PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported standard.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::GetHcpPrimitiveCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    bool                            modeSpecific)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    uint32_t            standard = CodecHal_GetStandardFromMode(mode);
    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;

    if (standard == CODECHAL_HEVC)
    {
        if (mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            maxSize =
                2 * mhw_vdbox_hcp_g9_kbl::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g9_kbl::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_PAK_INSERT_OBJECT_CMD::byteSize +
                mhw_mi_g9_X::MI_BATCH_BUFFER_START_CMD::byteSize;

            patchListMaxSize =
                2 * PATCH_LIST_COMMAND(HCP_REF_IDX_STATE_CMD) +
                2 * PATCH_LIST_COMMAND(HCP_WEIGHTOFFSET_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_PAK_INSERT_OBJECT_CMD) +
                2 * PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD); // One is for the PAK command and another one is for the BB when BRC and single task mode are on

        }
        else
        {
            maxSize =
                2 * mhw_vdbox_hcp_g9_kbl::HCP_REF_IDX_STATE_CMD::byteSize +
                2 * mhw_vdbox_hcp_g9_kbl::HCP_WEIGHTOFFSET_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_SLICE_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g9_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                2 * PATCH_LIST_COMMAND(HCP_REF_IDX_STATE_CMD) +
                2 * PATCH_LIST_COMMAND(HCP_WEIGHTOFFSET_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_SLICE_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
        }
    }
    else if (standard == CODECHAL_VP9)      // VP9 Clear decode does not require primitive level commands. VP9 DRM does. 
    {
        if (modeSpecific)                  // VP9 DRM
        {
            maxSize +=
                mhw_vdbox_hcp_g9_kbl::HCP_VP9_SEGMENT_STATE_CMD::byteSize * 8 +
                mhw_vdbox_hcp_g9_kbl::HCP_VP9_PIC_STATE_CMD::byteSize +
                mhw_vdbox_hcp_g9_kbl::HCP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g9_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(HCP_VP9_SEGMENT_STATE_CMD) * 8 +
                PATCH_LIST_COMMAND(HCP_VP9_PIC_STATE_CMD) +
                PATCH_LIST_COMMAND(HCP_BSD_OBJECT_CMD);
        }
    }
    else
    {
        MHW_ASSERTMESSAGE("Unsupported standard.");
        eStatus = MOS_STATUS_UNKNOWN;
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_kbl::HCP_PIPE_MODE_SELECT_CMD  *cmd = 
        (mhw_vdbox_hcp_g9_kbl::HCP_PIPE_MODE_SELECT_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceG9<mhw_vdbox_hcp_g9_kbl>::AddHcpPipeModeSelectCmd(cmdBuffer, params));

    m_cpInterface->SetProtectionSettingsForMfxPipeModeSelect((uint32_t *)cmd);

    // Without having the HprVp9ModeSwitchEco disabled, the gen9 devices
    // cause gpu to hang while decoding tiled 4k vp9 streams
    cmd->DW4.HprVp9ModeSwitchEcoDisable = 1;

    cmd->DW1.PakPipelineStreamoutEnable = params->bStreamOutEnabled;
    cmd->DW1.AdvancedRateControlEnable = params->bAdvancedRateControlEnable;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_kbl::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_kbl::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_kbl>::AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));

    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_PLANAR_420_8;
    }
    else
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_P010;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpEncodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_hcp_g9_kbl::HCP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_kbl::HCP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_kbl>::AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));

    cmd->DW2.YOffsetForUCbInPixel = params->psSurface->UPlaneOffset.iYOffset;
    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_PLANAR_420_8;
    }
    else
    {
        cmd->DW2.SurfaceFormat = HCP_SURFACE_FORMAT_P010;
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS  params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS                               resourceParams;
    MOS_SURFACE                                       details;
    mhw_vdbox_hcp_g9_kbl::HCP_PIPE_BUF_ADDR_STATE_CMD cmd;

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        UserFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED_ID;
        if (m_hevcDatRowStoreCache.bEnabled     ||
            m_hevcDfRowStoreCache.bEnabled      ||
            m_hevcSaoRowStoreCache.bEnabled     ||
            m_vp9HvdRowStoreCache.bEnabled      ||
            m_vp9DfRowStoreCache.bEnabled)
        {
            UserFeatureWriteData.Value.i32Data = 1;
        }
        MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, m_osInterface->pOsContext);
#endif

    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_HCP_DECODED_BUFFER_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

    // Decoded Picture
    // Caching policy change if any of below modes are true
    // Note for future dev: probably a good idea to add a macro for the below check 
    if (m_osInterface->osCpInterface->IsHMEnabled() ||
        m_osInterface->osCpInterface->IsIDMEnabled() ||
        m_osInterface->osCpInterface->IsSMEnabled())
    {
        cmd.DecodedPictureMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC_PARTIALENCSURFACE].Value;
    }
    else
    {
        cmd.DecodedPictureMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Value;
    }

    cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable =
        (params->PreDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
    cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode =
        (params->PreDeblockSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

    cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->psPreDeblockSurface->TileType);

    // For HEVC 8bit/10bit mixed case, register App's RenderTarget for specific use case
    if (params->presP010RTSurface != nullptr)
    {
        resourceParams.presResource = &(params->presP010RTSurface->OsResource);
        resourceParams.dwOffset = params->presP010RTSurface->dwOffset;
        resourceParams.pdwCmd = (cmd.DecodedPicture.DW0_1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        if (m_osInterface->bAllowExtraPatchToSameLoc)
        {
            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
        else //if not allowed to patch another OsResource to same location in cmd, just register resource here
        {
            MHW_MI_CHK_STATUS(m_osInterface->pfnRegisterResource(
                m_osInterface,
                resourceParams.presResource,
                resourceParams.bIsWritable,
                resourceParams.bIsWritable));
        }
    }

    resourceParams.presResource = &(params->psPreDeblockSurface->OsResource);
    resourceParams.dwOffset = params->psPreDeblockSurface->dwOffset;
    resourceParams.pdwCmd = (cmd.DecodedPicture.DW0_1.Value);
    resourceParams.dwLocationInCmd = 1;
    resourceParams.bIsWritable = true;

    MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    resourceParams.dwLsbNum = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
    MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->PreDeblockSurfMmcState));

    // Deblocking Filter Line Buffer
    cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

    cmd.DeblockingFilterLineBuffer.DW0_1.Graphicsaddress476 = 0;
    if (m_hevcDfRowStoreCache.bEnabled)
    {
        cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockingFilterLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockingFilterLineBuffer.DW0_1.Graphicsaddress476 = m_hevcDfRowStoreCache.dwAddress;
    }
    else if (m_vp9DfRowStoreCache.bEnabled)
    {
        cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockingFilterLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockingFilterLineBuffer.DW0_1.Graphicsaddress476 = m_vp9DfRowStoreCache.dwAddress;
    }
    else if (params->presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
    {
        resourceParams.presResource = params->presMfdDeblockingFilterRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockingFilterLineBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocking Filter Tile Line Buffer
    cmd.DeblockingFilterTileLineBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

    if (params->presDeblockingFilterTileRowStoreScratchBuffer != nullptr)
    {
        resourceParams.presResource = params->presDeblockingFilterTileRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockingFilterTileLineBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 7;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocking Filter Tile Column Buffer
    cmd.DeblockingFilterTileColumnBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

    if (params->presDeblockingFilterColumnRowStoreScratchBuffer != nullptr)
    {
        resourceParams.presResource = params->presDeblockingFilterColumnRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockingFilterTileColumnBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 10;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Metadata Line Buffer
    cmd.MetadataLineBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

    if (m_hevcDatRowStoreCache.bEnabled)
    {
        cmd.MetadataLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.MetadataLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.MetadataLineBuffer.DW0_1.Graphicsaddress476 = m_hevcDatRowStoreCache.dwAddress;
    }
    else if (params->presMetadataLineBuffer != nullptr)
    {
        resourceParams.presResource = params->presMetadataLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.MetadataLineBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 13;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Metadata Tile Line Buffer
    cmd.MetadataTileLineBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

    if (params->presMetadataTileLineBuffer != nullptr)
    {
        resourceParams.presResource = params->presMetadataTileLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.MetadataTileLineBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Metadata Tile Column Buffer
    cmd.MetadataTileColumnBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

    if (params->presMetadataTileColumnBuffer != nullptr)
    {
        resourceParams.presResource = params->presMetadataTileColumnBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.MetadataTileColumnBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 19;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // SAO Line Buffer
    cmd.SaoLineBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

    if (m_hevcSaoRowStoreCache.bEnabled)
    {
        cmd.SaoLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.SaoLineBufferMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.SaoLineBuffer.DW0_1.Graphicsaddress476 = m_hevcSaoRowStoreCache.dwAddress;
    }
    else if (params->presSaoLineBuffer != nullptr)
    {
        resourceParams.presResource = params->presSaoLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SaoLineBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 22;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // SAO Tile Line Buffer
    cmd.SaoTileLineBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

    if (params->presSaoTileLineBuffer != nullptr)
    {
        resourceParams.presResource = params->presSaoTileLineBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SaoTileLineBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 25;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // SAO Tile Column Buffer
    cmd.SaoTileColumnBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

    if (params->presSaoTileColumnBuffer != nullptr)
    {
        resourceParams.presResource = params->presSaoTileColumnBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SaoTileColumnBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 28;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Current Motion Vector Temporal Buffer
    cmd.CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MV_CODEC].Value;

    if (params->presCurMvTempBuffer != nullptr)
    {
        resourceParams.presResource = params->presCurMvTempBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CurrentMotionVectorTemporalBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 31;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Reference Picture Buffer
    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

    MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
    bool              firstRefPic = true;

    // NOTE: for both HEVC and VP9, set all the 8 ref pic addresses in HCP_PIPE_BUF_ADDR_STATE command to valid addresses for error concealment purpose
    for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        if (params->presReferences[i] != nullptr)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, params->presReferences[i], &details));

            if (firstRefPic)
            {
                MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, params->presReferences[i], &mmcMode));

                cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                firstRefPic = false;
            }

            resourceParams.presResource = params->presReferences[i];
            resourceParams.pdwCmd = (cmd.ReferencePictureBaseAddressRefaddr07[i].DW0_1.Value);
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = (i * 2) + 37; // * 2 to account for QW rather than DW
            resourceParams.bIsWritable = false;

            resourceParams.dwSharedMocsOffset = 53 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW53

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable =
        (mmcMode != MOS_MEMCOMP_DISABLED) ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode =
        (mmcMode == MOS_MEMCOMP_HORIZONTAL) ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = MOS_MFX_PIPE_BUF_ADDR;

    // Original Uncompressed Picture Source, Encoder only
    if (params->psRawSurface != nullptr)
    {
        MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &params->psRawSurface->OsResource, &mmcMode));

        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;
        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = (mmcMode != MOS_MEMCOMP_DISABLED) ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode = (mmcMode == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

        resourceParams.presResource = &params->psRawSurface->OsResource;
        resourceParams.dwOffset = params->psRawSurface->dwOffset;
        resourceParams.pdwCmd = (cmd.OriginalUncompressedPictureSource.DW0_1.Value);
        resourceParams.dwLocationInCmd = 54;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // StreamOut Data Destination, Decoder only
    if (params->presStreamOutBuffer != nullptr)
    {
        cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;
        cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionEnable =
            (params->StreamOutBufMmcState != MOS_MEMCOMP_DISABLED) ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.BaseAddressMemoryCompressionMode =
            (params->StreamOutBufMmcState == MOS_MEMCOMP_HORIZONTAL) ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        resourceParams.presResource = params->presStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.StreamoutDataDestination.DW0_1.Value);
        resourceParams.dwLocationInCmd = 57;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Decoded Picture Status / Error Buffer Base Address
    if (params->presLcuBaseAddressBuffer != nullptr)
    {
        cmd.DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC].Value;

        resourceParams.presResource = params->presLcuBaseAddressBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DecodedPictureStatusErrorBufferBaseAddressOrEncodedSliceSizeStreamoutBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 60;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // LCU ILDB StreamOut Buffer
    if (params->presLcuILDBStreamOutBuffer != nullptr)
    {
        cmd.LcuIldbStreamoutBufferMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_LCU_ILDB_STREAMOUT_CODEC].Value;

        resourceParams.presResource = params->presLcuILDBStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LcuIldbStreamoutBuffer.DW0_1.Value);
        resourceParams.dwLocationInCmd = 63;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Collocated Motion vector Temporal Buffer
    cmd.CollocatedMotionVectorTemporalBuffer07MemoryAddressAttributes.DW0.Value |=
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MV_CODEC].Value;

    for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
    {
        if (params->presColMvTempBuffer[i] != nullptr)
        {
            resourceParams.presResource = params->presColMvTempBuffer[i];
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.CollocatedMotionVectorTemporalBuffer07[i].DW0_1.Value);
            resourceParams.dwLocationInCmd = (i * 2) + 66;
            resourceParams.bIsWritable = false;

            resourceParams.dwSharedMocsOffset = 82 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW82

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = MOS_MFX_PIPE_BUF_ADDR;

    // VP9 Probability Buffer
    if (params->presVp9ProbBuffer != nullptr)
    {
        cmd.Vp9ProbabilityBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_PROBABILITY_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presVp9ProbBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.Vp9ProbabilityBufferReadWrite.DW0_1.Value);
        resourceParams.dwLocationInCmd = 83;
        resourceParams.bIsWritable = true;

        resourceParams.dwSharedMocsOffset = 85 - resourceParams.dwLocationInCmd;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // VP9 Segment Id Buffer
    if (params->presVp9SegmentIdBuffer != nullptr)
    {
        cmd.Vp9SegmentIdBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_SEGMENT_ID_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presVp9SegmentIdBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DW86_87.Value);
        resourceParams.dwLocationInCmd = 86;
        resourceParams.bIsWritable = true;

        resourceParams.dwSharedMocsOffset = 88 - resourceParams.dwLocationInCmd;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // HVD Line Row Store Buffer
    if (m_vp9HvdRowStoreCache.bEnabled)
    {
        cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Graphicsaddress476 = m_vp9HvdRowStoreCache.dwAddress;
    }
    else if (params->presHvdLineRowStoreBuffer != nullptr)
    {
        cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presHvdLineRowStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Value);
        resourceParams.dwLocationInCmd = 89;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // HVD Tile Row Store Buffer
    if (params->presHvdTileRowStoreBuffer != nullptr)
    {
        cmd.Vp9HvdTileRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC].Value;

        resourceParams.presResource = params->presHvdTileRowStoreBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.Vp9HvdTileRowstoreBufferReadWrite.DW0_1.Value);
        resourceParams.dwLocationInCmd = 92;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpIndObjBaseAddrCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    mhw_vdbox_hcp_g9_kbl::HCP_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

    // mode specific settings
    if (CodecHalIsDecodeModeVLD(params->Mode))
    {
        MHW_MI_CHK_NULL(params->presDataBuffer);

        cmd.HcpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Value;

        resourceParams.presResource = params->presDataBuffer;
        resourceParams.dwOffset = params->dwDataOffset;
        resourceParams.pdwCmd = (cmd.HcpIndirectBitstreamObjectBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.dwSize = params->dwDataSize;
        resourceParams.bIsWritable = false;

        // upper bound of the allocated resource will be set at 3 DW apart from address location
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

        MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
    }

    // following is for encoder
    if (!m_decodeInUse)
    {
        if (params->presMvObjectBuffer)
        {
            cmd.HcpIndirectCuObjectObjectMemoryAddressAttributes.DW0.Value |= 
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC].Value;

            resourceParams.presResource = params->presMvObjectBuffer;
            resourceParams.dwOffset = params->dwMvObjectOffset;
            resourceParams.pdwCmd = (cmd.DW6_7.Value);
            resourceParams.dwLocationInCmd = 6;
            resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwMvObjectSize, 0x1000);
            resourceParams.bIsWritable = false;

            // no upper bound for indirect CU object 
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presPakBaseObjectBuffer)
        {
            cmd.HcpPakBseObjectAddressMemoryAddressAttributes.DW0.Value |= 
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Value;

            resourceParams.presResource = params->presPakBaseObjectBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DW9_10.Value);
            resourceParams.dwLocationInCmd = 9;
            resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwPakBaseObjectSize, 0x1000);
            resourceParams.bIsWritable = true;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(pfnAddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpDecodePicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcPicParams);

    mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_kbl>::AddHcpDecodePicStateCmd(cmdBuffer, params));

    auto hevcPicParams = params->pHevcPicParams;

    cmd->DW5.BitDepthChromaMinus8   = hevcPicParams->bit_depth_chroma_minus8;
    cmd->DW5.BitDepthLumaMinus8     = hevcPicParams->bit_depth_luma_minus8;

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpEncodePicStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE       params)
{
    MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
    MHW_MI_CHK_NULL(params->pHevcEncPicParams);

    mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD cmd;

    auto hevcSeqParams = params->pHevcEncSeqParams;
    auto hevcPicParams = params->pHevcEncPicParams;

    cmd.DW1.Framewidthinmincbminus1      = hevcSeqParams->wFrameWidthInMinCbMinus1;

    /*  These two bits should reflect the same value to assigning together across DWORDs should reflect same value*/
    cmd.DW1.PakTransformSkipEnable       = 
    cmd.DW4.TransformSkipEnabledFlag     = hevcPicParams->transform_skip_enabled_flag;

    
    cmd.DW1.Frameheightinmincbminus1        = hevcSeqParams->wFrameHeightInMinCbMinus1;

    cmd.DW2.Mincusize                       = hevcSeqParams->log2_min_coding_block_size_minus3;
    cmd.DW2.CtbsizeLcusize                  = hevcSeqParams->log2_max_coding_block_size_minus3;
    cmd.DW2.Maxtusize                       = hevcSeqParams->log2_max_transform_block_size_minus2;
    cmd.DW2.Mintusize                       = hevcSeqParams->log2_min_transform_block_size_minus2;
    cmd.DW2.Minpcmsize                      = hevcSeqParams->log2_min_PCM_cb_size_minus3;
    cmd.DW2.Maxpcmsize                      = hevcSeqParams->log2_max_PCM_cb_size_minus3;

    cmd.DW3.Colpicisi                       = 0; 
    cmd.DW3.Curpicisi                       = 0; 

    cmd.DW4.SampleAdaptiveOffsetEnabledFlag         = params->bSAOEnable; // HW restriction, does not support SAO filtering for LCU size 16x16
    cmd.DW4.PcmEnabledFlag                          = hevcSeqParams->pcm_enabled_flag;
    cmd.DW4.CuQpDeltaEnabledFlag                    = hevcPicParams->cu_qp_delta_enabled_flag;
    cmd.DW4.DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth  = hevcPicParams->diff_cu_qp_delta_depth;
    cmd.DW4.PcmLoopFilterDisableFlag                = hevcSeqParams->pcm_loop_filter_disable_flag;
    cmd.DW4.ConstrainedIntraPredFlag                = 0; 
    cmd.DW4.Log2ParallelMergeLevelMinus2            = 0; 
    cmd.DW4.SignDataHidingFlag                      = 0; // currently not supported in encoder
    cmd.DW4.EntropyCodingSyncEnabledFlag            = 0; // not supported as per Dimas notes. PAK restriction
    cmd.DW4.TilesEnabledFlag                        = 0; // not supported in encoder
    cmd.DW4.WeightedPredFlag                        = hevcPicParams->weighted_pred_flag;
    cmd.DW4.WeightedBipredFlag                      = hevcPicParams->weighted_bipred_flag;
    cmd.DW4.Fieldpic                                = 0; 
    cmd.DW4.Bottomfield                             = 0; 
    cmd.DW4.AmpEnabledFlag                          = hevcSeqParams->amp_enabled_flag;
    cmd.DW4.TransquantBypassEnableFlag              = hevcPicParams->transquant_bypass_enabled_flag;
    cmd.DW4.StrongIntraSmoothingEnableFlag          = hevcSeqParams->strong_intra_smoothing_enable_flag;
    cmd.DW4.CuPacketStructure                       = 1 ; // 0 output from HW VME, 1/2 CL per CU, 1 NON-HwVME, Ext interface , 1 CL per CU

    cmd.DW5.PicCbQpOffset                                           = hevcPicParams->pps_cb_qp_offset & 0x1f;
    cmd.DW5.PicCrQpOffset                                           = hevcPicParams->pps_cr_qp_offset & 0x1f;
    cmd.DW5.MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra = hevcSeqParams->max_transform_hierarchy_depth_intra;
    cmd.DW5.MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter = hevcSeqParams->max_transform_hierarchy_depth_inter;
    cmd.DW5.PcmSampleBitDepthChromaMinus1                           = hevcSeqParams->pcm_sample_bit_depth_chroma_minus1;
    cmd.DW5.PcmSampleBitDepthLumaMinus1                             = hevcSeqParams->pcm_sample_bit_depth_luma_minus1;
    cmd.DW5.BitDepthChromaMinus8                                    = hevcSeqParams->bit_depth_chroma_minus8;
    cmd.DW5.BitDepthLumaMinus8                                      = hevcSeqParams->bit_depth_luma_minus8;

    cmd.DW6.LcuMaxBitsizeAllowed                            = hevcPicParams->LcuMaxBitsizeAllowed;
    cmd.DW6.Nonfirstpassflag                                = params->bNotFirstPass;
    cmd.DW6.LcumaxbitstatusenLcumaxsizereportmask           = 0;
    cmd.DW6.FrameszoverstatusenFramebitratemaxreportmask    = 0;
    cmd.DW6.FrameszunderstatusenFramebitrateminreportmask   = 0;
    cmd.DW6.LoadSlicePointerFlag                            = 0; // must be set to 0 for encoder

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpDecodeSliceStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcPicParams);
    MHW_MI_CHK_NULL(hevcSliceState->pHevcSliceParams);

    auto hevcPicParams = hevcSliceState->pHevcPicParams;
    auto hevcSliceParams = hevcSliceState->pHevcSliceParams;

    mhw_vdbox_hcp_g9_kbl::HCP_SLICE_STATE_CMD  *cmd =
        (mhw_vdbox_hcp_g9_kbl::HCP_SLICE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g9_kbl>::AddHcpDecodeSliceStateCmd(cmdBuffer, hevcSliceState));

    int32_t sliceQP = hevcSliceParams->slice_qp_delta + hevcPicParams->init_qp_minus26 + 26;
    cmd->DW3.SliceqpSignFlag = (sliceQP >= 0) ? 0 : 1;
    cmd->DW3.Sliceqp = ABS(sliceQP);

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpEncodeSliceStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hevcSliceState);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcSliceParams);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcPicParams);
    MHW_MI_CHK_NULL(hevcSliceState->pEncodeHevcSeqParams);

    mhw_vdbox_hcp_g9_kbl::HCP_SLICE_STATE_CMD   cmd;

    auto hevcSliceParams                    = hevcSliceState->pEncodeHevcSliceParams;
    auto hevcPicParams                      = hevcSliceState->pEncodeHevcPicParams;
    auto hevcSeqParams                      = hevcSliceState->pEncodeHevcSeqParams;

    uint32_t ctbSize    = 1 << (hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
    uint32_t widthInPix = (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) *
                          (hevcSeqParams->wFrameWidthInMinCbMinus1 + 1);
    uint32_t widthInCtb = (widthInPix / ctbSize) +
                          ((widthInPix % ctbSize) ? 1 : 0);  // round up

    uint32_t ctbAddr    = hevcSliceParams->slice_segment_address;
    cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder       = ctbAddr % widthInCtb;
    cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder       = ctbAddr / widthInCtb;

    
    ctbAddr                                                     = hevcSliceParams->slice_segment_address + hevcSliceParams->NumLCUsInSlice;
    cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder       = ctbAddr % widthInCtb;
    cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder       = ctbAddr / widthInCtb;
    

    cmd.DW3.SliceType                = hevcSliceParams->slice_type;
    cmd.DW3.Lastsliceofpic           = hevcSliceState->bLastSlice;
    cmd.DW3.SliceqpSignFlag          = ((hevcSliceParams->slice_qp_delta + hevcPicParams->QpY) >= 0) 
                                               ? 0 : 1; //8 bit will have 0 as sign bit adn 10 bit might have 1 as sign bit depending on Qp
    
    cmd.DW3.DependentSliceFlag          = 0; // Not supported on encoder
    cmd.DW3.SliceTemporalMvpEnableFlag  = hevcSliceParams->slice_temporal_mvp_enable_flag;
    cmd.DW3.Sliceqp                     = hevcSliceParams->slice_qp_delta + hevcPicParams->QpY;
    cmd.DW3.SliceCbQpOffset             = hevcSliceParams->slice_cb_qp_offset;
    cmd.DW3.SliceCrQpOffset             = hevcSliceParams->slice_cr_qp_offset;

    cmd.DW4.SliceHeaderDisableDeblockingFilterFlag              = hevcSliceParams->slice_deblocking_filter_disable_flag;
    cmd.DW4.SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder         = hevcSliceParams->tc_offset_div2;
    cmd.DW4.SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder     = hevcSliceParams->beta_offset_div2;
    cmd.DW4.SliceLoopFilterAcrossSlicesEnabledFlag              = 0;
    cmd.DW4.SliceSaoChromaFlag                                  = hevcSliceParams->slice_sao_chroma_flag;
    cmd.DW4.SliceSaoLumaFlag                                    = hevcSliceParams->slice_sao_luma_flag;
    cmd.DW4.MvdL1ZeroFlag                                       = 0; // Decoder only - set to 0 for encoder
    cmd.DW4.Islowdelay                                          = hevcSliceState->bIsLowDelay;
    cmd.DW4.CollocatedFromL0Flag                                = hevcSliceParams->collocated_from_l0_flag;
    cmd.DW4.Chromalog2Weightdenom                               = hevcSliceParams->luma_log2_weight_denom + hevcSliceParams->delta_chroma_log2_weight_denom;
    cmd.DW4.LumaLog2WeightDenom                                 = hevcSliceParams->luma_log2_weight_denom;
    cmd.DW4.CabacInitFlag                                       = hevcSliceParams->cabac_init_flag;
    cmd.DW4.Maxmergeidx                                         = hevcSliceParams->MaxNumMergeCand - 1;

    if (cmd.DW3.SliceTemporalMvpEnableFlag)
    {
        if (cmd.DW3.SliceType == hevcSliceI)
        {
            cmd.DW4.Collocatedrefidx = 0;
        }
        else
        {
            // need to check with Ce for DDI issues
            uint8_t collocatedFromL0Flag      = cmd.DW4.CollocatedFromL0Flag;

            uint8_t collocatedRefIndex        = hevcPicParams->CollocatedRefPicIndex;
            MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

            uint8_t collocatedFrameIdx        = hevcSliceState->pRefIdxMapping[collocatedRefIndex];
            MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

            cmd.DW4.Collocatedrefidx = collocatedFrameIdx;
        }
    }
    else
    {
        cmd.DW4.Collocatedrefidx     = 0;
    }

    cmd.DW5.Sliceheaderlength        = 0; // Decoder only, setting to 0 for Encoder

    if (!hevcPicParams->bUsedAsRef && hevcPicParams->CodingType != I_TYPE)
    {
        // non reference B frame
        cmd.DW6.Roundinter = 0;
        cmd.DW6.Roundintra = 8;
    }
    else
    {
        //Other frames
        cmd.DW6.Roundinter = 5;
        cmd.DW6.Roundintra = 11;
    }

    cmd.DW7.Cabaczerowordinsertionenable
                                        = 1;
    cmd.DW7.Emulationbytesliceinsertenable
                                        = 1;
    cmd.DW7.TailInsertionEnable      = (hevcPicParams->bLastPicInSeq || hevcPicParams->bLastPicInStream) && hevcSliceState->bLastSlice;
    cmd.DW7.SlicedataEnable          = 1;
    cmd.DW7.HeaderInsertionEnable    = 1;

    cmd.DW8.IndirectPakBseDataStartOffsetWrite
                                        = hevcSliceState->dwHeaderBytesInserted;

    // Transform skip related parameters
    if (hevcPicParams->transform_skip_enabled_flag)
    {
        cmd.DW9.TransformskipLambda                     = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_lambda;
        cmd.DW10.TransformskipNumzerocoeffsFactor0      = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numzerocoeffs_Factor0;
        cmd.DW10.TransformskipNumnonzerocoeffsFactor0   = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numnonzerocoeffs_Factor0;
        cmd.DW10.TransformskipNumzerocoeffsFactor1      = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numzerocoeffs_Factor1;
        cmd.DW10.TransformskipNumnonzerocoeffsFactor1   = hevcSliceState->EncodeHevcTransformSkipParams.Transformskip_Numnonzerocoeffs_Factor1;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, hevcSliceState->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpVp9PicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_PIC_STATE         params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pVp9PicParams);

    mhw_vdbox_hcp_g9_kbl::HCP_VP9_PIC_STATE_CMD cmd;
    auto vp9PicParams = params->pVp9PicParams;
    auto vp9RefList = params->ppVp9RefList;
    
    cmd.DW0.DwordLength                  = mhw_vdbox_hcp_g9_kbl::GetOpLength(12); //VP9_PIC_STATE command is common for both Decoder and Encoder. Decoder uses only 12 DWORDS of the generated 33 DWORDS

    uint32_t curFrameWidth               = vp9PicParams->FrameWidthMinus1 + 1;
    uint32_t curFrameHeight              = vp9PicParams->FrameHeightMinus1 + 1;
    bool isScaling                       = (curFrameWidth == params->dwPrevFrmWidth) && (curFrameHeight == params->dwPrevFrmHeight) ? false : true;

    cmd.DW1.FrameWidthInPixelsMinus1     = MOS_ALIGN_CEIL(curFrameWidth, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
    cmd.DW1.FrameHeightInPixelsMinus1    = MOS_ALIGN_CEIL(curFrameHeight, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

    cmd.DW2.FrameType                    = vp9PicParams->PicFlags.fields.frame_type;
    cmd.DW2.AdaptProbabilitiesFlag       = !vp9PicParams->PicFlags.fields.error_resilient_mode && !vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.IntraonlyFlag                = vp9PicParams->PicFlags.fields.intra_only;
    cmd.DW2.RefreshFrameContext          = vp9PicParams->PicFlags.fields.refresh_frame_context;
    cmd.DW2.ErrorResilientMode           = vp9PicParams->PicFlags.fields.error_resilient_mode;
    cmd.DW2.FrameParallelDecodingMode    = vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.FilterLevel                  = vp9PicParams->filter_level;
    cmd.DW2.SharpnessLevel               = vp9PicParams->sharpness_level;
    cmd.DW2.SegmentationEnabled          = vp9PicParams->PicFlags.fields.segmentation_enabled;
    cmd.DW2.SegmentationUpdateMap        = cmd.DW2.SegmentationEnabled && vp9PicParams->PicFlags.fields.segmentation_update_map;
    cmd.DW2.LosslessMode                 = vp9PicParams->PicFlags.fields.LosslessFlag;
    cmd.DW2.SegmentIdStreamoutEnable     = cmd.DW2.SegmentationUpdateMap;

    uint8_t segmentIDStreaminEnable = 0;
    if (vp9PicParams->PicFlags.fields.intra_only ||
        (vp9PicParams->PicFlags.fields.frame_type == CODEC_VP9_KEY_FRAME)) {
        segmentIDStreaminEnable = 1;
    } else if (vp9PicParams->PicFlags.fields.segmentation_enabled) {
        if (!vp9PicParams->PicFlags.fields.segmentation_update_map)
            segmentIDStreaminEnable = 1;
        else if (vp9PicParams->PicFlags.fields.segmentation_temporal_update)
            segmentIDStreaminEnable = 1;
    }
    if (vp9PicParams->PicFlags.fields.error_resilient_mode) {
            segmentIDStreaminEnable = 1;
    }
    // Resolution change will reset the segment ID buffer
    if (isScaling)
    {
        segmentIDStreaminEnable = 1;
    }

    cmd.DW2.SegmentIdStreaminEnable       = segmentIDStreaminEnable;

    cmd.DW3.Log2TileRow             = vp9PicParams->log2_tile_rows;        // No need to minus 1 here.
    cmd.DW3.Log2TileColumn          = vp9PicParams->log2_tile_columns;     // No need to minus 1 here.
    if (vp9PicParams->subsampling_x == 1 && vp9PicParams->subsampling_y == 1)
    {
        //4:2:0
        cmd.DW3.ChromaSamplingFormat = 0;
    }
    else if (vp9PicParams->subsampling_x == 1 && vp9PicParams->subsampling_y == 0)
    {
        //4:2:2
        cmd.DW3.ChromaSamplingFormat = 1;
    }
    else if (vp9PicParams->subsampling_x == 0 && vp9PicParams->subsampling_y == 0)
    {
        //4:4:4
        cmd.DW3.ChromaSamplingFormat = 2;
    }
    cmd.DW3.Bitdepthminus8 = vp9PicParams->BitDepthMinus8;
    cmd.DW3.ProfileLevel   = vp9PicParams->profile;
    
    cmd.DW10.UncompressedHeaderLengthInBytes70  = vp9PicParams->UncompressedHeaderLengthInBytes;
    cmd.DW10.FirstPartitionSizeInBytes150       = vp9PicParams->FirstPartitionSize;

    if (vp9PicParams->PicFlags.fields.frame_type && !vp9PicParams->PicFlags.fields.intra_only)
    {
        PCODEC_PICTURE refFrameList     = &(vp9PicParams->RefFrameList[0]);

        uint8_t lastRefPicIndex         = refFrameList[vp9PicParams->PicFlags.fields.LastRefIdx].FrameIdx;
        uint32_t lastRefFrameWidth      = vp9RefList[lastRefPicIndex]->dwFrameWidth;
        uint32_t lastRefFrameHeight     = vp9RefList[lastRefPicIndex]->dwFrameHeight;

        uint8_t goldenRefPicIndex       = refFrameList[vp9PicParams->PicFlags.fields.GoldenRefIdx].FrameIdx;
        uint32_t goldenRefFrameWidth    = vp9RefList[goldenRefPicIndex]->dwFrameWidth;
        uint32_t goldenRefFrameHeight   = vp9RefList[goldenRefPicIndex]->dwFrameHeight;

        uint8_t altRefPicIndex          = refFrameList[vp9PicParams->PicFlags.fields.AltRefIdx].FrameIdx;
        uint32_t altRefFrameWidth       = vp9RefList[altRefPicIndex]->dwFrameWidth;
        uint32_t altRefFrameHeight      = vp9RefList[altRefPicIndex]->dwFrameHeight;

        cmd.DW2.AllowHiPrecisionMv              = vp9PicParams->PicFlags.fields.allow_high_precision_mv;
        cmd.DW2.McompFilterType                 = vp9PicParams->PicFlags.fields.mcomp_filter_type;
        cmd.DW2.SegmentationTemporalUpdate      = cmd.DW2.SegmentationUpdateMap && vp9PicParams->PicFlags.fields.segmentation_temporal_update;

        cmd.DW2.RefFrameSignBias02              = vp9PicParams->PicFlags.fields.LastRefSignBias | 
                                                  (vp9PicParams->PicFlags.fields.GoldenRefSignBias << 1) | 
                                                  (vp9PicParams->PicFlags.fields.AltRefSignBias << 2);

        cmd.DW2.LastFrameType                   = !params->PrevFrameParams.fields.KeyFrame;

        // Reset UsePrevInFindMvReferences to zero if last picture has a different size,
        // Current picture is error-resilient mode, Last picture was intra_only or keyframe,
        // Last picture was not a displayed picture.
        cmd.DW2.UsePrevInFindMvReferences       =
                 !(vp9PicParams->PicFlags.fields.error_resilient_mode ||
                 params->PrevFrameParams.fields.KeyFrame ||
                 params->PrevFrameParams.fields.IntraOnly ||
                 !params->PrevFrameParams.fields.Display);
        // Reset UsePrevInFindMvReferences in case of resolution change on inter frames
        if (isScaling) {
            cmd.DW2.UsePrevInFindMvReferences   = 0;
        }

        cmd.DW4.HorizontalScaleFactorForLast    = (lastRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW4.VerticalScaleFactorForLast      = (lastRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW5.HorizontalScaleFactorForGolden  = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW5.VerticalScaleFactorForGolden    = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW6.HorizontalScaleFactorForAltref  = (altRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
        cmd.DW6.VerticalScaleFactorForAltref    = (altRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

        cmd.DW7.LastFrameWidthInPixelsMinus1    = lastRefFrameWidth - 1;
        cmd.DW7.LastFrameHieghtInPixelsMinus1   = lastRefFrameHeight - 1;

        cmd.DW8.GoldenFrameWidthInPixelsMinus1  = goldenRefFrameWidth - 1;
        cmd.DW8.GoldenFrameHieghtInPixelsMinus1 = goldenRefFrameHeight - 1;

        cmd.DW9.AltrefFrameWidthInPixelsMinus1  = altRefFrameWidth - 1;
        cmd.DW9.AltrefFrameHieghtInPixelsMinus1 = altRefFrameHeight - 1;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpVp9PicStateEncCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    PMHW_VDBOX_VP9_ENCODE_PIC_STATE params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pVp9PicParams);
    MHW_MI_CHK_NULL(params->ppVp9RefList);

    mhw_vdbox_hcp_g9_kbl::HCP_VP9_PIC_STATE_CMD cmd;

    auto vp9PicParams = params->pVp9PicParams;
    auto vp9RefList = params->ppVp9RefList;

    cmd.DW1.FrameWidthInPixelsMinus1    = vp9PicParams->SrcFrameWidthMinus1;
    cmd.DW1.FrameHeightInPixelsMinus1   = vp9PicParams->SrcFrameHeightMinus1;

    cmd.DW2.FrameType                   = vp9PicParams->PicFlags.fields.frame_type;
    cmd.DW2.AdaptProbabilitiesFlag      = !vp9PicParams->PicFlags.fields.error_resilient_mode && !vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.IntraonlyFlag               = vp9PicParams->PicFlags.fields.intra_only;
    cmd.DW2.AllowHiPrecisionMv          = vp9PicParams->PicFlags.fields.allow_high_precision_mv;
    cmd.DW2.McompFilterType             = vp9PicParams->PicFlags.fields.mcomp_filter_type;

    cmd.DW2.RefFrameSignBias02          = vp9PicParams->RefFlags.fields.LastRefSignBias | 
                                          (vp9PicParams->RefFlags.fields.GoldenRefSignBias << 1) | 
                                          (vp9PicParams->RefFlags.fields.AltRefSignBias << 2);

    cmd.DW2.HybridPredictionMode        = vp9PicParams->PicFlags.fields.comp_prediction_mode == 2;
    cmd.DW2.SelectableTxMode            = params->ucTxMode == 4;
    cmd.DW2.RefreshFrameContext         = vp9PicParams->PicFlags.fields.refresh_frame_context;
    cmd.DW2.ErrorResilientMode          = vp9PicParams->PicFlags.fields.error_resilient_mode;
    cmd.DW2.FrameParallelDecodingMode   = vp9PicParams->PicFlags.fields.frame_parallel_decoding_mode;
    cmd.DW2.FilterLevel                 = vp9PicParams->filter_level;
    cmd.DW2.SharpnessLevel              = vp9PicParams->sharpness_level;
    cmd.DW2.SegmentationEnabled         = vp9PicParams->PicFlags.fields.segmentation_enabled;
    cmd.DW2.SegmentationUpdateMap       = vp9PicParams->PicFlags.fields.segmentation_update_map;
    cmd.DW2.SegmentationTemporalUpdate  = vp9PicParams->PicFlags.fields.segmentation_temporal_update;
    cmd.DW2.LosslessMode                = vp9PicParams->PicFlags.fields.LosslessFlag;

    cmd.DW3.Log2TileColumn         = vp9PicParams->log2_tile_columns;
    cmd.DW3.Log2TileRow            = vp9PicParams->log2_tile_rows;
   
    if (vp9PicParams->PicFlags.fields.frame_type && !vp9PicParams->PicFlags.fields.intra_only)
    {
        uint32_t curFrameWidth         = vp9PicParams->SrcFrameWidthMinus1 + 1;
        uint32_t curFrameHeight        = vp9PicParams->SrcFrameHeightMinus1 + 1;
     
        PCODEC_PICTURE refFrameList    = &(vp9PicParams->RefFrameList[0]);

        cmd.DW2.LastFrameType   = !params->PrevFrameParams.fields.KeyFrame;

        cmd.DW2.UsePrevInFindMvReferences = vp9PicParams->PicFlags.fields.error_resilient_mode ||
            params->PrevFrameParams.fields.KeyFrame ||
            params->PrevFrameParams.fields.IntraOnly ||
            !params->PrevFrameParams.fields.Display ||
            (curFrameWidth != params->dwPrevFrmWidth) ||
            (curFrameHeight != params->dwPrevFrmHeight) ? 0 : 1;

        if ((vp9PicParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x01) || (vp9PicParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x01))
        {
            MHW_ASSERT(!CodecHal_PictureIsInvalid(refFrameList[vp9PicParams->RefFlags.fields.LastRefIdx]));

            uint8_t  lastRefPicIndex       = refFrameList[vp9PicParams->RefFlags.fields.LastRefIdx].FrameIdx;
            uint32_t lastRefFrameWidth     = vp9RefList[lastRefPicIndex]->dwFrameWidth;
            uint32_t lastRefFrameHeight    = vp9RefList[lastRefPicIndex]->dwFrameHeight;

            cmd.DW4.HorizontalScaleFactorForLast    = (lastRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            cmd.DW4.VerticalScaleFactorForLast      = (lastRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            cmd.DW7.LastFrameWidthInPixelsMinus1    = lastRefFrameWidth - 1;
            cmd.DW7.LastFrameHieghtInPixelsMinus1   = lastRefFrameHeight - 1;
        }

        if ((vp9PicParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x02) || (vp9PicParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x02))
        {
            MHW_ASSERT(!CodecHal_PictureIsInvalid(refFrameList[vp9PicParams->RefFlags.fields.GoldenRefIdx]));

            uint8_t  goldenRefPicIndex     = refFrameList[vp9PicParams->RefFlags.fields.GoldenRefIdx].FrameIdx;
            uint32_t goldenRefFrameWidth   = vp9RefList[goldenRefPicIndex]->dwFrameWidth;
            uint32_t goldenRefFrameHeight = vp9RefList[goldenRefPicIndex]->dwFrameHeight;

            cmd.DW5.HorizontalScaleFactorForGolden  = (goldenRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            cmd.DW5.VerticalScaleFactorForGolden    = (goldenRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            cmd.DW8.GoldenFrameWidthInPixelsMinus1  = goldenRefFrameWidth - 1;
            cmd.DW8.GoldenFrameHieghtInPixelsMinus1 = goldenRefFrameHeight - 1;
        }

        if ((vp9PicParams->RefFlags.fields.ref_frame_ctrl_l0 & 0x04) || (vp9PicParams->RefFlags.fields.ref_frame_ctrl_l1 & 0x04))
        {
            MHW_ASSERT(!CodecHal_PictureIsInvalid(refFrameList[vp9PicParams->RefFlags.fields.AltRefIdx]));

            uint8_t  altRefPicIndex    = refFrameList[vp9PicParams->RefFlags.fields.AltRefIdx].FrameIdx;
            uint32_t altRefFrameWidth  = vp9RefList[altRefPicIndex]->dwFrameWidth;
            uint32_t altRefFrameHeight = vp9RefList[altRefPicIndex]->dwFrameHeight;

            cmd.DW6.HorizontalScaleFactorForAltref  = (altRefFrameWidth * m_vp9ScalingFactor) / curFrameWidth;
            cmd.DW6.VerticalScaleFactorForAltref    = (altRefFrameHeight * m_vp9ScalingFactor) / curFrameHeight;

            cmd.DW9.AltrefFrameWidthInPixelsMinus1  = altRefFrameWidth - 1;
            cmd.DW9.AltrefFrameHieghtInPixelsMinus1 = altRefFrameHeight - 1;
        }
    }

    cmd.DW13.BaseQIndexSameAsLumaAc = vp9PicParams->LumaACQIndex;

    cmd.DW14.ChromaacQindexdelta    = Convert2SignMagnitude(vp9PicParams->ChromaACQIndexDelta, 5);
    cmd.DW14.ChromadcQindexdelta    = Convert2SignMagnitude(vp9PicParams->ChromaDCQIndexDelta, 5);
    cmd.DW14.LumaDcQIndexDelta      = Convert2SignMagnitude(vp9PicParams->LumaDCQIndexDelta, 5);

    cmd.DW15.LfRefDelta0            = Convert2SignMagnitude(vp9PicParams->LFRefDelta[0], 7);
    cmd.DW15.LfRefDelta1            = Convert2SignMagnitude(vp9PicParams->LFRefDelta[1], 7);
    cmd.DW15.LfRefDelta2            = Convert2SignMagnitude(vp9PicParams->LFRefDelta[2], 7);
    cmd.DW15.LfRefDelta3            = Convert2SignMagnitude(vp9PicParams->LFRefDelta[3], 7);

    cmd.DW16.LfModeDelta0           = Convert2SignMagnitude(vp9PicParams->LFModeDelta[0], 7);
    cmd.DW16.LfModeDelta0           = Convert2SignMagnitude(vp9PicParams->LFModeDelta[1], 7);

    cmd.DW17.Bitoffsetforlfrefdelta     = vp9PicParams->BitOffsetForLFRefDelta;
    cmd.DW17.Bitoffsetforlfmodedelta    = vp9PicParams->BitOffsetForLFModeDelta;

    cmd.DW18.Bitoffsetforlflevel        = vp9PicParams->BitOffsetForLFLevel;
    cmd.DW18.Bitoffsetforqindex         = vp9PicParams->BitOffsetForQIndex;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpVp9SegmentStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    PMHW_VDBOX_VP9_SEGMENT_STATE     params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    typename mhw_vdbox_hcp_g9_kbl::HCP_VP9_SEGMENT_STATE_CMD  cmd;
    void*  segData = nullptr;

    cmd.DW1.SegmentId = params->ucCurrentSegmentId;

    if (!this->m_decodeInUse)
    {
        CODEC_VP9_ENCODE_SEG_PARAMS             vp9SegData;

        vp9SegData = params->pVp9EncodeSegmentParams->SegData[params->ucCurrentSegmentId];

        if (params->pbSegStateBufferPtr)   // Use the seg data from this buffer (output of BRC)
        {
            segData = params->pbSegStateBufferPtr;
        }
        else    // Prepare the seg data
        {
            cmd.DW2.SegmentSkipped = vp9SegData.SegmentFlags.fields.SegmentSkipped;
            cmd.DW2.SegmentReference = vp9SegData.SegmentFlags.fields.SegmentReference;
            cmd.DW2.SegmentReferenceEnabled = vp9SegData.SegmentFlags.fields.SegmentReferenceEnabled;

            segData = &cmd;
        }
    }
    else
    {
        CODEC_VP9_SEG_PARAMS            vp9SegData;
        vp9SegData = params->pVp9SegmentParams->SegData[params->ucCurrentSegmentId];

        cmd.DW2.SegmentSkipped = vp9SegData.SegmentFlags.fields.SegmentReferenceSkipped;
        cmd.DW2.SegmentReference = vp9SegData.SegmentFlags.fields.SegmentReference;
        cmd.DW2.SegmentReferenceEnabled = vp9SegData.SegmentFlags.fields.SegmentReferenceEnabled;

        cmd.DW3.Filterlevelref0Mode0 = vp9SegData.FilterLevel[0][0];
        cmd.DW3.Filterlevelref0Mode1 = vp9SegData.FilterLevel[0][1];
        cmd.DW3.Filterlevelref1Mode0 = vp9SegData.FilterLevel[1][0];
        cmd.DW3.Filterlevelref1Mode1 = vp9SegData.FilterLevel[1][1];

        cmd.DW4.Filterlevelref2Mode0 = vp9SegData.FilterLevel[2][0];
        cmd.DW4.Filterlevelref2Mode1 = vp9SegData.FilterLevel[2][1];
        cmd.DW4.Filterlevelref3Mode0 = vp9SegData.FilterLevel[3][0];
        cmd.DW4.Filterlevelref3Mode1 = vp9SegData.FilterLevel[3][1];

        cmd.DW5.LumaDcQuantScaleDecodeModeOnly = vp9SegData.LumaDCQuantScale;
        cmd.DW5.LumaAcQuantScaleDecodeModeOnly = vp9SegData.LumaACQuantScale;

        cmd.DW6.ChromaDcQuantScaleDecodeModeOnly = vp9SegData.ChromaDCQuantScale;
        cmd.DW6.ChromaAcQuantScaleDecodeModeOnly = vp9SegData.ChromaACQuantScale;

        segData = &cmd;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, segData, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterfaceG9Kbl::AddHcpHevcPicBrcBuffer(
    PMOS_RESOURCE                   hcpImgStates,
    PMHW_VDBOX_HEVC_PIC_STATE        hevcSliceState)
{
    MOS_STATUS                                  eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(hcpImgStates);

    MOS_COMMAND_BUFFER                      constructedCmdBuf;
    mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD cmd;
    uint32_t*                               insertion = nullptr;
    MOS_LOCK_PARAMS                         lockFlags;
    m_brcNumPakPasses = hevcSliceState->brcNumPakPasses;

    MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
    lockFlags.WriteOnly = 1;
    uint8_t* data = (uint8_t*)m_osInterface->pfnLockResource(m_osInterface, hcpImgStates, &lockFlags);
    MHW_MI_CHK_NULL(data);

    constructedCmdBuf.pCmdBase   = (uint32_t *)data;
    constructedCmdBuf.pCmdPtr    = (uint32_t *)data;
    constructedCmdBuf.iOffset    = 0;
    constructedCmdBuf.iRemaining = BRC_IMG_STATE_SIZE_PER_PASS * (m_brcNumPakPasses);

    MHW_MI_CHK_STATUS(AddHcpPicStateCmd(&constructedCmdBuf, hevcSliceState));

    cmd = *(mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD *)data;

    for (uint32_t i = 0; i < m_brcNumPakPasses; i++)
    {
        if (i == 0)
        {
            cmd.DW6.Nonfirstpassflag = false;
        }
        else
        {
            cmd.DW6.Nonfirstpassflag = true;
        }

        cmd.DW6.FrameszoverstatusenFramebitratemaxreportmask  = true;
        cmd.DW6.FrameszunderstatusenFramebitrateminreportmask = true;
        cmd.DW6.LcumaxbitstatusenLcumaxsizereportmask         = false; // BRC update kernel does not consider if there is any LCU whose size is too big

        *(mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD *)data    = cmd;

        /* add batch buffer end insertion flag */
        insertion = (uint32_t*)(data + mhw_vdbox_hcp_g9_kbl::HCP_PIC_STATE_CMD::byteSize);
        *insertion = 0x05000000;

        data += BRC_IMG_STATE_SIZE_PER_PASS;
    }

    m_osInterface->pfnUnlockResource(m_osInterface, hcpImgStates);

    return eStatus;
}
