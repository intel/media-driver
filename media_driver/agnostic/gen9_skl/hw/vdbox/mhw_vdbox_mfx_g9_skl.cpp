/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mhw_vdbox_mfx_g9_skl.cpp
//! \brief    Defines functions for constructing Vdbox MFX commands on G9 SKL
//!

#include "mos_os_cp_interface_specific.h"
#include "mhw_vdbox_mfx_g9_skl.h"

MOS_STATUS MhwVdboxMfxInterfaceG9Skl::AddMfxPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

    mhw_vdbox_mfx_g9_skl::MFX_PIPE_BUF_ADDR_STATE_CMD cmd;

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        UserFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED_ID;
        if (m_intraRowstoreCache.bEnabled               ||
            m_deblockingFilterRowstoreCache.bEnabled    ||
            m_bsdMpcRowstoreCache.bEnabled              ||
            m_mprRowstoreCache.bEnabled)
        {
            UserFeatureWriteData.Value.i32Data = 1;
        }
        MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, m_osInterface->pOsContext);
#endif

    // Encoding uses both surfaces regardless of deblocking status
    if (params->psPreDeblockSurface != nullptr)
    {
        cmd.DW3.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Value;
        cmd.DW3.PreDeblockingMemoryCompressionEnable = (params->PreDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW3.PreDeblockingMemoryCompressionMode = (params->PreDeblockSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        cmd.DW3.PreDeblockingTiledResourceMode = Mhw_ConvertToTRMode(params->psPreDeblockSurface->TileType);

        resourceParams.presResource = &(params->psPreDeblockSurface->OsResource);
        resourceParams.dwOffset = params->psPreDeblockSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW1.Value);
        resourceParams.dwLocationInCmd = 1;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->PreDeblockSurfMmcState));
    }

    if (params->psPostDeblockSurface != nullptr)
    {
        cmd.DW6.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_POST_DEBLOCKING_CODEC].Value;
        cmd.DW6.PostDeblockingMemoryCompressionEnable = (params->PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW6.PostDeblockingMemoryCompressionMode = (params->PostDeblockSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        cmd.DW6.PostDeblockingTiledResourceMode = Mhw_ConvertToTRMode(params->psPostDeblockSurface->TileType);

        resourceParams.presResource = &(params->psPostDeblockSurface->OsResource);
        resourceParams.dwOffset = params->psPostDeblockSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW4.Value);
        resourceParams.dwLocationInCmd = 4;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->PostDeblockSurfMmcState));
    }

    if (params->psRawSurface != nullptr)
    {
        if (!m_decodeInUse)
        {
            MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &params->psRawSurface->OsResource, &mmcMode));

            cmd.DW9.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;
            cmd.DW9.OriginalUncompressedPictureMemoryCompressionEnable =
                mmcMode != MOS_MEMCOMP_DISABLED ? MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
            cmd.DW9.OriginalUncompressedPictureMemoryCompressionMode =
                mmcMode == MOS_MEMCOMP_HORIZONTAL ? MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;
        }
        else
        {
            cmd.DW9.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE].Value;
        }

        cmd.DW9.OriginalUncompressedPictureTiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

        resourceParams.presResource = &params->psRawSurface->OsResource;
        resourceParams.dwOffset = params->psRawSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW7.Value);
        resourceParams.dwLocationInCmd = 7;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presStreamOutBuffer != nullptr)
    {
        cmd.DW12.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

        cmd.DW12.StreamoutDataDestinationMemoryCompressionEnable = params->StreamOutBufMmcState != MOS_MEMCOMP_DISABLED ?
            MHW_MEDIA_MEMCOMP_ENABLED : MHW_MEDIA_MEMCOMP_DISABLED;
        cmd.DW12.StreamoutDataDestinationMemoryCompressionMode = params->StreamOutBufMmcState == MOS_MEMCOMP_HORIZONTAL ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;

        resourceParams.presResource = params->presStreamOutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW10.Value);
        resourceParams.dwLocationInCmd = 10;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        if (!m_decodeInUse)
        {
            cmd.DW54.MemoryObjectControlState =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

            resourceParams.presResource = params->presStreamOutBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = &(cmd.DW52.Value);
            resourceParams.dwLocationInCmd = 52;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    if (m_intraRowstoreCache.bEnabled)
    {
        cmd.DW15.IntraRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
        cmd.DW13.IntraRowStoreScratchBufferBaseAddress = m_intraRowstoreCache.dwAddress;
    }
    else if (params->presMfdIntraRowStoreScratchBuffer != nullptr)
    {
        cmd.DW15.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

        cmd.DW15.IntraRowStoreScratchBufferMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;

        resourceParams.presResource = params->presMfdIntraRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW13.Value);
        resourceParams.dwLocationInCmd = 13;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (m_deblockingFilterRowstoreCache.bEnabled)
    {
        cmd.DW18.DeblockingFilterRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
        cmd.DW16.DeblockingFilterRowStoreScratchBaseAddress =
            m_deblockingFilterRowstoreCache.dwAddress;
    }
    else if (params->presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
    {
        cmd.DW18.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;
        cmd.DW18.DeblockingFilterRowStoreScratchMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;

        resourceParams.presResource = params->presMfdDeblockingFilterRowStoreScratchBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW16.Value);
        resourceParams.dwLocationInCmd = 16;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    bool firstRefPic = true;
    for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
    {
        if (params->presReferences[i] != nullptr)
        {
            MOS_SURFACE resDetails;
            MOS_ZeroMemory(&resDetails, sizeof(resDetails));
            resDetails.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, params->presReferences[i], &resDetails));

            MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, params->presReferences[i], &mmcMode));

            if (mmcMode == MOS_MEMCOMP_HORIZONTAL)
            {
                cmd.DW61.Value |= (MHW_MEDIA_MEMCOMP_ENABLED << (i * 2)) | (MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL << (i * 2 + 1));
            }
            else if (mmcMode == MOS_MEMCOMP_VERTICAL)
            {
                cmd.DW61.Value |= (MHW_MEDIA_MEMCOMP_ENABLED << (i * 2)) | (MHW_MEDIA_MEMCOMP_MODE_VERTICAL << (i * 2 + 1));
            }
            else
            {
                cmd.DW61.Value |= MHW_MEDIA_MEMCOMP_DISABLED << (i * 2);
            }

            if (firstRefPic)
            {
                cmd.DW51.ReferencePictureTiledResourceMode = Mhw_ConvertToTRMode(resDetails.TileType);
                firstRefPic = false;
            }

            resourceParams.presResource = params->presReferences[i];
            resourceParams.dwOffset = resDetails.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.pdwCmd = &(cmd.Refpicbaseaddr[i].DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = (i * 2) + 19; // * 2 to account for QW rather than DW
            resourceParams.bIsWritable = false;

            resourceParams.dwSharedMocsOffset = 51 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW51

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    // There is only one control DW51 for all references
    cmd.DW51.MemoryObjectControlState =
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;

    if (params->presMacroblockIldbStreamOutBuffer1 != nullptr)
    {
        cmd.DW57.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC].Value;
        cmd.DW57.MacroblockIldbStreamoutBufferMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;
        resourceParams.presResource = params->presMacroblockIldbStreamOutBuffer1;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW55.Value);
        resourceParams.dwLocationInCmd = 55;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    if (params->presMacroblockIldbStreamOutBuffer2 != nullptr)
    {
        cmd.DW60.MemoryObjectControlState =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC].Value;
        cmd.DW60.SecondMacroblockIldbStreamoutBufferMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;
        resourceParams.presResource = params->presMacroblockIldbStreamOutBuffer2;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = &(cmd.DW58.Value);
        resourceParams.dwLocationInCmd = 58;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Add 4xDS surface for VDENC
    if (params->bVdencEnabled && params->ps4xDsSurface != nullptr)
    {
        resourceParams.presResource = &params->ps4xDsSurface->OsResource;
        resourceParams.dwOffset = params->ps4xDsSurface->dwOffset;
        resourceParams.pdwCmd = &(cmd.DW62.Value);
        resourceParams.dwLocationInCmd = 62;
        resourceParams.bIsWritable = true;

        MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
        MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, resourceParams.presResource, &mmcMode));

        if (mmcMode == MOS_MEMCOMP_DISABLED)
        {
            params->Ps4xDsSurfMmcState = MOS_MEMCOMP_DISABLED;
            cmd.DW64.ScaledReferenceSurfaceMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_DISABLED;
        }
        else
        {
            cmd.DW64.ScaledReferenceSurfaceMemoryCompressionEnable = MHW_MEDIA_MEMCOMP_ENABLED;
        }
        cmd.DW64.ScaledReferenceSurfaceMemoryCompressionMode = (params->Ps4xDsSurfMmcState == MOS_MEMCOMP_HORIZONTAL) ?
            MHW_MEDIA_MEMCOMP_MODE_HORIZONTAL : MHW_MEDIA_MEMCOMP_MODE_VERTICAL;
        cmd.DW64.ScaledReferenceSurfaceIndexToMemoryObjectControlStateMocsTables =
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value >> 1;

        cmd.DW64.ScaledReferenceSurfaceTiledResourceMode = Mhw_ConvertToTRMode(params->ps4xDsSurface->TileType);

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        MHW_MI_CHK_STATUS(m_osInterface->pfnSetMemoryCompressionMode(m_osInterface, resourceParams.presResource, params->Ps4xDsSurfMmcState));
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}
