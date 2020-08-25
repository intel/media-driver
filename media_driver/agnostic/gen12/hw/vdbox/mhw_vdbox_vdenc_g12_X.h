/*===================== begin_copyright_notice ==================================

Copyright (c) 2017-2020, Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/

//! \file     mhw_vdbox_vdenc_g12_X.h
//! \details  Defines functions for constructing Vdbox Vdenc commands on Gen12-based platforms
//!

#ifndef __MHW_VDBOX_VDENC_G12_X_H__
#define __MHW_VDBOX_VDENC_G12_X_H__

#include "mhw_vdbox_vdenc_generic.h"
#include "mhw_vdbox_vdenc_hwcmd_g12_X.h"
#include "mhw_vdbox_g12_X.h"

#define VDENCHEVC_RSC_OFFSET_C420OR422_DXX_LCU32OR64_4K_G12               1824
#define VDENCHEVC_RSC_OFFSET_C420OR422_DXX_LCU32OR64_8K_G12               2304
#define VDENCHEVC_RSC_OFFSET_C444_D8_LCU32OR64_4K_G12                     1568
#define VDENCHEVC_RSC_OFFSET_C444_D8_LCU32OR64_8K_G12                     2112
#define VDENCHEVC_RSC_OFFSET_C444_D10_LCU32OR64_4K_G12                    2336
#define VDENCHEVC_RSC_OFFSET_C444_D10_LCU32OR64_8K_G12                    1600
#define VDENCHEVC_RSC_OFFSET_C444_D12_LCU32OR64_4K_G12                    2336
#define VDENCHEVC_RSC_OFFSET_C444_D12_LCU32OR64_8K_G12                    1600

// TGL rowstore Cache Values
#define VP9VDENCROWSTORE_BASEADDRESS_1536                                 1536
#define VP9VDENCROWSTORE_BASEADDRESS_2304                                 2304
#define VP9VDENCROWSTORE_BASEADDRESS_2368                                 2368
#define VP9VDENCROWSTORE_BASEADDRESS_2112                                 2112
#define VP9VDENCROWSTORE_BASEADDRESS_1920                                 1920
#define VP9VDENCROWSTORE_BASEADDRESS_768                                  768
#define RESERVED_VDENC_ROWSTORE_BASEADDRESS                               2370
#define RESERVED_VDENC_IPDL_ROWSTORE_BASEADDRESS                          384
#define AVC_VDENC_IPDL_ROWSTORE_BASEADDRESS                               512

#define GEN12_AVC_VDENC_ROWSTORE_BASEADDRESS                                  1280
#define GEN12_AVC_VDENC_ROWSTORE_BASEADDRESS_MBAFF                            1536
#define GEN12_VP8_VDENC_ROWSTORE_BASEADDRESS                                  1536

#define OFFSET_OF(TYPE, MEMBER) ((size_t) & ((TYPE *)0)->MEMBER )

typedef struct _MHW_VDBOX_VDENC_CONTROL_STATE_PARAMS
{
    bool      bVdencInitialization;
}MHW_VDBOX_VDENC_CONTROL_STATE_PARAMS, *PMHW_VDBOX_VDENC_CONTROL_STATE_PARAMS;

struct MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12 : public MHW_VDBOX_VDENC_WALKER_STATE_PARAMS
{
    // GEN11+ tiling support
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12   pTileCodingParams = nullptr;
    uint32_t                                dwNumberOfPipes = 0;
    uint32_t                                dwTileId = 0;
    uint32_t                                IBCControl = 0;
    uint32_t                                RowStaticInfo_31_0 = 0;
    uint32_t                                RowStaticInfo_63_32 = 0;
    uint32_t                                RowStaticInfo_95_64 = 0;
    uint32_t                                RowStaticInfo_127_96 = 0;
    uint32_t                                ctbSize = 0;
    uint32_t                                minCodingBlkSize = 0;
    uint32_t                                frameWidthMinus1 = 0;
    uint32_t                                frameHeightMinus1 = 0;
};
using PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12 = MHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12 *;

struct MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12
{
    union
    {
        struct
        {
            uint16_t       bWaitDoneHEVC            : 1;
            uint16_t       bWaitDoneVDENC           : 1;
            uint16_t       bWaitDoneMFL             : 1;
            uint16_t       bWaitDoneMFX             : 1;
            uint16_t       bWaitDoneVDCmdMsgParser  : 1;
            uint16_t       bFlushHEVC               : 1;
            uint16_t       bFlushVDENC              : 1;
            uint16_t       bFlushMFL                : 1;
            uint16_t       bFlushMFX                : 1;
            uint16_t       bWaitDoneAV1             : 1;
            uint16_t       bFlushAV1                : 1;
            uint16_t                                : 5;
        };
        struct
        {
            uint16_t       Value;
        };
    }Flags;
};
using PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12 = MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12 *;

//!  MHW Vdbox Vdenc interface for Gen12
/*!
This class defines the Vdenc command construction functions for Gen12 platforms as template
*/
template <typename TVdencCmds>
class MhwVdboxVdencInterfaceG12 : public MhwVdboxVdencInterfaceGeneric<TVdencCmds>
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

    enum VDENC_WALKER_STATE_IBC_CONTROL
    {
        VDENC_WALKER_STATE_COMMAND_IBC_CONTROL_IBC_DISABLED_G12 = 0x0,
        VDENC_WALKER_STATE_COMMAND_IBC_CONTROL_IBC_ONLY_LBC_G12 = 0x1,
        VDENC_WALKER_STATE_COMMAND_IBC_CONTROL_IBC_ENABLED_TBCLBC_G12 = 0x3,
    };

    MOS_STATUS InitRowstoreUserFeatureSettings() override
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

    //!
    //! \brief    Translate MOS type format to vdenc surface raw format
    //! \details  VDBOX protected function to translate mos format to media state format
    //! \param    MOS_FORMAT  Format
    //!           [in] MOS type format
    //! \return   VdencSurfaceFormat
    //!           media state surface format
    //!
    VdencSurfaceFormat MosFormatToVdencSurfaceRawFormat(MOS_FORMAT format)
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
    //! \brief    Translate MOS type format to vdenc surface recon format
    //! \details  VDBOX protected function to translate mos format to media state recon format
    //! \param    MOS_FORMAT  Format
    //!           [in] MOS type format
    //! \return   VdencSurfaceFormat
    //!           media state surface format
    //!
    VdencSurfaceFormat MosFormatToVdencSurfaceReconFormat(MOS_FORMAT format)
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

    //!
    //! \brief    Adds VD Pipeline Flush command in command buffer
    //! \details  Client facing function to add VD Pipeline Flush command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    MOS_STATUS AddVdPipelineFlushCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        typename TVdencCmds::VD_PIPELINE_FLUSH_CMD cmd;
        PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12 paramsG12 = (PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12)params;

        cmd.DW1.HevcPipelineDone           = params->Flags.bWaitDoneHEVC;
        cmd.DW1.VdencPipelineDone          = params->Flags.bWaitDoneVDENC;
        cmd.DW1.MflPipelineDone            = params->Flags.bWaitDoneMFL;
        cmd.DW1.MfxPipelineDone            = params->Flags.bWaitDoneMFX;
        cmd.DW1.VdCommandMessageParserDone = params->Flags.bWaitDoneVDCmdMsgParser;
        cmd.DW1.HevcPipelineCommandFlush   = params->Flags.bFlushHEVC;
        cmd.DW1.VdencPipelineCommandFlush  = params->Flags.bFlushVDENC;
        cmd.DW1.MflPipelineCommandFlush    = params->Flags.bFlushMFL;
        cmd.DW1.MfxPipelineCommandFlush    = params->Flags.bFlushMFX;
        cmd.DW1.AvpPipelineDone            = paramsG12->Flags.bWaitDoneAV1;
        cmd.DW1.AvpPipelineCommandFlush    = paramsG12->Flags.bFlushAV1;

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxVdencInterfaceG12(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceGeneric<TVdencCmds>(osInterface)
    {
        MHW_FUNCTION_ENTER;

        this->m_rhoDomainStatsEnabled = true;
        InitRowstoreUserFeatureSettings();
    }

    inline virtual uint32_t GetVdencCmd1Size() override
    {
        return 0;
    }

    inline virtual uint32_t GetVdencCmd2Size() override
    {
        return 0;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG12() { }

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(rowstoreParams);

        bool is8bit = rowstoreParams->ucBitDepthMinus8 == 0;
        bool is10bit = rowstoreParams->ucBitDepthMinus8 == 1 || rowstoreParams->ucBitDepthMinus8 == 2;
        bool is12bit = rowstoreParams->ucBitDepthMinus8 > 2;
        bool isLcu32or64 = rowstoreParams->ucLCUSize == 32 || rowstoreParams->ucLCUSize == 64;
        bool isGt2k = rowstoreParams->dwPicWidth > 2048;
        bool isGt4k = rowstoreParams->dwPicWidth > 4096;
        bool isGt8k = rowstoreParams->dwPicWidth > 8192;
        uint32_t index = 0;

        bool avc = rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC;
        bool vp8 = rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP8;
        bool widthLE4K = rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_4K;
        bool mbaffOrField = rowstoreParams->bMbaff || !rowstoreParams->bIsFrame;
        this->m_vdencRowStoreCache.bEnabled = false;
        this->m_vdencRowStoreCache.dwAddress = 0;

        //VDENC row store cache setting for AVC and VP8
        this->m_vdencRowStoreCache.bEnabled = this->m_vdencRowStoreCache.bSupported && widthLE4K && (avc || vp8);
        this->m_vdencRowStoreCache.dwAddress = avc ? (mbaffOrField ? GEN12_AVC_VDENC_ROWSTORE_BASEADDRESS_MBAFF :
            GEN12_AVC_VDENC_ROWSTORE_BASEADDRESS) : GEN12_VP8_VDENC_ROWSTORE_BASEADDRESS;
        this->m_vdencRowStoreCache.dwAddress = this->m_vdencRowStoreCache.bEnabled ? this->m_vdencRowStoreCache.dwAddress : 0;

        if (this->m_vdencRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            if (rowstoreParams->ucChromaFormat != HCP_CHROMA_FORMAT_YUV444)
            {
                index = 2 * isGt4k + isLcu32or64;
            }
            else
            {
                uint32_t subidx = is12bit ? 2 : (is10bit ? 1 : 0);
                index = 4 + 6 * isLcu32or64 + 2 * subidx + isGt4k;
            }

            if (!isGt8k)
            {
                this->m_vdencRowStoreCache.bEnabled  = RowStoreCacheEnableHEVC[index][3];
                this->m_vdencRowStoreCache.dwAddress = RowStoreCacheAddrHEVC[index][3];
            }
        }

        //VP9 VDENC
        if (this->m_vdencRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            if ((rowstoreParams->ucChromaFormat >= HCP_CHROMA_FORMAT_YUV420) &&
                (rowstoreParams->ucChromaFormat <= HCP_CHROMA_FORMAT_YUV444))
            {
                index = 4 * (rowstoreParams->ucChromaFormat - HCP_CHROMA_FORMAT_YUV420) + 2 * (!is8bit) + isGt4k;
            }
            else
            {
                return MOS_STATUS_SUCCESS;
            }

            if (rowstoreParams->ucChromaFormat == HCP_CHROMA_FORMAT_YUV444 && !is8bit)
            {
                index += isGt2k;
            }

            if (!isGt8k)
            {
                this->m_vdencRowStoreCache.bEnabled = RowStoreCacheEnableVP9[index][3];
                this->m_vdencRowStoreCache.dwAddress = RowStoreCacheAddrVP9[index][3];
            }
        }

        if (this->m_vdencRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_ENCODE_RESERVED_0)
        {
            this->m_vdencRowStoreCache.bEnabled  = true;
            this->m_vdencRowStoreCache.dwAddress = RESERVED_VDENC_ROWSTORE_BASEADDRESS;

            //IPDL
            this->m_vdencIpdlRowstoreCache.dwAddress = RESERVED_VDENC_IPDL_ROWSTORE_BASEADDRESS;

        }
        else if (this->m_vdencRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            this->m_vdencRowStoreCache.bEnabled = true;

            //IPDL
            this->m_vdencIpdlRowstoreCache.dwAddress = AVC_VDENC_IPDL_ROWSTORE_BASEADDRESS;

        }

        return MOS_STATUS_SUCCESS;
    }

    uint32_t GetAvcMaxSize(uint32_t waAddDelayInVDEncDynamicSlice)
    {
        uint32_t maxSize =
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

        return maxSize;
    }

    MOS_STATUS GetVdencStateCommandsDataSize(
        uint32_t mode,
        uint32_t waAddDelayInVDEncDynamicSlice,
        uint32_t *commandsSize,
        uint32_t *patchListSize) override
    {
        MHW_FUNCTION_ENTER;

        uint32_t            maxSize = 0;
        uint32_t            patchListMaxSize = 0;
        uint32_t            standard = CodecHal_GetStandardFromMode(mode);

        if (standard == CODECHAL_AVC)
        {
            maxSize = GetAvcMaxSize(waAddDelayInVDEncDynamicSlice);
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
            *commandsSize = 0;
            *patchListSize = 0;
            return MOS_STATUS_UNKNOWN;
        }

        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    uint32_t GetAvcSliceMaxSize()
    {
        uint32_t maxSize =
            TVdencCmds::VDENC_WEIGHTSOFFSETS_STATE_CMD::byteSize +
            TVdencCmds::VDENC_WALKER_STATE_CMD::byteSize +
            TVdencCmds::VD_PIPELINE_FLUSH_CMD::byteSize;

        return maxSize;
    }

    MOS_STATUS GetVdencPrimitiveCommandsDataSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize) override
    {
        MHW_FUNCTION_ENTER;

        uint32_t            maxSize = 0;
        uint32_t            patchListMaxSize = 0;
        uint32_t            standard = CodecHal_GetStandardFromMode(mode);

        if (standard == CODECHAL_AVC)
        {
            maxSize = GetAvcSliceMaxSize();
            patchListMaxSize = VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES;
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported encode mode.");
            *commandsSize = 0;
            *patchListSize = 0;
            return MOS_STATUS_UNKNOWN;
        }

        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        auto paramsG12 = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(params);
        MHW_MI_CHK_NULL(paramsG12);
        typename TVdencCmds::VDENC_PIPE_MODE_SELECT_CMD cmd;

        cmd.DW1.StandardSelect                 = CodecHal_GetStandardFromMode(params->Mode);
        cmd.DW1.ScalabilityMode                = !(paramsG12->MultiEngineMode == MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY);
        if (CODECHAL_ENCODE_MODE_HEVC == params->Mode || CODECHAL_ENCODE_RESERVED_0 == params->Mode)
        {
            cmd.DW1.FrameStatisticsStreamOutEnable = paramsG12->bBRCEnabled || paramsG12->bLookaheadPass;
        }
        // AVC VENC to be optimized later
        else
        {
            cmd.DW1.FrameStatisticsStreamOutEnable = 1;
        }
        cmd.DW1.VdencPakObjCmdStreamOutEnable  = params->bVdencPakObjCmdStreamOutEnable;
        cmd.DW1.TlbPrefetchEnable              = 1;
        cmd.DW1.PakThresholdCheckEnable        = params->bDynamicSliceEnable;
        cmd.DW1.VdencStreamInEnable            = params->bVdencStreamInEnable;
        cmd.DW1.BitDepth                       = params->ucVdencBitDepthMinus8;

        if (CODECHAL_ENCODE_MODE_HEVC == params->Mode || CODECHAL_ENCODE_MODE_VP9 == params->Mode || CODECHAL_ENCODE_RESERVED_0 == params->Mode)
        {
            cmd.DW1.PakChromaSubSamplingType = params->ChromaType;
        }
        // by default RGB to YUV using full to studio range
        // can add a DDI flag to control if needed
        cmd.DW1.OutputRangeControlAfterColorSpaceConversion = 1;

        // for tile encoding
        cmd.DW1.TileReplayEnable = paramsG12->bTileBasedReplayMode;

        cmd.DW1.IsRandomAccess = paramsG12->bIsRandomAccess;

        //Restriction: When this pre-fetch is enabled,TLB Prefetch Enable in VDENC_PIPE_MODE select (DW 1 bit 7) should be disabled.
        cmd.DW2.HmeRegionPreFetchenable = 0;

        // TLB prefetch performance improvement
        if (params->Mode == CODECHAL_ENCODE_MODE_AVC)
        {
            cmd.DW3.PreFetchoffsetforsource = 7;
            cmd.DW3.Numverticalreqminus1Src = 0;
        }
        else if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || CODECHAL_ENCODE_RESERVED_0 == params->Mode)
        {
            cmd.DW3.PreFetchoffsetforsource = 4;
            cmd.DW3.Numverticalreqminus1Src = 1;
        }

        //Restriction : This field can be set only to planar source formats.
        if (params->Format != Format_NV12 && params->Format != Format_P010)
        {
            cmd.DW3.SourceChromaTlbPreFetchenable = 0;
        }

        // For RGB encoding
        if (paramsG12->bRGBEncodingMode)
        {
            cmd.DW1.RgbEncodingEnable    = 1;
            // To add the primary channel selection later here
        }

        // For parallel encode from display
        if (paramsG12->bWirelessEncodeEnabled)
        {
            cmd.DW5.CaptureMode                        = TVdencCmds::VDENC_PIPE_MODE_SELECT_CMD::CAPTURE_MODE_UNNAMED1;
            cmd.DW5.ParallelCaptureAndEncodeSessionId  = paramsG12->ucWirelessSessionId;
            cmd.DW5.TailPointerReadFrequency           = 0x50;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params) override
    {
        MOS_SURFACE details;
        uint8_t     refIdx;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD cmd;

        MOS_MEMCOMP_STATE   mmcMode = MOS_MEMCOMP_DISABLED;
        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_VDENC_PIPE_BUF_ADDR;

        if (params->psRawSurface != nullptr)
        {
            cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(params->RawSurfMmcState) ? 1 : 0;
            cmd.OriginalUncompressedPicture.PictureFields.DW0.CompressionType = MmcIsRc(params->RawSurfMmcState) ? 1 : 0;
            cmd.OriginalUncompressedPicture.PictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;

            cmd.OriginalUncompressedPicture.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

            resourceParams.presResource    = &params->psRawSurface->OsResource;
            resourceParams.dwOffset        = params->psRawSurface->dwOffset;
            resourceParams.pdwCmd          = (uint32_t*)&(cmd.OriginalUncompressedPicture.LowerAddress);
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
            resourceParams.pdwCmd          = (uint32_t*)&(cmd.RowStoreScratchBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = 16;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
        if (params->psFwdRefSurface0)
        {
            resourceParams.presResource = &params->psFwdRefSurface0->OsResource;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (uint32_t*)&(cmd.FwdRef0.LowerAddress);
            resourceParams.dwLocationInCmd = 22;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->psFwdRefSurface1)
        {
            resourceParams.presResource = &params->psFwdRefSurface1->OsResource;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (uint32_t*)&(cmd.FwdRef1.LowerAddress);
            resourceParams.dwLocationInCmd = 25;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->psFwdRefSurface2)
        {
            resourceParams.presResource = &params->psFwdRefSurface2->OsResource;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (uint32_t*)&(cmd.FwdRef2.LowerAddress);
            resourceParams.dwLocationInCmd = 28;
            resourceParams.bIsWritable = true;

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
            resourceParams.pdwCmd          = (uint32_t*)&(cmd.VdencStatisticsStreamout.LowerAddress);
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
            resourceParams.pdwCmd          = (uint32_t*)&(cmd.StreaminDataPicture.LowerAddress);
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
                    resourceParams.pdwCmd = (uint32_t*)&(cmd.FwdRef0.LowerAddress);
                    break;
                case 1:
                    resourceParams.pdwCmd = (uint32_t*)&(cmd.FwdRef1.LowerAddress);
                    break;
                case 2:
                    resourceParams.pdwCmd = (uint32_t*)&(cmd.FwdRef2.LowerAddress);
                    break;
                case 3:
                    resourceParams.pdwCmd = (uint32_t*)&(cmd.BwdRef0.LowerAddress);
                    break;
                default:
                    break;
                }

                mmcMode = (params->PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
                    params->PostDeblockSurfMmcState : params->PreDeblockSurfMmcState;
                switch (refIdx)
                {
                case 0:
                    cmd.FwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                    cmd.FwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                    cmd.FwdRef0.PictureFields.DW0.MemoryObjectControlState =
                        this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                    cmd.FwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                case 1:
                    cmd.FwdRef1.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                    cmd.FwdRef1.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                    cmd.FwdRef1.PictureFields.DW0.MemoryObjectControlState =
                        this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                    cmd.FwdRef1.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                case 2:
                    cmd.FwdRef2.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                    cmd.FwdRef2.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                    cmd.FwdRef2.PictureFields.DW0.MemoryObjectControlState =
                        this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                    cmd.FwdRef2.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    break;
                case 3:
                    cmd.BwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                    cmd.BwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                    cmd.BwdRef0.PictureFields.DW0.MemoryObjectControlState =
                        this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                    cmd.BwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
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
                    switch (refIdx)
                    {
                    case 0:
                        resourceParams.pdwCmd = (uint32_t*)&(cmd.DsFwdRef0.LowerAddress);
                        break;
                    case 1:
                        resourceParams.pdwCmd = (uint32_t*)&(cmd.DsFwdRef1.LowerAddress);
                        break;
                    default:
                        break;
                    }

                    mmcMode = params->Ps4xDsSurfMmcState;
                    switch (refIdx)
                    {
                    case 0:
                        cmd.DsFwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef0.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        break;
                    case 1:
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef1.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
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
                else if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_MODE_VP9 || params->Mode == CODECHAL_ENCODE_RESERVED_0)
                {
                    // 8x DS surface
                    MOS_ZeroMemory(&details, sizeof(details));
                    details.Format = Format_Invalid;
                    MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdenc8xDsSurface[refIdx], &details));

                    resourceParams.presResource    = params->presVdenc8xDsSurface[refIdx];
                    resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
                    resourceParams.dwLocationInCmd = (refIdx * 3) + 1;
                    resourceParams.bIsWritable     = false;
                    switch (refIdx)
                    {
                    case 0:
                        resourceParams.pdwCmd = (uint32_t*)&(cmd.DsFwdRef0.LowerAddress);
                        break;
                    case 1:
                        resourceParams.pdwCmd = (uint32_t*)&(cmd.DsFwdRef1.LowerAddress);
                        break;
                    default:
                        break;
                    }

                    mmcMode = params->Ps8xDsSurfMmcState;
                    switch (refIdx)
                    {
                    case 0:
                        cmd.DsFwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef0.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        break;
                    case 1:
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef1.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef1.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef1.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
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
                    switch (refIdx)
                    {
                    case 0:
                        resourceParams.pdwCmd = (uint32_t*)&(cmd.DsFwdRef04X.LowerAddress);
                        break;
                    case 1:
                        resourceParams.pdwCmd = (uint32_t*)&(cmd.DsFwdRef14X.LowerAddress);
                        break;
                    default:
                        break;
                    }

                    mmcMode = params->Ps8xDsSurfMmcState;
                    switch (refIdx)
                    {
                    case 0:
                        cmd.DsFwdRef04X.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef04X.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef04X.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef04X.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                        break;
                    case 1:
                        cmd.DsFwdRef14X.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef14X.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                        cmd.DsFwdRef14X.PictureFields.DW0.MemoryObjectControlState =
                            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                        cmd.DsFwdRef14X.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
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

        if (!params->isLowDelayB && (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_RESERVED_0))
        {
            if (params->presVdencReferences[refIdx])
            {
                // L1 references
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdencReferences[refIdx], &details));

                resourceParams.presResource = params->presVdencReferences[refIdx];
                resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = OFFSET_OF(typename TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD, BwdRef0) / sizeof(uint32_t);
                resourceParams.bIsWritable = false;
                resourceParams.pdwCmd = (uint32_t*)&(cmd.BwdRef0.LowerAddress);

                mmcMode = (params->PostDeblockSurfMmcState != MOS_MEMCOMP_DISABLED) ?
                    params->PostDeblockSurfMmcState : params->PreDeblockSurfMmcState;

                cmd.BwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                cmd.BwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                cmd.BwdRef0.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                cmd.BwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->presVdenc8xDsSurface[refIdx])
            {
                // 8x DS surface
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdenc8xDsSurface[refIdx], &details));

                resourceParams.presResource = params->presVdenc8xDsSurface[refIdx];
                resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = OFFSET_OF(typename TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD, DsBwdRef0) / sizeof(uint32_t);
                resourceParams.bIsWritable = false;
                resourceParams.pdwCmd = (uint32_t*)&(cmd.DsBwdRef0.LowerAddress);

                mmcMode = params->Ps8xDsSurfMmcState;

                cmd.DsBwdRef0.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                cmd.DsBwdRef0.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                cmd.DsBwdRef0.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                cmd.DsBwdRef0.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->presVdenc4xDsSurface[refIdx])
            {
                // 4x DS surface
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presVdenc4xDsSurface[refIdx], &details));

                resourceParams.presResource = params->presVdenc4xDsSurface[refIdx];
                resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = OFFSET_OF(typename TVdencCmds::VDENC_PIPE_BUF_ADDR_STATE_CMD, DsBwdRef04X) / sizeof(uint32_t);
                resourceParams.bIsWritable = false;
                resourceParams.pdwCmd = (uint32_t*)&(cmd.DsBwdRef04X.LowerAddress);

                mmcMode = params->Ps4xDsSurfMmcState;
                cmd.DsBwdRef04X.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(mmcMode) ? 1 : 0;
                cmd.DsBwdRef04X.PictureFields.DW0.CompressionType         = MmcIsRc(mmcMode) ? 1 : 0;
                cmd.DsBwdRef04X.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;
                cmd.DsBwdRef04X.PictureFields.DW0.TiledResourceMode = Mhw_ConvertToTRMode(details.TileType);

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        // extra surface for HEVC/VP9
        if ((params->Mode == CODECHAL_ENCODE_MODE_HEVC) || (params->Mode == CODECHAL_ENCODE_MODE_VP9) || (params->Mode == CODECHAL_ENCODE_RESERVED_0))
        {
            if (params->presColMvTempBuffer[0] != nullptr)
            {
                resourceParams.presResource    = params->presColMvTempBuffer[0];
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*)&(cmd.ColocatedMv.LowerAddress);
                resourceParams.dwLocationInCmd = 19;
                resourceParams.bIsWritable     = true;

                cmd.ColocatedMv.PictureFields.DW0.MemoryCompressionEnable = 0;
                cmd.ColocatedMv.PictureFields.DW0.CompressionType = 0;
                cmd.ColocatedMv.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->ps8xDsSurface != nullptr)
            {
                resourceParams.presResource    = &params->ps8xDsSurface->OsResource;
                resourceParams.dwOffset        = params->ps8xDsSurface->dwOffset;
                resourceParams.pdwCmd          = (uint32_t*)&(cmd.ScaledReferenceSurface8X.LowerAddress);
                resourceParams.dwLocationInCmd = 49;
                resourceParams.bIsWritable     = true;

                cmd.ScaledReferenceSurface8X.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(params->Ps8xDsSurfMmcState) ? 1 : 0;
                cmd.ScaledReferenceSurface8X.PictureFields.DW0.CompressionType         = MmcIsRc(params->Ps8xDsSurfMmcState) ? 1 : 0;
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
                resourceParams.pdwCmd          = (uint32_t*)&(cmd.ScaledReferenceSurface4X.LowerAddress);
                resourceParams.dwLocationInCmd = 52;
                resourceParams.bIsWritable     = true;

                cmd.ScaledReferenceSurface4X.PictureFields.DW0.MemoryCompressionEnable = MmcEnable(params->Ps4xDsSurfMmcState) ? 1 : 0;
                cmd.ScaledReferenceSurface4X.PictureFields.DW0.CompressionType = MmcIsRc(params->Ps4xDsSurfMmcState) ? 1 : 0;
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
                resourceParams.pdwCmd          = (uint32_t*)&(cmd.VdencCuRecordStreamOutBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 43;
                resourceParams.bIsWritable     = true;

                cmd.VdencCuRecordStreamOutBuffer.PictureFields.DW0.MemoryCompressionEnable = 0;
                cmd.VdencCuRecordStreamOutBuffer.PictureFields.DW0.CompressionType = 0;
                cmd.VdencCuRecordStreamOutBuffer.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->presVdencPakObjCmdStreamOutBuffer)
            {
                resourceParams.presResource    = params->presVdencPakObjCmdStreamOutBuffer;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*)&(cmd.VdencLcuPakObjCmdBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 46;
                resourceParams.bIsWritable     = true;

                cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryCompressionEnable = 0;
                cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.CompressionType = 0;
                cmd.VdencLcuPakObjCmdBuffer.PictureFields.DW0.MemoryObjectControlState =
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->presSegmentMapStreamOut)
            {
                resourceParams.presResource = params->presSegmentMapStreamOut;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*)&(cmd.Vp9SegmentationMapStreaminBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 55;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));

                resourceParams.presResource    = params->presSegmentMapStreamOut;
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (uint32_t*)&(cmd.Vp9SegmentationMapStreamoutBuffer.LowerAddress);
                resourceParams.dwLocationInCmd = 58;
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        // DW61: Weights Histogram Streamout offset
        // This parameter specifies the 64 byte aligned offset in the VDEnc
        // Statistics Streamout buffer where the luma and chroma histogram for
        // the weights/offsets determination is written out.

        // The first 2 CLs(cacheline=64bytes) are ENC frame statistics data.
        // The 3rd CL is for VDL1* stats (hits & misses which doesn't model).
        // Hence it's a dummy CL for us. Histogram stats start from 4th CL onwards.
        cmd.DW61.WeightsHistogramStreamoutOffset = 3 * MHW_CACHELINE_SIZE;

        if (params->presVdencTileRowStoreBuffer != nullptr)
        {
            cmd.VdencTileRowStoreBuffer.BufferPictureFields.DW0.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presVdencTileRowStoreBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (uint32_t*)&(cmd.VdencTileRowStoreBuffer.LowerAddress);
            resourceParams.dwLocationInCmd = 62;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presVdencCumulativeCuCountStreamoutSurface != nullptr)
        {
            resourceParams.presResource = params->presVdencCumulativeCuCountStreamoutSurface;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (uint32_t*)&(cmd.VdencCumulativeCuCountStreamoutSurface.LowerAddress);
            resourceParams.dwLocationInCmd = 65;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    uint32_t GetHWTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
    {
        uint32_t tileMode = 0;

        if (gmmTileEnabled)
        {
            return tileModeGMM;
        }

        switch (tileType)
        {
        case MOS_TILE_LINEAR:
            tileMode = 0;
            break;
        case MOS_TILE_YS:
            tileMode = 1;
            break;
        case MOS_TILE_X:
            tileMode = 2;
            break;
        default:
            tileMode = 3;
            break;
        }
        return tileMode;
    }

    MOS_STATUS AddVdencSrcSurfaceStateCmd(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params) override
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

        uint32_t tilemode             = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
        cmd.Dwords25.DW1.TiledSurface = (tilemode & 0x2) >> 1;
        cmd.Dwords25.DW1.TileWalk     = tilemode & 0x1;

        cmd.Dwords25.DW1.SurfaceFormat            = MosFormatToVdencSurfaceRawFormat(params->psSurface->Format);
        cmd.Dwords25.DW0.SurfaceFormatByteSwizzle = params->bDisplayFormatSwizzle;
        cmd.Dwords25.DW1.SurfacePitch             = params->psSurface->dwPitch - 1;
        cmd.Dwords25.DW2.YOffsetForUCb = cmd.Dwords25.DW3.YOffsetForVCr =
            MOS_ALIGN_CEIL(params->psSurface->UPlaneOffset.iYOffset, MHW_VDBOX_MFX_RAW_UV_PLANE_ALIGNMENT_GEN9);

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params) override
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

        if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_RESERVED_0)
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

        uint32_t tilemode             = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
        cmd.Dwords25.DW1.TiledSurface = (tilemode & 0x2) >> 1;
        cmd.Dwords25.DW1.TileWalk     = tilemode & 0x1;

        cmd.Dwords25.DW1.SurfaceFormat = MosFormatToVdencSurfaceReconFormat(params->psSurface->Format);

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
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params,
        uint8_t                   numSurfaces) override
    {
        uint32_t tilemode = 0;
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->psSurface);

        typename TVdencCmds::VDENC_DS_REF_SURFACE_STATE_CMD cmd;

        if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_RESERVED_0)
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

        tilemode                      = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
        cmd.Dwords25.DW1.TiledSurface = (tilemode & 0x2) >> 1;
        cmd.Dwords25.DW1.TileWalk     = tilemode & 0x1;

        cmd.Dwords25.DW1.SurfaceFormat    = TVdencCmds::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8;
        cmd.Dwords25.DW1.SurfacePitch     = params->psSurface->dwPitch - 1;
        cmd.Dwords25.DW2.YOffsetForUCb    = cmd.Dwords25.DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;

        // 2nd surface
        if (numSurfaces > 1)
        {
            params = params + 1;          // Increment pointer to move from 1st surface to 2nd surface.
            MHW_MI_CHK_NULL(params);
            MHW_MI_CHK_NULL(params->psSurface);

            if (params->Mode == CODECHAL_ENCODE_MODE_HEVC || params->Mode == CODECHAL_ENCODE_RESERVED_0)
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

            tilemode                          = GetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
            cmd.Dwords69.DW1.TiledSurface     = (tilemode & 0x2) >> 1;
            cmd.Dwords69.DW1.TileWalk         = tilemode & 0x1;

            cmd.Dwords69.DW1.SurfaceFormat    = TVdencCmds::VDENC_Surface_State_Fields_CMD::SURFACE_FORMAT_PLANAR_420_8;
            cmd.Dwords69.DW1.SurfacePitch     = params->psSurface->dwPitch - 1;
            cmd.Dwords69.DW2.YOffsetForUCb    = cmd.Dwords69.DW3.YOffsetForVCr = params->psSurface->UPlaneOffset.iYOffset;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencImgStateCmd(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_BATCH_BUFFER         batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pEncodeAvcSeqParams);
        MHW_MI_CHK_NULL(params->pEncodeAvcPicParams);
 
        auto paramsG12 = dynamic_cast<PMHW_VDBOX_AVC_IMG_PARAMS_G12>(params);
        MHW_MI_CHK_NULL(paramsG12);

        typename TVdencCmds::VDENC_IMG_STATE_CMD cmd;

        auto avcSeqParams   = params->pEncodeAvcSeqParams;
        auto avcPicParams   = params->pEncodeAvcPicParams;
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
            cmd.DW2.BidirectionalWeight       = 0x20;
            cmd.DW4.SubPelMode                = 3;
            cmd.DW4.BmeDisableForFbrMessage   = 1;
            cmd.DW4.InterSadMeasureAdjustment = 2;
            cmd.DW5.CrePrefetchEnable         = 1;
            cmd.DW8.NonSkipZeroMvCostAdded    = 1;
            cmd.DW8.NonSkipMbModeCostAdded    = 1;
            cmd.DW9.Mode0Cost                 = 7;
            cmd.DW9.Mode1Cost                 = 26;
            cmd.DW9.Mode2Cost                 = 30;
            cmd.DW9.Mode3Cost                 = 57;
            cmd.DW10.Mode4Cost                = 8;
            cmd.DW10.Mode5Cost                = 2;
            cmd.DW10.Mode6Cost                = 4;
            cmd.DW10.Mode7Cost                = 6;
            cmd.DW11.Mode8Cost                = 5;
            cmd.DW11.Mode9Cost                = 0;
            cmd.DW11.RefIdCost                = 4;
            cmd.DW12.MvCost0                  = 0;
            cmd.DW12.MvCost1                  = 6;
            cmd.DW12.MvCost2                  = 6;
            cmd.DW12.MvCost3                  = 9;
            cmd.DW13.MvCost4                  = 10;
            cmd.DW13.MvCost5                  = 13;
            cmd.DW13.MvCost6                  = 14;
            cmd.DW13.MvCost7                  = 24;
            cmd.DW31.SadHaarThreshold0        = 800;
            cmd.DW32.SadHaarThreshold1        = 1600;
            cmd.DW32.SadHaarThreshold2        = 2400;
            cmd.DW34.MidpointSadHaar          = 0x640;
        }

        cmd.DW1.VdencPerfmode                   = params->bVDEncPerfModeEnabled;
        cmd.DW1.Transform8X8Flag                = avcPicParams->transform_8x8_mode_flag;
        cmd.DW3.PictureWidth                    = params->wPicWidthInMb;
        cmd.DW4.ForwardTransformSkipCheckEnable = this->m_vdencFTQEnabled[avcSeqParams->TargetUsage];
        cmd.DW4.BlockBasedSkipEnabled           = this->m_vdencBlockBasedSkipEnabled[avcSeqParams->TargetUsage];
        cmd.DW5.CrePrefetchEnable               = params->bCrePrefetchEnable;
        cmd.DW5.PictureHeightMinusOne           = params->wPicHeightInMb - 1;
        cmd.DW5.PictureType                     = avcPicParams->CodingType - 1;
        cmd.DW5.ConstrainedIntraPredictionFlag  = avcPicParams->constrained_intra_pred_flag;

        if (paramsG12->bVDEncUltraModeEnabled)
        {
            cmd.DW1.VdencPerfmode  = true;
            cmd.DW1.VdencUltraMode = true;
        }

        // HME Ref1 Disable should be set as 0 when VDEnc Perf Mode is enabled 
        if ((avcPicParams->CodingType != I_TYPE) && 
            (!params->pEncodeAvcSliceParams->num_ref_idx_l0_active_minus1) &&
            (!params->bVDEncPerfModeEnabled))
        {
            cmd.DW5.HmeRef1Disable = true;
        }

        if (avcSeqParams->EnableSliceLevelRateCtrl)
        {
            cmd.DW5.MbSliceThresholdValue = params->dwMbSlcThresholdValue;
        }

        cmd.DW6.SliceMacroblockHeightMinusOne = params->wSlcHeightInMb - 1;

        cmd.DW8.LumaIntraPartitionMask = avcPicParams->transform_8x8_mode_flag ? 0 : TVdencCmds::VDENC_IMG_STATE_CMD::LUMA_INTRA_PARTITION_MASK_UNNAMED2;

        cmd.DW14.QpPrimeY = avcPicParams->QpY + avcSliceParams->slice_qp_delta;

        if (params->pVDEncModeCost)
        {
            cmd.DW9.Mode0Cost  = *(params->pVDEncModeCost);
            cmd.DW9.Mode1Cost  = *(params->pVDEncModeCost + 1);
            cmd.DW9.Mode2Cost  = *(params->pVDEncModeCost + 2);
            cmd.DW9.Mode3Cost  = *(params->pVDEncModeCost + 3);

            cmd.DW10.Mode4Cost = *(params->pVDEncModeCost + 4);
            cmd.DW10.Mode5Cost = *(params->pVDEncModeCost + 5);
            cmd.DW10.Mode6Cost = *(params->pVDEncModeCost + 6);
            cmd.DW10.Mode7Cost = *(params->pVDEncModeCost + 7);

            cmd.DW11.Mode8Cost = *(params->pVDEncModeCost + 8);
            cmd.DW11.RefIdCost = *(params->pVDEncModeCost + 10);
        }
        if (params->pVDEncMvCost)
        {
            cmd.DW12.MvCost0 = *(params->pVDEncMvCost);
            cmd.DW12.MvCost1 = *(params->pVDEncMvCost + 1);
            cmd.DW12.MvCost2 = *(params->pVDEncMvCost + 2);
            cmd.DW12.MvCost3 = *(params->pVDEncMvCost + 3);
            cmd.DW13.MvCost4 = *(params->pVDEncMvCost + 4);
            cmd.DW13.MvCost5 = *(params->pVDEncMvCost + 5);
            cmd.DW13.MvCost6 = *(params->pVDEncMvCost + 6);
            cmd.DW13.MvCost7 = *(params->pVDEncMvCost + 7);
        }

        cmd.DW27.MaxVmvR = params->dwMaxVmvR;

        if (params->pVDEncHmeMvCost)
        {
            cmd.DW28.HmeMvCost0 = *(params->pVDEncHmeMvCost);
            cmd.DW28.HmeMvCost1 = *(params->pVDEncHmeMvCost + 1);
            cmd.DW28.HmeMvCost2 = *(params->pVDEncHmeMvCost + 2);
            cmd.DW28.HmeMvCost3 = *(params->pVDEncHmeMvCost + 3);
            cmd.DW29.HmeMvCost4 = *(params->pVDEncHmeMvCost + 4);
            cmd.DW29.HmeMvCost5 = *(params->pVDEncHmeMvCost + 5);
            cmd.DW29.HmeMvCost6 = *(params->pVDEncHmeMvCost + 6);
            cmd.DW29.HmeMvCost7 = *(params->pVDEncHmeMvCost + 7);
        }

        // HMEOffset is in range of -128 to 127, clip value to within range
        if (avcPicParams->bEnableHMEOffset)
        {
            cmd.DW7.Hme0XOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[0][0][0], -128, 127);
            cmd.DW7.Hme0YOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[0][0][1], -128, 127);
            cmd.DW7.Hme1XOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[1][0][0], -128, 127);
            cmd.DW7.Hme1YOffset = MOS_CLAMP_MIN_MAX(avcPicParams->HMEOffset[1][0][1], -128, 127);
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

            if (!params->bVdencBRCEnabled && avcPicParams->EnableRollingIntraRefresh == ROLLING_I_DISABLED && paramsG12->bStreamInMbQpEnabled)
            {
                cmd.DW34.MbLevelQpEnable = 1;
            }
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
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS params) override
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
            auto paramsG12 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12>(params);
            MHW_MI_CHK_NULL(paramsG12);

            MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
            MHW_MI_CHK_NULL(params->pHevcEncPicParams);
            MHW_MI_CHK_NULL(params->pEncodeHevcSliceParams);

            auto seqParams   = params->pHevcEncSeqParams;
            auto picParams   = params->pHevcEncPicParams;
            auto sliceParams = params->pEncodeHevcSliceParams;

            uint32_t ctbSize     = 1 << (seqParams->log2_max_coding_block_size_minus3 + 3);
            uint32_t widthInPix  = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameWidthInMinCbMinus1 + 1);
            uint32_t widthInCtb  = (widthInPix / ctbSize) + ((widthInPix % ctbSize) ? 1 : 0);  // round up
            uint32_t heightInPix = (1 << (seqParams->log2_min_coding_block_size_minus3 + 3)) * (seqParams->wFrameHeightInMinCbMinus1 + 1);
            uint32_t heightInCtb = (heightInPix / ctbSize) + ((heightInPix % ctbSize) ? 1 : 0);  // round up
            uint32_t shift     = seqParams->log2_max_coding_block_size_minus3 - seqParams->log2_min_coding_block_size_minus3;

            cmd.DW3.Log2WeightDenomLuma = cmd.DW3.HevcLog2WeightDemonLuma =
                (picParams->weighted_pred_flag || picParams->weighted_bipred_flag) ? (picParams->bEnableGPUWeightedPrediction ? 6 : sliceParams->luma_log2_weight_denom) : 0;

            if (paramsG12->pTileCodingParams == nullptr)
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
                cmd.DW1.MbLcuStartXPosition          = paramsG12->pTileCodingParams->TileStartLCUX;
                cmd.DW1.MbLcuStartYPosition          = paramsG12->pTileCodingParams->TileStartLCUY;

                cmd.DW2.NextsliceMbLcuStartXPosition = paramsG12->pTileCodingParams->TileStartLCUX + (paramsG12->pTileCodingParams->TileWidthInMinCbMinus1 >> shift) + 1;
                cmd.DW2.NextsliceMbStartYPosition    = paramsG12->pTileCodingParams->TileStartLCUY + (paramsG12->pTileCodingParams->TileHeightInMinCbMinus1 >> shift) + 1;

                cmd.DW4.TileStartCtbX                = paramsG12->pTileCodingParams->TileStartLCUX * ctbSize;
                cmd.DW4.TileStartCtbY                = paramsG12->pTileCodingParams->TileStartLCUY * ctbSize;

                cmd.DW5.TileWidth                    = ((paramsG12->pTileCodingParams->TileWidthInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3)) - 1;
                cmd.DW5.TileHeight                   = ((paramsG12->pTileCodingParams->TileHeightInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3)) - 1;
                cmd.DW1.FirstSuperSlice              = 1;
                cmd.DW3.NumParEngine                 = paramsG12->dwNumberOfPipes;
                cmd.DW3.TileNumber                   = paramsG12->dwTileId;
                cmd.DW3.TileRowStoreSelect           = paramsG12->pTileCodingParams->TileRowStoreSelect;
                cmd.DW8.TileStreamoutOffsetEnable    = 1;
                cmd.DW8.TileStreamoutOffset          = paramsG12->dwTileId * 19;

                cmd.DW6.StreaminOffsetEnable         = 1;
                cmd.DW6.TileStreaminOffset           = paramsG12->pTileCodingParams->TileStreaminOffset;

                // PAK Object StreamOut Offset Computation 
                uint32_t tileLCUStreamOutByteOffset = 0;
                if (paramsG12->pTileCodingParams->TileStartLCUX != 0 || paramsG12->pTileCodingParams->TileStartLCUY != 0)
                {
                    uint32_t ctbSize = 1 << (seqParams->log2_max_coding_block_size_minus3 + 3);
                    uint32_t NumOfCUInLCU = (ctbSize >> 3) * (ctbSize >> 3);  // Min CU size is 8
                    uint32_t ImgWidthInLCU = (((seqParams->wFrameWidthInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3)) + ctbSize - 1) / ctbSize;
                    uint32_t ImgHeightInLCU = (((seqParams->wFrameHeightInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3)) + ctbSize - 1) / ctbSize;
                    uint32_t NumLCUsCurLocation = paramsG12->pTileCodingParams->TileStartLCUY * ImgWidthInLCU + paramsG12->pTileCodingParams->TileStartLCUX * 
                                                  ((((paramsG12->pTileCodingParams->TileHeightInMinCbMinus1 + 1) << (seqParams->log2_min_coding_block_size_minus3 + 3)) + ctbSize - 1) / ctbSize);
                    //For PAKObject Surface
                    tileLCUStreamOutByteOffset = 2 * BYTES_PER_DWORD * NumLCUsCurLocation * (NUM_PAK_DWS_PER_LCU + NumOfCUInLCU * NUM_DWS_PER_CU);
                    //Add 1 CL for size info at the beginning of each tile
                    tileLCUStreamOutByteOffset += MHW_CACHELINE_SIZE;
                    //CL alignment at end of every tile
                    tileLCUStreamOutByteOffset = MOS_ROUNDUP_DIVIDE(tileLCUStreamOutByteOffset, MHW_CACHELINE_SIZE);
                }

                cmd.DW9.TileLcuStreamOutOffset = tileLCUStreamOutByteOffset;
                cmd.DW9.LcuStreamOutOffsetEnable = 0x1;

                if (cmd.DW4.TileStartCtbY == 0)
                {
                    //RowStore Offset Computation
                    uint32_t num32x32InX = (cmd.DW4.TileStartCtbX) / 32;
                    cmd.DW7.RowStoreOffsetEnable = 1;
                    cmd.DW7.TileRowstoreOffset = num32x32InX;
                }
            }

            // for IBC
            cmd.DW11.Value &= 0xfffc00ff;
            cmd.DW12.Value = (cmd.DW12.Value & 0xfcffffff) | 0x1000000;
            cmd.DW12.IbcControl = params->pHevcEncPicParams->pps_curr_pic_ref_enabled_flag ?
                paramsG12->IBCControl : VDENC_WALKER_STATE_COMMAND_IBC_CONTROL_IBC_DISABLED_G12;

            cmd.DW12.PaletteModeEnable = (seqParams->palette_mode_enabled_flag != 0) ? 1 : 0;
            uint32_t sliceQP = picParams->QpY + sliceParams->slice_qp_delta;
            uint32_t index;
            if (sliceQP <= 12)
            {
                index = 0;
            }
            else if (sliceQP > 12 && sliceQP <= 47)
            {
                index = (sliceQP - 8) / 5;
            }
            else if (sliceQP > 47 && sliceQP <= 49)
            {
                index = 8;
            }
            else
            {
                index = 9;
            }
            const uint32_t table1[10] = {0x50001,0x50001,0x50001,0x50002,0x90002,0x90002,0x90002,0xd0002,0x190002,0x210003};
            cmd.DW12.Value = (cmd.DW12.Value & 0xff80fff8) | table1[index];
            const uint32_t table2[10] = {0x2000a,0x2000a,0x2000a,0x4000a,0x8000a,0xc0010,0xc0018,0xc0018,0x100020,0x180030};
            cmd.DW13.Value = table2[index];
            const uint32_t table3[10] = {0x101004,0x101004,0x101004,0xc1008,0x42004,0x42006,0x13f06,0x13f06,0x13f0c,0x13006};
            cmd.DW14.Value = (cmd.DW14.Value & 0xffe0c0c0) | table3[index];
            const uint32_t table4[10] = {0x100004,0x100004,0x100004,0x100004,0x200004,0x300004,0x400004,0x600004,0x800004,0x1000004};
            cmd.DW15.Value = (cmd.DW15.Value & 0xfc00) | table4[index];

            if (seqParams->bit_depth_luma_minus8 > 0 && seqParams->palette_mode_enabled_flag)
            {

                const uint32_t table1[10] = {0x3,0x3,0x3,0x4,0x4,0x4,0x4,0x4,0x4,0x5};
                cmd.DW12.Value  = (cmd.DW12.Value & 0xfffffff8) | table1[index];
                const uint32_t table2[10] = {0x80028,0x80028,0x80028,0x100028,0x200028,0x300040,0x300060,0x300060,0x400080,0x6000c0};
                cmd.DW13.Value  = table2[index];
                const uint32_t table3[10] = {0x400000,0x400000,0x400000,0x400000,0x800000,0xc00000,0x1000000,0x1800000,0x2000000,0x4000000};
                cmd.DW15.Value = (cmd.DW15.Value & 0xffff) | table3[index];
            }

            cmd.DW12.Value &= 0xffff00ff;
            cmd.DW14.Value = (cmd.DW14.Value & 0x9fffff) | 0xc8400000;
            cmd.DW16.Value = (cmd.DW16.Value & 0xffffff) | 0xa6000000;
            if (seqParams->TargetUsage == 7)
            {
                cmd.DW16.Value = (cmd.DW16.Value & 0xffc0c0c0) | 0x313131;
            }
            else
            {
                cmd.DW16.Value = (cmd.DW16.Value & 0xffc0c0c0) | 0x3f3f3f;
            }
        }
        else if (params->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            auto paramsG12 = dynamic_cast<PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS_G12>(params);
            MHW_MI_CHK_NULL(paramsG12);
            MHW_MI_CHK_NULL(params->pVp9EncPicParams);
            auto vp9PicParams = params->pVp9EncPicParams;
            auto tileCodingParams = paramsG12->pTileCodingParams;

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
                cmd.DW1.MbLcuStartXPosition          = tileCodingParams->TileStartLCUX;
                cmd.DW1.MbLcuStartYPosition          = tileCodingParams->TileStartLCUY;

                cmd.DW5.TileWidth                    = ((tileCodingParams->TileWidthInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
                cmd.DW5.TileHeight                   = ((tileCodingParams->TileHeightInMinCbMinus1 + 1) * CODEC_VP9_MIN_BLOCK_HEIGHT) - 1;

                cmd.DW4.TileStartCtbX                = tileCodingParams->TileStartLCUX * CODEC_VP9_SUPER_BLOCK_WIDTH;
                cmd.DW4.TileStartCtbY                = tileCodingParams->TileStartLCUY * CODEC_VP9_SUPER_BLOCK_HEIGHT;

                cmd.DW2.NextsliceMbLcuStartXPosition = CODECHAL_GET_WIDTH_IN_BLOCKS((cmd.DW4.TileStartCtbX + cmd.DW5.TileWidth + 1), CODEC_VP9_SUPER_BLOCK_WIDTH);
                cmd.DW2.NextsliceMbStartYPosition    = CODECHAL_GET_HEIGHT_IN_BLOCKS((cmd.DW4.TileStartCtbY + cmd.DW5.TileHeight + 1), CODEC_VP9_SUPER_BLOCK_HEIGHT);

                cmd.DW1.FirstSuperSlice              = 1;
                cmd.DW3.NumParEngine                 = paramsG12->dwNumberOfPipes;
                cmd.DW3.TileNumber                   = paramsG12->dwTileId;

                //Frame Stats Offset
                cmd.DW8.TileStreamoutOffsetEnable = 1;
                cmd.DW8.TileStreamoutOffset = (paramsG12->dwTileId * 19); // 3 CLs or 48 DWs of statistics data + 16CLs or 256 DWs of Histogram data

                uint32_t tileStartXInSBs = (cmd.DW4.TileStartCtbX / CODEC_VP9_SUPER_BLOCK_WIDTH);
                uint32_t tileStartYInSBs = (cmd.DW4.TileStartCtbY / CODEC_VP9_SUPER_BLOCK_HEIGHT);
                //Aligned Tile height & frame width
                uint32_t tileHeightInSBs = (cmd.DW5.TileHeight + 1 + (CODEC_VP9_SUPER_BLOCK_HEIGHT - 1)) / CODEC_VP9_SUPER_BLOCK_HEIGHT;
                uint32_t frameWidthInSBs = (vp9PicParams->SrcFrameWidthMinus1 + 1 + (CODEC_VP9_SUPER_BLOCK_WIDTH - 1)) / CODEC_VP9_SUPER_BLOCK_WIDTH;

                cmd.DW6.StreaminOffsetEnable = 1;
                //StreamIn data is 4 CLs per LCU
                cmd.DW6.TileStreaminOffset = (tileStartYInSBs * frameWidthInSBs + tileStartXInSBs * tileHeightInSBs) * (4);

                //Compute PAK Object StreamOut Offsets
                uint32_t tileLCUStreamOutOffsetInCachelines = 0;
                if (cmd.DW4.TileStartCtbY != 0 || cmd.DW4.TileStartCtbX != 0)
                {
                    //Aligned Tile width & frame height
                    uint32_t numOfSBs = tileStartYInSBs * frameWidthInSBs + tileStartXInSBs * tileHeightInSBs;
                    //max LCU size is 64, min Cu size is 8
                    uint32_t maxNumOfCUInSB = (CODEC_VP9_SUPER_BLOCK_HEIGHT / CODEC_VP9_MIN_BLOCK_HEIGHT) * (CODEC_VP9_SUPER_BLOCK_WIDTH / CODEC_VP9_MIN_BLOCK_WIDTH);
                    //(num of SBs in a tile) *  (num of cachelines needed per SB)
                    tileLCUStreamOutOffsetInCachelines = numOfSBs * (MOS_ROUNDUP_DIVIDE((2 * BYTES_PER_DWORD * (NUM_PAK_DWS_PER_LCU + maxNumOfCUInSB * NUM_DWS_PER_CU)), MHW_CACHELINE_SIZE));
                }

                cmd.DW9.LcuStreamOutOffsetEnable = 1;
                cmd.DW9.TileLcuStreamOutOffset = tileLCUStreamOutOffsetInCachelines;

                if (cmd.DW4.TileStartCtbY == 0)
                {
                    //RowStore Offset Computation
                    uint32_t num32x32sInX = (cmd.DW4.TileStartCtbX) / 32;
                    cmd.DW7.RowStoreOffsetEnable = 1;
                    cmd.DW7.TileRowstoreOffset = num32x32sInX;
                }
            }
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencAvcWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                cmdBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS params) override
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
        PMOS_COMMAND_BUFFER                   cmdBuffer,
        PMHW_BATCH_BUFFER                     batchBuffer,
        PMHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS params) override
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
        cmd.DW2.HevcVp9WeightsBackwardReference0 = 1;
        cmd.DW2.HevcVp9OffsetBackwardReference0  = 0;

        // Luma Offsets and Weights
        if (params->bWeightedPredEnabled)
        {
            uint32_t  refPicListNum = 0;
            // DWORD 1
            cmd.DW1.WeightsForwardReference0 = CodecHal_Clip3(-128, 127,
                params->LumaWeights[refPicListNum][0] + params->dwDenom);
            cmd.DW1.OffsetForwardReference0  = params->LumaOffsets[refPicListNum][0];
            cmd.DW1.WeightsForwardReference1 = CodecHal_Clip3(-128, 127,
                params->LumaWeights[refPicListNum][1] + params->dwDenom);
            cmd.DW1.OffsetForwardReference1  = params->LumaOffsets[refPicListNum][1];

            // DWORD 2
            cmd.DW2.WeightsForwardReference2 = CodecHal_Clip3(-128, 127,
                params->LumaWeights[refPicListNum][2] + params->dwDenom);
            cmd.DW2.OffsetForwardReference2  = params->LumaOffsets[refPicListNum][2];
            if (!params->isLowDelay)
            {
                refPicListNum = 1;
                cmd.DW2.HevcVp9WeightsBackwardReference0 = CodecHal_Clip3(-128, 127,
                    params->LumaWeights[refPicListNum][0] + params->dwDenom);
                cmd.DW2.HevcVp9OffsetBackwardReference0 = params->LumaOffsets[refPicListNum][0];
            }
            else
            {
                cmd.DW2.HevcVp9WeightsBackwardReference0 = cmd.DW1.WeightsForwardReference0;
                cmd.DW2.HevcVp9OffsetBackwardReference0 = cmd.DW1.OffsetForwardReference0;
            }
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
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_VDENC_CMD1_PARAMS  params) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencCmd2Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD2_STATE params) override
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Adds VDEnc Pipe Control State command in command buffer
    //! \details  Client facing function to add VDEnc Pipe Control State command in command buffer
    //! \param    PMOS_COMMAND_BUFFER  cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    PMHW_VDBOX_VDENC_CONTROL_STATE_PARAMS params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddVdencControlStateCmd(
        PMOS_COMMAND_BUFFER                   cmdBuffer,
        PMHW_VDBOX_VDENC_CONTROL_STATE_PARAMS params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TVdencCmds::VDENC_CONTROL_STATE_CMD    cmd;


        if (params->bVdencInitialization)
        {
            cmd.DW1.VdencInitialization     = 1;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid parameter for VDENC_CONTROL_STATE.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CreateMhwVdboxPipeModeSelectParams() override
    {
        auto pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12);

        return pipeModeSelectParams;
    }

    void ReleaseMhwVdboxPipeModeSelectParams(PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams) override
    {
        MOS_Delete(pipeModeSelectParams);
    }
};

