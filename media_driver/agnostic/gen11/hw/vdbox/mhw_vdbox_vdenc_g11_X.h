/*
* Copyright (c) 2017-2020, Intel Corporation
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

//! \file     mhw_vdbox_vdenc_g11_X.h
//! \details  Defines functions for constructing Vdbox Vdenc commands on Gen11-based platforms
//!

#ifndef __MHW_VDBOX_VDENC_G11_X_H__
#define __MHW_VDBOX_VDENC_G11_X_H__

#include "mhw_vdbox_vdenc_generic.h"
#include "mhw_vdbox_vdenc_hwcmd_g11_X.h"
#include "mhw_vdbox_g11_X.h"

#define MHW_VDBOX_PICWIDTH_1920                                               1920
#define MHW_VDBOX_PICWIDTH_3840                                               3840
#define VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_LESS_THAN_1920               360
#define VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_BETWEEN_1920_AND_3840        480
#define VP9VDENCROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920        180
#define VP9VDENCROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920_4208B  570

struct MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11 : public MHW_VDBOX_VDENC_WALKER_STATE_PARAMS
{
    // GEN11+ tiling support
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11   pTileCodingParams = nullptr;
    uint32_t                                dwNumberOfPipes = 0;
    uint32_t                                dwTileId = 0;
};
using PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11 = MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11 *;

//!  MHW Vdbox Vdenc interface for Gen11
/*!
This class defines the Vdenc command construction functions for Gen11 platforms as template
*/
template <class TVdencCmds>
class MhwVdboxVdencInterfaceG11 : public MhwVdboxVdencInterfaceGeneric<TVdencCmds>
{
protected:
    enum CommandsNumberOfAddresses
    {
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES = 1,
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES = 1,
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 21
    };

    enum VdencSurfaceFormat
    {
        vdencSurfaceFormatYuv422          = 0x0,
        vdencSurfaceFormatRgba4444        = 0x1,
        vdencSurfaceFormatYuv444          = 0x2,
        vdencSurfaceFormatP010Variant     = 0x3,
        vdencSurfaceFormatPlanar420_8     = 0x4,
        vdencSurfaceFormatYcrcbSwapy422   = 0x5,
        vdencSurfaceFormatYcrcbSwapuv422  = 0x6,
        vdencSurfaceFormatYcrcbSwapuvy422 = 0x7,
        vdencSurfaceFormatY216            = 0x8,
        vdencSurfaceFormatY210            = 0x8,  // Same value is used to represent Y216 and Y210
        vdencSurfaceFormatRgba_10_10_10_2 = 0x9,
        vdencSurfaceFormatY410            = 0xa,
        vdencSurfaceFormatNv21            = 0xb,
        vdencSurfaceFormatY416            = 0xc,
        vdencSurfaceFormatP010            = 0xd,
        vdencSurfaceFormatPlanarP016      = 0xe,
        vdencSurfaceFormatY8Unorm         = 0xf,
        vdencSurfaceFormatY16             = 0x10,
        vdencSurfaceFormatY216Variant     = 0x11,
        vdencSurfaceFormatY416Variant     = 0x12,
        vdencSurfaceFormatYuyvVariant     = 0x13,
        vdencSurfaceFormatAyuvVariant     = 0x14,
    };

    //!
    //! \brief    Translate MOS type format to Vdenc surface raw format
    //! \details  VDBOX protected function to translate mos format to media state format
    //! \param    MOS_FORMAT  Format
    //!           [in] MOS type format
    //! \return   VdencSurfaceFormat
    //!           media state surface format
    //!
    VdencSurfaceFormat MosFormatToVdencSurfaceRawFormat(
        MOS_FORMAT format)
    {
        MHW_FUNCTION_ENTER;

        switch (format)
        {
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
        case Format_A8B8G8R8:
            return vdencSurfaceFormatRgba4444;
        case Format_NV12:
        case Format_NV11:
        case Format_P208:
        case Format_IMC1:
        case Format_IMC2:
        case Format_IMC3:
        case Format_IMC4:
            return vdencSurfaceFormatPlanar420_8;
        case Format_400P:
        case Format_P8:
            return vdencSurfaceFormatY8Unorm;
        case Format_UYVY:
            return vdencSurfaceFormatYcrcbSwapy422;
        case Format_YVYU:
            return vdencSurfaceFormatYcrcbSwapuv422;
        case Format_VYUY:
            return vdencSurfaceFormatYcrcbSwapuvy422;
        case Format_444P:
        case Format_AYUV:
            return vdencSurfaceFormatYuv444;
        case Format_YUY2:
        case Format_YUYV:
            return vdencSurfaceFormatYuv422;
        case Format_P010:
            return vdencSurfaceFormatP010;
        case Format_R10G10B10A2:
        case Format_B10G10R10A2:
            return vdencSurfaceFormatRgba_10_10_10_2;
        // Only Y210 supported now, allocated as Y216 format by 3D driver
        case Format_Y210:
        case Format_Y216:
            return vdencSurfaceFormatY216;
        case Format_Y410:
            return vdencSurfaceFormatY410;
        case Format_NV21:
            return vdencSurfaceFormatNv21;
        default:
            return vdencSurfaceFormatPlanar420_8;
        }

        return vdencSurfaceFormatYuv422;
    }

