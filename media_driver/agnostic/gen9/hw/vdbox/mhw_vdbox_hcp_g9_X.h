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
//! \file     mhw_vdbox_hcp_g9_X.h
//! \brief    Defines functions for constructing Vdbox HCP commands on Gen9-based platforms
//!

#ifndef __MHW_VDBOX_HCP_G9_X_H__
#define __MHW_VDBOX_HCP_G9_X_H__

#include "mhw_vdbox_hcp_generic.h"
#include "mhw_mmio_g9.h"

// CU Record structure
struct EncodeHevcCuDataG9
{
    //DW0
    uint32_t                cu_size : 2;
    uint32_t                cu_pred_mode : 1;
    uint32_t                cu_transquant_bypass_flag : 1;
    uint32_t                cu_part_mode : 3;
    uint32_t                Res_DW0_7 : 1;
    uint32_t                intra_chroma_mode : 3;
    uint32_t                Res_DW0_11_15 : 5;
    uint32_t                CU_QP : 6;
    uint32_t                Res_DW0_22_23 : 2;
    uint32_t                interpred_idc : 8;

    //DW1
    uint8_t               intra_mode[4];

    //DW2 and DW3
    int16_t               mvx_l0[4];

    //DW4 and DW5
    int16_t               mvy_l0[4];

    //DW6 and DW7
    int16_t               mvx_l1[4];

    //DW8 and DW9
    int16_t               mvy_l1[4];

    //DW10
    uint32_t                ref_idx_l0_PU1 : 4;
    uint32_t                ref_idx_l0_PU2 : 4;
    uint32_t                ref_idx_l0_PU3 : 4;
    uint32_t                ref_idx_l0_PU4 : 4;
    uint32_t                ref_idx_l1_PU1 : 4;
    uint32_t                ref_idx_l1_PU2 : 4;
    uint32_t                ref_idx_l1_PU3 : 4;
    uint32_t                ref_idx_l1_PU4 : 4;

    //DW11
    uint32_t                tu_size : 32;

    //DW12
    uint32_t                tu_xform_Yskip : 16;
    uint32_t                Res_DW12_16_27 : 12;
    uint32_t                tu_countm1 : 4;

    //DW13
    uint32_t                tu_xform_Uskip : 16;
    uint32_t                tu_xform_Vskip : 16;

    //DW14
    uint32_t                Res_DW14 : 32;

    //DW15
    uint32_t                Res_DW15 : 32;

};

//!  MHW Vdbox Hcp interface for Gen9
/*!
This class defines the Hcp command construction functions for Gen9 platforms as template
*/
template <class THcpCmds>
class MhwVdboxHcpInterfaceG9 : public MhwVdboxHcpInterfaceGeneric<THcpCmds>
{
protected:

    enum HEVC_QM_TYPES
    {
        HEVC_QM_4x4     = 0,
        HEVC_QM_8x8     = 1,
        HEVC_QM_16x16   = 2,
        HEVC_QM_32x32   = 3
    };

    static const uint32_t m_veboxRgbHistogramSizePerSlice = 256 * 4;
    static const uint32_t m_veboxNumRgbChannel = 3;
    static const uint32_t m_veboxAceHistogramSizePerFramePerSlice = 256 * 4;
    static const uint32_t m_veboxNumFramePreviousCurrent = 2;

    static const uint32_t m_veboxMaxSlices = 2;
    static const uint32_t m_veboxRgbHistogramSize = m_veboxRgbHistogramSizePerSlice * m_veboxNumRgbChannel * m_veboxMaxSlices;
    static const uint32_t m_veboxLaceHistogram256BinPerBlock = 256 * 2;
    static const uint32_t m_veboxStatisticsSize = 32 * 4;

    //!
    //! \brief  Constructor
    //!
    MhwVdboxHcpInterfaceG9(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse)
        : MhwVdboxHcpInterfaceGeneric<THcpCmds>(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;

        this->m_hevcEncCuRecordSize = sizeof(EncodeHevcCuDataG9);
        InitMmioRegisters();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxHcpInterfaceG9() { MHW_FUNCTION_ENTER; }

    void InitMmioRegisters()
    {
        MmioRegistersHcp *mmioRegisters = &this->m_mmioRegisters[MHW_VDBOX_NODE_1];

        mmioRegisters->hcpEncImageStatusMaskRegOffset                    = HCP_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpEncImageStatusCtrlRegOffset                    = HCP_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpEncBitstreamBytecountFrameRegOffset            = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpEncBitstreamSeBitcountFrameRegOffset           = HCP_ENC_BIT_STREAM_SE_BIT_COUNT_FRAME_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpEncBitstreamBytecountFrameNoHeaderRegOffset    = HCP_ENC_BIT_STREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpEncQpStatusCountRegOffset                      = HCP_ENC_QP_STATUS_COUNT_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpEncSliceCountRegOffset                         = HCP_ENC_SLICE_COUNT_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpEncVdencModeTimerRegOffset                     = HCP_ENC_VDENC_MODE_TIMER_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpVp9EncBitstreamBytecountFrameRegOffset         = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpVp9EncBitstreamBytecountFrameNoHeaderRegOffset = HCP_VP9_ENC_BITSTREAM_BYTE_COUNT_FRAME_NO_HEADER_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpVp9EncImageStatusMaskRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_MASK_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpVp9EncImageStatusCtrlRegOffset                 = HCP_VP9_ENC_IMAGE_STATUS_CTRL_REG_OFFSET_INIT_G9;
        mmioRegisters->csEngineIdOffset                                  = CS_ENGINE_ID_OFFSET_INIT_G9;
        mmioRegisters->hcpDecStatusRegOffset                             = HCP_DEC_STATUS_REG_OFFSET_INIT_G9;
        mmioRegisters->hcpCabacStatusRegOffset                           = HCP_CABAC_STATUS_REG_OFFSET_INIT_G9;

    }

    void InitRowstoreUserFeatureSettings(MOS_CONTEXT_HANDLE mosCtx)
    {
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

        userFeatureData.u32Data = 0;

        userFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ROWSTORE_CACHE_DISABLE_ID,
            &userFeatureData,
            mosCtx);
#endif // _DEBUG || _RELEASE_INTERNAL
        this->m_rowstoreCachingSupported = userFeatureData.i32Data ? false : true;

        if (this->m_rowstoreCachingSupported)
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_HEVCDATROWSTORECACHE_DISABLE_ID,
                &userFeatureData,
                mosCtx);
#endif // _DEBUG || _RELEASE_INTERNAL
            this->m_hevcDatRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_HEVCDFROWSTORECACHE_DISABLE_ID,
                &userFeatureData,
                mosCtx);
#endif // _DEBUG || _RELEASE_INTERNAL
            this->m_hevcDfRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;

            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_HEVCSAOROWSTORECACHE_DISABLE_ID,
                &userFeatureData,
                mosCtx);
