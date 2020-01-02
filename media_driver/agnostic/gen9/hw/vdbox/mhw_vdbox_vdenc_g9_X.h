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

//! \file     mhw_vdbox_vdenc_g9_X.h
//! \details  Defines functions for constructing Vdbox Vdenc commands on Gen9-based platforms
//!

#ifndef __MHW_VDBOX_VDENC_G9_X_H__
#define __MHW_VDBOX_VDENC_G9_X_H__

#include "mhw_vdbox_vdenc_generic.h"

//!  MHW Vdbox Vdenc interface for Gen9
/*!
This class defines the Vdenc command construction functions for Gen9 platforms as template
*/
template <class TVdencCmds>
class MhwVdboxVdencInterfaceG9 : public MhwVdboxVdencInterfaceGeneric<TVdencCmds>
{
public:
    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG9() { }

protected:
    enum CommandsNumberOfAddresses
    {
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 12
    };

    //!
    //! \brief  Constructor
    //!
    MhwVdboxVdencInterfaceG9<TVdencCmds>(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceGeneric<TVdencCmds>(osInterface)
    {
        MHW_FUNCTION_ENTER;

        this->InitRowstoreUserFeatureSettings();
    }

    MOS_STATUS InitRowstoreUserFeatureSettings()
    {
        MHW_FUNCTION_ENTER;

        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MEDIA_FEATURE_TABLE *skuTable = this->m_osInterface->pfnGetSkuTable(this->m_osInterface);

        MHW_MI_CHK_NULL(skuTable);

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        userFeatureData.u32Data = 0;

        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ROWSTORE_CACHE_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        this->m_rowstoreCachingSupported = userFeatureData.i32Data ? false : true;

        if (this->m_rowstoreCachingSupported)
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_VDENCROWSTORECACHE_DISABLE_ID,
                &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
            this->m_vdencRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(rowstoreParams);

        if (this->m_vdencRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            this->m_vdencRowStoreCache.bEnabled = true;

            if (rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_2K)
            {
                this->m_vdencRowStoreCache.dwAddress = VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_LESS_THAN_2K;
            }
            else if (rowstoreParams->dwPicWidth >= MHW_VDBOX_PICWIDTH_2K && rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_3K)
            {
                this->m_vdencRowStoreCache.dwAddress = VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_3K;
            }
            else if (rowstoreParams->dwPicWidth >= MHW_VDBOX_PICWIDTH_3K && rowstoreParams->dwPicWidth < MHW_VDBOX_PICWIDTH_4K)
            {
                this->m_vdencRowStoreCache.dwAddress = VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_BETWEEN_3K_AND_4K;
            }
            else
            {
                this->m_vdencRowStoreCache.dwAddress = 0;
                this->m_vdencRowStoreCache.bEnabled = false;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetVdencStateCommandsDataSize(
        uint32_t                        mode,
        uint32_t                        waAddDelayInVDEncDynamicSlice,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize)
    {
        MHW_FUNCTION_ENTER;

        uint32_t            maxSize = 0;
        uint32_t            patchListMaxSize = 0;
        uint32_t            standard = CodecHal_GetStandardFromMode(mode);

        if (standard == CODECHAL_AVC)
        {
            maxSize =
                TVdencCmds::VDENC_PIPE_MODE_SELECT_CMD::byteSize +
                TVdencCmds::VDENC_SRC_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_REF_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_DS_REF_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                TVdencCmds::VDENC_CONST_QPT_STATE_CMD::byteSize +
                TVdencCmds::VDENC_IMG_STATE_CMD::byteSize +
                TVdencCmds::VDENC_WALKER_STATE_CMD::byteSize +
                TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize;

            if (waAddDelayInVDEncDynamicSlice)
            {
                maxSize += TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize * MHW_VDBOX_VDENC_DYNAMIC_SLICE_WA_COUNT;
            }

            patchListMaxSize = VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported encode mode.");
            *commandsSize  = 0;
            *patchListSize = 0;
            return MOS_STATUS_UNKNOWN;
        }

        *commandsSize  = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CreateMhwVdboxPipeModeSelectParams()
    {
        auto pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS);

        return pipeModeSelectParams;
    }

    void ReleaseMhwVdboxPipeModeSelectParams(PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams)
    {
        MOS_Delete(pipeModeSelectParams);
    }

    MOS_STATUS AddVdencPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TVdencCmds::VDENC_PIPE_MODE_SELECT_CMD cmd;

        cmd.DW1.StandardSelect                 = CodecHal_GetStandardFromMode(params->Mode);
        cmd.DW1.FrameStatisticsStreamOutEnable = 1;
        cmd.DW1.TlbPrefetchEnable              = params->bTlbPrefetchEnable;
        cmd.DW1.PakThresholdCheckEnable        = params->bDynamicSliceEnable;
        cmd.DW1.VdencStreamInEnable            = params->bVdencStreamInEnable;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS  params)
    {
        MOS_SURFACE details;
        uint8_t     refIdx;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(this->m_osInterface);

        typename TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD cmd;

        MOS_MEMCOMP_STATE   mmcMode = MOS_MEMCOMP_DISABLED;
        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum        = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType   = MOS_VDENC_PIPE_BUF_ADDR;

        if (params->psRawSurface != nullptr)
        {

            MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, &params->psRawSurface->OsResource, &mmcMode));

            cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryCompressionEnable =
                mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
            cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryCompressionMode =
                mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
            cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;

            cmd.OriginalUncompressedPicture.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

            resourceParams.presResource    = &params->psRawSurface->OsResource;
            resourceParams.dwOffset        = params->psRawSurface->dwOffset;
            resourceParams.pdwCmd          = &(cmd.OriginalUncompressedPicture.LowerAddress.DW0.Value);
            resourceParams.dwLocationInCmd = 10;
            resourceParams.bIsWritable     = false;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (this->m_vdencRowStoreCache.bEnabled)
        {
            cmd.RowStoreScratchBuffer.BufferPictureFields.DW0.CacheSelect = TVdencCmds::VDENC_Surface_Control_Bits_CMD::CACHE_SELECT_UNNAMED1;
            cmd.RowStoreScratchBuffer.LowerAddress.DW0.Value              = this->m_vdencRowStoreCache.dwAddress << 6;
        }
        else if (params->presVdencIntraRowStoreScratchBuffer != nullptr)
        {
            cmd.RowStoreScratchBuffer.BufferPictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC].Value;

            resourceParams.presResource    = params->presVdencIntraRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.RowStoreScratchBuffer.LowerAddress.DW0.Value);
            resourceParams.dwLocationInCmd = 16;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presVdencStreamOutBuffer != nullptr)
        {
            cmd.VdencStatisticsStreamout.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

            resourceParams.presResource    = params->presVdencStreamOutBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.VdencStatisticsStreamout.LowerAddress.DW0.Value);
            resourceParams.dwLocationInCmd = 34;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presVdencStreamInBuffer != nullptr)
        {
            cmd.StreaminDataPicture.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_STREAMIN_CODEC].Value;

