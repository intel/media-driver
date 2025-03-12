/*
* Copyright (c) 2009-2023, Intel Corporation
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
//! \file      mos_os.c 
//! \brief 
//!
//!
//! \file     mos_os.c
//! \brief    Unified OS Formats
//! \details  Unified OS Formats
//!

#include "mos_os.h"
#include "mos_util_debug.h"
#include "mos_interface.h"
#include "media_user_setting.h"

#if MOS_MEDIASOLO_SUPPORTED
void *   _MOS_INTERFACE::pvSoloContext = nullptr; 
uint32_t _MOS_INTERFACE::soloRefCnt = 0;
#endif  // MOS_MEDIASOLO_SUPPORTED

extern MhwCpInterface* Create_MhwCpInterface(PMOS_INTERFACE osInterface);
extern void Delete_MhwCpInterface(MhwCpInterface* mhwInterface);

#if !EMUL
extern CodechalSecureDecodeInterface* Create_SecureDecodeInterface(CodechalSetting* codechalSettings, CodechalHwInterface* hwInterfaceInput);
extern void Delete_SecureDecodeInterface(CodechalSecureDecodeInterface* codechalSecureDecodeInterface);
#endif

static constexpr GMM_RESOURCE_USAGE_TYPE GmmResourceUsage[MOS_HW_RESOURCE_DEF_MAX] =
    {
        //
        // CODEC USAGES
        //
        GMM_RESOURCE_USAGE_BEGIN_CODEC,
        GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC,
        GMM_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC_PARTIALENCSURFACE,
        GMM_RESOURCE_USAGE_POST_DEBLOCKING_CODEC,
        GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_ENCODE,
        GMM_RESOURCE_USAGE_ORIGINAL_UNCOMPRESSED_PICTURE_DECODE,
        GMM_RESOURCE_USAGE_STREAMOUT_DATA_CODEC,
        GMM_RESOURCE_USAGE_INTRA_ROWSTORE_SCRATCH_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC,
        GMM_RESOURCE_USAGE_MACROBLOCK_STATUS_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE,
        GMM_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC,
        GMM_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE,
        GMM_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC,
        GMM_RESOURCE_USAGE_BSDMPC_ROWSTORE_SCRATCH_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_MPR_ROWSTORE_SCRATCH_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_BITPLANE_READ_CODEC,
        GMM_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_SURFACE_CURR_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_REF_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_MV_DATA_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_FF,
        GMM_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE_DST,
        GMM_RESOURCE_USAGE_SURFACE_ME_DISTORTION_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_BRC_ME_DISTORTION_ENCODE,
        GMM_RESOURCE_USAGE_PAK_OBJECT_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_FLATNESS_CHECK_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_MBENC_CURBE_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_MAD_ENCODE,
        GMM_RESOURCE_USAGE_VP8_BLOCK_MODE_COST_ENCODE,
        GMM_RESOURCE_USAGE_VP8_MB_MODE_COST_ENCODE,
        GMM_RESOURCE_USAGE_VP8_MBENC_OUTPUT_ENCODE,
        GMM_RESOURCE_USAGE_VP8_HISTOGRAM_ENCODE,
        GMM_RESOURCE_USAGE_VP8_L3_LLC_ENCODE,
        GMM_RESOURCE_USAGE_MFX_STANDALONE_DEBLOCKING_CODEC,
        GMM_RESOURCE_USAGE_HCP_MD_CODEC,
        GMM_RESOURCE_USAGE_HCP_SAO_CODEC,
        GMM_RESOURCE_USAGE_HCP_MV_CODEC,
        GMM_RESOURCE_USAGE_HCP_STATUS_ERROR_CODEC,
        GMM_RESOURCE_USAGE_HCP_LCU_ILDB_STREAMOUT_CODEC,
        GMM_RESOURCE_USAGE_VP9_PROBABILITY_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_VP9_SEGMENT_ID_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_VP9_HVD_ROWSTORE_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_MBDISABLE_SKIPMAP_CODEC,
        GMM_RESOURCE_USAGE_VDENC_ROW_STORE_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_VDENC_STREAMIN_CODEC,
        GMM_RESOURCE_USAGE_SURFACE_MB_QP_CODEC,
        GMM_RESOURCE_USAGE_MACROBLOCK_ILDB_STREAM_OUT_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_SSE_SRC_PIXEL_ROW_STORE_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_SLICE_STATE_STREAM_OUT_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_CABAC_SYNTAX_STREAM_OUT_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_PRED_COL_STORE_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_SURFACE_PAK_IMAGESTATE_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_MBENC_BRC_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_MB_BRC_CONST_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_BRC_MB_QP_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_BRC_ROI_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_SLICE_MAP_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_WP_DOWNSAMPLED_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_VDENC_IMAGESTATE_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_UNCACHED,
        GMM_RESOURCE_USAGE_SURFACE_ELLC_ONLY,
        GMM_RESOURCE_USAGE_SURFACE_ELLC_LLC_ONLY,
        GMM_RESOURCE_USAGE_SURFACE_ELLC_LLC_L3,
        GMM_RESOURCE_USAGE_SURFACE_BRC_HISTORY_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_SOFTWARE_SCOREBOARD_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ME_MV_DATA_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_MV_DISTORTION_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_4XME_DISTORTION_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_INTRA_DISTORTION_ENCODE,
        GMM_RESOURCE_USAGE_MB_STATS_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_PAK_STATS_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_PIC_STATE_READ_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_PIC_STATE_WRITE_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_COMBINED_ENC_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_BRC_CONSTANT_DATA_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_INTERMEDIATE_CU_RECORD_SURFACE_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_SCRATCH_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_LCU_LEVEL_DATA_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_HISTORY_INPUT_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_HISTORY_OUTPUT_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_DEBUG_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_CONSTANT_TABLE_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_CU_RECORD_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_MV_TEMPORAL_BUFFER_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_CU_PACKET_FOR_PAK_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_BCOMBINED1_ENCODE,
        GMM_RESOURCE_USAGE_SURFACE_ENC_BCOMBINED2_ENCODE,
        GMM_RESOURCE_USAGE_FRAME_STATS_STREAMOUT_DATA_CODEC,
        GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_LINE_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_TILE_COLUMN_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_HCP_MD_TILE_LINE_CODEC,
        GMM_RESOURCE_USAGE_HCP_MD_TILE_COLUMN_CODEC,
        GMM_RESOURCE_USAGE_HCP_SAO_TILE_LINE_CODEC,
        GMM_RESOURCE_USAGE_HCP_SAO_TILE_COLUMN_CODEC,
        GMM_RESOURCE_USAGE_VP9_PROBABILITY_COUNTER_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_HUC_VIRTUAL_ADDR_REGION_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_SIZE_STREAMOUT_CODEC,
        GMM_RESOURCE_USAGE_COMPRESSED_HEADER_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_PROBABILITY_DELTA_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_TILE_RECORD_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_TILE_SIZE_STAS_BUFFER_CODEC,
        GMM_RESOURCE_USAGE_END_CODEC,

        //
        // CM USAGES
        //
        CM_RESOURCE_USAGE_SurfaceState,
        CM_RESOURCE_USAGE_StateHeap,
        CM_RESOURCE_USAGE_NO_L3_SurfaceState,
        CM_RESOURCE_USAGE_NO_LLC_ELLC_SurfaceState,
        CM_RESOURCE_USAGE_NO_LLC_SurfaceState,
        CM_RESOURCE_USAGE_NO_ELLC_SurfaceState,
        CM_RESOURCE_USAGE_NO_LLC_L3_SurfaceState,
        CM_RESOURCE_USAGE_NO_ELLC_L3_SurfaceState,
        CM_RESOURCE_USAGE_NO_CACHE_SurfaceState,
        CM_RESOURCE_USAGE_L1_Enabled_SurfaceState,

        //
        // MP USAGES
        //
        MP_RESOURCE_USAGE_BEGIN,
        MP_RESOURCE_USAGE_DEFAULT,
        MP_RESOURCE_USAGE_DEFAULT_FF,
        MP_RESOURCE_USAGE_DEFAULT_RCS,
        MP_RESOURCE_USAGE_SurfaceState,
        MP_RESOURCE_USAGE_SurfaceState_FF,
        MP_RESOURCE_USAGE_SurfaceState_RCS,
        MP_RESOURCE_USAGE_AGE3_SurfaceState,
        MP_RESOURCE_USAGE_EDRAM_SurfaceState,
        MP_RESOURCE_USAGE_EDRAM_AGE3_SurfaceState,
        MP_RESOURCE_USAGE_No_L3_SurfaceState,
        MP_RESOURCE_USAGE_No_LLC_L3_SurfaceState,
        MP_RESOURCE_USAGE_No_LLC_L3_AGE_SurfaceState,
        MP_RESOURCE_USAGE_No_LLC_eLLC_L3_AGE_SurfaceState,
        MP_RESOURCE_USAGE_PartialEnc_No_LLC_L3_AGE_SurfaceState,
        MP_RESOURCE_USAGE_END,

        // MHW - SFC
        MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface,                    //!< SFC output surface
        MHW_RESOURCE_USAGE_Sfc_CurrentOutputSurface_PartialEncSurface,  //!< SFC output surface for partial secure surfaces
        MHW_RESOURCE_USAGE_Sfc_AvsLineBufferSurface,                    //!< SFC AVS Line buffer Surface
        MHW_RESOURCE_USAGE_Sfc_IefLineBufferSurface,                    //!< SFC IEF Line buffer Surface

        // Camera  caputure.
        GMM_RESOURCE_USAGE_CAMERA_CAPTURE,
        // Media GMM Resource USAGES
        GMM_RESOURCE_USAGE_CCS_MEDIA_WRITABLE,

        // Media BLT copy USAGES
        GMM_RESOURCE_USAGE_BLT_SOURCE,
        GMM_RESOURCE_USAGE_BLT_DESTINATION,

        // PAT Media Usages
        GMM_RESOURCE_USAGE_MEDIA_BATCH_BUFFERS,
        // DECODE
        GMM_RESOURCE_USAGE_DECODE_INPUT_BITSTREAM,
        GMM_RESOURCE_USAGE_DECODE_INPUT_REFERENCE,
        GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ,
        GMM_RESOURCE_USAGE_DECODE_INTERNAL_WRITE,
        GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_CACHE,
        GMM_RESOURCE_USAGE_DECODE_INTERNAL_READ_WRITE_NOCACHE,
        GMM_RESOURCE_USAGE_DECODE_OUTPUT_PICTURE,
        GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_WRITE,
        GMM_RESOURCE_USAGE_DECODE_OUTPUT_STATISTICS_READ_WRITE,
        // ENCODE
        GMM_RESOURCE_USAGE_ENCODE_INPUT_RAW,
        GMM_RESOURCE_USAGE_ENCODE_INPUT_RECON,
        GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ,
        GMM_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE,
        GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE,
        GMM_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE,
        GMM_RESOURCE_USAGE_ENCODE_EXTERNAL_READ,
        GMM_RESOURCE_USAGE_ENCODE_OUTPUT_PICTURE,
        GMM_RESOURCE_USAGE_ENCODE_OUTPUT_BITSTREAM,
        GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_WRITE,
        GMM_RESOURCE_USAGE_ENCODE_OUTPUT_STATISTICS_READ_WRITE,
        // VP
        GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_FF,
        GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_FF,
        GMM_RESOURCE_USAGE_VP_INTERNAL_READ_FF,
        GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_FF,
        GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_FF,
        GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF,
        GMM_RESOURCE_USAGE_VP_INPUT_PICTURE_RENDER,
        GMM_RESOURCE_USAGE_VP_INPUT_REFERENCE_RENDER,
        GMM_RESOURCE_USAGE_VP_INTERNAL_READ_RENDER,
        GMM_RESOURCE_USAGE_VP_INTERNAL_WRITE_RENDER,
        GMM_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER,
        GMM_RESOURCE_USAGE_VP_OUTPUT_PICTURE_RENDER,
        // CP
        GMM_RESOURCE_USAGE_CP_EXTERNAL_READ,
        GMM_RESOURCE_USAGE_CP_INTERNAL_WRITE,
};

//! \brief    Unified OS add command to command buffer
//! \details  Offset returned is dword aligned but size requested can be byte aligned
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in/out] Pointer to Command Buffer
//! \param    void  *pCmd
//!           [in] Command Pointer
//! \param    uint32_t dwCmdSize
//!           [in] Size of command in bytes
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_AddCommand(
    PMOS_COMMAND_BUFFER     pCmdBuffer,
    const void              *pCmd,
    uint32_t                dwCmdSize)
{
    return MosInterface::AddCommand(pCmdBuffer, pCmd, dwCmdSize);
}

//!
//! \brief    Unified OS fill Resource
//! \details  Locks the surface and fills the resource with data
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \param    uint32_t dwSize
//!           [in] Size of the Buffer
//! \param    uint8_t iValue
//!           [in] Value to be filled
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_OsFillResource(
    PMOS_INTERFACE    pOsInterface,
    PMOS_RESOURCE     pOsResource,
    uint32_t          dwSize,
    uint8_t           iValue)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pOsResource);
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    uint8_t *       pByte = nullptr;
    MOS_LOCK_PARAMS lockFlags   = {};
    uint32_t        size = 0;
#ifndef VPSOLO_EMUL
    if (pOsResource->pGmmResInfo)
    {
        size = (uint32_t)pOsResource->pGmmResInfo->GetSizeSurface();
    }
    if (dwSize > size)
    {
        MOS_OS_ASSERTMESSAGE("dwSize (%x)> size (%x)", dwSize, size);
    }
#endif
    // Lock the surface for writing
    lockFlags.WriteOnly = 1;
    pByte = (uint8_t*)pOsInterface->pfnLockResource(
                        pOsInterface,
                        pOsResource,
                        &lockFlags);

    MOS_OS_CHK_NULL_RETURN(pByte);

    MOS_FillMemory(pByte, dwSize, iValue);

    // Unlock the surface
    MOS_OS_CHK_STATUS_RETURN(pOsInterface->pfnUnlockResource(pOsInterface, pOsResource));

    return eStatus;
}

//! \brief    Unified OS Get bits per pixel
//! \details  Gets Bits Per Pixel
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \param    uint32_t *piBpp
//!           [out] Bits per pixel
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_OsGetBitsPerPixel(
    PMOS_INTERFACE            pOsInterface,
    MOS_FORMAT                Format,
    uint32_t                  *piBpp)
{
    MOS_STATUS eStatus;
    MOS_UNUSED(pOsInterface);

    eStatus = MOS_STATUS_SUCCESS;

    switch(Format)
    {
    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
    case Format_AYUV:
    case Format_R32U:
    case Format_R32S:
    case Format_R32F:
    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        *piBpp = 32;
        break;

    case Format_P010:
    case Format_P016:
    case Format_R8G8B8:
        *piBpp = 24;
        break;

    case Format_R5G6B5:
    case Format_A8P8:
    case Format_A8L8:
    case Format_R16U:
    case Format_V8U8:
    case Format_R16F:
        *piBpp = 16;
        break;

    case Format_YUY2:
    case Format_YUYV:
    case Format_YVYU:
    case Format_UYVY:
    case Format_VYUY:
        *piBpp = 16;
        break;

    case Format_AI44:
    case Format_IA44:
    case Format_A8:
    case Format_P8:
    case Format_L8:
    case Format_A4L4:
        *piBpp = 8;
        break;

    case Format_IMC1:
    case Format_IMC3:
    case Format_IMC2:
    case Format_IMC4:
    case Format_NV12:
    case Format_YV12:
    case Format_I420:
    case Format_IYUV:
        *piBpp = 12;
        break;

    case Format_400P:
        *piBpp = 8;
        break;

    case Format_411P:
        *piBpp = 12;
        break;

    case Format_422H:
    case Format_422V:
        *piBpp = 16;
        break;

    case Format_444P:
    case Format_RGBP:
    case Format_BGRP:
        *piBpp = 24;
        break;

    case Format_YVU9:
        *piBpp = 9;
        break;

    case Format_Buffer:
    case Format_RAW:
    case Format_R8U:
    case Format_R8UN:
       *piBpp = 8;
        break;

    default:
        MOS_OS_ASSERTMESSAGE("Format '%d' not supported.", Format);
        *piBpp = 0;
        eStatus = MOS_STATUS_UNKNOWN;
        break;
    }

    return eStatus;
}

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
//!
//! \brief    Get Platform Name
//! \details  This function retrieve the platform name 
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \param    buffer 
//!           [out] Buffer to contain platform name
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_GetPlatformName(
    PMOS_INTERFACE pOsInterface,
    char           *buffer)
{
    MOS_SecureStrcpy(buffer, MOS_COMMAND_BUFFER_PLATFORM_LEN, "N/A");

    return MOS_STATUS_SUCCESS;
}

void Mos_AddIndirectState(
    PMOS_INTERFACE      pOsInterface,
    uint32_t            stateSize,
    uint32_t            *pIndirectState,
    uint32_t            *gfxAddressBottom,
    uint32_t            *gfxAddressTop,
    const char          *stateName)
{
    MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface);
    if (pOsInterface->apoMosEnabled)
    {
        MOS_OS_CHK_NULL_NO_STATUS_RETURN(pOsInterface->osStreamState);
        INDIRECT_STATE_INFO stateinfoarray = {};
        stateinfoarray.stateSize           = stateSize;
        stateinfoarray.indirectState       = pIndirectState;
        stateinfoarray.gfxAddressBottom    = gfxAddressBottom;
        stateinfoarray.gfxAddressTop       = gfxAddressTop;
        stateinfoarray.stateName           = stateName;
        pOsInterface->osStreamState->indirectStateInfo.push_back(stateinfoarray);
    }
    return;
}

//!
//! \brief    Dump command buffers
//! \details  This function dumps the command buffer just before rendering it to the GPU.
//!           The output is saved to %TEMP%\Command_buffer_dumps.
//!           A GPU engine prefix is printed for each command buffer, to let the user know which command streamer is being used.
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to command buffer structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_DumpCommandBuffer(
    PMOS_INTERFACE      pOsInterface,
    PMOS_COMMAND_BUFFER pCmdBuffer)
{
    static uint32_t dwCommandBufferNumber   = 0;
    MOS_STATUS      eStatus                 = MOS_STATUS_UNKNOWN;
    char            *pOutputBuffer          = nullptr;
    // Each hex value should have 9 chars.
    uint32_t        SIZE_OF_ONE_WORD        = 9;
    uint32_t        dwBytesWritten          = 0;
    uint32_t        dwNumberOfDwords        = 0;
    uint32_t        dwSizeToAllocate        = 0;
    char            sFileName[MOS_MAX_HLT_FILENAME_LEN];
    // Maximum length of engine name is 6
    char            sEngName[6];
    char            *psFileNameAfterPrefix  = nullptr;
    size_t          nSizeFileNamePrefix     = 0;

    MOS_OS_CHK_NULL_RETURN(pOsInterface);
    MOS_OS_CHK_NULL_RETURN(pCmdBuffer);

    if (pOsInterface->apoMosEnabled)
    {
        return MosInterface::DumpCommandBuffer(pOsInterface->osStreamState, pCmdBuffer);
    }

    // Set the name of the engine that is going to be used.
    MOS_GPU_CONTEXT sGpuContext = pOsInterface->pfnGetGpuContext(pOsInterface);
    switch (sGpuContext)
    {
        case MOS_GPU_CONTEXT_VIDEO:
        case MOS_GPU_CONTEXT_VIDEO2:
        case MOS_GPU_CONTEXT_VIDEO3:
        case MOS_GPU_CONTEXT_VIDEO4:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO2:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO3:
        case MOS_GPU_CONTEXT_VIDEO5:
        case MOS_GPU_CONTEXT_VIDEO6:
        case MOS_GPU_CONTEXT_VIDEO7:
            MOS_SecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_VIDEO_ENGINE);
            break;
        case MOS_GPU_CONTEXT_TEE:
            MOS_SecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_TEE_ENGINE);
            break;
        case MOS_GPU_CONTEXT_RENDER:
        case MOS_GPU_CONTEXT_RENDER2:
        case MOS_GPU_CONTEXT_RENDER3:
        case MOS_GPU_CONTEXT_RENDER4:
        case MOS_GPU_CONTEXT_RENDER_RA:
        case MOS_GPU_CONTEXT_COMPUTE:
        case MOS_GPU_CONTEXT_COMPUTE_RA:
        case MOS_GPU_CONTEXT_CM_COMPUTE:
            MOS_SecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_RENDER_ENGINE);
            break;
        case MOS_GPU_CONTEXT_VEBOX:
            MOS_SecureStrcpy(sEngName, sizeof(sEngName), MOS_COMMAND_BUFFER_VEBOX_ENGINE);
            break;
        default:
            MOS_OS_ASSERTMESSAGE("Unsupported GPU context.");
            return MOS_STATUS_UNKNOWN;
    }

    dwNumberOfDwords = pCmdBuffer->iOffset / sizeof(uint32_t);

    dwSizeToAllocate =
        dwNumberOfDwords * (SIZE_OF_ONE_WORD + 1) // Add 1 byte for the space following each Dword.
        +        3       *  SIZE_OF_ONE_WORD;     // For engine and platform names.

    // Alloc output buffer.
    pOutputBuffer = (char *)MOS_AllocAndZeroMemory(dwSizeToAllocate);
    MOS_OS_CHK_NULL_RETURN(pOutputBuffer);

    dwBytesWritten = MOS_SecureStringPrint(
                         pOutputBuffer,
                         SIZE_OF_ONE_WORD * 3,
                         SIZE_OF_ONE_WORD * 3,
                         "Eng=%s Plat=%s ",
                         sEngName, pOsInterface->sPlatformName);

    if (pOsInterface->bDumpCommandBufferToFile)
    {
        // Set the file name.
        memcpy(sFileName, pOsInterface->sDirName, MOS_MAX_HLT_FILENAME_LEN);
        nSizeFileNamePrefix = strnlen(sFileName, sizeof(sFileName));
        MOS_SecureStringPrint(
            sFileName + nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            "%c%s%c%s_%d.txt",
            MOS_DIR_SEPERATOR, MOS_COMMAND_BUFFER_OUT_DIR,
            MOS_DIR_SEPERATOR, MOS_COMMAND_BUFFER_OUT_FILE, dwCommandBufferNumber);

        // Write the output buffer to file.
        MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosWriteFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
    }

    if (pOsInterface->bDumpCommandBufferAsMessages)
    {
        MOS_OS_NORMALMESSAGE(pOutputBuffer);
    }

    MOS_ZeroMemory(pOutputBuffer, dwBytesWritten);
    dwBytesWritten = 0;

    // Fill in the output buffer with the command buffer dwords.
    for (uint32_t dwIndex = 0; dwIndex < dwNumberOfDwords; dwIndex++)
    {
        dwBytesWritten += MOS_SecureStringPrint(
            pOutputBuffer + dwBytesWritten,
            SIZE_OF_ONE_WORD + 1,
            SIZE_OF_ONE_WORD + 1,
            "%.8x ",
            pCmdBuffer->pCmdBase[dwIndex]);

        if (dwBytesWritten % (SIZE_OF_ONE_WORD + 1) == 0)
        {
            if (pOsInterface->bDumpCommandBufferToFile)
            {
                MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
            }
            if (pOsInterface->bDumpCommandBufferAsMessages)
            {
                MOS_OS_NORMALMESSAGE(pOutputBuffer);
            }

            MOS_ZeroMemory(pOutputBuffer, dwBytesWritten);
            dwBytesWritten = 0;
        }
    }

    if (pOsInterface->bDumpCommandBufferToFile)
    {
        MOS_OS_CHK_STATUS_RETURN(MosUtilities::MosAppendFileFromPtr((const char *)sFileName, pOutputBuffer, dwBytesWritten));
    }

    if (pOsInterface->bDumpCommandBufferAsMessages)
    {
        MOS_OS_NORMALMESSAGE(pOutputBuffer);
    }

    dwCommandBufferNumber++;

    eStatus = MOS_STATUS_SUCCESS;

    // Free the memory.
    if (pOutputBuffer)
    {
        MOS_FreeMemAndSetNull(pOutputBuffer);
    }

    return eStatus;
}

//! \brief    Unified dump command buffer initialization
//! \details  check if dump command buffer was enabled and create the output directory
//! \param    PMOS_INTERFACE pOsInterface
//!           [in/out] Pointer to OS Interface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_DumpCommandBufferInit(
    PMOS_INTERFACE pOsInterface)
{
    char                                sFileName[MOS_MAX_HLT_FILENAME_LEN] = {};
    MOS_STATUS                          eStatus                             = MOS_STATUS_UNKNOWN;
    size_t                              nSizeFileNamePrefix                 = 0;
    MediaUserSettingSharedPtr           userSettingPtr                      = nullptr;
    uint32_t                            value                               = 0;
    MOS_OS_CHK_NULL_RETURN(pOsInterface);

    userSettingPtr = pOsInterface->pfnGetUserSettingInstance(pOsInterface);

    // Setup member function and variable.
    pOsInterface->pfnDumpCommandBuffer  = Mos_DumpCommandBuffer;
    pOsInterface->pfnAddIndirectState   = Mos_AddIndirectState;

    // Check if command buffer dump was enabled in user feature.
    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_BUFFER_ENABLE,
        MediaUserSetting::Group::Device);

    pOsInterface->bDumpCommandBuffer            = (value != 0);
    pOsInterface->bDumpCommandBufferToFile      = ((value & 1) != 0);
    pOsInterface->bDumpCommandBufferAsMessages  = ((value & 2) != 0);

    if (pOsInterface->bDumpCommandBufferToFile)
    {
        // Create output directory.
        eStatus = MosUtilDebug::MosLogFileNamePrefix(pOsInterface->sDirName, userSettingPtr);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create log file prefix. Status = %d", eStatus);
            return eStatus;
        }

        memcpy(sFileName, pOsInterface->sDirName, MOS_MAX_HLT_FILENAME_LEN);
        nSizeFileNamePrefix = strnlen(sFileName, sizeof(sFileName));
        MOS_SecureStringPrint(
            sFileName + nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            sizeof(sFileName) - nSizeFileNamePrefix,
            "%c%s",
            MOS_DIR_SEPERATOR, MOS_COMMAND_BUFFER_OUT_DIR);

        eStatus = MosUtilities::MosCreateDirectory(sFileName);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            MOS_OS_NORMALMESSAGE("Failed to create output directory. Status = %d", eStatus);
            return eStatus;
        }
    }

    Mos_GetPlatformName(pOsInterface, pOsInterface->sPlatformName);

    return MOS_STATUS_SUCCESS;
}
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED

std::shared_ptr<GpuCmdResInfoDump> GpuCmdResInfoDump::m_instance = nullptr;

const GpuCmdResInfoDump *GpuCmdResInfoDump::GetInstance(PMOS_CONTEXT mosCtx)
{
    if (m_instance == nullptr)
    {
        m_instance = std::make_shared<GpuCmdResInfoDump>(mosCtx);
    }
    return m_instance.get();
}

GpuCmdResInfoDump::GpuCmdResInfoDump(PMOS_CONTEXT mosCtx)
{
    MediaUserSettingSharedPtr   userSettingPtr  = nullptr;
    MediaUserSetting::Value     value;

    userSettingPtr = MosInterface::MosGetUserSettingInstance(mosCtx);

    ReadUserSetting(
        userSettingPtr,
        m_dumpEnabled,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_ENABLE,
        MediaUserSetting::Group::Device);

    if (!m_dumpEnabled)
    {
        return;
    }

    ReadUserSetting(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_DUMP_COMMAND_INFO_PATH,
        MediaUserSetting::Group::Device);

    auto path = value.ConstString();
    if(path.size() > 0)
    {
        m_path = path;
        if (path.back() != '/' && path.back() != '\\')
        {
            m_path += '/';
        }
    }
    m_path = m_path + "gpuCmdResInfo_" + std::to_string(MosUtilities::MosGetPid()) + ".txt";
}

void GpuCmdResInfoDump::Dump(PMOS_INTERFACE pOsInterface) const
{
    if (!m_dumpEnabled)
    {
        return;
    }

    using std::endl;

    std::ofstream outputFile;
    outputFile.open(m_path, std::ios_base::app);
    MOS_OS_ASSERT(outputFile.is_open());

    auto &cmdResInfoPtrs = GetCmdResPtrs(pOsInterface);

    outputFile << "--PerfTag: " << std::to_string(pOsInterface->pfnGetPerfTag(pOsInterface)) << " --Cmd Num: "
        << cmdResInfoPtrs.size() << " --Dump Count: " << ++m_cnt << endl;

    outputFile << "********************************CMD Paket Begin********************************" << endl;
    for (auto e : cmdResInfoPtrs)
    {
        Dump(e, outputFile);
    }
    outputFile << "********************************CMD Paket End**********************************" << endl << endl;

    outputFile.close();
}

const char *GpuCmdResInfoDump::GetResType(MOS_GFXRES_TYPE resType) const
{
    switch (resType)
    {
    case MOS_GFXRES_INVALID:
        return "MOS_GFXRES_INVALID";
    case MOS_GFXRES_BUFFER:
        return "MOS_GFXRES_BUFFER";
    case MOS_GFXRES_2D:
        return "MOS_GFXRES_2D";
    case MOS_GFXRES_VOLUME:
        return "MOS_GFXRES_VOLUME";
    default:
        return "";
    }
}

const char *GpuCmdResInfoDump::GetTileType(MOS_TILE_TYPE tileType) const
{
    switch (tileType)
    {
    case MOS_TILE_X:
        return "MOS_TILE_X";
    case MOS_TILE_Y:
        return "MOS_TILE_Y";
    case MOS_TILE_YF:
        return "MOS_TILE_YF";
    case MOS_TILE_YS:
        return "MOS_TILE_YS";
    case MOS_TILE_LINEAR:
        return "MOS_TILE_LINEAR";
    case MOS_TILE_INVALID:
        return "MOS_TILE_INVALID";
    default:
        return "";
    }
}
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

//! \brief    Unified OS Initializes OS Interface
//! \details  OS Interface initilization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in/out] Pointer to OS Interface
//! \param    MOS_CONTEXT_HANDLE osDriverContext
//!           [in] Pointer to Driver context
//! \param    MOS_COMPONENT component
//!           [in] OS component
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_InitOsInterface(
    PMOS_INTERFACE     pOsInterface,
    MOS_CONTEXT_HANDLE osDriverContext,
    MOS_COMPONENT      component)
{
    MOS_OS_CHK_NULL_RETURN(pOsInterface);
#if !EMUL
    MOS_OS_CHK_NULL_RETURN(osDriverContext);
#endif
    MOS_STATUS                  eStatus = MOS_STATUS_UNKNOWN;
    MediaUserSettingSharedPtr   userSettingPtr = nullptr;
    PMOS_CONTEXT                pOsDriverContext = (PMOS_CONTEXT)osDriverContext;
    // Setup Member functions
    pOsInterface->pfnFillResource                       = Mos_OsFillResource;
    pOsInterface->pfnGetBitsPerPixel                    = Mos_OsGetBitsPerPixel;
    pOsInterface->pfnAddCommand                         = Mos_AddCommand;
    pOsInterface->pfnVirtualEngineSupported             = Mos_CheckVirtualEngineSupported;
    pOsInterface->pfnGetResourceCachePolicyMemoryObject = Mos_GetResourceCachePolicyMemoryObject;

    pOsInterface->pfnIsAsyncDevice                      = Mos_IsAsyncDevice;
    pOsInterface->pfnMosFmtToOsFmt                      = Mos_MosFmtToOsFmt;
    pOsInterface->pfnOsFmtToMosFmt                      = Mos_OsFmtToMosFmt;
    pOsInterface->pfnMosFmtToGmmFmt                     = Mos_MosFmtToGmmFmt;
    pOsInterface->pfnGmmFmtToMosFmt                     = Mos_GmmFmtToMosFmt;
    pOsInterface->pfnWaitForCmdCompletion               = Mos_WaitForCmdCompletion;
    pOsInterface->pfnGetResourceArrayIndex              = Mos_GetResourceArrayIndex;
    pOsInterface->pfnSetupAttributeVeBuffer             = Mos_SetupAttributeVeBuffer;
    pOsInterface->pfnGetAttributeVeBuffer               = Mos_GetAttributeVeBuffer;
    pOsInterface->pfnSetupCurrentCmdListAndPool         = Mos_SetupCurrentCmdListAndPool;
    pOsInterface->pfnGmmToMosResourceUsageType          = Mos_GmmToMosResourceUsageType;
    pOsInterface->pfnGetAdapterInfo                     = Mos_GetAdapterInfo;
    pOsInterface->pfnIsCompressibelSurfaceSupported     = Mos_IsCompressibelSurfaceSupported;
    pOsInterface->pfnDestroyVirtualEngineState          = Mos_DestroyVirtualEngineState;
    pOsInterface->pfnGetResourceHandle                  = Mos_GetResourceHandle;
    pOsInterface->pfnGetRtLogResourceInfo               = Mos_GetRtLogResourceInfo;
    pOsInterface->pfnResetResource                      = Mos_ResetMosResource;
    pOsInterface->pfnVerifyMosSurface                   = Mos_VerifyMosSurface;

    pOsInterface->pfnCreateMhwCpInterface               = Create_MhwCpInterface;
    pOsInterface->pfnDeleteMhwCpInterface               = Delete_MhwCpInterface;
    pOsInterface->pfnInsertCacheSetting                 = Mos_InsertCacheSetting;
    pOsInterface->pfnGetCacheSetting                    = Mos_GetCacheSetting;
#if !EMUL
    pOsInterface->pfnCreateSecureDecodeInterface        = Create_SecureDecodeInterface;
    pOsInterface->pfnDeleteSecureDecodeInterface        = Delete_SecureDecodeInterface;
#endif
#if (_DEBUG || _RELEASE_INTERNAL)
    pOsInterface->pfnGetVeEngineCount                   = Mos_GetVeEngineCount;
    pOsInterface->pfnGetEngineLogicIdByIdx              = Mos_GetEngineLogicId;
    pOsInterface->pfnSetGpuVirtualAddress               = MOS_SetGpuVirtualAddress;
#endif

    pOsInterface->Component                 = component;
    pOsInterface->modulizedMosEnabled       = true;
    pOsInterface->osContextPtr              = nullptr;
    pOsInterface->veDefaultEnable           = true;

    pOsInterface->streamIndex = 0;
    pOsInterface->bSimIsActive = 0;

    eStatus = Mos_Specific_InitInterface(pOsInterface, pOsDriverContext);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        MOS_OS_ASSERTMESSAGE("Mos_Specific_InitInterface FAILED, errno = 0x%x", eStatus);
        return eStatus;
    }

    userSettingPtr = pOsInterface->pfnGetUserSettingInstance(pOsInterface);

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    Mos_DumpCommandBufferInit(pOsInterface);
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_DEBUG || _RELEASE_INTERNAL)
    // Report if pre-si environment is in use
    ReportUserSettingForDebug(
        userSettingPtr,
        __MEDIA_USER_FEATURE_VALUE_SIM_IN_USE,
        pOsInterface->bSimIsActive,
        MediaUserSetting::Group::Device);
#endif

    if (pOsInterface->apoMosEnabled || pOsInterface->apoMosForLegacyRuntime)
    {
        pOsInterface->osStreamState->osCpInterface = pOsInterface->osCpInterface;
    }

    return eStatus;
}

#if !EMUL
MEMORY_OBJECT_CONTROL_STATE Mos_CachePolicyGetMemoryObject(
    MOS_HW_RESOURCE_DEF MosUsage,
    GMM_CLIENT_CONTEXT  *pGmmClientContext)
{
    MOS_OS_ASSERT(pGmmClientContext);

    GMM_RESOURCE_USAGE_TYPE usage = GmmResourceUsage[MosUsage];
    if (pGmmClientContext->GetCachePolicyElement(usage).Initialized)
    {
        return pGmmClientContext->CachePolicyGetMemoryObject(nullptr, usage);
    }
    else
    {
        return pGmmClientContext->GetCachePolicyUsage()[GMM_RESOURCE_USAGE_UNKNOWN].MemoryObjectOverride;
    }
}
#endif

#ifndef SKIP_VE_DEFINE
MOS_STATUS Mos_CheckVirtualEngineSupported(
    PMOS_INTERFACE      osInterface,
    bool                isNotEncode,
    bool                veDefaultEnable)
{
    MOS_STATUS                eStatus        = MOS_STATUS_SUCCESS;
    MediaUserSettingSharedPtr userSettingPtr = nullptr;
    uint32_t                  value          = 0;

    MOS_OS_CHK_NULL_RETURN(osInterface);

    userSettingPtr = osInterface->pfnGetUserSettingInstance(osInterface);
    if (isNotEncode)
    {
        //UMD Decode Virtual Engine Override
        // 0: disable. can set to 1 only when KMD VE is enabled.
        // Default value is 1 if not set this key
        osInterface->bSupportVirtualEngine = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        ReadUserSettingForDebug(
            userSettingPtr,
            value,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_DECODE_VIRTUAL_ENGINE,
            MediaUserSetting::Group::Device);
        osInterface->bSupportVirtualEngine = value ? true : false;
#endif

        auto skuTable = osInterface->pfnGetSkuTable(osInterface);
        MOS_OS_CHK_NULL_RETURN(skuTable);
        if (osInterface->bSupportVirtualEngine && MEDIA_IS_SKU(skuTable, FtrContextBasedScheduling))
        {
            osInterface->ctxBasedScheduling = true;
        }
        else
        {
            osInterface->ctxBasedScheduling = false;
        }

        if (osInterface->pfnCheckVirtualEngineSupported)
        {
            osInterface->pfnCheckVirtualEngineSupported(osInterface);
        }

        osInterface->multiNodeScaling = osInterface->ctxBasedScheduling && MEDIA_IS_SKU(skuTable, FtrVcs2) ? true : false;

#if (_DEBUG || _RELEASE_INTERNAL)
        value = osInterface->ctxBasedScheduling ? true : false;
        ReportUserSettingForDebug(
            userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_DECODE_VE_CTXSCHEDULING,
            value,
            MediaUserSetting::Group::Device);
#endif
    }
    else
    {
        //UMD Encode Virtual Engine Override
        osInterface->bSupportVirtualEngine = true;
#if (_DEBUG || _RELEASE_INTERNAL)
        ReadUserSettingForDebug(
            userSettingPtr,
            value,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_ENCODE_VIRTUAL_ENGINE,
            MediaUserSetting::Group::Device);
        osInterface->bSupportVirtualEngine = value ? true : false;
#endif

        auto skuTable = osInterface->pfnGetSkuTable(osInterface);
        MOS_OS_CHK_NULL_RETURN(skuTable);
        if (osInterface->bSupportVirtualEngine && MEDIA_IS_SKU(skuTable, FtrContextBasedScheduling))
        {
            osInterface->ctxBasedScheduling = true;
        }
        else
        {
            osInterface->ctxBasedScheduling = false;
        }
        osInterface->multiNodeScaling = osInterface->ctxBasedScheduling && MEDIA_IS_SKU(skuTable, FtrVcs2) ? true : false;
    }

    if (osInterface->apoMosEnabled)
    {
        // Update ctx based scheduling flag also in APO MOS stream state
        MOS_OS_CHK_NULL_RETURN(osInterface->osStreamState);
        osInterface->osStreamState->ctxBasedScheduling = osInterface->ctxBasedScheduling;
        osInterface->osStreamState->multiNodeScaling   = osInterface->multiNodeScaling;
    }
    MOS_OS_VERBOSEMESSAGE("Virtual Engine Context based SCheduling enabled:%d.\n", osInterface->ctxBasedScheduling);
    MOS_OS_VERBOSEMESSAGE("Virtual Engine Multi-node Scaling enabled:%d.\n", osInterface->multiNodeScaling);

    return eStatus;
}
#endif // !SKIP_VE_DEFINE

MEMORY_OBJECT_CONTROL_STATE Mos_GetResourceCachePolicyMemoryObject(
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       resource)
{
    MEMORY_OBJECT_CONTROL_STATE memObjCtrlState = {};

    if(resource == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("resource == nullptr");
        return memObjCtrlState;
    }

    memObjCtrlState = resource->memObjCtrlState;
    if(osInterface == nullptr)
    {
        MOS_OS_ASSERTMESSAGE("osInterface == nullptr");
        return memObjCtrlState;
    }

    auto gmmClientContext = osInterface->pfnGetGmmClientContext(osInterface);

    if(resource->bConvertedFromDDIResource &&
       resource->mocsMosResUsageType >= MOS_HW_RESOURCE_USAGE_MEDIA_BATCH_BUFFERS)
    {
        memObjCtrlState = MosInterface::GetCachePolicyMemoryObject(gmmClientContext, resource->mocsMosResUsageType);
    }

    if (resource->mocsMosResUsageType >= (sizeof(GmmResourceUsage) / sizeof(GmmResourceUsage[0])))
    {
        MOS_OS_ASSERTMESSAGE("mocsMosResUsageType out of bound");
        resource->mocsMosResUsageType = MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC;
    }
    GMM_RESOURCE_USAGE_TYPE usage = GmmResourceUsage[resource->mocsMosResUsageType];
    if ((memObjCtrlState.DwordValue == 0)
#if !EMUL
        && !gmmClientContext->GetCachePolicyElement(usage).Initialized
#endif
    )
    {
        memObjCtrlState = MosInterface::GetDefaultCachePolicyMemoryObject(gmmClientContext);
    }
    return memObjCtrlState;
}

bool Mos_IsAsyncDevice(
    MOS_STREAM_HANDLE   streamState)
{
    return MosInterface::IsAsyncDevice(streamState);
}

uint32_t Mos_MosFmtToOsFmt(
    MOS_FORMAT          format)
{
    return MosInterface::MosFmtToOsFmt(format);
}

MOS_FORMAT Mos_OsFmtToMosFmt(
    uint32_t            format)
{
    return MosInterface::OsFmtToMosFmt(format);
}

GMM_RESOURCE_FORMAT Mos_MosFmtToGmmFmt(
    MOS_FORMAT          format)
{
    return MosInterface::MosFmtToGmmFmt(format);
}

MOS_FORMAT Mos_GmmFmtToMosFmt(
    GMM_RESOURCE_FORMAT format)
{
    return MosInterface::GmmFmtToMosFmt(format);
}

MOS_STATUS Mos_WaitForCmdCompletion(
    MOS_STREAM_HANDLE   streamState,
    GPU_CONTEXT_HANDLE  gpuCtx)
{
    return MosInterface::WaitForCmdCompletion(streamState, gpuCtx);
}

uint32_t Mos_GetResourceArrayIndex(
    PMOS_RESOURCE       resource)
{
    return MosInterface::GetResourceArrayIndex(resource);
}

MOS_STATUS Mos_SetupAttributeVeBuffer(
    MOS_STREAM_HANDLE       streamState,
    COMMAND_BUFFER_HANDLE   cmdBuffer)
{
    return MosInterface::SetupAttributeVeBuffer(streamState, cmdBuffer);
}

MOS_CMD_BUF_ATTRI_VE *Mos_GetAttributeVeBuffer(
    COMMAND_BUFFER_HANDLE   cmdBuffer)
{
    return MosInterface::GetAttributeVeBuffer(cmdBuffer);
}

MOS_STATUS Mos_SetupCurrentCmdListAndPool(
    PMOS_INTERFACE          osInterface,
    MOS_STREAM_HANDLE       streamState)
{
    return MosInterface::SetupCurrentCmdListAndPoolFromOsInterface(osInterface, streamState);
}

MOS_HW_RESOURCE_DEF Mos_GmmToMosResourceUsageType(
    GMM_RESOURCE_USAGE_TYPE gmmResUsage)
{
    return MosInterface::GmmToMosResourceUsageType(gmmResUsage);
}

ADAPTER_INFO *Mos_GetAdapterInfo(
    MOS_STREAM_HANDLE       streamState)
{
    return MosInterface::GetAdapterInfo(streamState);
}

bool Mos_IsCompressibelSurfaceSupported(
    MEDIA_FEATURE_TABLE     *skuTable)
{
    return MosInterface::IsCompressibelSurfaceSupported(skuTable);
}

MOS_STATUS Mos_DestroyVirtualEngineState(
    MOS_STREAM_HANDLE       streamState)
{
    return MosInterface::DestroyVirtualEngineState(streamState);
}

uint64_t Mos_GetResourceHandle(
    MOS_STREAM_HANDLE       streamState,
    PMOS_RESOURCE           osResource)
{
    return MosInterface::GetResourceHandle(streamState, osResource);
}

MOS_STATUS Mos_VerifyMosSurface(
    PMOS_SURFACE            mosSurface,
    bool&                   bIsValid)
{
    return MosInterface::VerifyMosSurface(mosSurface, bIsValid);
}

void Mos_GetRtLogResourceInfo(
    PMOS_INTERFACE          osInterface,
    PMOS_RESOURCE           &osResource,
    uint32_t                &size)
{
    return MosInterface::GetRtLogResourceInfo(osInterface, osResource, size);
}

void Mos_ResetMosResource(
    PMOS_RESOURCE           resource)
{
    return MosInterface::MosResetResource(resource);
}

bool Mos_InsertCacheSetting(CACHE_COMPONENTS id, std::map<uint64_t, MOS_CACHE_ELEMENT> *cacheTablesPtr)
{
    return RegisterCacheSettings(id, cacheTablesPtr);
}

bool Mos_GetCacheSetting(MOS_COMPONENT id, uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_CACHE_ELEMENT &element, bool isHeapSurf)
{
    return LoadCacheSettings(id, feature, bOut, engineType, element, isHeapSurf);
}

#if (_DEBUG || _RELEASE_INTERNAL)
uint8_t Mos_GetVeEngineCount(
    MOS_STREAM_HANDLE       streamState)
{
    return MosInterface::GetVeEngineCount(streamState);
}

uint8_t Mos_GetEngineLogicId(
    MOS_STREAM_HANDLE       streamState,
    uint32_t                instanceIdx)
{
    return MosInterface::GetEngineLogicId(streamState, instanceIdx);
}

MOS_STATUS MOS_SetGpuVirtualAddress(
    PMOS_RESOURCE          pResource,
    uint64_t               address)
{
    return MosInterface::SetGpuVirtualAddress(pResource, address);
}

#endif

void *MosStreamState::pvSoloContext = nullptr; 