#endif // _DEBUG || _RELEASE_INTERNAL
            this->m_hevcSaoRowStoreCache.bSupported = userFeatureData.i32Data ? false : true;
        }
    }

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(rowstoreParams);

        if (this->m_hevcDatRowStoreCache.bSupported && (rowstoreParams->Mode == CODECHAL_DECODE_MODE_HEVCVLD))
        {
            this->m_hevcDatRowStoreCache.bEnabled = true;
            if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_8K)
            {
                this->m_hevcDatRowStoreCache.dwAddress = HEVCDATROWSTORE_BASEADDRESS;
            }
            else
            {
                this->m_hevcDatRowStoreCache.dwAddress = 0;
                this->m_hevcDatRowStoreCache.bEnabled = false;
            }
        }

        if (this->m_hevcDfRowStoreCache.bSupported && (rowstoreParams->Mode == CODECHAL_DECODE_MODE_HEVCVLD))
        {
            this->m_hevcDfRowStoreCache.bEnabled = true;
            if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K)
            {
                this->m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
            }
            else if ((rowstoreParams->dwPicWidth > MHW_VDBOX_PICWIDTH_2K) && (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_4K))
            {
                if (rowstoreParams->ucBitDepthMinus8 == 0)
                {
                    this->m_hevcDfRowStoreCache.dwAddress = HEVCDFROWSTORE_BASEADDRESS_PICWIDTH_BETWEEN_2K_AND_4K;
                }
                else
                {
                    this->m_hevcDfRowStoreCache.dwAddress = 0;
                    this->m_hevcDfRowStoreCache.bEnabled = false;
                }
            }
            else
            {
                this->m_hevcDfRowStoreCache.dwAddress = 0;
                this->m_hevcDfRowStoreCache.bEnabled = false;
            }
        }

        if (this->m_hevcSaoRowStoreCache.bSupported && (rowstoreParams->Mode == CODECHAL_DECODE_MODE_HEVCVLD))
        {
            this->m_hevcSaoRowStoreCache.bEnabled = true;
            if (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K)
            {
                if (rowstoreParams->ucBitDepthMinus8 == 0)
                {
                    this->m_hevcSaoRowStoreCache.dwAddress = HEVCSAOROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
                }
                else
                {
                    this->m_hevcSaoRowStoreCache.dwAddress = 0;
                    this->m_hevcSaoRowStoreCache.bEnabled = false;
                }
            }
            else
            {
                this->m_hevcSaoRowStoreCache.dwAddress = 0;
                this->m_hevcSaoRowStoreCache.bEnabled = false;
            }
        }

        if (this->m_vp9HvdRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP9VLD)
        {
            this->m_vp9HvdRowStoreCache.bEnabled = true;
            if ((rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_8K && rowstoreParams->ucBitDepthMinus8 == 0) ||
                (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K && rowstoreParams->ucBitDepthMinus8 == 2))
            {
                this->m_vp9HvdRowStoreCache.dwAddress = VP9HVDROWSTORE_BASEADDRESS;
            }
            else
            {
                this->m_vp9HvdRowStoreCache.dwAddress = 0;
                this->m_vp9HvdRowStoreCache.bEnabled = false;
            }
        }

        if (this->m_vp9DfRowStoreCache.bSupported && rowstoreParams->Mode == CODECHAL_DECODE_MODE_VP9VLD)
        {
            this->m_vp9DfRowStoreCache.bEnabled = true;
            if ((rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_2K && rowstoreParams->ucBitDepthMinus8 == 0) ||
                (rowstoreParams->dwPicWidth <= MHW_VDBOX_PICWIDTH_1K && rowstoreParams->ucBitDepthMinus8 == 2))
            {
                this->m_vp9DfRowStoreCache.dwAddress = VP9DFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K;
            }
            else
            {
                this->m_vp9DfRowStoreCache.dwAddress = 0;
                this->m_vp9DfRowStoreCache.bEnabled = false;
            }
        }

        return eStatus;
    }

    MOS_STATUS GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
        PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(hcpBufSizeParam);

        uint32_t mvtSize = 0;
        uint32_t mvtbSize = 0;
        uint32_t bufferSize = 0;
        uint8_t  maxBitDepth = hcpBufSizeParam->ucMaxBitDepth;
        uint32_t picWidth = hcpBufSizeParam->dwPicWidth;
        uint32_t picHeight = hcpBufSizeParam->dwPicHeight;
        uint32_t picHeightLCU = picHeight >> 4;//assume smallest LCU to get max height
        uint32_t picWidthLCU = picWidth >> 4;
        uint8_t  shiftParam = (maxBitDepth == 10) ? 2 : 3;

        switch (bufferType)
        {
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
            bufferSize = ((picWidth + 31) & 0xFFFFFFE0) >> shiftParam;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
            bufferSize = ((picHeight + picHeightLCU * 6 + 31) & 0xFFFFFFE0) >> shiftParam;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
            bufferSize = (((((picWidth + 15) >> 4) * 188 + picWidthLCU * 9 + 1023) >> 9) + 1) & (-2);
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
            bufferSize = (((((picWidth + 15) >> 4) * 172 + picWidthLCU * 9 + 1023) >> 9) + 1) & (-2);
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
            bufferSize = (((((picHeight + 15) >> 4) * 176 + picHeightLCU * 89 + 1023) >> 9) + 1) & (-2);
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE:
            bufferSize = (((((picWidth >> 1) + picWidthLCU * 3) + 15) & 0xFFFFFFF0) >> shiftParam);
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE:
            bufferSize = (((((picWidth >> 1) + picWidthLCU * 6) + 15) & 0xFFFFFFF0) >> shiftParam);
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL:
            bufferSize = (((((picHeight >> 1) + picHeightLCU * 6) + 15) & 0xFFFFFFF0) >> shiftParam);
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
            mvtSize = ((((picWidth + 63) >> 6)*(((picHeight + 15) >> 4)) + 1)&(-2));
            mvtbSize = ((((picWidth + 31) >> 5)*(((picHeight + 31) >> 5)) + 1)&(-2));
            bufferSize = MOS_MAX(mvtSize, mvtbSize);
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }
        hcpBufSizeParam->dwBufferSize = bufferSize * MHW_CACHELINE_SIZE;

        return eStatus;
    }

    MOS_STATUS GetVp9BufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
        PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam)

    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(hcpBufSizeParam);

        uint32_t bufferSize = 0;
        uint32_t dblkRsbSizeMultiplier = 0;
        uint32_t dblkCsbSizeMultiplier = 0;

        uint8_t  maxBitDepth = hcpBufSizeParam->ucMaxBitDepth;
        uint32_t widthInSb = hcpBufSizeParam->dwPicWidth;
        uint32_t heightInSb = hcpBufSizeParam->dwPicHeight;
        HCP_CHROMA_FORMAT_IDC chromaFormat = (HCP_CHROMA_FORMAT_IDC)hcpBufSizeParam->ucChromaFormat;
        uint32_t sizeScale = (maxBitDepth > 8) ? 2 : 1;

        if (chromaFormat == HCP_CHROMA_FORMAT_YUV420)
        {
            dblkRsbSizeMultiplier = sizeScale * 18;
            dblkCsbSizeMultiplier = sizeScale * 17;
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            MHW_ASSERTMESSAGE("Format not supported.");
            return eStatus;
        }

        switch (bufferType)
        {
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
            bufferSize = widthInSb * dblkRsbSizeMultiplier;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
            bufferSize = heightInSb * dblkCsbSizeMultiplier;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
            bufferSize = widthInSb * 5;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
            bufferSize = heightInSb * 5;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_COLL_MV_TEMPORAL:
            bufferSize = widthInSb * heightInSb * 9;
            break;
        case MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID:
            bufferSize = widthInSb * heightInSb;
            break;
        case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE:
        case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE:
            bufferSize = widthInSb;
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }

        hcpBufSizeParam->dwBufferSize = bufferSize * MHW_CACHELINE_SIZE;

        return eStatus;
    }

    MOS_STATUS IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
        PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(reallocParam);

        bool     realloc = false;
        uint32_t picWidth = reallocParam->dwPicWidth;
        uint32_t picHeight = reallocParam->dwPicHeight;
        uint32_t picWidthAlloced = reallocParam->dwPicWidthAlloced;
        uint32_t picHeightAlloced = reallocParam->dwPicHeightAlloced;

        switch (bufferType)
        {
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_LINE:
            realloc = (picWidth > picWidthAlloced) ? true : false;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_SAO_TILE_COL:
            realloc = (picHeight > picHeightAlloced) ? true : false;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
            realloc = (picWidth > picWidthAlloced || picHeight > picHeightAlloced) ? true : false;
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }
        reallocParam->bNeedBiggerSize = realloc;

        return eStatus;
    }

    MOS_STATUS IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
        PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(reallocParam);

        bool     realloc = false;
        uint32_t widthInSb = reallocParam->dwPicWidth;
        uint32_t heightInSb = reallocParam->dwPicHeight;
        uint32_t picWidthInSbAlloced = reallocParam->dwPicWidthAlloced;
        uint32_t picHeightInSbAlloced = reallocParam->dwPicHeightAlloced;

        switch (bufferType)
        {
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_LINE:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_LINE:
        case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_LINE:
        case MHW_VDBOX_VP9_INTERNAL_BUFFER_HVD_TILE:
            realloc = (widthInSb > picWidthInSbAlloced) ? true : false;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_DBLK_TILE_COL:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_META_TILE_COL:
            realloc = (heightInSb > picHeightInSbAlloced) ? true : false;
            break;
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_CURR_MV_TEMPORAL:
        case MHW_VDBOX_HCP_INTERNAL_BUFFER_COLL_MV_TEMPORAL:
        case MHW_VDBOX_VP9_INTERNAL_BUFFER_SEGMENT_ID:
            realloc = (heightInSb > picHeightInSbAlloced || widthInSb > picWidthInSbAlloced) ? true : false;
            break;
        default:
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            break;
        }

        reallocParam->bNeedBiggerSize = realloc;

        return eStatus;
    }

    MOS_STATUS AddHcpVp9PicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_VP9_PIC_STATE         params)
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddHcpVp9PicStateEncCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_VP9_ENCODE_PIC_STATE  params)
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddHcpHevcVp9RdoqStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params)
    {
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddHcpPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(params);

        typename THcpCmds::HCP_PIPE_MODE_SELECT_CMD  cmd;

        cmd.DW1.CodecStandardSelect = CodecHal_GetStandardFromMode(params->Mode) - CODECHAL_HCP_BASE;
        cmd.DW1.DeblockerStreamoutEnable = params->bDeblockerStreamOutEnable;

        if (this->m_decodeInUse)
        {
            cmd.DW1.CodecSelect = cmd.CODEC_SELECT_DECODE;
        }
        else
        {
            cmd.DW1.CodecSelect = cmd.CODEC_SELECT_ENCODE;
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpQmStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_QM_PARAMS             params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename THcpCmds::HCP_QM_STATE_CMD cmd;

        if (params->Standard == CODECHAL_HEVC)
        {
            uint8_t*   qMatrix = nullptr;
            MHW_MI_CHK_NULL(params->pHevcIqMatrix);

            qMatrix = (uint8_t*)cmd.Quantizermatrix;

            for (uint8_t sizeId = 0; sizeId < 4; sizeId++)            // 4x4, 8x8, 16x16, 32x32
            {
                for (uint8_t predType = 0; predType < 2; predType++)  // Intra, Inter
                {
                    for (uint8_t color = 0; color < 3; color++)       // Y, Cb, Cr
                    {
                        if ((sizeId == 3) && (color != 0))
                            break;

                        cmd.DW1.Sizeid = sizeId;
                        cmd.DW1.PredictionType = predType;
                        cmd.DW1.ColorComponent = color;
                        switch (sizeId)
                        {
                        case HEVC_QM_4x4:
                        case HEVC_QM_8x8:
                        default:
                            cmd.DW1.DcCoefficient = 0;
                            break;
                        case HEVC_QM_16x16:
                            cmd.DW1.DcCoefficient = params->pHevcIqMatrix->ListDC16x16[3 * predType + color];
                            break;
                        case HEVC_QM_32x32:
                            cmd.DW1.DcCoefficient = params->pHevcIqMatrix->ListDC32x32[predType];
                            break;
                        }

                        if (sizeId == HEVC_QM_4x4)
                        {
                            for (uint8_t i = 0; i < 4; i++)
                            {
                                for (uint8_t ii = 0; ii < 4; ii++)
                                {
                                    qMatrix[4 * i + ii] = params->pHevcIqMatrix->List4x4[3 * predType + color][4 * i + ii];
                                }
                            }
                        }
                        else if (sizeId == HEVC_QM_8x8)
                        {
                            for (uint8_t i = 0; i < 8; i++)
                            {
                                for (uint8_t ii = 0; ii < 8; ii++)
                                {
                                    qMatrix[8 * i + ii] = params->pHevcIqMatrix->List8x8[3 * predType + color][8 * i + ii];
                                }
                            }
                        }
                        else if (sizeId == HEVC_QM_16x16)
                        {
                            for (uint8_t i = 0; i < 8; i++)
                            {
                                for (uint8_t ii = 0; ii < 8; ii++)
                                {
                                    qMatrix[8 * i + ii] = params->pHevcIqMatrix->List16x16[3 * predType + color][8 * i + ii];
                                }
                            }
                        }
                        else // 32x32
                        {
                            for (uint8_t i = 0; i < 8; i++)
                            {
                                for (uint8_t ii = 0; ii < 8; ii++)
                                {
                                    qMatrix[8 * i + ii] = params->pHevcIqMatrix->List32x32[predType][8 * i + ii];
                                }
                            }
                        }

                        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
                    }
                }
            }
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    MOS_STATUS AddHcpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS  params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS                            resourceParams;
        typename THcpCmds::HCP_PIPE_BUF_ADDR_STATE_CMD cmd;
        bool                                           firstRefPic = true;

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        UserFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED_ID;
        if (this->m_hevcDatRowStoreCache.bEnabled     ||
            this->m_hevcDfRowStoreCache.bEnabled      ||
            this->m_hevcSaoRowStoreCache.bEnabled     ||
            this->m_vp9HvdRowStoreCache.bEnabled      ||
            this->m_vp9DfRowStoreCache.bEnabled)
        {
            UserFeatureWriteData.Value.i32Data = 1;
        }
        MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, this->m_osInterface->pOsContext);
#endif

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_HCP_DECODED_BUFFER_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

        // Decoded Picture
        // Caching policy change if any of below modes are true
        // Note for future dev: probably a good idea to add a macro for the below check
        if (this->m_osInterface->osCpInterface->IsHMEnabled() ||
            this->m_osInterface->osCpInterface->IsIDMEnabled() ||
            this->m_osInterface->osCpInterface->IsSMEnabled())
        {
            cmd.DecodedPictureMemoryAddressAttributes.DW0.Value |= this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC_PARTIALENCSURFACE].Value;
        }
        else
        {
            cmd.DecodedPictureMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Value;
        }

        cmd.DecodedPictureMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->psPreDeblockSurface->TileType);

        // For HEVC 8bit/10bit mixed case, register App's RenderTarget for specific use case
        if (params->presP010RTSurface != nullptr)
        {
            resourceParams.presResource = &(params->presP010RTSurface->OsResource);
            resourceParams.dwOffset = params->presP010RTSurface->dwOffset;
            resourceParams.pdwCmd = (cmd.DecodedPicture.DW0_1.Value);
            resourceParams.dwLocationInCmd = 1;
            resourceParams.bIsWritable = true;
            
            if (this->m_osInterface->bAllowExtraPatchToSameLoc)
            {
                MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
            else //if not allowed to patch another OsResource to same location in cmd, just register resource here
            {
                MHW_MI_CHK_STATUS(this->m_osInterface->pfnRegisterResource(
                    this->m_osInterface,
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

        MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
            this->m_osInterface,
            cmdBuffer,
            &resourceParams));

        resourceParams.dwLsbNum = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;

        // Deblocking Filter Line Buffer
        if (this->m_hevcDfRowStoreCache.bEnabled)
        {
            cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.DeblockingFilterLineBuffer.DW0_1.Graphicsaddress476 = this->m_hevcDfRowStoreCache.dwAddress;
        }
        else if (this->m_vp9DfRowStoreCache.bEnabled)
        {
            cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.DeblockingFilterLineBuffer.DW0_1.Graphicsaddress476 = this->m_vp9DfRowStoreCache.dwAddress;
        }
        else if (params->presMfdDeblockingFilterRowStoreScratchBuffer != nullptr)
        {
            cmd.DeblockingFilterLineBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presMfdDeblockingFilterRowStoreScratchBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DeblockingFilterLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 4;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Deblocking Filter Tile Line Buffer
        if (params->presDeblockingFilterTileRowStoreScratchBuffer != nullptr)
        {
            cmd.DeblockingFilterTileLineBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presDeblockingFilterTileRowStoreScratchBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DeblockingFilterTileLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 7;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Deblocking Filter Tile Column Buffer
        if (params->presDeblockingFilterColumnRowStoreScratchBuffer != nullptr)
        {
            cmd.DeblockingFilterTileColumnBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presDeblockingFilterColumnRowStoreScratchBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DeblockingFilterTileColumnBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 10;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Metadata Line Buffer
        if (this->m_hevcDatRowStoreCache.bEnabled)
        {
            cmd.MetadataLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.MetadataLineBuffer.DW0_1.Graphicsaddress476 = this->m_hevcDatRowStoreCache.dwAddress;
        }
        else if (params->presMetadataLineBuffer != nullptr)
        {
            cmd.MetadataLineBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

            resourceParams.presResource = params->presMetadataLineBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = cmd.MetadataLineBuffer.DW0_1.Value;
            resourceParams.dwLocationInCmd = 13;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Metadata Tile Line Buffer
        if (params->presMetadataTileLineBuffer != nullptr)
        {
            cmd.MetadataTileLineBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

            resourceParams.presResource = params->presMetadataTileLineBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.MetadataTileLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 16;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Metadata Tile Column Buffer
        if (params->presMetadataTileColumnBuffer != nullptr)
        {
            cmd.MetadataTileColumnBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MD_CODEC].Value;

            resourceParams.presResource = params->presMetadataTileColumnBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.MetadataTileColumnBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 19;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // SAO Line Buffer
        if (this->m_hevcSaoRowStoreCache.bEnabled)
        {
            cmd.SaoLineBufferMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.SaoLineBuffer.DW0_1.Graphicsaddress476 = this->m_hevcSaoRowStoreCache.dwAddress;
        }
        else if (params->presSaoLineBuffer != nullptr)
        {
            cmd.SaoLineBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

            resourceParams.presResource = params->presSaoLineBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.SaoLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 22;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // SAO Tile Line Buffer
        if (params->presSaoTileLineBuffer != nullptr)
        {
            cmd.SaoTileLineBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

            resourceParams.presResource = params->presSaoTileLineBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.SaoTileLineBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 25;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // SAO Tile Column Buffer
        if (params->presSaoTileColumnBuffer != nullptr)
        {
            cmd.SaoTileColumnBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_SAO_CODEC].Value;

            resourceParams.presResource = params->presSaoTileColumnBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.SaoTileColumnBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 28;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Current Motion Vector Temporal Buffer
        if (params->presCurMvTempBuffer != nullptr)
        {
            cmd.CurrentMotionVectorTemporalBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MV_CODEC].Value;

            resourceParams.presResource = params->presCurMvTempBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.CurrentMotionVectorTemporalBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 31;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Reference Picture Buffer
        cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.Value |=
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Value;

        // NOTE: for both HEVC and VP9, set all the 8 ref pic addresses in HCP_PIPE_BUF_ADDR_STATE command to valid addresses for error concealment purpose
        for (uint32_t i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
        {
            if (params->presReferences[i] != nullptr)
            {
                MOS_SURFACE  details;
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osInterface->pfnGetResourceInfo(this->m_osInterface, params->presReferences[i], &details));

                if (firstRefPic)
                {
                    cmd.ReferencePictureBaseAddressMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                    firstRefPic = false;
                }

                resourceParams.presResource = params->presReferences[i];
                resourceParams.pdwCmd = (cmd.ReferencePictureBaseAddressRefaddr07[i].DW0_1.Value);
                resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
                resourceParams.dwLocationInCmd = (i * 2) + 37; // * 2 to account for QW rather than DW
                resourceParams.bIsWritable = false;

                resourceParams.dwSharedMocsOffset = 53 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW53

                MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = MOS_MFX_PIPE_BUF_ADDR;

        // Original Uncompressed Picture Source, Encoder only
        if (params->psRawSurface != nullptr)
        {
            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE].Value;

            cmd.OriginalUncompressedPictureSourceMemoryAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->psRawSurface->TileType);

            resourceParams.presResource = &params->psRawSurface->OsResource;
            resourceParams.dwOffset = params->psRawSurface->dwOffset;
            resourceParams.pdwCmd = (cmd.OriginalUncompressedPictureSource.DW0_1.Value);
            resourceParams.dwLocationInCmd = 54;
            resourceParams.bIsWritable = false;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // StreamOut Data Destination, Decoder only
        if (params->presStreamOutBuffer != nullptr)
        {
            cmd.StreamoutDataDestinationMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_STREAMOUT_DATA_CODEC].Value;

            resourceParams.presResource = params->presStreamOutBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.StreamoutDataDestination.DW0_1.Value);
            resourceParams.dwLocationInCmd = 57;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Decoded Picture Status / Error Buffer Base Address
        if (params->presLcuBaseAddressBuffer != nullptr)
        {
            cmd.DecodedPictureStatusErrorBufferBaseAddressMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC].Value;

            resourceParams.presResource = params->presLcuBaseAddressBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DecodedPictureStatusErrorBufferBaseAddressOrEncodedSliceSizeStreamoutBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 60;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // LCU ILDB StreamOut Buffer
        if (params->presLcuILDBStreamOutBuffer != nullptr)
        {
            cmd.LcuIldbStreamoutBufferMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_LCU_ILDB_STREAMOUT_CODEC].Value;

            resourceParams.presResource = params->presLcuILDBStreamOutBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.LcuIldbStreamoutBuffer.DW0_1.Value);
            resourceParams.dwLocationInCmd = 63;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Collocated Motion vector Temporal Buffer
        cmd.CollocatedMotionVectorTemporalBuffer07MemoryAddressAttributes.DW0.Value |= 
            this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_HCP_MV_CODEC].Value;

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

                MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // VP9 Probability Buffer
        if (params->presVp9ProbBuffer != nullptr)
        {
            cmd.Vp9ProbabilityBufferReadWriteMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_PROBABILITY_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presVp9ProbBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.Vp9ProbabilityBufferReadWrite.DW0_1.Value);
            resourceParams.dwLocationInCmd = 83;
            resourceParams.bIsWritable = true;

            resourceParams.dwSharedMocsOffset = 85 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW88

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // VP9 Segment Id Buffer
        if (params->presVp9SegmentIdBuffer != nullptr)
        {
            cmd.Vp9SegmentIdBufferReadWriteMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_SEGMENT_ID_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presVp9SegmentIdBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DW86_87.Value);
            resourceParams.dwLocationInCmd = 86;
            resourceParams.bIsWritable = true;

            resourceParams.dwSharedMocsOffset = 88 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW88

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // Reset dwSharedMocsOffset
        resourceParams.dwSharedMocsOffset = 0;

        // HVD Line Row Store Buffer
        if (this->m_vp9HvdRowStoreCache.bEnabled)
        {
            cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = BUFFER_TO_INTERNALMEDIASTORAGE;
            cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Graphicsaddress476 = this->m_vp9HvdRowStoreCache.dwAddress;
        }
        else if (params->presHvdLineRowStoreBuffer != nullptr)
        {
            cmd.Vp9HvdLineRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presHvdLineRowStoreBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.Vp9HvdLineRowstoreBufferReadWrite.DW0_1.Value);
            resourceParams.dwLocationInCmd = 89;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        // HVD Tile Row Store Buffer
        if (params->presHvdTileRowStoreBuffer != nullptr)
        {
            cmd.Vp9HvdTileRowstoreBufferReadWriteMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC].Value;

            resourceParams.presResource = params->presHvdTileRowStoreBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.Vp9HvdTileRowstoreBufferReadWrite.DW0_1.Value);
            resourceParams.dwLocationInCmd = 92;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS resourceParams;
        typename THcpCmds::HCP_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        // mode specific settings
        if (CodecHalIsDecodeModeVLD(params->Mode))
        {
            MHW_MI_CHK_NULL(params->presDataBuffer);

            cmd.HcpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value |=
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Value;

            resourceParams.presResource = params->presDataBuffer;
            resourceParams.dwOffset = params->dwDataOffset;
            resourceParams.pdwCmd = (cmd.HcpIndirectBitstreamObjectBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 1;
            resourceParams.dwSize = params->dwDataSize;
            resourceParams.bIsWritable = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));

            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
        }

        // following is for encoder
        if (!this->m_decodeInUse)
        {
            if (params->presMvObjectBuffer)
            {
                cmd.HcpIndirectCuObjectObjectMemoryAddressAttributes.DW0.Value |=
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC].Value;

                resourceParams.presResource = params->presMvObjectBuffer;
                resourceParams.dwOffset = params->dwMvObjectOffset;
                resourceParams.pdwCmd = (cmd.DW6_7.Value);
                resourceParams.dwLocationInCmd = 6;
                resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwMvObjectSize, 0x1000);
                resourceParams.bIsWritable = false;

                // no upper bound for indirect CU object
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

                MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }

            if (params->presPakBaseObjectBuffer)
            {
                cmd.HcpPakBseObjectAddressMemoryAddressAttributes.DW0.Value |=
                    this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Value;

                resourceParams.presResource = params->presPakBaseObjectBuffer;
                resourceParams.dwOffset = 0;
                resourceParams.pdwCmd = (cmd.DW9_10.Value);
                resourceParams.dwLocationInCmd = 9;
                resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwPakBaseObjectSize, 0x1000);
                resourceParams.bIsWritable = true;

                // upper bound of the allocated resource will be set at 3 DW apart from address location
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

                MHW_MI_CHK_STATUS(this->pfnAddResourceToCmd(
                    this->m_osInterface,
                    cmdBuffer,
                    &resourceParams));
            }
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpFqmStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_QM_PARAMS             params)
    {
        MOS_STATUS   eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename THcpCmds::HCP_FQM_STATE_CMD cmd;

        if (params->Standard == CODECHAL_HEVC)
        {
            MHW_MI_CHK_NULL(params->pHevcIqMatrix);

            auto      iqMatrix = params->pHevcIqMatrix;
            uint16_t  *fqMatrix = (uint16_t*)cmd.Quantizermatrix;

            /* 4x4 */
            for (uint8_t i = 0; i < 32; i++)
            {
                cmd.Quantizermatrix[i] = 0;
            }
            for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
            {
                cmd.DW1.IntraInter = intraInter;
                cmd.DW1.Sizeid = 0;
                cmd.DW1.ColorComponent = 0;

                for (uint8_t i = 0; i < 16; i++)
                {
                    fqMatrix[i] =
                        GetReciprocalScalingValue(iqMatrix->List4x4[3 * intraInter][i]);
                }

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
            }

            /* 8x8, 16x16 and 32x32 */
            for (uint8_t i = 0; i < 32; i++)
            {
                cmd.Quantizermatrix[i] = 0;
            }
            for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
            {
                cmd.DW1.IntraInter = intraInter;
                cmd.DW1.Sizeid = 1;
                cmd.DW1.ColorComponent = 0;

                for (uint8_t i = 0; i < 64; i++)
                {
                    fqMatrix[i] =
                        GetReciprocalScalingValue(iqMatrix->List8x8[3 * intraInter][i]);
                }

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
            }

            /* 16x16 DC */
            for (uint8_t i = 0; i < 32; i++)
            {
                cmd.Quantizermatrix[i] = 0;
            }
            for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
            {
                cmd.DW1.IntraInter = intraInter;
                cmd.DW1.Sizeid = 2;
                cmd.DW1.ColorComponent = 0;
                cmd.DW1.FqmDcValue1Dc = GetReciprocalScalingValue(iqMatrix->ListDC16x16[3 * intraInter]);

                for (uint8_t i = 0; i < 64; i++)
                {
                    fqMatrix[i] =
                        GetReciprocalScalingValue(iqMatrix->List16x16[3 * intraInter][i]);
                }

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
            }

            /* 32x32 DC */
            for (uint8_t i = 0; i < 32; i++)
            {
                cmd.Quantizermatrix[i] = 0;
            }
            for (uint8_t intraInter = 0; intraInter <= 1; intraInter++)
            {
                cmd.DW1.IntraInter = intraInter;
                cmd.DW1.Sizeid = 3;
                cmd.DW1.ColorComponent = 0;
                cmd.DW1.FqmDcValue1Dc = GetReciprocalScalingValue(iqMatrix->ListDC32x32[intraInter]);

                for (uint8_t i = 0; i < 64; i++)
                {
                    fqMatrix[i] =
                        GetReciprocalScalingValue(iqMatrix->List32x32[intraInter][i]);
                }

                MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
            }
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
        }

        return eStatus;
    }

    MOS_STATUS AddHcpEncodePicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pHevcEncSeqParams);
        MHW_MI_CHK_NULL(params->pHevcEncPicParams);

        typename THcpCmds::HCP_PIC_STATE_CMD  cmd;

        auto hevcSeqParams  = params->pHevcEncSeqParams;
        auto hevcPicParams  = params->pHevcEncPicParams;

        cmd.DW1.Framewidthinmincbminus1         = hevcSeqParams->wFrameWidthInMinCbMinus1;
        cmd.DW1.Frameheightinmincbminus1        = hevcSeqParams->wFrameHeightInMinCbMinus1;

        cmd.DW2.Mincusize                       = hevcSeqParams->log2_min_coding_block_size_minus3;
        cmd.DW2.CtbsizeLcusize                  = hevcSeqParams->log2_max_coding_block_size_minus3;
        cmd.DW2.Maxtusize                       = hevcSeqParams->log2_max_transform_block_size_minus2;
        cmd.DW2.Mintusize                       = hevcSeqParams->log2_min_transform_block_size_minus2;
        cmd.DW2.Minpcmsize                      = 0;
        cmd.DW2.Maxpcmsize                      = 0;

        cmd.DW3.Colpicisi                       = 0; // MBZ
        cmd.DW3.Curpicisi                       = 0; // MBZ

        cmd.DW4.SampleAdaptiveOffsetEnabledFlag         = 0;
        cmd.DW4.PcmEnabledFlag                          = 0;
        cmd.DW4.CuQpDeltaEnabledFlag                    = hevcPicParams->cu_qp_delta_enabled_flag;
        cmd.DW4.DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth  = hevcPicParams->diff_cu_qp_delta_depth;
        cmd.DW4.PcmLoopFilterDisableFlag                = 1;
        cmd.DW4.ConstrainedIntraPredFlag                = 0;
        cmd.DW4.Log2ParallelMergeLevelMinus2            = 0;
        cmd.DW4.SignDataHidingFlag                      = 0;
        cmd.DW4.TilesEnabledFlag                        = 0;
        cmd.DW4.WeightedPredFlag                        = hevcPicParams->weighted_pred_flag;
        cmd.DW4.WeightedBipredFlag                      = hevcPicParams->weighted_bipred_flag;
        cmd.DW4.Fieldpic = 0;
        cmd.DW4.Bottomfield = 0;
        cmd.DW4.TransformSkipEnabledFlag                = hevcPicParams->transform_skip_enabled_flag;
        cmd.DW4.AmpEnabledFlag                          = hevcSeqParams->amp_enabled_flag;
        cmd.DW4.Reserved152                             = hevcPicParams->LcuMaxBitsizeAllowed > 0;
        cmd.DW4.TransquantBypassEnableFlag              = hevcPicParams->transquant_bypass_enabled_flag;
        cmd.DW4.StrongIntraSmoothingEnableFlag          = hevcSeqParams->strong_intra_smoothing_enable_flag;

        cmd.DW5.PicCbQpOffset                                           = hevcPicParams->pps_cb_qp_offset & 0x1f;
        cmd.DW5.PicCrQpOffset                                           = hevcPicParams->pps_cr_qp_offset & 0x1f;
        cmd.DW5.MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra = hevcSeqParams->max_transform_hierarchy_depth_intra;
        cmd.DW5.MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter = hevcSeqParams->max_transform_hierarchy_depth_inter;
        cmd.DW5.PcmSampleBitDepthChromaMinus1   = 7;
        cmd.DW5.PcmSampleBitDepthLumaMinus1     = 7;
    
        cmd.DW6.LcumaxbitstatusenLcumaxsizereportmask         = 1;
        cmd.DW6.FrameszoverstatusenFramebitratemaxreportmask  = 1;
        cmd.DW6.FrameszunderstatusenFramebitrateminreportmask = 1;

        cmd.DW6.LcuMaxBitsizeAllowed = hevcPicParams->LcuMaxBitsizeAllowed;
        if (params->maxFrameSize && params->currPass)
        {
            cmd.DW6.Nonfirstpassflag = 1;
        }
        else
        {
            cmd.DW6.Nonfirstpassflag = 0;
        }

        // Set this to max value
        cmd.DW7.Framebitratemax                               = (1 << 14) - 1;

        // Set this to Kilo Byte (=1) - Framebitratemax is in units of 4KBytes
        cmd.DW7.Framebitratemaxunit                           = 1;

        // Set this to min available value
        cmd.DW8.Framebitratemin                               = 0;

        // Set this to Kilo Byte (=1) - Framebitratemin is in units of 4KBytes
        cmd.DW8.Framebitrateminunit                           = 1;

        // Set frame bitrate max and min delta to 0
        cmd.DW9.Framebitratemindelta                          = 0;
        cmd.DW9.Framebitratemaxdelta                          = 0;

        cmd.DW10_11.Framedeltaqpmax                           = 0;
        cmd.DW12_13.Framedeltaqpmin                           = 0;

        // Set frame delta QP max and min range array as [0, 1, 2, 4, 8, 16, 32, 255]
        // Delta QP range = [Framebitratemaxdelta*(FramedeltaQpmaxrange[n]>>5), Framebitratemaxdelta*(FramedeltaQpmaxrange[n]>>5)]
        cmd.DW14_15.Value[0]                                  =
            cmd.DW16_17.Value[0]                              = (4 << 24) | (2 << 16) | (1 << 8);
        cmd.DW14_15.Value[1]                                  =
            cmd.DW16_17.Value[1]                              = (255 << 24) | (32 << 16) | (16 << 8) | 8;

        // Add for multiple pass
        if (params->maxFrameSize > 0 && params->deltaQp)
        {
            uint8_t hevcMaxPassNum = 8;

            // When current pass is less than the max number of pass, set the delta QP.
            if (params->currPass < hevcMaxPassNum)
            {
                cmd.DW10_11.Value[0] = (params->deltaQp[params->currPass] << 24) | (params->deltaQp[params->currPass] << 16) |
                    (params->deltaQp[params->currPass] << 8) | params->deltaQp[params->currPass];
                cmd.DW10_11.Value[1] = (params->deltaQp[params->currPass] << 24) | (params->deltaQp[params->currPass] << 16) |
                    (params->deltaQp[params->currPass] << 8) | params->deltaQp[params->currPass];
            }

            // If the calculated value of max frame size exceeded 14 bits, need set the unit as 4K byte. Else, set the unit as 32 byte.
            if (params->maxFrameSize >= (0x1 << 14) * 32)
            {
                cmd.DW7.Framebitratemaxunit = 1;
                cmd.DW7.Framebitratemax = params->maxFrameSize >> 12;
                cmd.DW9.Framebitratemaxdelta = params->maxFrameSize >> 13;
            }
            else
            {
                cmd.DW7.Framebitratemaxunit = 0;
                cmd.DW7.Framebitratemax = params->maxFrameSize >> 5;
                cmd.DW9.Framebitratemaxdelta = params->maxFrameSize >> 6;
            }
        }
    
        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpEncodeSliceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(hevcSliceState);

        typename THcpCmds::HCP_SLICE_STATE_CMD cmd;

        auto hevcSliceParams = hevcSliceState->pEncodeHevcSliceParams;
        auto hevcPicParams   = hevcSliceState->pEncodeHevcPicParams;
        auto hevcSeqParams   = hevcSliceState->pEncodeHevcSeqParams;

        uint32_t ctbSize    = 1 << (hevcSeqParams->log2_max_coding_block_size_minus3 + 3);
        uint32_t widthInPix = (1 << (hevcSeqParams->log2_min_coding_block_size_minus3 + 3)) *
                              (hevcSeqParams->wFrameWidthInMinCbMinus1 + 1);
        uint32_t widthInCtb = (widthInPix / ctbSize) +
                              ((widthInPix % ctbSize) ? 1 : 0);  // round up

        uint32_t ctbAddr    = hevcSliceParams->slice_segment_address;

        cmd.DW1.SlicestartctbxOrSliceStartLcuXEncoder   = ctbAddr % widthInCtb;
        cmd.DW1.SlicestartctbyOrSliceStartLcuYEncoder   = ctbAddr / widthInCtb;

        ctbAddr = hevcSliceParams->slice_segment_address + hevcSliceParams->NumLCUsInSlice;
        cmd.DW2.NextslicestartctbxOrNextSliceStartLcuXEncoder = ctbAddr % widthInCtb;
        cmd.DW2.NextslicestartctbyOrNextSliceStartLcuYEncoder = ctbAddr / widthInCtb;

        cmd.DW3.SliceType                               = hevcSliceParams->slice_type;
        cmd.DW3.Lastsliceofpic                          = hevcSliceState->bLastSlice;
        cmd.DW3.DependentSliceFlag                      = hevcSliceParams->dependent_slice_segment_flag;
        cmd.DW3.SliceTemporalMvpEnableFlag              = hevcSliceParams->slice_temporal_mvp_enable_flag;
        cmd.DW3.Sliceqp                                 = hevcSliceParams->slice_qp_delta + hevcPicParams->QpY;
        cmd.DW3.SliceCbQpOffset                         = hevcSliceParams->slice_cb_qp_offset;
        cmd.DW3.SliceCrQpOffset                         = hevcSliceParams->slice_cr_qp_offset;

        cmd.DW4.SliceHeaderDisableDeblockingFilterFlag          = hevcSliceParams->slice_deblocking_filter_disable_flag;
        cmd.DW4.SliceTcOffsetDiv2OrFinalTcOffsetDiv2Encoder     = hevcSliceParams->tc_offset_div2;
        cmd.DW4.SliceBetaOffsetDiv2OrFinalBetaOffsetDiv2Encoder = hevcSliceParams->beta_offset_div2;
        cmd.DW4.SliceLoopFilterAcrossSlicesEnabledFlag  = 0;
        cmd.DW4.SliceSaoChromaFlag                      = 0;
        cmd.DW4.SliceSaoLumaFlag                        = 0;
        cmd.DW4.MvdL1ZeroFlag                           = 0;
        cmd.DW4.Islowdelay                              = hevcSliceState->bIsLowDelay;
        cmd.DW4.CollocatedFromL0Flag                    = hevcSliceParams->collocated_from_l0_flag;
        cmd.DW4.Chromalog2Weightdenom                   = hevcSliceParams->luma_log2_weight_denom + hevcSliceParams->delta_chroma_log2_weight_denom;
        cmd.DW4.LumaLog2WeightDenom                     = hevcSliceParams->luma_log2_weight_denom;
        cmd.DW4.CabacInitFlag                           = hevcSliceParams->cabac_init_flag;
        cmd.DW4.Maxmergeidx                             = hevcSliceParams->MaxNumMergeCand - 1;

        if (cmd.DW3.SliceTemporalMvpEnableFlag)
        {
            if (cmd.DW3.SliceType == MhwVdboxHcpInterface::hevcSliceI)
            {
                cmd.DW4.Collocatedrefidx = 0;
            }
            else
            {
                // need to check with Ce for DDI issues
                uint8_t collocatedFromL0Flag = cmd.DW4.CollocatedFromL0Flag;

                uint8_t collocatedRefIndex   = hevcPicParams->CollocatedRefPicIndex;
                MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

                uint8_t collocatedFrameIdx = hevcSliceState->pRefIdxMapping[collocatedRefIndex];
                MHW_ASSERT(collocatedRefIndex < CODEC_MAX_NUM_REF_FRAME_HEVC);

                cmd.DW4.Collocatedrefidx = collocatedFrameIdx;
            }
        }
        else
        {
             cmd.DW4.Collocatedrefidx   = 0;
        }

        cmd.DW5.Sliceheaderlength       = 0;

        if(!hevcPicParams->bUsedAsRef && hevcPicParams->CodingType != I_TYPE)
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

        cmd.DW7.Cabaczerowordinsertionenable            = 1;
        cmd.DW7.Emulationbytesliceinsertenable          = 1;
        cmd.DW7.HeaderInsertionEnable                   = 1;
        cmd.DW7.TailInsertionEnable                     =
                (hevcPicParams->bLastPicInSeq || hevcPicParams->bLastPicInStream) && hevcSliceState->bLastSlice;
        cmd.DW7.SlicedataEnable                         = 1;

        cmd.DW8.IndirectPakBseDataStartOffsetWrite      = hevcSliceState->dwHeaderBytesInserted;

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, hevcSliceState->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

        return eStatus;
    }

    MOS_STATUS AddHcpPakInsertObject(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS     params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && params->pBatchBufferForPakSlices == nullptr)
        {
            MHW_ASSERTMESSAGE("There was no valid buffer to add the HW command to.");
        }

        typename THcpCmds::HCP_PAK_INSERT_OBJECT_CMD cmd;
        uint32_t dwordsUsed = cmd.dwSize;

        if (params->bLastPicInSeq && params->bLastPicInStream)
        {
            uint32_t dwPadding[3];

            dwordsUsed += sizeof(dwPadding) / sizeof(dwPadding[0]);

            cmd.DW0.DwordLength = OP_LENGTH(dwordsUsed);
            cmd.DW1.Headerlengthexcludefrmsize = 0;
            cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = 1;
            cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = 1;
            cmd.DW1.EmulationflagEmulationbytebitsinsertenable = 0;
            cmd.DW1.SkipemulbytecntSkipEmulationByteCount = 0;
            cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = 16;
            cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;
            cmd.DW1.IndirectPayloadEnable = 0;

            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, params->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

            dwPadding[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
            dwPadding[1] = (1L | (1L << 24));
            dwPadding[2] = (HEVC_NAL_UT_EOB << 1) | (1L << 8);
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, params->pBatchBufferForPakSlices, &dwPadding[0], sizeof(dwPadding)));
        }
        else if (params->bLastPicInSeq || params->bLastPicInStream)
        {
            uint32_t dwLastPicInSeqData[2], dwLastPicInStreamData[2];

            dwordsUsed += params->bLastPicInSeq * 2 + params->bLastPicInStream * 2;

            cmd.DW0.DwordLength = OP_LENGTH(dwordsUsed);
            cmd.DW1.Headerlengthexcludefrmsize = 0;
            cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = 1;
            cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = 1;
            cmd.DW1.EmulationflagEmulationbytebitsinsertenable = 0;
            cmd.DW1.SkipemulbytecntSkipEmulationByteCount = 0;
            cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = 8;
            cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;
            cmd.DW1.IndirectPayloadEnable = 0;

            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, params->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

            if (params->bLastPicInSeq)
            {
                dwLastPicInSeqData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOS << 1) << 24));
                dwLastPicInSeqData[1] = 1;  // nuh_temporal_id_plus1
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, params->pBatchBufferForPakSlices, &dwLastPicInSeqData[0], sizeof(dwLastPicInSeqData)));
            }

            if (params->bLastPicInStream)
            {
                dwLastPicInStreamData[0] = (uint32_t)((1 << 16) | ((HEVC_NAL_UT_EOB << 1) << 24));
                dwLastPicInStreamData[1] = 1; // nuh_temporal_id_plus1
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, params->pBatchBufferForPakSlices, &dwLastPicInStreamData[0], sizeof(dwLastPicInStreamData)));
            }
        }
        else
        {
            uint32_t byteSize = (params->dwBitSize + 7) >> 3;
            uint32_t dataBitsInLastDw = params->dwBitSize % 32;
            if (dataBitsInLastDw == 0)
            {
                dataBitsInLastDw = 32;
            }

            dwordsUsed += (MOS_ALIGN_CEIL(byteSize, sizeof(uint32_t))) / sizeof(uint32_t);
            cmd.DW0.DwordLength = OP_LENGTH(dwordsUsed);
            cmd.DW1.Headerlengthexcludefrmsize = 0;
            cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = params->bEndOfSlice;
            cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = params->bLastHeader;
            cmd.DW1.EmulationflagEmulationbytebitsinsertenable = params->bEmulationByteBitsInsert;
            cmd.DW1.SkipemulbytecntSkipEmulationByteCount = params->uiSkipEmulationCheckCount;
            cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;
            cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;
            cmd.DW1.IndirectPayloadEnable = 0;

            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, params->pBatchBufferForPakSlices, &cmd, cmd.byteSize));

            if (byteSize)
            {
                MHW_MI_CHK_NULL(params->pBsBuffer);
                MHW_MI_CHK_NULL(params->pBsBuffer->pBase);
                uint8_t *data = (uint8_t*)(params->pBsBuffer->pBase + params->dwOffset);
                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, params->pBatchBufferForPakSlices, data, byteSize));
            }
        }

        return eStatus;
    }


    MOS_STATUS AddHcpHevcPicBrcBuffer(
        PMOS_RESOURCE                   hcpImgStates,
        PMHW_VDBOX_HEVC_PIC_STATE        hevcPicState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(hcpImgStates);

        MOS_COMMAND_BUFFER constructedCmdBuf;
        typename THcpCmds::HCP_PIC_STATE_CMD  cmd;
        uint32_t* insertion = nullptr;
        MOS_LOCK_PARAMS lockFlags;
        this->m_brcNumPakPasses = hevcPicState->brcNumPakPasses;

        MOS_ZeroMemory(&lockFlags, sizeof(MOS_LOCK_PARAMS));
        lockFlags.WriteOnly = 1;
        uint8_t *data = (uint8_t*)this->m_osInterface->pfnLockResource(this->m_osInterface, hcpImgStates, &lockFlags);
        MHW_MI_CHK_NULL(data);

        constructedCmdBuf.pCmdBase      = (uint32_t *)data;
        constructedCmdBuf.pCmdPtr       = (uint32_t *)data;
        constructedCmdBuf.iOffset       = 0;
        constructedCmdBuf.iRemaining    = BRC_IMG_STATE_SIZE_PER_PASS * (this->m_brcNumPakPasses);

        MHW_MI_CHK_STATUS(this->AddHcpPicStateCmd(&constructedCmdBuf, hevcPicState));

        cmd = *(typename THcpCmds::HCP_PIC_STATE_CMD *)data;

        for (uint32_t i = 0; i < this->m_brcNumPakPasses; i++)
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

            *(typename THcpCmds::HCP_PIC_STATE_CMD *)data = cmd;

            /* add batch buffer end insertion flag */
            insertion = (uint32_t*)(data + THcpCmds::HCP_PIC_STATE_CMD::byteSize);
            *insertion = 0x05000000;

            data += BRC_IMG_STATE_SIZE_PER_PASS;
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnUnlockResource(this->m_osInterface, hcpImgStates));

        return eStatus;
    }

    MOS_STATUS GetOsResLaceOrAceOrRgbHistogramBufferSize(
        uint32_t                        width,
        uint32_t                        height,
        uint32_t                       *size)
    {
        MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

        *size = this->m_veboxRgbHistogramSize;

        uint32_t sizeLace = MOS_ROUNDUP_DIVIDE(height, 64) *
            MOS_ROUNDUP_DIVIDE(width, 64)  *
            this->m_veboxLaceHistogram256BinPerBlock;

        uint32_t sizeNoLace = m_veboxAceHistogramSizePerFramePerSlice *
            this->m_veboxNumFramePreviousCurrent                   *
            this->m_veboxMaxSlices;

        *size += MOS_MAX(sizeLace, sizeNoLace);

        return eStatus;
    }

    MOS_STATUS GetOsResStatisticsOutputBufferSize(
        uint32_t                        width,
        uint32_t                        height,
        uint32_t                       *size)
    {
        MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

        width  = MOS_ALIGN_CEIL(width, 64);
        height = MOS_ROUNDUP_DIVIDE(height, 4) + MOS_ROUNDUP_DIVIDE(this->m_veboxStatisticsSize * sizeof(uint32_t), width);
        *size   = width * height;

        return eStatus;
    }

public:
    inline uint32_t GetHcpHevcVp9RdoqStateCommandSize()
    {
        return THcpCmds::HEVC_VP9_RDOQ_STATE_CMD::byteSize;
    }
};

#endif