    //!
    //! \brief    Translate MOS type format to Vdenc surface recon format
    //! \details  VDBOX protected function to translate mos format to media state recon format
    //! \param    MOS_FORMAT  Format
    //!           [in] MOS type format
    //! \return   VdencSurfaceFormat
    //!           media state surface format
    //!
    VdencSurfaceFormat MosFormatToVdencSurfaceReconFormat(
        MOS_FORMAT format)
    {
        MHW_FUNCTION_ENTER;

        switch (format)
        {
        case Format_A8R8G8B8:
        case Format_X8R8G8B8:
        case Format_A8B8G8R8:
            return vdencSurfaceFormatRgba4444;
        case Format_NV12:
        case Format_IMC1:
        case Format_IMC2:
        case Format_IMC3:
        case Format_IMC4:
            return vdencSurfaceFormatPlanar420_8;
        case Format_400P:
        case Format_P8:
            return vdencSurfaceFormatY8Unorm;
        case Format_UYVY:
            return vdencSurfaceFormatYcrcbSwapy422;
        case Format_YVYU:
            return vdencSurfaceFormatYcrcbSwapuv422;
        case Format_VYUY:
            return vdencSurfaceFormatYcrcbSwapuvy422;
        case Format_444P:
        case Format_AYUV:
            return vdencSurfaceFormatAyuvVariant;
        case Format_YUY2:
        case Format_YUYV:
            return vdencSurfaceFormatYuyvVariant;
        case Format_P010:
            return vdencSurfaceFormatP010Variant;
        case Format_R10G10B10A2:
            return vdencSurfaceFormatRgba_10_10_10_2;
        case Format_Y216:
            return vdencSurfaceFormatY216Variant;
        case Format_Y410:
            return vdencSurfaceFormatY416Variant;
        case Format_NV21:
            return vdencSurfaceFormatNv21;
        default:
            return vdencSurfaceFormatPlanar420_8;
        }
    }

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxVdencInterfaceG11<TVdencCmds>(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceGeneric<TVdencCmds>(osInterface)
    {
        MHW_FUNCTION_ENTER;

        this->m_rhoDomainStatsEnabled = true;
        this->InitRowstoreUserFeatureSettings();
    }

    inline uint32_t GetVdencCmd1Size()
    {
        return 0;
    }

    inline uint32_t GetVdencCmd2Size()
    {
        return 0;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG11() { }

    MOS_STATUS InitRowstoreUserFeatureSettings()
    {
        MHW_FUNCTION_ENTER;

        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MEDIA_FEATURE_TABLE *skuTable = this->m_osInterface->pfnGetSkuTable(this->m_osInterface);

        MHW_MI_CHK_NULL(skuTable);

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

        if (this->m_osInterface->bSimIsActive)
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
            this->m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        this->m_rowstoreCachingSupported = userFeatureData.i32Data ? false : true;

        if (this->m_rowstoreCachingSupported)
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_VDENCROWSTORECACHE_DISABLE_ID,
                &userFeatureData,
                this->m_osInterface->pOsContext);
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

        if (this->m_vdencRowStoreCache.bSupported)
        {
            this->m_vdencRowStoreCache.bEnabled = true;

            if (rowstoreParams->Mode == CODECHAL_ENCODE_MODE_HEVC)
            {
                if (rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_1920 && rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_3840)
                {
                    this->m_vdencRowStoreCache.dwAddress = VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_BETWEEN_1920_AND_3840;
                }
                else if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_1920 && 
                    rowstoreParams->ucChromaFormat == HCP_CHROMA_FORMAT_YUV444 && 
                    rowstoreParams->ucBitDepthMinus8 > 0)
                {
                    this->m_vdencRowStoreCache.dwAddress = VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_LESS_THAN_1920;
                }
                else
                {
                    this->m_vdencRowStoreCache.dwAddress = 0;
                    this->m_vdencRowStoreCache.bEnabled = false;
                }
            }
            else if (rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP9)
            {
                if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_1920)
                {
                    if (rowstoreParams->ucBitDepthMinus8 == 0 && rowstoreParams->ucChromaFormat != HCP_CHROMA_FORMAT_YUV444)
                    {
                        this->m_vdencRowStoreCache.dwAddress = VP9VDENCROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920_4208B;
                    }
                    else
                    {
                        this->m_vdencRowStoreCache.dwAddress = VP9VDENCROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920;
                    }
                }
                else if (rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_1920 && rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_3840)
                {
                    this->m_vdencRowStoreCache.dwAddress = VDENCROWSTORE_FRAME_BASEADDRESS_PICWIDTH_LESS_THAN_1920;
                }
                else
                {
                    this->m_vdencRowStoreCache.bEnabled  = false;
                    this->m_vdencRowStoreCache.dwAddress = 0;
                }
            }
            else
            {
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
        else if (standard == CODECHAL_HEVC)
        {
            maxSize =
                TVdencCmds::VDENC_PIPE_MODE_SELECT_CMD::byteSize +
                TVdencCmds::VDENC_SRC_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_REF_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_DS_REF_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                TVdencCmds::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize +
                TVdencCmds::VDENC_WALKER_STATE_CMD::byteSize +
                TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize;

            if (waAddDelayInVDEncDynamicSlice)
            {
                maxSize += TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize * MHW_VDBOX_VDENC_DYNAMIC_SLICE_WA_COUNT;
            }

            patchListMaxSize = VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES;
        }
        else if (standard == CODECHAL_VP9)
        {
            maxSize =
                TVdencCmds::VDENC_PIPE_MODE_SELECT_CMD::byteSize +
                TVdencCmds::VDENC_SRC_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_REF_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_DS_REF_SURFACE_STATE_CMD::byteSize +
                TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD::byteSize +
                TVdencCmds::VDENC_WALKER_STATE_CMD::byteSize +
                TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize;

            if (waAddDelayInVDEncDynamicSlice)
            {
                maxSize += TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize * MHW_VDBOX_VDENC_DYNAMIC_SLICE_WA_COUNT;
            }

            patchListMaxSize =
                MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES +
                MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES +
                VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES;
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

    MOS_STATUS GetVdencPrimitiveCommandsDataSize(
        uint32_t                        mode,
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
                TVdencCmds::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize +
                TVdencCmds::VDENC_WALKER_STATE_CMD::byteSize +
                TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize;

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

    MOS_STATUS AddVdencPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
 
        auto paramsG11 = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11>(params);
        MHW_MI_CHK_NULL(paramsG11);
        typename TVdencCmds::VDENC_PIPE_MODE_SELECT_CMD cmd;

        cmd.DW1.StandardSelect                 = CodecHal_GetStandardFromMode(params->Mode);
        cmd.DW1.ScalabilityMode                = !(paramsG11->MultiEngineMode == MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY);
        cmd.DW1.FrameStatisticsStreamOutEnable = 1;
        cmd.DW1.VdencPakObjCmdStreamOutEnable  = params->bVdencPakObjCmdStreamOutEnable;
        cmd.DW1.TlbPrefetchEnable              = params->bTlbPrefetchEnable;
        cmd.DW1.PakThresholdCheckEnable        = params->bDynamicSliceEnable;
        cmd.DW1.VdencStreamInEnable            = params->bVdencStreamInEnable;
        cmd.DW1.BitDepth                       = params->ucVdencBitDepthMinus8;

        if (CODECHAL_ENCODE_MODE_HEVC == params->Mode || CODECHAL_ENCODE_MODE_VP9 == params->Mode)
        {
            cmd.DW1.PakChromaSubSamplingType   = params->ChromaType;
        }
        // by default RGB to YUV using full to studio range
        // can add a DDI flag to control if needed
        cmd.DW1.OutputRangeControlAfterColorSpaceConversion = 1;

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
            resourceParams.pdwCmd          = (uint32_t*) &(cmd.OriginalUncompressedPicture.LowerAddress);
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
        else if (!Mos_ResourceIsNull(params->presVdencIntraRowStoreScratchBuffer))
        {
            cmd.RowStoreScratchBuffer.BufferPictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC].Value;

            resourceParams.presResource    = params->presVdencIntraRowStoreScratchBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (uint32_t*) &(cmd.RowStoreScratchBuffer.LowerAddress);
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
            resourceParams.dwOffset        = params->dwVdencStatsStreamOutOffset;
            resourceParams.pdwCmd          = (uint32_t*) &(cmd.VdencStatisticsStreamout.LowerAddress);
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
            resourceParams.pdwCmd          = (uint32_t*) &(cmd.StreaminDataPicture.LowerAddress);
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
                    resourceParams.pdwCmd = (uint32_t*) &(cmd.FwdRef0.LowerAddress);
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
                    resourceParams.pdwCmd = (uint32_t*) &(cmd.FwdRef1.LowerAddress);
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
                    resourceParams.pdwCmd = (uint32_t*) &(cmd.FwdRef2.LowerAddress);
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
                if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
                {
                    // 4x DS surface for VDEnc
                    MOS_ZeroMemory(&details, sizeof(details));
                    details.Format = Format_Invalid;
                    MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdenc4xDsSurface[refIdx], &details));

                    resourceParams.presResource    = params->presVdenc4xDsSurface[refIdx];
                    resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                    resourceParams.dwLocationInCmd = (refIdx * 3) + 1;
                    resourceParams.bIsWritable     = false;

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
                        cmd.DsFwdRef0.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        resourceParams.pdwCmd = (uint32_t*) &(cmd.DsFwdRef0.LowerAddress);
                        break;
                    case 1:
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionEnable =
                            mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionMode =
                            mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef1.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        resourceParams.pdwCmd = (uint32_t*) &(cmd.DsFwdRef1.LowerAddress);
                        break;
                    default:
                        break;
                    }

                    MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                        this->m_osInterface,
                        cmdBuffer,
                        &resourceParams));
                }
                else if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_MODE_VP9)
                {
                    // 8x DS surface
                    MOS_ZeroMemory(&details, sizeof(details));
                    details.Format = Format_Invalid;
                    MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdenc8xDsSurface[refIdx], &details));

                    resourceParams.presResource    = params->presVdenc8xDsSurface[refIdx];
                    resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                    resourceParams.dwLocationInCmd = (refIdx * 3) + 1;
                    resourceParams.bIsWritable     = false;

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
                        cmd.DsFwdRef0.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        resourceParams.pdwCmd = (uint32_t*) &(cmd.DsFwdRef0.LowerAddress);
                        break;
                    case 1:
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionEnable =
                            mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionMode =
                            mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef1.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        resourceParams.pdwCmd = (uint32_t*) &(cmd.DsFwdRef1.LowerAddress);
                        break;
                    default:
                        break;
                    }

                    MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                        this->m_osInterface,
                        cmdBuffer,
                        &resourceParams));

                    // 4x DS surface
                    MOS_ZeroMemory(&details, sizeof(details));
                    details.Format = Format_Invalid;
                    MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdenc4xDsSurface[refIdx], &details));

                    resourceParams.presResource    = params->presVdenc4xDsSurface[refIdx];
                    resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                    resourceParams.dwLocationInCmd = (refIdx * 3) + 37;
                    resourceParams.bIsWritable     = false;

                    MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));
                    switch (refIdx)
                    {
                    case 0:
                        cmd.DsFwdRef04X.PictureFields.DW0.MemoryCompressionEnable =
                            mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                        cmd.DsFwdRef04X.PictureFields.DW0.MemoryCompressionMode =
                            mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                        cmd.DsFwdRef04X.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef04X.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        resourceParams.pdwCmd = (uint32_t*)&(cmd.DsFwdRef04X.LowerAddress);
                        break;
                    case 1:
                        cmd.DsFwdRef14X.PictureFields.DW0.MemoryCompressionEnable =
                            mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                        cmd.DsFwdRef14X.PictureFields.DW0.MemoryCompressionMode =
                            mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                            : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                        cmd.DsFwdRef14X.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef14X.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        resourceParams.pdwCmd = (uint32_t*) &(cmd.DsFwdRef14X.LowerAddress);
                        break;
                    default:
                        break;
                    }

                    MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                        this->m_osInterface,
                        cmdBuffer,
                        &resourceParams));
                }
                else
                {
                    MHW_ASSERTMESSAGE("Encode mode = %d not supported", params->Mode);
                    return MOS_STATUS_INVALID_PARAMETER;
                }
            }
        }

        // extra surface for HEVC/VP9
        if ((params->Mode == CODECHAL_ENCODE_MODE_HEVC) || (params->Mode == CODECHAL_ENCODE_MODE_VP9))
        {
            if (params->presColMvTempBuffer[0] != nullptr)
            {
                resourceParams.presResource    = params->presColMvTempBuffer[0];
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*) &(cmd.ColocatedMv.LowerAddress);
                resourceParams.dwLocationInCmd = 19;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));
                cmd.ColocatedMv.PictureFields.DW0.MemoryCompressionEnable =
                    mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                cmd.ColocatedMv.PictureFields.DW0.MemoryCompressionMode =
                    mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                cmd.ColocatedMv.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_ENC_MV_TEMPORAL_BUFFER_ENCODE].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->ps8xDsSurface != nullptr)
            {
                resourceParams.presResource    = &params->ps8xDsSurface->OsResource;
                resourceParams.dwOffset        = params->ps8xDsSurface->dwOffset;
                resourceParams.pdwCmd          = (uint32_t*) &(cmd.ScaledReferenceSurface8X.LowerAddress);
                resourceParams.dwLocationInCmd = 49;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));
                cmd.ScaledReferenceSurface8X.PictureFields.DW0.MemoryCompressionEnable =
                    mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                cmd.ScaledReferenceSurface8X.PictureFields.DW0.MemoryCompressionMode =
                    mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                cmd.ScaledReferenceSurface8X.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->ps4xDsSurface != nullptr)
            {
                resourceParams.presResource    = &params->ps4xDsSurface->OsResource;
                resourceParams.dwOffset        = params->ps4xDsSurface->dwOffset;
                resourceParams.pdwCmd          = (uint32_t*) &(cmd.ScaledReferenceSurface4X.LowerAddress);
                resourceParams.dwLocationInCmd = 52;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));
                cmd.ScaledReferenceSurface4X.PictureFields.DW0.MemoryCompressionEnable =
                    mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                cmd.ScaledReferenceSurface4X.PictureFields.DW0.MemoryCompressionMode =
                    mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                cmd.ScaledReferenceSurface4X.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            // CuRecord stream-out buffer, not used so far
            if (params->presVdencCuObjStreamOutBuffer)
            {
                resourceParams.presResource    = params->presVdencCuObjStreamOutBuffer;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*) &(cmd.VdencCuRecordStreamOutBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 43;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));
                cmd.VdencCuRecordStreamOutBuffer.PictureFields.DW0.MemoryCompressionEnable =
                    mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                cmd.VdencCuRecordStreamOutBuffer.PictureFields.DW0.MemoryCompressionMode =
                    mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                cmd.VdencCuRecordStreamOutBuffer.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->presVdencPakObjCmdStreamOutBuffer)
            {
                resourceParams.presResource    = params->presVdencPakObjCmdStreamOutBuffer;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*) &(cmd.VdencLcuPakObjCmdBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 46;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetMemoryCompressionMode(this->m_osInterface, resourceParams.presResource, &mmcMode));
                cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryCompressionEnable =
                    mmcMode != MOS_MEMCOMP_DISABLED ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_ENABLE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_ENABLE_DISABLE;
                cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryCompressionMode =
                    mmcMode == MOS_MEMCOMP_HORIZONTAL ? TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_HORIZONTALCOMPRESSIONMODE
                    : TVdencCmds::VDENC_Surface_Control_Bits_CMD::MEMORY_COMPRESSION_MODE_VERTICALCOMPRESSIONMODE;
                cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->presSegmentMapStreamOut)
            {
                resourceParams.presResource    = params->presSegmentMapStreamOut;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*) &(cmd.Vp9SegmentationMapStreaminBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 55;
                resourceParams.bIsWritable     = true;

                cmd.Vp9SegmentationMapStreaminBuffer.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_STREAMIN_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));

                resourceParams.presResource    = params->presSegmentMapStreamOut;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*) &(cmd.Vp9SegmentationMapStreamoutBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 58;
                resourceParams.bIsWritable     = true;

                cmd.Vp9SegmentationMapStreamoutBuffer.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        // DW61: Weights Histogram Streamout offset
        // This parameter specifies the 64 byte aligned offset in the VDEnc Statistics Streamout buffer where the luma and chroma histogram for the weights/offsets determination is written out.

        // Hard-coding the value 
        // The first 2 CLs(cacheline=64bytes) are ENC frame statistics data. 
        // The 3rd CL is for VDL1* stats (hits & misses). 
        // Hence it's a dummy CL for us. Histogram stats start from 4th CL onwards. 
        cmd.DW61.WeightsHistogramStreamoutOffset = 3 * MHW_CACHELINE_SIZE;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencSrcSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->psSurface);

        typename TVdencCmds::VDENC_SRC_SURFACE_STATE_CMD cmd;

        cmd.Dwords25.DW0.Width               = params->dwActualWidth - 1;
        cmd.Dwords25.DW0.Height              = params->dwActualHeight - 1;
        cmd.Dwords25.DW0.ColorSpaceSelection = params->bColorSpaceSelection;

        cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

        cmd.Dwords25.DW1.TiledSurface = IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

        if (cmd.Dwords25.DW1.TiledSurface)
        {
            cmd.Dwords25.DW1.TileWalk = (params->psSurface->TileType);
        }

        if (params->psSurface->TileType == MOS_TILE_LINEAR)
        {
            cmd.Dwords25.DW1.TileWalk = 0;
        }

        cmd.Dwords25.DW1.SurfaceFormat            = this->MosFormatToVdencSurfaceRawFormat(params->psSurface->Format);
        cmd.Dwords25.DW0.SurfaceFormatByteSwizzle = params->bDisplayFormatSwizzle;
        cmd.Dwords25.DW1.SurfacePitch             = params->psSurface->dwPitch - 1;
        cmd.Dwords25.DW2.YOffsetForUCb            = cmd.Dwords25.DW3.YOffsetForVCr =
            MOS_ALIGN_CEIL(params->psSurface->UPlaneOffset.iYOffset, MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9);

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

        if (params->bVdencDynamicScaling)
        {
            if (params->ucSurfaceStateId == CODECHAL_HCP_LAST_SURFACE_ID)
            {
                cmd.DW1.SurfaceId = 4;
            }
            else if (params->ucSurfaceStateId == CODECHAL_HCP_GOLDEN_SURFACE_ID)
            {
                cmd.DW1.SurfaceId = 5;
            }
            else if (params->ucSurfaceStateId == CODECHAL_HCP_ALTREF_SURFACE_ID)
            {
                cmd.DW1.SurfaceId = 6;
            }
        }

        if (params->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            cmd.Dwords25.DW0.Width  = params->dwActualWidth - 1;
            cmd.Dwords25.DW0.Height = params->dwActualHeight - 1;
        }
        else
        {
            cmd.Dwords25.DW0.Width  = params->psSurface->dwWidth - 1;
            cmd.Dwords25.DW0.Height = params->psSurface->dwHeight - 1;
        }

        cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

        cmd.Dwords25.DW1.TiledSurface = IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

        if (cmd.Dwords25.DW1.TiledSurface)
        {
            cmd.Dwords25.DW1.TileWalk = (params->psSurface->TileType);
        }

        cmd.Dwords25.DW1.SurfaceFormat = this->MosFormatToVdencSurfaceReconFormat(params->psSurface->Format);

        if (cmd.Dwords25.DW1.SurfaceFormat == TVdencCmds::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_P010)
        {
            cmd.Dwords25.DW1.SurfaceFormat = TVdencCmds::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_P010_VARIANT;
        }

        cmd.Dwords25.DW1.SurfacePitch = params->psSurface->dwPitch - 1;
        cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;

        if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatY416Variant ||
            cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatAyuvVariant)
        {
            /* Y410/Y416 Reconstructed format handling */
            if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatY416Variant)
                cmd.Dwords25.DW1.SurfacePitch = params->psSurface->dwPitch / 2 - 1;
            /* AYUV Reconstructed format handling */
            if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatAyuvVariant)
                cmd.Dwords25.DW1.SurfacePitch = params->psSurface->dwPitch / 4 - 1;

            cmd.Dwords25.DW2.YOffsetForUCb = params->dwReconSurfHeight;
            cmd.Dwords25.DW3.YOffsetForVCr = params->dwReconSurfHeight << 1;
        }
        else if (cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatY216Variant ||
            cmd.Dwords25.DW1.SurfaceFormat == vdencSurfaceFormatYuyvVariant)
        {
            cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr = params->dwReconSurfHeight;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencDsRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params,
        uint8_t                              numSurfaces)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->psSurface);

        typename TVdencCmds::VDENC_DS_REF_SURFACE_STATE_CMD cmd;

        if (params->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            cmd.Dwords25.DW0.Width  = params->dwActualWidth - 1;
            cmd.Dwords25.DW0.Height = params->dwActualHeight - 1;
        }
        else
        {
            cmd.Dwords25.DW0.Width  = params->psSurface->dwWidth - 1;
            cmd.Dwords25.DW0.Height = params->psSurface->dwHeight - 1;
        }
        cmd.Dwords25.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

        cmd.Dwords25.DW1.TiledSurface =
            IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

        if (cmd.Dwords25.DW1.TiledSurface)
        {
            cmd.Dwords25.DW1.TileWalk = (params->psSurface->TileType);
        }

        cmd.Dwords25.DW1.SurfaceFormat = TVdencCmds::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8;
        cmd.Dwords25.DW1.SurfacePitch  = params->psSurface->dwPitch - 1;
        cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;

        // 2nd surface
        if (numSurfaces > 1)
        {
            params = params + 1;          // Increment pointer to move from 1st surface to 2nd surface.
            MHW_MI_CHK_NULL(params);
            MHW_MI_CHK_NULL(params->psSurface);

            if (params->Mode == CODECHAL_ENCODE_MODE_HEVC)
            {
                cmd.Dwords69.DW0.Width  = params->dwActualWidth - 1;
                cmd.Dwords69.DW0.Height = params->dwActualHeight - 1;
            }
            else
            {
                cmd.Dwords69.DW0.Width  = params->psSurface->dwWidth - 1;
                cmd.Dwords69.DW0.Height = params->psSurface->dwHeight - 1;
            }
            cmd.Dwords69.DW0.CrVCbUPixelOffsetVDirection = params->ucVDirection;

            cmd.Dwords69.DW1.TiledSurface = IS_TILE_FORMAT(params->psSurface->TileType) ? 1 : 0;

            if (cmd.Dwords69.DW1.TiledSurface)
            {
                cmd.Dwords69.DW1.TileWalk = (params->psSurface->TileType);
            }

            cmd.Dwords69.DW1.SurfaceFormat = TVdencCmds::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8;
            cmd.Dwords69.DW1.SurfacePitch  = params->psSurface->dwPitch - 1;
            cmd.Dwords69.DW2.YOffsetForUCb = cmd.Dwords69.DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencImgStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS        params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pEncodeAvcSeqParams);
        MHW_MI_CHK_NULL(params->pEncodeAvcPicParams);

        typename TVdencCmds::VDENC_IMG_STATE_CMD cmd;

        auto avcSeqParams  = params->pEncodeAvcSeqParams;
        auto avcPicParams  = params->pEncodeAvcPicParams;
        auto avcSliceParams = params->pEncodeAvcSliceParams;

        // initialize
        cmd.DW1.VdencExtendedPakObjCmdEnable         = 1;
        cmd.DW2.UnidirectionalMixDisable             = false;
        cmd.DW4.IntraSadMeasureAdjustment            = 2;
        cmd.DW4.SubMacroblockSubPartitionMask        = 0x70;
        cmd.DW8.BilinearFilterEnable                 = false;
        cmd.DW9.Mode0Cost                            = 10;
        cmd.DW9.Mode1Cost                            = 0;
        cmd.DW9.Mode2Cost                            = 3;
        cmd.DW9.Mode3Cost                            = 30;
        cmd.DW20.PenaltyForIntra16X16NondcPrediction = 36;
        cmd.DW20.PenaltyForIntra8X8NondcPrediction   = 12;
        cmd.DW20.PenaltyForIntra4X4NondcPrediction   = 4;
        cmd.DW22.Smallmbsizeinword                   = 0xff;
        cmd.DW22.Largembsizeinword                   = 0xff;
        cmd.DW27.MaxHmvR                             = 0x2000;
        cmd.DW27.MaxVmvR                             = 0x200;
        cmd.DW33.Maxdeltaqp                          = 0x0f;

        // initialize for P frame
        if (avcPicParams->CodingType != I_TYPE)
        {
            cmd.DW2.BidirectionalWeight              = 0x20;
            cmd.DW4.SubPelMode                       = 3;
            cmd.DW4.BmeDisableForFbrMessage          = 1;
            cmd.DW4.InterSadMeasureAdjustment        = 2;
            cmd.DW5.CrePrefetchEnable                = 1;
            cmd.DW8.NonSkipZeroMvCostAdded           = 1;
            cmd.DW8.NonSkipMbModeCostAdded           = 1;
            cmd.DW9.Mode0Cost                        = 7;
            cmd.DW9.Mode1Cost                        = 26;
            cmd.DW9.Mode2Cost                        = 30;
            cmd.DW9.Mode3Cost                        = 57;
            cmd.DW10.Mode4Cost                       = 8;
            cmd.DW10.Mode5Cost                       = 2;
            cmd.DW10.Mode6Cost                       = 4;
            cmd.DW10.Mode7Cost                       = 6;
            cmd.DW11.Mode8Cost                       = 5;
            cmd.DW11.Mode9Cost                       = 0;
            cmd.DW11.RefIdCost                       = 4;
            cmd.DW12.MvCost0                         = 0;
            cmd.DW12.MvCost1                         = 6;
            cmd.DW12.MvCost2                         = 6;
            cmd.DW12.MvCost3                         = 9;
            cmd.DW13.MvCost4                         = 10;
            cmd.DW13.MvCost5                         = 13;
            cmd.DW13.MvCost6                         = 14;
            cmd.DW13.MvCost7                         = 24;
            cmd.DW31.SadHaarThreshold0               = 800;
            cmd.DW32.SadHaarThreshold1               = 1600;
            cmd.DW32.SadHaarThreshold2               = 2400;
            cmd.DW34.MidpointSadHaar                 = 0x640;
        }

        cmd.DW1.VdencPerfmode                        = params->bVDEncPerfModeEnabled;
        cmd.DW1.Transform8X8Flag                     = avcPicParams->transform_8x8_mode_flag;
        cmd.DW3.PictureWidth                         = params->wPicWidthInMb;
        cmd.DW4.ForwardTransformSkipCheckEnable      = this->m_vdencFTQEnabled[avcSeqParams->TargetUsage];
        cmd.DW4.BlockBasedSkipEnabled                = this->m_vdencBlockBasedSkipEnabled[avcSeqParams->TargetUsage];
        cmd.DW5.CrePrefetchEnable                    = params->bCrePrefetchEnable;
        cmd.DW5.PictureHeightMinusOne                = params->wPicHeightInMb - 1;
        cmd.DW5.PictureType                          = avcPicParams->CodingType - 1;
        cmd.DW5.ConstrainedIntraPredictionFlag       = avcPicParams->constrained_intra_pred_flag;

        // HME Ref1 Disable should be set as 0 when VDEnc Perf Mode is enabled 
        if ((avcPicParams->CodingType != I_TYPE) &&
            (!params->pEncodeAvcSliceParams->num_ref_idx_l0_active_minus1) &&
            (!params->bVDEncPerfModeEnabled))
        {
            cmd.DW5.HmeRef1Disable                   = true;
        }

        if (avcSeqParams->EnableSliceLevelRateCtrl)
        {
            cmd.DW5.MbSliceThresholdValue            = params->dwMbSlcThresholdValue;
        }

        cmd.DW6.SliceMacroblockHeightMinusOne        = params->wSlcHeightInMb - 1;

        cmd.DW8.LumaIntraPartitionMask               = avcPicParams->transform_8x8_mode_flag ? 0 : TVdencCmds::VDENC_IMG_STATE_CMD::LUMA_INTRA_PARTITION_MASK_UNNAMED2;

        cmd.DW14.QpPrimeY                            = avcPicParams->QpY + avcSliceParams->slice_qp_delta;

        if (params->pVDEncModeCost)
        {
            cmd.DW9.Mode0Cost                        = *(params->pVDEncModeCost);
            cmd.DW9.Mode1Cost                        = *(params->pVDEncModeCost + 1);
            cmd.DW9.Mode2Cost                        = *(params->pVDEncModeCost + 2);
            cmd.DW9.Mode3Cost                        = *(params->pVDEncModeCost + 3);

            cmd.DW10.Mode4Cost                       = *(params->pVDEncModeCost + 4);
            cmd.DW10.Mode5Cost                       = *(params->pVDEncModeCost + 5);
            cmd.DW10.Mode6Cost                       = *(params->pVDEncModeCost + 6);
            cmd.DW10.Mode7Cost                       = *(params->pVDEncModeCost + 7);

            cmd.DW11.Mode8Cost                       = *(params->pVDEncModeCost + 8);
            cmd.DW11.RefIdCost                       = *(params->pVDEncModeCost + 10);
        }
        if (params->pVDEncMvCost)
        {
            cmd.DW12.MvCost0                         = *(params->pVDEncMvCost);
            cmd.DW12.MvCost1                         = *(params->pVDEncMvCost + 1);
            cmd.DW12.MvCost2                         = *(params->pVDEncMvCost + 2);
            cmd.DW12.MvCost3                         = *(params->pVDEncMvCost + 3);
            cmd.DW13.MvCost4                         = *(params->pVDEncMvCost + 4);
            cmd.DW13.MvCost5                         = *(params->pVDEncMvCost + 5);
            cmd.DW13.MvCost6                         = *(params->pVDEncMvCost + 6);
            cmd.DW13.MvCost7                         = *(params->pVDEncMvCost + 7);
        }

        cmd.DW27.MaxVmvR = params->dwMaxVmvR;

        if (params->pVDEncHmeMvCost)
        {
            cmd.DW28.HmeMvCost0                      = *(params->pVDEncHmeMvCost);
            cmd.DW28.HmeMvCost1                      = *(params->pVDEncHmeMvCost + 1);
            cmd.DW28.HmeMvCost2                      = *(params->pVDEncHmeMvCost + 2);
            cmd.DW28.HmeMvCost3                      = *(params->pVDEncHmeMvCost + 3);
            cmd.DW29.HmeMvCost4                      = *(params->pVDEncHmeMvCost + 4);
            cmd.DW29.HmeMvCost5                      = *(params->pVDEncHmeMvCost + 5);
            cmd.DW29.HmeMvCost6                      = *(params->pVDEncHmeMvCost + 6);
            cmd.DW29.HmeMvCost7                      = *(params->pVDEncHmeMvCost + 7);
        }

        // HMEOffset is in range of -128 to 127, clip value to within range
        if (avcPicParams->bEnableHMEOffset)
        {
            cmd.DW7.Hme0XOffset                      = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[0][0][0], -128, 127);
            cmd.DW7.Hme0YOffset                      = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[0][0][1], -128, 127);
            cmd.DW7.Hme1XOffset                      = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[1][0][0], -128, 127);
            cmd.DW7.Hme1YOffset                      = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[1][0][1], -128, 127);
        }

        // Rolling-I settings
        if ((avcPicParams->CodingType != I_TYPE) && (avcPicParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED))
        {
            cmd.DW21.IntraRefreshEnableRollingIEnable = avcPicParams->EnableRollingIntraRefresh != ROLLING_I_DISABLED ? 1 : 0;
            cmd.DW21.IntraRefreshMode                 = avcPicParams->EnableRollingIntraRefresh == ROLLING_I_ROW ? 0 : 1;        // 0->Row based ; 1->Column based
            cmd.DW21.IntraRefreshMBPos                = avcPicParams->IntraRefreshMBNum;
            cmd.DW21.IntraRefreshMBSizeMinusOne       = avcPicParams->IntraRefreshUnitinMB;
            cmd.DW21.QpAdjustmentForRollingI          = avcPicParams->IntraRefreshQPDelta;

            auto waTable = this->m_osInterface->pfnGetWaTable(this->m_osInterface);
            MHW_MI_CHK_NULL(waTable);

            // WA to prevent error propagation from top-right direction.
            // Disable prediction modes 3, 7 for 4x4
            // and modes 0, 2, 3, 4, 5, 7 for 8x8 (due to filtering)
            if (avcPicParams->EnableRollingIntraRefresh == ROLLING_I_COLUMN &&
                MEDIA_IS_WA(waTable, Wa_18011246551))
            {
                cmd.DW17.AvcIntra4X4ModeMask = 0x88;
                cmd.DW17.AvcIntra8X8ModeMask = 0xBD;
            }
        }

        // Setting MinMaxQP values if they are presented
        if (avcPicParams->ucMaximumQP && avcPicParams->ucMinimumQP)
        {
            cmd.DW33.MaxQp = avcPicParams->ucMaximumQP;
            cmd.DW33.MinQp = avcPicParams->ucMinimumQP;
        }
        else
        {
            // Set default values
            cmd.DW33.MaxQp = 0x33;
            cmd.DW33.MinQp = 0x0a;
        }

        // VDEnc CQP case ROI settings, BRC ROI will be handled in HuC FW
        if (!params->bVdencBRCEnabled && avcPicParams->NumROI)
        {
            MHW_ASSERT(avcPicParams->NumROI < 4);

            int8_t priorityLevelOrDQp[ENCODE_VDENC_AVC_MAX_ROI_NUMBER_G9] = { 0 };

            for (uint8_t i = 0; i < avcPicParams->NumROI; i++)
            {
                int8_t dQpRoi = avcPicParams->ROI[i].PriorityLevelOrDQp;

                // clip delta qp roi to VDEnc supported range 
                priorityLevelOrDQp[i] = (char)CodecHal_Clip3(
                    ENCODE_VDENC_AVC_MIN_ROI_DELTA_QP_G9, ENCODE_VDENC_AVC_MAX_ROI_DELTA_QP_G9, dQpRoi);
            }

            cmd.DW34.RoiEnable = true;

            // Zone0 is reserved for non-ROI region
            cmd.DW30.RoiQpAdjustmentForZone1 = priorityLevelOrDQp[0];
            cmd.DW30.RoiQpAdjustmentForZone2 = priorityLevelOrDQp[1];
            cmd.DW30.RoiQpAdjustmentForZone3 = priorityLevelOrDQp[2];
        }

        if (avcSeqParams->RateControlMethod != RATECONTROL_CQP)
        {
            cmd.DW30.QpAdjustmentForShapeBestIntra4X4Winner = 0;
            cmd.DW30.QpAdjustmentForShapeBestIntra8X8Winner = 0;
            cmd.DW30.QpAdjustmentForShapeBestIntra16X16Winner = 0;

            cmd.DW31.BestdistortionQpAdjustmentForZone0 = 0;
            cmd.DW31.BestdistortionQpAdjustmentForZone1 = 1;
            cmd.DW31.BestdistortionQpAdjustmentForZone2 = 2;
            cmd.DW31.BestdistortionQpAdjustmentForZone3 = 3;
        }

        if (params->bVdencBRCEnabled && avcPicParams->NumDirtyROI && params->bVdencStreamInEnabled)
        {
            cmd.DW34.RoiEnable = true;
        }

        if (params->bVdencStreamInEnabled)
        {
            cmd.DW34.FwdPredictor0MvEnable = 1;
            cmd.DW34.PpmvDisable           = 1;
        }
        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TVdencCmds::VDENC_WALKER_STATE_CMD cmd;

        if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            MHW_MI_CHK_NULL(params->pAvcSeqParams);
            MHW_MI_CHK_NULL(params->pAvcSlcParams);

            auto avcSeqParams = params->pAvcSeqParams;
            auto avcSlcParams = params->pAvcSlcParams;

            cmd.DW1.MbLcuStartYPosition = avcSlcParams->first_mb_in_slice / CODECHAL_GET_WIDTH_IN_MACROBLOCKS(avcSeqParams->FrameWidth);

            cmd.DW2.NextsliceMbStartYPosition = (avcSlcParams->first_mb_in_slice + avcSlcParams->NumMbsForSlice) / CODECHAL_GET_WIDTH_IN_MACROBLOCKS(avcSeqParams->FrameWidth);

            if (cmd.DW2.NextsliceMbStartYPosition > (uint32_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(avcSeqParams->FrameHeight))
            {
                cmd.DW2.NextsliceMbStartYPosition = (uint32_t)CODECHAL_GET_HEIGHT_IN_MACROBLOCKS(avcSeqParams->FrameHeight);
            }

            cmd.DW3.Log2WeightDenomLuma = avcSlcParams->luma_log2_weight_denom;

            cmd.DW5.TileWidth = avcSeqParams->FrameWidth - 1;
        }
        else if (params->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            auto paramsG11 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11>(params);
            MHW_MI_CHK_NULL(paramsG11);
            MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
            MHW_MI_CHK_NULL(params->pHevcEncPicParams);
            MHW_MI_CHK_NULL(params->pEncodeHevcSliceParams);

            auto seqParams = params->pHevcEncSeqParams;
            auto picParams = params->pHevcEncPicParams;
            auto sliceParams = params->pEncodeHevcSliceParams;

            uint32_t ctbSize     = 1 << (seqParams->log2_max_coding_block_size_minus3 + 3);
            uint32_t widthInPix  = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameWidthInMinCbMinus1 + 1);
            uint32_t widthInCtb  = (widthInPix / ctbSize) + ((widthInPix % ctbSize) ? 1 : 0);  // round up
            uint32_t heightInPix = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameHeightInMinCbMinus1 + 1);
            uint32_t heightInCtb = (heightInPix / ctbSize) + ((heightInPix % ctbSize) ? 1 : 0);  // round up
            uint32_t shift     = seqParams->log2_max_coding_block_size_minus3 - seqParams->log2_min_coding_block_size_minus3;

            cmd.DW3.Log2WeightDenomLuma = cmd.DW3.HevcLog2WeightDemonLuma = 
                (picParams->weighted_pred_flag || picParams->weighted_bipred_flag) ? (picParams->bEnableGPUWeightedPrediction ? 6 : sliceParams->luma_log2_weight_denom) : 0;

            if (paramsG11->pTileCodingParams == nullptr)
            {
                // No tiling support
                cmd.DW1.MbLcuStartYPosition          = sliceParams->slice_segment_address / widthInCtb;
                cmd.DW2.NextsliceMbLcuStartXPosition = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / heightInCtb;
                cmd.DW2.NextsliceMbStartYPosition    = (sliceParams->slice_segment_address + sliceParams->NumLCUsInSlice) / widthInCtb;
                cmd.DW5.TileWidth                    = widthInPix - 1;
                cmd.DW5.TileHeight                   = heightInPix - 1;
            }
            else
            {
                cmd.DW1.MbLcuStartXPosition          = paramsG11->pTileCodingParams->TileStartLCUX;
                cmd.DW1.MbLcuStartYPosition          = paramsG11->pTileCodingParams->TileStartLCUY;

                cmd.DW2.NextsliceMbLcuStartXPosition = paramsG11->pTileCodingParams->TileStartLCUX + (paramsG11->pTileCodingParams->TileWidthInMinCbMinus1 >> shift) + 1;
                cmd.DW2.NextsliceMbStartYPosition    = paramsG11->pTileCodingParams->TileStartLCUY + (paramsG11->pTileCodingParams->TileHeightInMinCbMinus1 >> shift) + 1;

                cmd.DW4.TileStartCtbX                = paramsG11->pTileCodingParams->TileStartLCUX * ctbSize;
                cmd.DW4.TileStartCtbY                = paramsG11->pTileCodingParams->TileStartLCUY * ctbSize;

                cmd.DW5.TileWidth                    = ((paramsG11->pTileCodingParams->TileWidthInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3)) - 1;
                cmd.DW5.TileHeight                   = ((paramsG11->pTileCodingParams->TileHeightInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3)) - 1;
                cmd.DW1.FirstSuperSlice              = 1;
                cmd.DW3.NumParEngine                 = paramsG11->dwNumberOfPipes;
                cmd.DW3.TileNumber                   = paramsG11->dwTileId;

                //Aligned Tile height & frame width
                uint32_t frameHeightInLCU            = ((seqParams->wFrameHeightInMinCbMinus1 + 1) * (1 << (seqParams->log2_min_coding_block_size_minus3 + 3))) / ctbSize;
                //StreamIn data is 4 CLs per LCU, Offset is programmed every tile
                cmd.DW6.StreaminOffsetEnable         = 1;
                cmd.DW6.TileStreaminOffset           = paramsG11->pTileCodingParams->TileStreaminOffset;

                cmd.DW8.TileStreamoutOffsetEnable    = 1;
                cmd.DW8.TileStreamoutOffset          = paramsG11->dwTileId * 19;

                //PAK Object StreamOut Offset Computation : Only on Tile Columns
                if (cmd.DW4.TileStartCtbY == 0)
                {
                    //RowStore Offset Computation
                    uint32_t num32x32InX             = (cmd.DW4.TileStartCtbX) / 32;
                    cmd.DW7.RowStoreOffsetEnable     = 1;
                    cmd.DW7.TileRowstoreOffset       = num32x32InX;

                    cmd.DW9.LcuStreamOutOffsetEnable = 1;
                    //Aligned Tile width & frame height
                    uint32_t widthInLCU              = cmd.DW4.TileStartCtbX / ctbSize;

                    // we need additional buffer for (1) 1 CL for size info at the beginning of each tile column (max of 4 vdbox in scalability mode)
                    // (2) CL alignment at end of every tile column
                    // as a result, increase the height by 1 for offset purposes
                    uint32_t numOfLCU                = widthInLCU*(frameHeightInLCU + 1);
                    uint32_t maxNumCUInLCU           = (64 / 8)*(64 / 8); //max LCU size is 64, min Cu size is 8

                    uint32_t tileLCUStreamOutByteOffset = 2 * BYTES_PER_DWORD * numOfLCU * (NUM_PAK_DWS_PER_LCU + maxNumCUInLCU * NUM_DWS_PER_CU);
                    cmd.DW9.TileLcuStreamOutOffset      = MOS_ROUNDUP_DIVIDE(tileLCUStreamOutByteOffset, MHW_CACHELINE_SIZE);
                }
            }
        }
        else if (params->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            auto paramsG11 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G11>(params);
            MHW_MI_CHK_NULL(paramsG11);
            MHW_MI_CHK_NULL(params->pVp9EncPicParams);
            auto vp9PicParams     = params->pVp9EncPicParams;
            auto tileCodingParams = paramsG11->pTileCodingParams;

            cmd.DW8.TileStreamoutOffsetEnable = 1;
            cmd.DW6.StreaminOffsetEnable      = 1;

            if (tileCodingParams == nullptr)
            {
                cmd.DW2.NextsliceMbLcuStartXPosition = CODECHAL_GET_WIDTH_IN_BLOCKS(vp9PicParams->SrcFrameWidthMinus1, CODEC_VP9_SUPER_BLOCK_WIDTH);
                cmd.DW2.NextsliceMbStartYPosition    = CODECHAL_GET_HEIGHT_IN_BLOCKS(vp9PicParams->SrcFrameHeightMinus1, CODEC_VP9_SUPER_BLOCK_HEIGHT);
                cmd.DW5.TileWidth                    = vp9PicParams->SrcFrameWidthMinus1;
                cmd.DW5.TileHeight                   = vp9PicParams->SrcFrameHeightMinus1;
                cmd.DW1.FirstSuperSlice              = 1;
            }
            else
            {
                cmd.DW1.MbLcuStartXPosition = tileCodingParams->TileStartLCUX;
                cmd.DW1.MbLcuStartYPosition = tileCodingParams->TileStartLCUY;

                cmd.DW5.TileWidth  = ((tileCodingParams->TileWidthInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
                cmd.DW5.TileHeight = ((tileCodingParams->TileHeightInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

                cmd.DW4.TileStartCtbX = tileCodingParams->TileStartLCUX * CODEC_VP9_SUPER_BLOCK_WIDTH;
                cmd.DW4.TileStartCtbY = tileCodingParams->TileStartLCUY * CODEC_VP9_SUPER_BLOCK_HEIGHT;

                cmd.DW2.NextsliceMbLcuStartXPosition = CODECHAL_GET_WIDTH_IN_BLOCKS((cmd.DW4.TileStartCtbX + cmd.DW5.TileWidth + 1), CODEC_VP9_SUPER_BLOCK_WIDTH);
                cmd.DW2.NextsliceMbStartYPosition    = CODECHAL_GET_HEIGHT_IN_BLOCKS((cmd.DW4.TileStartCtbY + cmd.DW5.TileHeight + 1), CODEC_VP9_SUPER_BLOCK_HEIGHT);

                cmd.DW1.FirstSuperSlice = 1;
                cmd.DW3.NumParEngine    = paramsG11->dwNumberOfPipes;
                cmd.DW3.TileNumber      = paramsG11->dwTileId;

                uint32_t tileStartXInSBs = (cmd.DW4.TileStartCtbX / CODEC_VP9_SUPER_BLOCK_WIDTH);
                uint32_t tileStartYInSBs = (cmd.DW4.TileStartCtbY / CODEC_VP9_SUPER_BLOCK_HEIGHT);
                //Aligned Tile height & frame width
                uint32_t tileHeightInSBs = (cmd.DW5.TileHeight + 1 + (CODEC_VP9_SUPER_BLOCK_HEIGHT - 1)) / CODEC_VP9_SUPER_BLOCK_HEIGHT;
                uint32_t frameWidthInSBs = (vp9PicParams->SrcFrameWidthMinus1 + 1 + (CODEC_VP9_SUPER_BLOCK_WIDTH - 1)) / CODEC_VP9_SUPER_BLOCK_WIDTH;

                cmd.DW6.TileStreaminOffset = (tileStartYInSBs * frameWidthInSBs + tileStartXInSBs * tileHeightInSBs) * (4); //StreamIn data is 4 CLs per LCU

                                                                                                                            //Frame Stats Offset
                cmd.DW8.TileStreamoutOffset = (paramsG11->dwTileId * 19); // 3 CLs or 48 DWs of statistics data + 16CLs or 256 DWs of Histogram data              

                                                                          // If Tile Column, compute PAK Object StreamOut Offsets 
                if (cmd.DW4.TileStartCtbY == 0)
                {
                    //RowStore Offset Computation
                    uint32_t num32x32sInX        = (cmd.DW4.TileStartCtbX) / 32;
                    cmd.DW7.RowStoreOffsetEnable = 1;
                    cmd.DW7.TileRowstoreOffset   = num32x32sInX;

                    //Aligned Tile width & frame height
                    uint32_t widthInSBs                    = (cmd.DW4.TileStartCtbX) / CODEC_VP9_SUPER_BLOCK_WIDTH;
                    uint32_t frameHeightInSBs              = ((vp9PicParams->SrcFrameHeightMinus1 + 1) + (CODEC_VP9_SUPER_BLOCK_HEIGHT - 1)) / CODEC_VP9_SUPER_BLOCK_HEIGHT;
                    uint32_t numOfSBs                      = widthInSBs * (frameHeightInSBs + 1);
                    uint32_t maxNumOfCUInSB                = (CODEC_VP9_SUPER_BLOCK_HEIGHT / CODEC_VP9_MIN_BLOCK_HEIGHT)*(CODEC_VP9_SUPER_BLOCK_WIDTH / CODEC_VP9_MIN_BLOCK_WIDTH); //max LCU size is 64, min Cu size is 8
                    uint32_t tileLCUStreamOutOffsetInBytes = 2 * 4 * ((numOfSBs * 5) + (numOfSBs*maxNumOfCUInSB * 8));

                    cmd.DW9.LcuStreamOutOffsetEnable       = 1;
                    cmd.DW9.TileLcuStreamOutOffset         = MOS_ROUNDUP_DIVIDE(tileLCUStreamOutOffsetInBytes, MHW_CACHELINE_SIZE);
                }
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencAvcWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pAvcPicParams);

        typename TVdencCmds::VDENC_WEIGHTSOFFSETS_STATE_CMD cmd;

        auto avcPicParams = params->pAvcPicParams;

        if (avcPicParams->weighted_pred_flag == 1)
        {
            cmd.DW1.WeightsForwardReference0 = params->Weights[0][0][0][0];
            cmd.DW1.OffsetForwardReference0  = params->Weights[0][0][0][1];
            cmd.DW1.WeightsForwardReference1 = params->Weights[0][1][0][0];
            cmd.DW1.OffsetForwardReference1  = params->Weights[0][1][0][1];
            cmd.DW2.WeightsForwardReference2 = params->Weights[0][2][0][0];
            cmd.DW2.OffsetForwardReference2  = params->Weights[0][2][0][1];
        }
        //set to default value when weighted prediction not enabled
        else
        {
            cmd.DW1.WeightsForwardReference0 = 1;
            cmd.DW1.OffsetForwardReference0  = 0;
            cmd.DW1.WeightsForwardReference1 = 1;
            cmd.DW1.OffsetForwardReference1  = 0;
            cmd.DW2.WeightsForwardReference2 = 1;
            cmd.DW2.OffsetForwardReference2  = 0;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                     cmdBuffer,
        PMHW_BATCH_BUFFER                       batchBuffer,
        PMHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS   params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        typename TVdencCmds::VDENC_WEIGHTSOFFSETS_STATE_CMD cmd;

        cmd.DW1.WeightsForwardReference0         = 1;
        cmd.DW1.OffsetForwardReference0          = 0;
        cmd.DW1.WeightsForwardReference1         = 1;
        cmd.DW1.OffsetForwardReference1          = 0;
        cmd.DW2.WeightsForwardReference2         = 1;
        cmd.DW2.OffsetForwardReference2          = 0;
        cmd.DW3.HevcVp9WeightsForwardReference0  = 1;
        cmd.DW3.HevcVp9OffsetForwardReference0   = 0;
        cmd.DW3.HevcVp9WeightsForwardReference1  = 1;
        cmd.DW3.HevcVp9OffsetForwardReference1   = 0;
        cmd.DW4.HevcVp9WeightsForwardReference2  = 1;
        cmd.DW4.HevcVp9OffsetForwardReference2   = 0;
        cmd.DW4.HevcVp9WeightsBackwardReference0 = 1;
        cmd.DW4.HevcVp9OffsetBackwardReference0  = 0;

        // Luma Offsets and Weights
        if (params->bWeightedPredEnabled)
        {
            uint32_t  refPicListNum = 0;
            // DWORD 3
            cmd.DW3.HevcVp9WeightsForwardReference0  = CodecHal_Clip3(-128, 127,
                                                        params->LumaWeights[refPicListNum][0] + params->dwDenom);
            cmd.DW3.HevcVp9OffsetForwardReference0   = params->LumaOffsets[refPicListNum][0];
            cmd.DW3.HevcVp9WeightsForwardReference1  = CodecHal_Clip3(-128, 127,
                                                        params->LumaWeights[refPicListNum][1] + params->dwDenom);
            cmd.DW3.HevcVp9OffsetForwardReference1   = params->LumaOffsets[refPicListNum][1];
            // DWORD 4
            cmd.DW4.HevcVp9WeightsForwardReference2  = CodecHal_Clip3(-128, 127,
                                                        params->LumaWeights[refPicListNum][2] + params->dwDenom);
            cmd.DW4.HevcVp9OffsetForwardReference2   = params->LumaOffsets[refPicListNum][2];
            // HEVC VDEnc currently only supports LDB. LDB doesn't have backward reference.
            cmd.DW4.HevcVp9WeightsBackwardReference0 = 0;
            cmd.DW4.HevcVp9OffsetBackwardReference0  = 0;
        }

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

   MOS_STATUS AddVdencCmd1Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD1_PARAMS        params)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencCmd2Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD2_STATE         params)
    {
        return MOS_STATUS_SUCCESS;
    }

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CreateMhwVdboxPipeModeSelectParams()
    {
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G11);

        return pipeModeSelectParams;
    }

    void ReleaseMhwVdboxPipeModeSelectParams(PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams) 
    {
        MOS_Delete(pipeModeSelectParams);
    }
};

//!  MHW Vdbox Vdenc interface for Gen11 ICL
/*!
This class defines the Vdenc command construction functions for Gen11 platform
*/
class MhwVdboxVdencInterfaceG11Icl : public MhwVdboxVdencInterfaceG11<mhw_vdbox_vdenc_g11_X>
{
public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxVdencInterfaceG11Icl(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceG11(osInterface)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG11Icl() { }

};

#endif