//!  MHW Vdbox Vdenc interface for Gen12 X
/*!
This class defines the Vdenc command construction functions for Gen12 common platforms
*/
class MhwVdboxVdencInterfaceG12X : public MhwVdboxVdencInterfaceG12<mhw_vdbox_vdenc_g12_X>
{
public:
        //!
    //! \brief  Constructor
    //!
    MhwVdboxVdencInterfaceG12X(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceG12(osInterface)
    {
        MHW_FUNCTION_ENTER;
    }

    inline uint32_t GetVdencCmd1Size() override
    {
        return mhw_vdbox_vdenc_g12_X::VDENC_CMD1_CMD::byteSize;
    }

    inline uint32_t GetVdencCmd2Size() override
    {
        return mhw_vdbox_vdenc_g12_X::VDENC_CMD2_CMD::byteSize;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG12X()
    {
        MHW_FUNCTION_ENTER;
    }

    MOS_STATUS AddVdencCmd1Cmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_VDENC_CMD1_PARAMS  params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        typename mhw_vdbox_vdenc_g12_X::VDENC_CMD1_CMD cmd;

        cmd.DW22.Value = 0x10101010;
        cmd.DW23.Value = 0x10101010;
        cmd.DW24.Value = 0x10101010;
        cmd.DW25.Value = 0x10101010;
        cmd.DW26.Value = 0x10101010;
        cmd.DW27.Value = 0x10101010;
        cmd.DW28.Value = 0x10101010;
        cmd.DW29.Value = 0x10101010;
        if (params->Mode == CODECHAL_ENCODE_MODE_HEVC)
        {
            MHW_MI_CHK_NULL(params->pHevcEncPicParams);
            auto hevcPicParams = params->pHevcEncPicParams;
            MHW_MI_CHK_NULL(params->pHevcEncSlcParams);
            auto hevcSlcParams = params->pHevcEncSlcParams;

            const uint32_t table1[3] = {0x5030200,0x5030200,0x5030200};
            const uint8_t indexTable1[3] = {0,1,2};
            cmd.DW1.Value = table1[indexTable1[hevcPicParams->CodingType - 1]];
            const uint32_t table2[3] = {0xb090806,0xb090806,0xb090806};
            const uint8_t indexTable2[3] = {0,1,2};
            cmd.DW2.Value = table2[indexTable2[hevcPicParams->CodingType - 1]];

            cmd.DW3.Value = 0x1c140c04;
            cmd.DW4.Value = 0x3c342c24;
            cmd.DW5.Value = 0x5c544c44;
            cmd.DW6.Value = 0x1c140c04;
            cmd.DW7.Value = 0x3c342c24;
            cmd.DW8.Value = 0x5c544c44;
            cmd.DW14.Value = 0x0;
            cmd.DW15.Value = 0x0;
            cmd.DW16.Value &= 0xffff0000;
            cmd.DW19.Value = (cmd.DW19.Value & 0xff0000ff) | 0x140400;
            cmd.DW20.Value = 0x14141414;
            cmd.DW21.Value = 0x14141414;
            if (params->bHevcVisualQualityImprovement)
            {
                auto qpPrimeYac = CodecHal_Clip3(10, 51, hevcPicParams->QpY + hevcSlcParams->slice_qp_delta);
                if (qpPrimeYac >= 22 && qpPrimeYac <= 51)
                {
                    const uint32_t table3[30] = {0x0,0x60000,0xc0000,0x120000,0x190000,0x1f0000,0x250000,0x2c0000,0x320000,0x380000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000,0x3f0000};
                    cmd.DW14.Value = (cmd.DW14.Value & 0xff00ffff) |table3[qpPrimeYac - 22];
                }
            }

            if (params->pHevcEncPicParams->CodingType == I_TYPE)
            {
                cmd.DW10.Value = 0x23131f0f;
                cmd.DW11.Value = (cmd.DW11.Value & 0xffff0000) | 0x2313;
                cmd.DW12.Value = 0x3e5c445c;
                cmd.DW13.Value = (cmd.DW13.Value & 0xff00) | 0x1e040044;
                cmd.DW16.Value = (cmd.DW16.Value & 0xffff) | 0x70000;
                cmd.DW17.Value = 0xd0e1007;
                cmd.DW18.Value = (cmd.DW18.Value & 0xffffff00) | 0x32;
                cmd.DW18.Value = (cmd.DW18.Value & 0xffff00ff) | (hevcPicParams->NumROI ? 0 : 0x1e00);
                cmd.DW30.Value = (cmd.DW30.Value & 0xff000000) | 0x101010;
            }
            else if (params->pHevcEncPicParams->CodingType == B_TYPE)
            {
                cmd.DW10.Value = 0x23131f0f;
                cmd.DW11.Value = 0x331b2313;
                cmd.DW12.Value = 0x476e4d6e;
                cmd.DW13.Value = 0x3604004d;
                cmd.DW16.Value = (cmd.DW16.Value & 0xffff) | 0x4150000;
                cmd.DW17.Value = 0x23231415;
                cmd.DW18.Value = (cmd.DW18.Value & 0xffffff00) | 0x3f;
                cmd.DW18.Value = (cmd.DW18.Value & 0xffff00ff) | (hevcPicParams->NumROI ? 0 : 0x4400);
                cmd.DW30.Value = (cmd.DW30.Value & 0xff000000) | 0x232323;
            }
        }
        else if (params->Mode == CODECHAL_ENCODE_MODE_VP9)
        {
            MHW_MI_CHK_NULL(params->pVp9EncPicParams);

            auto vp9PicParams = params->pVp9EncPicParams;
            auto qp = vp9PicParams->LumaACQIndex;
            auto vp9FrameType = vp9PicParams->PicFlags.fields.frame_type;

            const uint32_t table1[2][256] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                            0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
            const uint8_t indexTable1[2] = {0,1};
            cmd.DW1.Value = table1[indexTable1[vp9FrameType]][qp];
            const uint32_t table2[2][256] = {0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                            0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
            const uint8_t indexTable2[2] = {0,1};
            cmd.DW2.Value = table2[indexTable2[vp9FrameType]][qp];

            cmd.DW3.Value = 0x1a130b04;
            cmd.DW4.Value = 0x3f312922;
            cmd.DW5.Value = 0x7164584b;
            cmd.DW6.Value = 0x1a130b04;
            cmd.DW7.Value = 0x3f312922;
            cmd.DW8.Value = 0x7164584b;
            cmd.DW20.Value = 0xc0c0c0c;
            cmd.DW21.Value = 0xc0c0c0c;
            cmd.DW22.Value = 0x10101010;
            cmd.DW23.Value = 0x10101010;
            cmd.DW24.Value = 0x10101010;
            cmd.DW25.Value = 0x10101010;
            cmd.DW26.Value = 0x10101010;
            cmd.DW27.Value = 0x10101010;
            cmd.DW28.Value = 0x10101010;
            cmd.DW29.Value = 0x10101010;
            if(vp9FrameType == CODEC_VP9_KEY_FRAME)
            {
                cmd.DW9.Value &= 0xff000000;
                cmd.DW10.Value = 0x0;
                cmd.DW11.Value = 0x0;
                cmd.DW12.Value = 0x0;
                cmd.DW13.Value = 0x32000000;
                cmd.DW14.Value = 0x1f5e0000;
                cmd.DW15.Value = 0x1f5e0000;
                cmd.DW16.Value = (cmd.DW16.Value & 0xff000000) | 0x4b0000;
                cmd.DW17.Value = 0x3219194b;
                cmd.DW18.Value = 0x4b5e0026;
                cmd.DW19.Value = (cmd.DW19.Value & 0xff) | 0x7d7d0000;
            }
            else
            {
                cmd.DW9.Value = (cmd.DW9.Value & 0xff000000) | 0x26191f;
                cmd.DW10.Value = 0x1e0b1e0b;
                cmd.DW11.Value = 0x19001e0b;
                cmd.DW12.Value = 0x19321f4b;
                cmd.DW13.Value = 0x4404001f;
                cmd.DW14.Value = 0x30900000;
                cmd.DW15.Value = 0x30900000;
                cmd.DW16.Value = (cmd.DW16.Value & 0xff000000) | 0x260000;
                cmd.DW17.Value = 0x13194b26;
                cmd.DW18.Value = 0x3f5e0d5e;
                cmd.DW19.Value = (cmd.DW19.Value & 0xff) | 0x4e320f00;
            }
        }

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencCmd2Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD2_STATE params) override
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);
        auto extParams = std::static_pointer_cast<MHW_VDBOX_VDENC_CMD2_STATE>(params);
        MHW_MI_CHK_NULL(extParams);

        typename mhw_vdbox_vdenc_g12_X::VDENC_CMD2_CMD cmd;

        cmd.DW2.Value = (cmd.DW2.Value & 0xdff00000) | 0x2005aff3;
        cmd.DW3.Value = 0xfe02ff01;
        cmd.DW4.Value = 0xfc04fd03;
        cmd.DW5.Value = (cmd.DW5.Value & 0xff7e03ff) | 0x80ac00;
        cmd.DW7.Value = (cmd.DW7.Value & 0xffd90ff0) | 0x62003;
        cmd.DW9.Value = (cmd.DW9.Value & 0xffff) | 0x43840000;
        cmd.DW12.Value = 0xffffffff;
        cmd.DW15.Value = 0x4e201f40;
        cmd.DW16.Value = (cmd.DW16.Value & 0xf0ff0000) | 0xf003300;
        cmd.DW17.Value = (cmd.DW17.Value & 0xfff00000) | 0x2710;
        cmd.DW19.Value = (cmd.DW19.Value & 0x80ffffff) | 0x18000000;
        cmd.DW21.Value &= 0xfffffff;
        cmd.DW22.Value = 0x1f001102;
        cmd.DW23.Value = 0xaaaa1f00;
        cmd.DW27.Value = (cmd.DW27.Value & 0xffff0000) | 0x1a1a;
        if (params->pHevcEncPicParams && params->pHevcEncSeqParams)
        {
            MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
            MHW_MI_CHK_NULL(params->pHevcEncPicParams);
            MHW_MI_CHK_NULL(params->pHevcEncSlcParams);
            auto hevcSeqParams = params->pHevcEncSeqParams;
            auto hevcPicParams = params->pHevcEncPicParams;
            auto hevcSlcParams = params->pHevcEncSlcParams;

            bool vdencROIEnabled = params->bROIStreamInEnabled;
            bool vdencStreamInEnabled = params->bStreamInEnabled;

            cmd.DW1.FrameWidthInPixelsMinusOne = ((hevcSeqParams->wFrameWidthInMinCbMinus1 + 1) <<
                (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) - 1;
            cmd.DW1.FrameHeightInPixelsMinusOne = ((hevcSeqParams->wFrameHeightInMinCbMinus1 + 1) <<
                (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) - 1;

            cmd.DW2.PictureType = (hevcPicParams->CodingType == I_TYPE) ? 0 : (extParams->bIsLowDelayB ? 3 : 2);
            cmd.DW2.TemporalMvpEnableFlag = (hevcPicParams->CodingType == I_TYPE) ? 0 : params->pHevcEncSeqParams->sps_temporal_mvp_enable_flag;
            cmd.DW2.TransformSkip = hevcPicParams->transform_skip_enabled_flag;

            cmd.DW5.NumRefIdxL0Minus1 = hevcSlcParams->num_ref_idx_l0_active_minus1;
            cmd.DW5.NumRefIdxL1Minus1 = hevcSlcParams->num_ref_idx_l1_active_minus1;

            cmd.DW5.Value = (cmd.DW5.Value & 0xff83ffff) | 0x400000;
            cmd.DW14.Value = (cmd.DW14.Value & 0xffff) | 0x7d00000;
            cmd.DW15.Value = 0x4e201f40;
            cmd.DW17.Value = (cmd.DW17.Value & 0xfff00000) | 0x2710;
            cmd.DW18.Value = (cmd.DW18.Value & 0xffff) | 0x600000;
            cmd.DW19.Value = (cmd.DW19.Value & 0xffff0000) | 0xc0;
            cmd.DW20.Value &= 0xfffeffff;
            cmd.DW7.TilingEnable = hevcPicParams->tiles_enabled_flag;
            cmd.DW37.TileReplayEnable = extParams->bTileReplayEnable;

            if (hevcPicParams->CodingType != I_TYPE)
            {
                uint8_t   refFrameId;
                int8_t    diffPoc;

                refFrameId = hevcSlcParams->RefPicList[0][0].FrameIdx;
                diffPoc = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : hevcPicParams->RefFramePOCList[refFrameId]) - hevcPicParams->CurrPicOrderCnt;
                cmd.DW3.PocNumberForRefid0InL0   = -diffPoc;
                cmd.DW2.LongTermReferenceFlagsL0  = (refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0 : CodecHal_PictureIsLongTermRef(hevcPicParams->RefFrameList[refFrameId]);
                refFrameId = hevcSlcParams->RefPicList[0][1].FrameIdx;
                diffPoc = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : hevcPicParams->RefFramePOCList[refFrameId]) - hevcPicParams->CurrPicOrderCnt;
                cmd.DW3.PocNumberForRefid1InL0    = -diffPoc;
                cmd.DW2.LongTermReferenceFlagsL0 |= ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0 : CodecHal_PictureIsLongTermRef(hevcPicParams->RefFrameList[refFrameId])) << 1;
                refFrameId = hevcSlcParams->RefPicList[0][2].FrameIdx;
                diffPoc                        = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : hevcPicParams->RefFramePOCList[refFrameId]) - hevcPicParams->CurrPicOrderCnt;
                cmd.DW4.PocNumberForRefid2InL0    = -diffPoc;
                cmd.DW2.LongTermReferenceFlagsL0 |= ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0 : CodecHal_PictureIsLongTermRef(hevcPicParams->RefFrameList[refFrameId])) << 2;

                refFrameId = hevcSlcParams->RefPicList[1][0].FrameIdx;
                diffPoc                          = ((refFrameId >= CODEC_MAX_NUM_REF_FRAME_HEVC) ? 0x0 : hevcPicParams->RefFramePOCList[refFrameId]) - hevcPicParams->CurrPicOrderCnt;
                cmd.DW3.PocNumberForRefid0InL1   = -diffPoc;
                cmd.DW2.LongTermReferenceFlagsL1 = CodecHal_PictureIsLongTermRef(hevcPicParams->RefFrameList[refFrameId]);

                cmd.DW3.PocNumberForRefid1InL1 = cmd.DW3.PocNumberForRefid1InL0;
                cmd.DW4.PocNumberForRefid2InL1 = cmd.DW4.PocNumberForRefid2InL0;
            }
            else
            {
                cmd.DW3.Value = cmd.DW4.Value = 0;
            }