            resourceParams.presResource    = params->presVdencStreamInBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = &(cmd.StreaminDataPicture.LowerAddress.DW0.Value);
            resourceParams.dwLocationInCmd = 13;
            resourceParams.bIsWritable     = false;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        for (refIdx = 0; refIdx <= params->dwNumRefIdxL0ActiveMinus1; refIdx++)
        {
            if (params->presVdencReferences[refIdx])
            {
                // L0 references
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdencReferences[refIdx], &details));

                resourceParams.presResource    = params->presVdencReferences[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = (refIdx * 3) + 22;
                resourceParams.bIsWritable     = false;
                switch (refIdx)
                {
                case 0:
                    resourceParams.pdwCmd = &(cmd.FwdRef0.LowerAddress.DW0.Value);
                    break;
                case 1:
                    resourceParams.pdwCmd = &(cmd.FwdRef1.LowerAddress.DW0.Value);
                    break;
                case 2:
                    resourceParams.pdwCmd = &(cmd.FwdRef2.LowerAddress.DW0.Value);
                    break;
                default:
                    break;
                }

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));

                switch (refIdx)
                {
                case 0:
                    cmd.FwdRef0.PictureFields.DW0.MemoryCompressionEnable =
                        mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                    cmd.FwdRef0.PictureFields.DW0.MemoryCompressionMode =
                        mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                    cmd.FwdRef0.PictureFields.DW0.MemoryObjectControlState =
                        this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                    cmd.FwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                case 1:
                    cmd.FwdRef1.PictureFields.DW0.MemoryCompressionEnable =
                        mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                    cmd.FwdRef1.PictureFields.DW0.MemoryCompressionMode =
                        mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                    cmd.FwdRef1.PictureFields.DW0.MemoryObjectControlState =
                        this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                    cmd.FwdRef1.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                case 2:
                    cmd.FwdRef2.PictureFields.DW0.MemoryCompressionEnable =
                        mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                    cmd.FwdRef2.PictureFields.DW0.MemoryCompressionMode =
                        mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                    cmd.FwdRef2.PictureFields.DW0.MemoryObjectControlState =
                        this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                    cmd.FwdRef2.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                default:
                    break;
                }

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            // so far VDEnc only support 2 4x/8x DS Ref Pictures
            if ((refIdx <= 1) && params->presVdenc4xDsSurface[refIdx])
            {
                // 4x DS surface for VDEnc
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdenc4xDsSurface[refIdx], &details));

                resourceParams.presResource    = params->presVdenc4xDsSurface[refIdx];
                resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = (refIdx * 3) + 1;
                resourceParams.bIsWritable     = false;
                switch (refIdx)
                {
                case 0:
                    resourceParams.pdwCmd = &(cmd.DsFwdRef0.LowerAddress.DW0.Value);
                    break;
                case 1:
                    resourceParams.pdwCmd = &(cmd.DsFwdRef1.LowerAddress.DW0.Value);
                    break;
                default:
                    break;
                }

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));

                switch (refIdx)
                {
                case 0:
                    cmd.DsFwdRef0.PictureFields.DW0.MemoryCompressionEnable =
                        mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                    cmd.DsFwdRef0.PictureFields.DW0.MemoryCompressionMode =
                        mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                    cmd.DsFwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                case 1:
                    cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionEnable =
                        mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                    cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionMode =
                        mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                        : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                    cmd.DsFwdRef1.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                default:
                    break;
                }

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->psSurface);

        typename TVdencCmds::VDENC_REF_SURFACE_STATE_CMD cmd;

        cmd.Dwords25.DW0.Width                       = params->psSurface->dwWidth - 1;
        cmd.Dwords25.DW0.Height                      = params->psSurface->dwHeight - 1;
        cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

        cmd.Dwords25.DW1.TiledSurface = IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

        if (cmd.Dwords25.DW1.TiledSurface)
        {
            cmd.Dwords25.DW1.TileWalk = (params->psSurface->TileType);
        }

        cmd.Dwords25.DW1.SurfaceFormat    = this->MosToMediaStateFormat(params->psSurface->Format); //dwSurfaceFormat;  should be 4
        cmd.Dwords25.DW1.InterleaveChroma = 1;
        cmd.Dwords25.DW1.SurfacePitch     = params->psSurface->dwPitch - 1;
        cmd.Dwords25.DW2.YOffsetForUCb    = cmd.Dwords25.DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencDsRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params,
        uint8_t                              numSurfaces)
    {
        MHW_FUNCTION_ENTER;

        MOS_UNUSED(numSurfaces);

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->psSurface);

        typename TVdencCmds::VDENC_DS_REF_SURFACE_STATE_CMD cmd;

        cmd.Dwords25.DW0.Width                       = params->psSurface->dwWidth - 1;
        cmd.Dwords25.DW0.Height                      = params->psSurface->dwHeight - 1;
        cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

        cmd.Dwords25.DW1.TiledSurface                = IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

        if (cmd.Dwords25.DW1.TiledSurface)
        {
            cmd.Dwords25.DW1.TileWalk = (params->psSurface->TileType);
        }
        cmd.Dwords25.DW1.SurfaceFormat    = this->MosToMediaStateFormat(params->psSurface->Format); //dwSurfaceFormat;  should be 4
        cmd.Dwords25.DW1.InterleaveChroma = 1;
        cmd.Dwords25.DW1.SurfacePitch     = params->psSurface->dwPitch - 1;
        cmd.Dwords25.DW2.YOffsetForUCb    = cmd.Dwords25.DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                     cmdBuffer,
        PMHW_BATCH_BUFFER                       batchBuffer,
        PMHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS   params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(batchBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddVdencCmd1Cmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_VDENC_CMD1_PARAMS  params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(batchBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencCmd2Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD2_STATE params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(batchBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_SUCCESS;
    }

public:
    inline uint32_t GetVdencCmd1Size()
    {
        return 0;
    }

    inline uint32_t GetVdencCmd2Size()
    {
        return 0;
    }
};

#endif