            switch (hevcSeqParams->TargetUsage)
            {
            case 1:                                 // Quality mode
                cmd.DW2.Value &= 0xdfffffff;
                cmd.DW2.Value = (cmd.DW2.Value & 0xfffffffc) | (hevcPicParams->CodingType == I_TYPE ? 2 : 3);
                cmd.DW7.Value &= 0xfffffeff;
                cmd.DW7.Value = (cmd.DW7.Value & 0xfffff7ff) | (hevcPicParams->CodingType == I_TYPE ? 0x800 : 0);
                cmd.DW9.Value = (cmd.DW9.Value & 0xfffff) | 0x43800000;
                cmd.DW12.Value = 0xffffffff;
                cmd.DW34.Value = (cmd.DW34.Value & 0xffffff) | 0x21000000;
                break;
            case 4:                                 // Normal mode
                cmd.DW2.Value &= 0xdfffffff;
                cmd.DW7.Value &= 0xfffffeff;
                cmd.DW9.Value = (cmd.DW9.Value & 0xfffff) | 0x43800000;
                cmd.DW12.Value = 0xce4014a0;
                cmd.DW34.Value = (cmd.DW34.Value & 0xffffff) | 0x21000000;
                break;
            case 7:                                 // Speed mode
                cmd.DW2.Value = (cmd.DW2.Value & 0xdfffffff) | 0x20000000;
                cmd.DW7.Value = (cmd.DW7.Value & 0xfff7feff) | 0x80100;
                cmd.DW9.Value = (cmd.DW9.Value & 0xffff) | 0x22420000;
                cmd.DW12.Value = 0x89800dc0;
                cmd.DW34.Value = (cmd.DW34.Value & 0xffffff) | 0x20000000;
                break;
            default:
                MHW_ASSERTMESSAGE("Invalid TU provided!");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (hevcSlcParams->num_ref_idx_l0_active_minus1 == 0)
            {
                cmd.DW7.Value = (cmd.DW7.Value & 0xfff7ffff) | 0x80000;
            }

            if (!extParams->bIsLowDelayB)
            {
                cmd.DW7.Value &= 0xfff7ffff;
            }

            cmd.DW7.VdencStreamInEnable = vdencStreamInEnabled;
            cmd.DW7.PakOnlyMultiPassEnable = params->bPakOnlyMultipassEnable;

            if (extParams->bIsLowDelayB)
            {
                cmd.DW8.Value = 0;
                cmd.DW9.Value &= 0xffff0000;
            }
            else
            {
                cmd.DW8.Value = 0x54555555;
                cmd.DW9.Value = (cmd.DW9.Value & 0xffff0000) | 0x5555;
            }

            cmd.DW16.MinQp = hevcPicParams->BRCMinQp < 0x0a ? 10 : hevcPicParams->BRCMinQp;
            cmd.DW16.MaxQp = hevcPicParams->BRCMaxQp < 0x0a ? 51 : (hevcPicParams->BRCMaxQp > 0x33 ? 0x33 : hevcPicParams->BRCMaxQp);
            cmd.DW17.TemporalMVEnableForIntegerSearch = cmd.DW2.TemporalMvpEnableFlag & extParams->bIsLowDelayB;

            if (vdencROIEnabled)
            {
                int8_t priorityLevelOrDQp[ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10] = { 0 };

                cmd.DW5.StreaminRoiEnable = vdencROIEnabled;

                for (uint8_t i = 0; i < ENCODE_VDENC_HEVC_MAX_STREAMINROI_G10; i++)
                {
                    priorityLevelOrDQp[i] = (int8_t)CodecHal_Clip3(
                        ENCODE_VDENC_HEVC_MIN_ROI_DELTA_QP_G10, ENCODE_VDENC_HEVC_MAX_ROI_DELTA_QP_G10, hevcPicParams->ROIDistinctDeltaQp[i]);
                }

                cmd.DW13.RoiQpAdjustmentForZone1Stage3 = priorityLevelOrDQp[0];
                cmd.DW13.RoiQpAdjustmentForZone2Stage3 = priorityLevelOrDQp[1];
                cmd.DW13.RoiQpAdjustmentForZone3Stage3 = priorityLevelOrDQp[2];
            }

            if ((hevcPicParams->bEnableRollingIntraRefresh) && (hevcPicParams->CodingType != I_TYPE))
            {
                uint32_t rollingILimit = (hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_ROW) ?
                    MOS_ROUNDUP_DIVIDE(cmd.DW1.FrameHeightInPixelsMinusOne + 1, 32) :
                    MOS_ROUNDUP_DIVIDE(cmd.DW1.FrameWidthInPixelsMinusOne + 1, 32);

                cmd.DW7.Value = (cmd.DW7.Value & 0xfffffbff) | 0x400;
                cmd.DW7.Value = (cmd.DW7.Value & 0xfffffbff) | ((hevcSeqParams->TargetUsage == 1) ? 0x400 : 0);
                cmd.DW21.Value = (cmd.DW21.Value & 0xfdffffff) | 0x2000000;
                cmd.DW21.QpAdjustmentForRollingI = MOS_CLAMP_MIN_MAX(hevcPicParams->QpDeltaForInsertedIntra, -8, 7);
                cmd.DW21.IntraRefreshMode = (hevcPicParams->bEnableRollingIntraRefresh == ROLLING_I_ROW) ? 0 : 1;
                cmd.DW21.IntraRefreshMBSizeMinusOne = hevcPicParams->IntraInsertionSize - 1;
                cmd.DW21.IntraRefreshPos = hevcPicParams->IntraInsertionLocation;
                cmd.DW36.IntraRefreshBoundaryRef0 = CodecHal_Clip3(0, rollingILimit, hevcPicParams->RollingIntraReferenceLocation[0] - 1);
                cmd.DW36.IntraRefreshBoundaryRef1 = CodecHal_Clip3(0, rollingILimit, hevcPicParams->RollingIntraReferenceLocation[1] - 1);
                cmd.DW36.IntraRefreshBoundaryRef2 = CodecHal_Clip3(0, rollingILimit, hevcPicParams->RollingIntraReferenceLocation[2] - 1);
            }

            const uint32_t table1[3][42] = {0x30002,0x30002,0x30002,0x30003,0x40004,0x40005,0x50006,0x60008,0x6000a,0x7000c,0x8000f,0x90013,0xa0018,0xb001e,0xc0026,0xe0030,0x10003d,0x12004d,0x140061,0x16007a,0x19009a,0x1c00c2,0x1f00f4,0x230133,0x270183,0x2c01e8,0x320266,0x380306,0x3e03cf,0x4604cd,0x4f060c,0x58079f,0x63099a,0x6f0c18,0x7d0f3d,0x8c1333,0x9d1831,0xb11e7a,0xc62666,0xdf3062,0xfa3cf5,0x1184ccd,
                                           0x30003,0x30003,0x30003,0x40003,0x40004,0x50005,0x50007,0x60008,0x6000a,0x7000d,0x80011,0x90015,0xa001a,0xb0021,0xd002a,0xe0034,0x100042,0x120053,0x140069,0x170084,0x1a00a6,0x1d00d2,0x210108,0x24014d,0x2901a3,0x2e0210,0x34029a,0x3a0347,0x410421,0x490533,0x52068d,0x5c0841,0x670a66,0x740d1a,0x821082,0x9214cd,0xa41a35,0xb82105,0xce299a,0xe8346a,0x1044209,0x1245333,
                                           0x30003,0x30003,0x30003,0x40003,0x40004,0x50005,0x50007,0x60008,0x6000a,0x7000d,0x80011,0x90015,0xa001a,0xb0021,0xd002a,0xe0034,0x100042,0x120053,0x140069,0x170084,0x1a00a6,0x1d00d2,0x210108,0x24014d,0x2901a3,0x2e0210,0x34029a,0x3a0347,0x410421,0x490533,0x52068d,0x5c0841,0x670a66,0x740d1a,0x821082,0x9214cd,0xa41a35,0xb82105,0xce299a,0xe8346a,0x1044209,0x1245333};
            const uint8_t indexTable1[3] = {0,1,2};
            cmd.DW26.Value = (cmd.DW26.Value & 0xfe000000) | table1[indexTable1[hevcPicParams->CodingType - 1]][hevcPicParams->QpY + hevcSlcParams->slice_qp_delta - 10];

            if (params->bHevcVisualQualityImprovement)
            {
                auto qpPrimeYac = CodecHal_Clip3(10, 51, hevcPicParams->QpY + hevcSlcParams->slice_qp_delta);
                if (qpPrimeYac >= 22 && qpPrimeYac <= 51 && hevcSlcParams->slice_type == SLICE_I)
                {
                    const uint32_t table2[3][30] = {0xa,0xb,0xd,0xf,0x11,0x14,0x17,0x1a,0x1e,0x22,0x27,0x2d,0x33,0x3b,0x43,0x4d,0x57,0x64,0x72,0x82,0x95,0xa7,0xbb,0xd2,0xec,0x109,0x129,0x14e,0x177,0x1a5,
                                                    0xa,0xc,0xe,0x10,0x12,0x15,0x18,0x1b,0x1f,0x23,0x29,0x2f,0x35,0x3d,0x46,0x50,0x5b,0x68,0x77,0x88,0x9b,0xae,0xc3,0xdb,0xf6,0x114,0x136,0x15c,0x186,0x1b6,
                                                    0xa,0xc,0xe,0x10,0x12,0x15,0x18,0x1b,0x1f,0x23,0x29,0x2f,0x35,0x3d,0x46,0x50,0x5b,0x68,0x77,0x88,0x9b,0xae,0xc3,0xdb,0xf6,0x114,0x136,0x15c,0x186,0x1b6};
                    const uint8_t indexTable2[3] = {0,1,2};
                    cmd.DW26.SadQpLambda  = table2[indexTable2[hevcPicParams->CodingType - 1]][qpPrimeYac - 22];
                }

                if (hevcPicParams->QpY >= 22 && hevcPicParams->QpY <= 51)
                {
                    cmd.DW6.Value = (cmd.DW6.Value & 0xc00fffff) | 0x1fb00000;
                }
            }
            cmd.DW27.QpPrimeYAc = hevcPicParams->QpY + hevcSlcParams->slice_qp_delta;
            cmd.DW27.Value &= 0xffffff00;
            cmd.DW35.Value = (cmd.DW35.Value & 0xfffff0ff) | 0x700;

            if (params->bUseDefaultQpDeltas)
            {
                cmd.DW13.Value = (cmd.DW13.Value & 0xffff) | 0xf0120000;
                if (hevcPicParams->CodingType == I_TYPE)
                {
                    cmd.DW14.Value = (cmd.DW14.Value & 0xffff0000) | 0x21db;
                    cmd.DW16.Value = (cmd.DW16.Value & 0xf00ffff) | 0x10000;
                    cmd.DW18.Value = 0x600000;
                    cmd.DW19.Value = (cmd.DW19.Value & 0xffff0000) | 0xc0;
                }
                else // LDB frames
                {
                    cmd.DW14.Value = (cmd.DW14.Value & 0xffff0000) | 0x21ed;
                    cmd.DW16.Value = (cmd.DW16.Value & 0xf00ffff) | 0xd0010000;
                    cmd.DW18.Value = 0x60010f;
                    cmd.DW19.Value = (cmd.DW19.Value & 0xffff0000) | 0xc0;
                }
            }

            if (params->bRoundingEnabled)
            {
                cmd.DW28.Value = 0x7d00fa0;
                cmd.DW29.Value = 0x2bc0bb8;
                cmd.DW30.Value = 0x32003e8;
                cmd.DW31.Value = 0x1f4012c;
                cmd.DW32.Value = (cmd.DW32.Value & 0xffff0000) | 0x190;

                const uint32_t table3[3][3] = {0x88220000,0x99220000,0xaa220000,
                                              0x88330000,0x99330000,0xaa330000,
                                              0x88440000,0x99440000,0xaa440000};
                cmd.DW32.Value = (cmd.DW32.Value & 0xffff) | table3[params->roundInterValue - 2][params->roundIntraValue - 8];

                const uint32_t table4[3][3] = {0x22882222,0x22992222,0x22aa2222,
                                              0x33883333,0x33993333,0x33aa3333,
                                              0x44884444,0x44994444,0x44aa4444};
                cmd.DW33.Value = table4[params->roundInterValue - 2][params->roundIntraValue - 8];

                const uint32_t table5[3][3] = {0x228822,0x229922,0x22aa22,
                                              0x338833,0x339933,0x33aa33,
                                              0x448844,0x449944,0x44aa44};
                cmd.DW34.Value = (cmd.DW34.Value & 0xff000000) | table5[params->roundInterValue - 2][params->roundIntraValue - 8];
            }

            if (hevcPicParams->pps_curr_pic_ref_enabled_flag)
            {
                cmd.DW5.NumRefIdxL0Minus1++;
                cmd.DW35.Value &= 0xffff1fff;
                cmd.DW37.Value = (cmd.DW37.Value & 0xffffff9f) | 0x40;

                if (hevcPicParams->CodingType == I_TYPE)
                {
                    cmd.DW2.Value = (cmd.DW2.Value & 0xf8fffffc) | 0x1000002;
                    cmd.DW3.Value &= 0xffffff00;
                    cmd.DW5.Value &= 0xf0ffffff;
                }
                else
                {
                    switch (cmd.DW5.NumRefIdxL0Minus1)
                    {
                    case 0:
                        cmd.DW2.Value |= 0x1000000;
                        cmd.DW3.Value &= 0xffffff00;
                        break;
                    case 1:
                        cmd.DW2.Value |= 0x2000000;
                        cmd.DW3.Value &= 0xff00ffff;
                        break;
                    case 2:
                        cmd.DW2.Value |= 0x4000000;
                        cmd.DW4.Value &= 0xffffff00;
                        break;
                    case 3:
                        cmd.DW2.Value |= 0x0;
                        cmd.DW4.Value &= 0xff00ffff;
                        break;
                    default:
                        MHW_ASSERTMESSAGE("Invalid NumRefIdxL0");
                        return MOS_STATUS_INVALID_PARAMETER;
                    }
                }
            }
            if (hevcSeqParams->palette_mode_enabled_flag)
            {
                cmd.DW37.Value &= 0xffe0ffe0;
                cmd.DW38.Value &= 0xffffffbf;
                uint32_t index = hevcSeqParams->bit_depth_luma_minus8 == 0 ? 0 : 1;
                const uint32_t table1[2] = {0x8000fc,0x20003f0};
                cmd.DW39.Value = table1[index];

                const uint32_t table2[2] = {0xb10080,0x2c40200};
                cmd.DW40.Value = table2[index];

                const uint32_t table3[2] = {0x300aa,0xc02a8};
                cmd.DW41.Value = table3[index];

                const uint32_t table4[2] = {0xd30069,0x34c01a4};
                cmd.DW42.Value = table4[index];
                const uint32_t table5[2] = {0xe000e9,0x38003a4};
                cmd.DW43.Value = table5[index];

                const uint32_t table6[2] = {0x940003,0x250000c};
                cmd.DW44.Value = table6[index];

                const uint32_t table7[2] = {0x56004d,0x1580134};
                cmd.DW45.Value = table7[index];
                const uint32_t table8[2] = {0x9500fd,0x25403f4};
                cmd.DW46.Value = table8[index];

                const uint32_t table9[2] = {0x17002d,0x5c00b4};
                cmd.DW47.Value = table9[index];

                const uint32_t table10[2] = {0xfd001f,0x3f4007c};
                cmd.DW48.Value = table10[index];
                const uint32_t table11[2] = {0x2006c,0x801b0};
                cmd.DW49.Value = table11[index];

                const uint32_t table12[2] = {0x800080,0x2000200};
                cmd.DW50.Value = table12[index];
            }

            MHW_MI_CHK_NULL(extParams->pRefIdxMapping);
            for (int i = 0; i < params->ucNumRefIdxL0ActiveMinus1 + 1; i++)
            {
                uint8_t refFrameIDx = hevcSlcParams->RefPicList[0][i].FrameIdx;
                if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
                {
                    *((uint8_t *)&cmd.DW11.Value + i) = *(extParams->pRefIdxMapping + refFrameIDx);
                }
            }
            for (int i = params->ucNumRefIdxL0ActiveMinus1 + 1; i < 3; i++)
            {
                *((uint8_t *)&cmd.DW11.Value + i) = 0x7;
            }
            if (hevcPicParams->pps_curr_pic_ref_enabled_flag)
            {
                *((uint8_t *)&cmd.DW11.Value + params->ucNumRefIdxL0ActiveMinus1 + 1) = extParams->recNotFilteredID;
            }
            if (!extParams->bIsLowDelayB)
            {
                 uint8_t refFrameIDx = hevcSlcParams->RefPicList[1][0].FrameIdx;
                 if (refFrameIDx < CODEC_MAX_NUM_REF_FRAME_HEVC)
                 {
                     const uint8_t bwdOffset = 3;
                     *((uint8_t *)&cmd.DW11.Value + bwdOffset) = *(extParams->pRefIdxMapping + refFrameIDx);
                 }
            }
            cmd.DW11.Value = (cmd.DW11.Value & 0x7fffffff) | 0x80000000;
        }
        else if (params->pVp9EncPicParams)
        {
            MHW_MI_CHK_NULL(params->pVp9EncPicParams);
            auto vp9PicParams = params->pVp9EncPicParams;
            MHW_MI_CHK_NULL(params->pVp9EncSeqParams);
            auto vp9SeqParams = params->pVp9EncSeqParams;

            cmd.DW1.FrameWidthInPixelsMinusOne = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameWidthMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;
            cmd.DW1.FrameHeightInPixelsMinusOne = MOS_ALIGN_CEIL(vp9PicParams->SrcFrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH) - 1;

            cmd.DW2.Value = (cmd.DW2.Value & 0x8ff00000) | 0x5aff3;
            cmd.DW5.Value = (cmd.DW5.Value & 0xf000300) | 0x80ac00;
            cmd.DW6.Value = 0x20080200;
            cmd.DW7.Value = (cmd.DW7.Value & 0x190fd0) | 0x62003;
            cmd.DW9.Value = (cmd.DW9.Value & 0xfff0ffff) | 0x40000;
            cmd.DW14.Value = (cmd.DW14.Value & 0xffff) | 0x1f40000;
            cmd.DW15.Value = 0x138807d0;
            cmd.DW16.Value = (cmd.DW16.Value & 0xff0000) | 0xf00ff00;
            cmd.DW17.Value = (cmd.DW17.Value & 0xfff00000) | 0x2710;
            cmd.DW18.Value = 0x80000;
            cmd.DW19.Value = (cmd.DW19.Value & 0xffff0000) | 0x40;
            cmd.DW22.Value = 0x1f001f00;
            cmd.DW23.Value = 0x6a1a1f00;
            cmd.DW28.Value = 0x7d00fa0;
            cmd.DW29.Value = 0x2bc0bb8;
            cmd.DW30.Value = 0x32003e8;
            cmd.DW31.Value = 0x1f4012c;
            cmd.DW32.Value = 0x55220190;
            cmd.DW33.Value = 0x22552222;
            cmd.DW34.Value = 0x21225522;
            cmd.DW2.PictureType = vp9PicParams->PicFlags.fields.frame_type;
            cmd.DW2.TemporalMvpEnableFlag = params->temporalMVpEnable;

            if (vp9PicParams->PicFlags.fields.frame_type) // P_FRAME
            {
                cmd.DW5.NumRefIdxL0Minus1 = params->ucNumRefIdxL0ActiveMinus1;
                cmd.DW7.Value = (cmd.DW7.Value & 0xfff7ffff) | 0x80000;
            }
            else // I_FRAME
            {
                cmd.DW5.Value &= 0xf0ffffff;
                cmd.DW7.Value &= 0xfff7ffff;
            }

            cmd.DW7.SegmentationEnable = (vp9PicParams->PicFlags.fields.frame_type == 0) ? 0 : vp9PicParams->PicFlags.fields.segmentation_enabled;
            cmd.DW7.TilingEnable = (vp9PicParams->log2_tile_columns != 0) || (vp9PicParams->log2_tile_rows != 0);
            cmd.DW7.PakOnlyMultiPassEnable = params->bPakOnlyMultipassEnable;
            cmd.DW7.VdencStreamInEnable = params->bStreamInEnabled;

            if (params->bSegmentationEnabled)
            {
                cmd.DW7.SegmentationEnable = true;
                cmd.DW7.SegmentationMapTemporalPredictionEnable = vp9PicParams->PicFlags.fields.frame_type ? (params->bPrevFrameSegEnabled ? 1 : 0) : 0;
                cmd.DW7.VdencStreamInEnable = params->bStreamInEnabled;

                MHW_MI_CHK_NULL(params->pVp9SegmentState);
                MHW_MI_CHK_NULL(params->pVp9SegmentState->pVp9EncodeSegmentParams);

                cmd.DW24.QpForSeg0 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[0].SegmentQIndexDelta;
                cmd.DW24.QpForSeg1 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[1].SegmentQIndexDelta;
                cmd.DW24.QpForSeg2 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[2].SegmentQIndexDelta;
                cmd.DW24.QpForSeg3 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[3].SegmentQIndexDelta;

                cmd.DW25.QpForSeg4 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[4].SegmentQIndexDelta;
                cmd.DW25.QpForSeg5 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[5].SegmentQIndexDelta;
                cmd.DW25.QpForSeg6 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[6].SegmentQIndexDelta;
                cmd.DW25.QpForSeg7 = vp9PicParams->LumaACQIndex + params->pVp9SegmentState->pVp9EncodeSegmentParams->SegData[7].SegmentQIndexDelta;
            }
            else
            {
                cmd.DW24.QpForSeg0 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;
                cmd.DW24.QpForSeg1 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;
                cmd.DW24.QpForSeg2 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;
                cmd.DW24.QpForSeg3 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;

                cmd.DW25.QpForSeg4 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;
                cmd.DW25.QpForSeg5 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;
                cmd.DW25.QpForSeg6 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;
                cmd.DW25.QpForSeg7 = vp9PicParams->LumaACQIndex + vp9PicParams->LumaDCQIndexDelta;
            }

            cmd.DW26.SadQpLambda = params->usSADQPLambda;
            cmd.DW26.RdQpLambda = params->usRDQPLambda;
            cmd.DW26.Vp9DynamicSliceEnable = params->bDynamicScalingEnabled;

            cmd.DW27.QpPrimeYAc = vp9PicParams->LumaACQIndex;
            cmd.DW27.QpPrimeYDc = cmd.DW27.QpPrimeYAc + vp9PicParams->LumaDCQIndexDelta;

            switch (vp9SeqParams->TargetUsage)
            {
            case 1:     // Quality mode
            case 4:     // Normal mode
                cmd.DW2.Value &= 0xdfffffff;
                cmd.DW7.Value &= 0xfff7feff;
                cmd.DW9.Value = (cmd.DW9.Value & 0xfffff) | 0x43800000;
                cmd.DW34.Value = (cmd.DW34.Value & 0xffffff) | 0x21000000;
                break;
            case 7:     // Speed mode
                cmd.DW2.Value = (cmd.DW2.Value & 0xdfffffff) | 0x20000000;
                cmd.DW7.Value = (cmd.DW7.Value & 0xfff7feff) | 0x80100;
                cmd.DW9.Value = (cmd.DW9.Value & 0xffff) | 0x22420000;
                cmd.DW34.Value = (cmd.DW34.Value & 0xffffff) | 0x20000000;
                break;
            default:
                MHW_ASSERTMESSAGE("Invalid TU provided!");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            if (params->ucNumRefIdxL0ActiveMinus1 == 0)
            {
                cmd.DW7.Value = (cmd.DW7.Value & 0xfff7ffff) | 0x80000;
            }
        }

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
            return MOS_STATUS_NULL_POINTER;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }
};

#endif // __MHW_VDBOX_VDENC_G12_X_H__
