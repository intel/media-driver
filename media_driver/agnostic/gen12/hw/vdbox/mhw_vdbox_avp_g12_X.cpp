/*
* Copyright (c) 2020-2024, Intel Corporation
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
//! \file     mhw_vdbox_avp_g12_X.cpp
//! \brief    Constructs VdBox AVP commands on Gen12-based platforms

#include "mhw_vdbox_avp_g12_X.h"
#include "mhw_mi_hwcmd_g12_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g12_X.h"

MhwVdboxAvpInterfaceG12::~MhwVdboxAvpInterfaceG12()
{
    MHW_FUNCTION_ENTER;

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
    UserFeatureWriteData.ValueID                           = __MEDIA_USER_FEATURE_VALUE_IS_CODEC_ROW_STORE_CACHE_ENABLED_ID;
    if (m_btdlRowstoreCache.bEnabled ||
        m_smvlRowstoreCache.bEnabled ||
        m_ipdlRowstoreCache.bEnabled ||
        m_dflyRowstoreCache.bEnabled ||
        m_dfluRowstoreCache.bEnabled ||
        m_dflvRowstoreCache.bEnabled ||
        m_cdefRowstoreCache.bEnabled)
    {
        UserFeatureWriteData.Value.i32Data = 1;
    }
    MOS_UserFeature_WriteValues_ID(nullptr, &UserFeatureWriteData, 1, m_osInterface->pOsContext);
#endif

}

void MhwVdboxAvpInterfaceG12::InitRowstoreUserFeatureSettings()
{
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;

    memset(&userFeatureData, 0, sizeof(userFeatureData));
    if (m_osInterface->bSimIsActive)
    {
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
        m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
    m_rowstoreCachingSupported = userFeatureData.i32Data ? false : true;

    if (m_rowstoreCachingSupported)
    {
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1BTDLROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_btdlRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1SMVLROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_smvlRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1IPDLROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_ipdlRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1DFLYROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_dflyRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1DFLUROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_dfluRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1DFLVROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_dflvRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;

        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_AV1CDEFROWSTORECACHE_DISABLE_ID,
            &userFeatureData,
            m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_cdefRowstoreCache.bSupported = userFeatureData.i32Data ? false : true;
    }
}

MOS_STATUS MhwVdboxAvpInterfaceG12::GetRowstoreCachingAddrs(
    PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(rowstoreParams);
    
    //BTDL
    if (m_btdlRowstoreCache.bSupported)
    {
        m_btdlRowstoreCache.bEnabled     = true;
        m_btdlRowstoreCache.dwAddress    = 0;
    }

    //SMVL
    if (m_smvlRowstoreCache.bSupported)
    {
        m_smvlRowstoreCache.bEnabled     = true;
        m_smvlRowstoreCache.dwAddress    = 128;
    }

    //IPDL
    if (m_ipdlRowstoreCache.bSupported)
    {
        m_ipdlRowstoreCache.bEnabled     = true;
        m_ipdlRowstoreCache.dwAddress    = 384;
    }

    //DFLY
    if (m_dflyRowstoreCache.bSupported)
    {
        m_dflyRowstoreCache.bEnabled     = true;
        m_dflyRowstoreCache.dwAddress    = 640;
    }

    //DFLU
    if (m_dfluRowstoreCache.bSupported)
    {
        m_dfluRowstoreCache.bEnabled     = true;
        m_dfluRowstoreCache.dwAddress    = 1344;
    }

    //DFLV
    if (m_dflvRowstoreCache.bSupported)
    {
        m_dflvRowstoreCache.bEnabled     = true;
        m_dflvRowstoreCache.dwAddress    = 1536;
    }

    //CDEF
    if (m_cdefRowstoreCache.bSupported)
    {
        m_cdefRowstoreCache.bEnabled     = true;
        m_cdefRowstoreCache.dwAddress    = 1728;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::GetAvpStateCommandSize(
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(commandsSize);
    MHW_MI_CHK_NULL(patchListSize);

    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;
  
    maxSize =
        mhw_vdbox_vdenc_g12_X::VD_PIPELINE_FLUSH_CMD::byteSize          +
        mhw_mi_g12_X::MI_FLUSH_DW_CMD::byteSize                         +
        mhw_vdbox_avp_g12_X::AVP_PIPE_MODE_SELECT_CMD::byteSize         +
        mhw_vdbox_avp_g12_X::AVP_SURFACE_STATE_CMD::byteSize * 11       +
        mhw_vdbox_avp_g12_X::AVP_PIPE_BUF_ADDR_STATE_CMD::byteSize      +
        mhw_vdbox_avp_g12_X::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize  +
        mhw_vdbox_avp_g12_X::AVP_SEGMENT_STATE_CMD::byteSize * 8        +
        mhw_vdbox_avp_g12_X::AVP_INLOOP_FILTER_STATE_CMD::byteSize      +
        mhw_vdbox_avp_g12_X::AVP_INTER_PRED_STATE_CMD::byteSize;

    patchListMaxSize =
        PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD)           +
        PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD)                 +
        PATCH_LIST_COMMAND(AVP_PIPE_MODE_SELECT_CMD)        +
        PATCH_LIST_COMMAND(AVP_SURFACE_STATE_CMD) * 11      +
        PATCH_LIST_COMMAND(AVP_PIPE_BUF_ADDR_STATE_CMD)     +
        PATCH_LIST_COMMAND(AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
        PATCH_LIST_COMMAND(AVP_SEGMENT_STATE_CMD) * 8       +
        PATCH_LIST_COMMAND(AVP_INTER_PRED_STATE_CMD)        +
        PATCH_LIST_COMMAND(AVP_INLOOP_FILTER_STATE_CMD);

        if(m_decodeInUse)
        {
            maxSize += 
                mhw_vdbox_avp_g12_X::AVP_PIC_STATE_CMD::byteSize +
                mhw_mi_g12_X::VD_CONTROL_STATE_CMD::byteSize * 2;

            patchListMaxSize += PATCH_LIST_COMMAND(AVP_PIC_STATE_CMD);

            MHW_CHK_NULL_RETURN(params);
            auto paramsG12 = dynamic_cast<PMHW_VDBOX_STATE_CMDSIZE_PARAMS_G12>(params);
            MHW_CHK_NULL_RETURN(paramsG12);
            if (paramsG12->bScalableMode)
            {
                // VD_CONTROL_STATE AVP lock and unlock
                maxSize += 2 * mhw_mi_g12_X::VD_CONTROL_STATE_CMD::byteSize;
            }
        }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::GetAvpPrimitiveCommandSize(
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(commandsSize);
    MHW_MI_CHK_NULL(patchListSize);

    uint32_t            maxSize = 0;
    uint32_t            patchListMaxSize = 0;

    if(m_decodeInUse)
    {
        if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrAV1VLDLSTDecoding) && !m_disableLstCmd)
        {
            maxSize =
                mhw_vdbox_avp_g12_X::AVP_TILE_CODING_CMD_LST::byteSize +
                mhw_vdbox_avp_g12_X::AVP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(AVP_TILE_CODING_CMD_LST)                +
                PATCH_LIST_COMMAND(AVP_BSD_OBJECT_CMD);
        }
        else
        {
            maxSize =
                mhw_vdbox_avp_g12_X::AVP_TILE_CODING_CMD::byteSize +
                mhw_vdbox_avp_g12_X::AVP_BSD_OBJECT_CMD::byteSize +
                mhw_mi_g12_X::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(AVP_TILE_CODING_CMD) +
                PATCH_LIST_COMMAND(AVP_BSD_OBJECT_CMD);
        }
    }

    *commandsSize = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::GetAv1BufferSize(
    MhwVdboxAvpInternalBufferType       bufferType,
    MhwVdboxAvpBufferSizeParams         *avpBufSizeParam)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(avpBufSizeParam);

    uint32_t sbPerFrmWid        = avpBufSizeParam->m_picWidth;
    uint32_t sbPerFrmHgt        = avpBufSizeParam->m_picHeight;
    uint32_t sbPerTileWid       = avpBufSizeParam->m_tileWidth;
    uint32_t bufferSize         = 0;
    uint32_t totalSbPerFrame    = sbPerFrmWid * sbPerFrmHgt;
    uint32_t index              = (uint32_t)bufferType;
    uint32_t maxCuPerSB         = avpBufSizeParam->m_isSb128x128 ? 256 : 64;

    MHW_ASSERT(avpBufSizeParam->m_bitDepthIdc == 0 || avpBufSizeParam->m_bitDepthIdc == 1);

    switch (bufferType)
    {
        //Rowstore buffers, Total CLs = (#CLs_per_SB * num_of_SB_per_tile_width)
        case bsdLineBuf:
        case spatialMvLineBuf:
        case intraPredLine:
        case deblockLineYBuf:
        case deblockLineUBuf:
        case deblockLineVBuf:
            bufferSize = sbPerTileWid * CodecAv1BufferSize[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128];
            break;
        case cdefLineBuf:
            bufferSize = sbPerTileWid * CodecAv1BufferSize[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128]
                            + CodecAv1BufferSizeExt[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128];
            break;
        //Tile storage - tile line buffers, Total CLs = (#CLs_per_SB * num_of_SB_per_row) = (#CLs_per_SB * num_of_SB_per_frame_width)
        case bsdTileLineBuf:
        case spatialMvTileLineBuf:
        case intraPredTileLine:
        case deblockTileLineYBuf:
        case deblockTileLineUBuf:
        case deblockTileLineVBuf:
            bufferSize = sbPerFrmWid * CodecAv1BufferSize[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128];
            break;
        //Tile storage - tile column buffers, Total CLs = (#CLs_per_SB * num_of_SB_per_column) = (#CLs_per_SB * num_of_SB_per_frame_height)
        case deblockTileColYBuf:
        case deblockTileColUBuf:
        case deblockTileColVBuf:
            bufferSize = sbPerFrmHgt * CodecAv1BufferSize[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128];
            break;
        // Tile storage, per tile number
        case cdefTopLeftCornerBuf:
            bufferSize = avpBufSizeParam->m_curFrameTileNum;
            break;
        case cdefMetaTileLine:
            bufferSize = avpBufSizeParam->m_numTileCol;
            break;
        case lrTileLineY:
            bufferSize = avpBufSizeParam->m_numTileCol * 7;
            break;
        case lrTileLineU:
        case lrTileLineV:
            bufferSize = avpBufSizeParam->m_numTileCol * 5;
            break;
        // Tile storage, - tile line buffers, with extra size
        case cdefTileLineBuf:
            bufferSize = sbPerFrmWid * CodecAv1BufferSize[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128]
                + CodecAv1BufferSizeExt[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128];
            break;
        // Tile storage, - tile column buffers, with extra size
        case cdefTileColBuf:
        case cdefMetaTileCol:
        case superResTileColYBuf:
        case superResTileColUBuf:
        case superResTileColVBuf:
        case lrTileColYBuf:
        case lrTileColUBuf:
        case lrTileColVBuf:
        case lrMetaTileCol:
            bufferSize = sbPerFrmHgt * CodecAv1BufferSize[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128]
                + CodecAv1BufferSizeExt[index][avpBufSizeParam->m_bitDepthIdc][avpBufSizeParam->m_isSb128x128]
                + MOS_PAGE_SIZE; //Add one page size here to fix page fault issue
            break;
        //frame buffer
        case segmentIdBuf:
            bufferSize = ((avpBufSizeParam->m_isSb128x128)? 8 : 2) * totalSbPerFrame;
            break;
        case mvTemporalBuf:
            bufferSize = ((avpBufSizeParam->m_isSb128x128) ? 16 : 4) * totalSbPerFrame;
            break;
        case frameStatusErrBuf:
        case dbdStreamoutBuf:
            bufferSize = 1;
            break;
        case tileSzStreamOutBuf:
            bufferSize = avpBufSizeParam->m_numTileCol * avpBufSizeParam->m_numTileCol * MHW_CACHELINE_SIZE;
            break;
        case tileStatStreamOutBuf:
            bufferSize = 512;
            break;
        case cuStreamoutBuf:
            bufferSize = MOS_ALIGN_CEIL(totalSbPerFrame * maxCuPerSB * 8, MHW_CACHELINE_SIZE); // Each CU streams out 8 bytes
            break;
        case sseLineBuf:
        case sseTileLineBuf:
            if (avpBufSizeParam->m_numOfActivePipes > 1)
            {
                // To be added for scalability case
            }
            else
            {
                bufferSize = (sbPerFrmWid + 3) * MHW_CACHELINE_SIZE * (8 + 8) << 1; // 8 for Luma, 8 for Chroma
            }
            break;
        default:
            return MOS_STATUS_INVALID_PARAMETER;

    }

    avpBufSizeParam->m_bufferSize = bufferSize * MHW_CACHELINE_SIZE;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::IsAv1BufferReallocNeeded(
    MhwVdboxAvpInternalBufferType       bufferType,
    MhwVdboxAvpBufferReallocParams      *reallocParam)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(reallocParam);

    uint32_t   widthInSb, heightInSb, picWidthInSbAlloced, picHeightInSbAlloced;
    bool       realloc = false;

    MHW_MI_CHK_NULL(reallocParam);

    widthInSb               = reallocParam->m_picWidth;
    heightInSb              = reallocParam->m_picHeight;
    picWidthInSbAlloced     = reallocParam->m_picWidthAlloced;
    picHeightInSbAlloced    = reallocParam->m_picHeightAlloced;

    uint32_t sbPerTileWid = reallocParam->m_tileWidth;
    MHW_ASSERT(reallocParam->m_bitDepthIdc == 0 || reallocParam->m_bitDepthIdc == 1);

    uint32_t index = (uint32_t)bufferType;
    uint32_t prevSize, curSize;
    switch (bufferType)
    {
        //Rowstore buffers, size per SB
        case bsdLineBuf:
        case spatialMvLineBuf:
        case intraPredLine:
        case deblockLineYBuf:
        case deblockLineUBuf:
        case deblockLineVBuf:
        case cdefLineBuf:
            realloc = false;
            break;
        //Tile storage - tile line buffer
        case bsdTileLineBuf:
        case spatialMvTileLineBuf:
        case intraPredTileLine:
        case deblockTileLineYBuf:
        case deblockTileLineUBuf:
        case deblockTileLineVBuf:
        case cdefTileLineBuf:
            realloc = (widthInSb > picWidthInSbAlloced);
            break;
        // Tile storage - tile column buffer
        case deblockTileColYBuf:
        case deblockTileColUBuf:
        case deblockTileColVBuf:
        case cdefTileColBuf:
        case cdefMetaTileCol:
        case superResTileColYBuf:
        case superResTileColUBuf:
        case superResTileColVBuf:
        case lrTileColYBuf:
        case lrTileColUBuf:
        case lrTileColVBuf:
        case lrMetaTileCol:
            realloc = (heightInSb > picHeightInSbAlloced);
            break;
        // Tile storage, per tile number
        case cdefTopLeftCornerBuf:
            realloc = (reallocParam->m_curFrameTileNum > reallocParam->m_prevFrameTileNum);
            break;
        case cdefMetaTileLine:
        case lrTileLineY:
        case lrTileLineU:
        case lrTileLineV:
            realloc = reallocParam->m_numTileCol > reallocParam->m_numTileColAllocated;
            break;
        //frame buffer
        case segmentIdBuf:
        case mvTemporalBuf:
            realloc = (widthInSb * heightInSb) > reallocParam->m_bufferSizeAlloced;//Note: SB128/SB64 is per sequence info, driver assumes one device supports one sequence only.
            break;
        case frameStatusErrBuf:
        case dbdStreamoutBuf:
            realloc = 0;
            break;
        default:
            return MOS_STATUS_INVALID_PARAMETER;
    }

    reallocParam->m_needBiggerSize = realloc;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    auto paramsG12 = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(params);
    MHW_MI_CHK_NULL(paramsG12);
    mhw_vdbox_avp_g12_X::AVP_PIPE_MODE_SELECT_CMD   cmd;

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select...
    MHW_MI_CHK_STATUS(m_miInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    if (m_decodeInUse)
    {
        cmd.DW1.CodecSelect = cmd.CODEC_SELECT_DECODE;
    }
    else
    {
        cmd.DW1.CodecSelect = cmd.CODEC_SELECT_ENCODE;
    }

    cmd.DW1.CdefOutputStreamoutEnableFlag               = false;
    cmd.DW1.LoopRestorationOutputStreamoutEnableFlag    = false;
    cmd.DW1.PicStatusErrorReportEnable                  = false;
    cmd.DW1.CodecStandardSelect                         = 2;
    cmd.DW1.MultiEngineMode                             = paramsG12->MultiEngineMode;
    cmd.DW1.PipeWorkingMode                             = paramsG12->PipeWorkMode;
    cmd.DW1.TileBasedEngine                             = paramsG12->bTileBasedReplayMode;
    cmd.DW3.PicStatusErrorReportId                      = false;
    cmd.DW5.PhaseIndicator                              = paramsG12->ucPhaseIndicator;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, params->pBatchBuffer, &cmd, sizeof(cmd)));

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select...
    MHW_MI_CHK_STATUS(m_miInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
{
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_avp_g12_X::AVP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_avp_g12_X::AVP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxAvpInterfaceGeneric<mhw_vdbox_avp_g12_X>::AddAvpDecodeSurfaceStateCmd(cmdBuffer, params));

    if (params->ucBitDepthLumaMinus8 == 0 && params->ucBitDepthChromaMinus8 == 0)
    {
        if (params->ChromaType == HCP_CHROMA_FORMAT_YUV420 && params->psSurface->Format == Format_NV12)// 4:2:0 8bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_PLANAR4208;
        }
        else if (params->ChromaType == HCP_CHROMA_FORMAT_YUV420 && params->psSurface->Format == Format_P010)// 4:2:0 10bit surface
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_P010;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else if ((params->ucBitDepthLumaMinus8 == 2) && (params->ucBitDepthChromaMinus8 == 2))
    {
        if (params->ChromaType == HCP_CHROMA_FORMAT_YUV420 && params->psSurface->Format == Format_P010)// 4:2:0 10b
        {
            cmd->DW2.SurfaceFormat = cmd->SURFACE_FORMAT_P010;
        }
        else
        {
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    cmd->DW3.DefaultAlphaValue = 0;

    uint32_t DW4 = 0;
    if(MmcEnable(params->mmcState))
    {
        DW4 |= ((~params->mmcSkipMask) & 0xff);
    }
    if(MmcIsRc(params->mmcState))
    {
        DW4 |= 0xff00;
    }
    
    cmd->DW4.Value             = (DW4 | params->dwCompressionFormat << 16);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    MhwVdboxAvpPipeBufAddrParams     *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_SURFACE details;
    mhw_vdbox_avp_g12_X::AVP_PIPE_BUF_ADDR_STATE_CMD cmd;

    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));

    // 1. MHW_VDBOX_HCP_GENERAL_STATE_SHIFT(6) may not work with DecodedPicture
    // since it needs to be 4k aligned
    resourceParams.dwLsbNum = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

    // Reference Picture Base Address. Only one control DW17 for all references
    cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12.Index;

    bool                firstRefPic = true;
    MOS_MEMCOMP_STATE   mmcMode = MOS_MEMCOMP_DISABLED;

    // NOTE: all the 8 ref pic addresses has been set to valid addresses for error concealment purpose
    // m_references[8] follows the order LAST_FRAME->LAST2_FRAME->LAST3_FRAME->GOLDEN_FRAME->BWDREF_FRAME->ALTREF2_FRAME->ALTREF_FRAME.
    for (uint32_t i = 0; i < av1TotalRefsPerFrame; i++)
    {
        // Reference Picture Buffer
        if (params->m_references[i] != nullptr)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, params->m_references[i], &details));

            if (firstRefPic)
            {
                cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(details.TileType);
                firstRefPic = false;
            }

            resourceParams.presResource = params->m_references[i];
            resourceParams.pdwCmd = (cmd.ReferenceFrameBufferBaseAddressRefaddr07[i].DW0_1.Value);
            resourceParams.dwOffset = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.dwLocationInCmd = (i * 2) + 1;
            resourceParams.bIsWritable = false;
            resourceParams.dwSharedMocsOffset = 17 - resourceParams.dwLocationInCmd;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    //Decoded Output Frame Buffer
    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Gen12.Index;
    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnable(params->m_preDeblockSurfMmcState) ? 1 : 0;
    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.CompressionType = MmcIsRc(params->m_preDeblockSurfMmcState) ? 1 : 0;

    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressTiledResourceMode = Mhw_ConvertToTRMode(params->m_decodedPic->TileType);

    //Same MMC status for deblock and reference surfaces
    cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = cmd.DecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable;
    cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.CompressionType = cmd.DecodedOutputFrameBufferAddressAttributes.DW0.CompressionType;

    resourceParams.presResource = &(params->m_decodedPic->OsResource);
    resourceParams.dwOffset = params->m_decodedPic->dwOffset;
    resourceParams.pdwCmd = (cmd.DecodedOutputFrameBufferAddress.DW0_1.Value);
    resourceParams.dwLocationInCmd = 18;
    resourceParams.bIsWritable = true;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    //IntraBC Decoded Output Frame buffer
    if (params->m_intrabcDecodedOutputFrameBuffer != nullptr)
    {
        cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_intrabcDecodedOutputFrameBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntrabcDecodedOutputFrameBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 24;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        //This surface should not have memory compression turned on
        cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = 0;
        cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.CompressionType =0;
    }

    // CDF Table Initialization Buffer
    if (params->m_cdfTableInitializationBuffer != nullptr)
    {
        cmd.CdfTablesInitializationBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdfTableInitializationBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdfTablesInitializationBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 27;
        resourceParams.bIsWritable = false;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // CDF Tables Backward Adaptation Buffer
    if (params->m_cdfTableBwdAdaptationBuffer != nullptr)
    {
        cmd.CdfTablesBackwardAdaptationBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdfTableBwdAdaptationBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdfTablesBackwardAdaptationBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 30;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Reset dwSharedMocsOffset
    //resourceParams.dwSharedMocsOffset = 0;

    // AV1 Segment Id Read Buffer
    if (params->m_segmentIdReadBuffer != nullptr)
    {
        cmd.Av1SegmentIdReadBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_segmentIdReadBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.Av1SegmentIdReadBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 33;
        resourceParams.bIsWritable = true;

        resourceParams.dwSharedMocsOffset = 35 - resourceParams.dwLocationInCmd;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // AV1 Segment Id Write Buffer
    if (params->m_segmentIdWriteBuffer != nullptr)
    {
        cmd.Av1SegmentIdWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_segmentIdWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.Av1SegmentIdWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 36;
        resourceParams.bIsWritable = true;

        resourceParams.dwSharedMocsOffset = 38 - resourceParams.dwLocationInCmd;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    //Collocated MV Temporal buffers
    cmd.CollocatedMotionVectorTemporalBufferBaseAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

    for (uint32_t i = 0; i < av1TotalRefsPerFrame; i++)
    {
        if (params->m_colMvTemporalBuffer[i] != nullptr)
        {
            resourceParams.presResource = params->m_colMvTemporalBuffer[i];
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.CollocatedMotionVectorTemporalBufferBaseAddressTmvaddr07[i].DW0_1.Value);
            resourceParams.dwLocationInCmd = (i * 2) + 39;
            resourceParams.bIsWritable = true;
            resourceParams.dwSharedMocsOffset = 55 - resourceParams.dwLocationInCmd;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
    }

    // Current Motion Vector Temporal Buffer
    if (params->m_curMvTemporalBuffer != nullptr)
    {
        cmd.CurrentFrameMotionVectorWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_curMvTemporalBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CurrentFrameMotionVectorWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 56;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;
    // Bitstream Decode Line Rowstore Buffer
    if (m_btdlRowstoreCache.bEnabled)
    {
        cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddress.DW0_1.BaseAddress = m_btdlRowstoreCache.dwAddress;
    }
    else if (params->m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer != nullptr)
    {
        cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 62;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Bitstream Decode Tile Line Rowstore Buffer
    if (params->m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer != nullptr)
    {
        cmd.BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 65;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Reset dwSharedMocsOffset
    resourceParams.dwSharedMocsOffset = 0;
    // Intra Prediction Line Rowstore Read/Write Buffer
    if (m_ipdlRowstoreCache.bEnabled)
    {
        cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.IntraPredictionLineRowstoreReadWriteBufferAddress.DW0_1.BaseAddress = m_ipdlRowstoreCache.dwAddress;
    }
    else if (params->m_intraPredictionLineRowstoreReadWriteBuffer != nullptr)
    {
        cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_intraPredictionLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntraPredictionLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 68;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Intra Prediction Tile Line Rowstore Buffer
    if (params->m_intraPredictionTileLineRowstoreReadWriteBuffer != nullptr)
    {
        cmd.IntraPredictionTileLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_intraPredictionTileLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntraPredictionTileLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 71;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Spatial Motion Vector Line Buffer
    if (m_smvlRowstoreCache.bEnabled)
    {
        cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.SpatialMotionVectorLineReadWriteBufferAddress.DW0_1.BaseAddress = m_smvlRowstoreCache.dwAddress;
    }
    else if (params->m_spatialMotionVectorLineReadWriteBuffer != nullptr)
    {
        cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_spatialMotionVectorLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SpatialMotionVectorLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 74;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Spatial Motion Vector Tile Line Buffer
    if (params->m_spatialMotionVectorCodingTileLineReadWriteBuffer != nullptr)
    {
        cmd.SpatialMotionVectorTileLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_spatialMotionVectorCodingTileLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SpatialMotionVectorCodingTileLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 77;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    //Loop Restoration Meta Tile Column Read/Write Buffer
    if (params->m_loopRestorationMetaTileColumnReadWriteBuffer != nullptr)
    {
        cmd.LoopRestorationMetaTileColumnReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_loopRestorationMetaTileColumnReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationMetaTileColumnReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 80;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    //Deblocker Filter Control Parameters Line Read Write Buffer
    if (params->m_loopRestorationFilterTileReadWriteLineYBuffer != nullptr)
    {
        cmd.LoopRestorationFilterTileReadWriteLineYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_loopRestorationFilterTileReadWriteLineYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileReadWriteLineYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 83;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    //Deblocker Filter Control Parameters Tile Line Read Write Buffer
    if (params->m_loopRestorationFilterTileReadWriteLineUBuffer != nullptr)
    {
        cmd.LoopRestorationFilterTileReadWriteLineUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_loopRestorationFilterTileReadWriteLineUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileReadWriteLineUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 86;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    //Deblocker Filter Control Parameters Tile Column Read Write Buffer
    if (params->m_loopRestorationFilterTileReadWriteLineVBuffer != nullptr)
    {
        cmd.LoopRestorationFilterTileReadWriteLineVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_loopRestorationFilterTileReadWriteLineVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileReadWriteLineVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 89;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Line Read Write Y Buffer
    if (m_dflyRowstoreCache.bEnabled)
    {
        cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockerFilterLineReadWriteYBufferAddress.DW0_1.BaseAddress = m_dflyRowstoreCache.dwAddress;
    }
    else if (params->m_deblockerFilterLineReadWriteYBuffer != nullptr)
    {
        cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterLineReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterLineReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 92;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Line Read Write U Buffer
    if (m_dfluRowstoreCache.bEnabled)
    {
        cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockerFilterLineReadWriteUBufferAddress.DW0_1.BaseAddress = m_dfluRowstoreCache.dwAddress;
    }
    else if (params->m_deblockerFilterLineReadWriteUBuffer != nullptr)
    {
        cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterLineReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterLineReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 95;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }
    // Deblocker Filter Line Read Write V Buffer
    if (m_dflvRowstoreCache.bEnabled)
    {
        cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockerFilterLineReadWriteVBufferAddress.DW0_1.BaseAddress = m_dflvRowstoreCache.dwAddress;
    }
    else if (params->m_deblockerFilterLineReadWriteVBuffer != nullptr)
    {
        cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterLineReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterLineReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 98;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Tile Line Read Write Y Buffer
    if (params->m_deblockerFilterTileLineReadWriteYBuffer != nullptr)
    {
        cmd.DeblockerFilterTileLineReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterTileLineReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileLineReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 101;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Tile Line Read Write V Buffer
    if (params->m_deblockerFilterTileLineReadWriteVBuffer != nullptr)
    {
        cmd.DeblockerFilterTileLineReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterTileLineReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileLineReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 104;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Tile Line Read Write U Buffer
    if (params->m_deblockerFilterTileLineReadWriteUBuffer != nullptr)
    {
        cmd.DeblockerFilterTileLineReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterTileLineReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileLineReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 107;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Tile Column Read Write Y Buffer
    if (params->m_deblockerFilterTileColumnReadWriteYBuffer != nullptr)
    {
        cmd.DeblockerFilterTileColumnReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterTileColumnReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileColumnReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 110;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Tile Column Read Write U Buffer
    if (params->m_deblockerFilterTileColumnReadWriteUBuffer != nullptr)
    {
        cmd.DeblockerFilterTileColumnReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterTileColumnReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileColumnReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 113;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Deblocker Filter Tile Column Read Write V Buffer
    if (params->m_deblockerFilterTileColumnReadWriteVBuffer != nullptr)
    {
        cmd.DeblockerFilterTileColumnReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12.Index;

        resourceParams.presResource = params->m_deblockerFilterTileColumnReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileColumnReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 116;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Cdef Filter Line Read Write Y Buffer
    if (m_cdefRowstoreCache.bEnabled)
    {
        cmd.CdefFilterLineReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.CdefFilterLineReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.CdefFilterLineReadWriteBufferAddress.DW0_1.BaseAddress = m_cdefRowstoreCache.dwAddress;
    }
    else if (params->m_cdefFilterLineReadWriteBuffer != nullptr)
    {
        cmd.CdefFilterLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdefFilterLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 119;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Cdef Filter Tile Line Read Write Y Buffer
    if (params->m_cdefFilterTileLineReadWriteBuffer != nullptr)
    {
        cmd.CdefFilterTileLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdefFilterTileLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterTileLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 128;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Cdef Filter Tile Line Read Write U Buffer
    if (params->m_cdefFilterTileColumnReadWriteBuffer != nullptr)
    {
        cmd.CdefFilterTileColumnReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdefFilterTileColumnReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterTileColumnReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 137;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Cdef Filter Tile Line Read Write V Buffer
    if (params->m_cdefFilterMetaTileLineReadWriteBuffer != nullptr)
    {
        cmd.CdefFilterMetaTileLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdefFilterMetaTileLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterMetaTileLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 140;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Cdef Filter Tile Column Read Write Y Buffer
    if (params->m_cdefFilterMetaTileColumnReadWriteBuffer != nullptr)
    {
        cmd.CdefFilterMetaTileColumnReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdefFilterMetaTileColumnReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterMetaTileColumnReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 143;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Cdef Filter Top Left Corner Read Write Buffer
    if (params->m_cdefFilterTopLeftCornerReadWriteBuffer != nullptr)
    {
        cmd.CdefFilterTopLeftCornerReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_cdefFilterTopLeftCornerReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterTopLeftCornerReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 146;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Super-Res Tile Column Read Write Y Buffer
    if (params->m_superResTileColumnReadWriteYBuffer != nullptr)
    {
        cmd.SuperResTileColumnReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_superResTileColumnReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SuperResTileColumnReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 149;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Super-Res Tile Column Read Write U Buffer
    if (params->m_superResTileColumnReadWriteUBuffer != nullptr)
    {
        cmd.SuperResTileColumnReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_superResTileColumnReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SuperResTileColumnReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 152;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Super-Res Tile Column Read Write V Buffer
    if (params->m_superResTileColumnReadWriteVBuffer != nullptr)
    {
        cmd.SuperResTileColumnReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_superResTileColumnReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SuperResTileColumnReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 155;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Loop Restoration Filter Tile Column Read Write Y Buffer
    if (params->m_loopRestorationFilterTileColumnReadWriteYBuffer != nullptr)
    {
        cmd.LoopRestorationFilterTileColumnReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_loopRestorationFilterTileColumnReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileColumnReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 158;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Loop Restoration Filter Tile Column Read Write U Buffer
    if (params->m_loopRestorationFilterTileColumnReadWriteUBuffer != nullptr)
    {
        cmd.LoopRestorationFilterTileColumnReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_loopRestorationFilterTileColumnReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileColumnReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 161;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Loop Restoration Filter Tile Column Read Write V Buffer
    if (params->m_loopRestorationFilterTileColumnReadWriteVBuffer != nullptr)
    {
        cmd.LoopRestorationFilterTileColumnReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_loopRestorationFilterTileColumnReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileColumnReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 164;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Decoded Frame Status Error Buffer
    if (params->m_decodedFrameStatusErrorBuffer != nullptr)
    {
        cmd.DecodedFrameStatusErrorBufferBaseAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_decodedFrameStatusErrorBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DecodedFrameStatusErrorBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 176;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    // Decoded Block Data Streamout Buffer
    if (params->m_decodedBlockDataStreamoutBuffer != nullptr)
    {
        cmd.DecodedBlockDataStreamoutBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12.Index;

        resourceParams.presResource = params->m_decodedBlockDataStreamoutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DecodedBlockDataStreamoutBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 179;
        resourceParams.bIsWritable = true;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

    return eStatus;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpIndObjBaseAddrCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(params);

    MHW_RESOURCE_PARAMS resourceParams;
    mhw_vdbox_avp_g12_X::AVP_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.dwLsbNum         = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
    resourceParams.HwCommandType    = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

    // mode specific settings
    if(m_decodeInUse)
    {
        MHW_MI_CHK_NULL(params->presDataBuffer);

        cmd.AvpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value |=
            m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Gen12.Index;

        resourceParams.presResource     = params->presDataBuffer;
        resourceParams.dwOffset         = params->dwDataOffset;
        resourceParams.pdwCmd           = &(cmd.AvpIndirectBitstreamObjectBaseAddress.DW0_1.Value[0]);
        resourceParams.dwLocationInCmd  = 1;
        resourceParams.dwSize           = params->dwDataSize;
        resourceParams.bIsWritable      = false;

        // upper bound of the allocated resource will be set at 3 DW apart from address location
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpDecodePicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    MhwVdboxAvpPicStateParams        *params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->m_picParams);

    mhw_vdbox_avp_g12_X::AVP_PIC_STATE_CMD cmd;
    auto picParams = params->m_picParams;

    cmd.DW1.FrameWidthInPixelMinus1 = picParams->m_frameWidthMinus1;//down-scaled frame width
    cmd.DW1.FrameHeightInPixelMinus1 = picParams->m_frameHeightMinus1;

    if (picParams->m_seqInfoFlags.m_fields.m_subsamplingX == 1 && picParams->m_seqInfoFlags.m_fields.m_subsamplingY == 1)
    {
        if (picParams->m_seqInfoFlags.m_fields.m_monoChrome)
        {
            //4:0:0
            cmd.DW2.SequenceChromaSubsamplingFormat = 0;
        }
        else
        {
            //4:2:0
            cmd.DW2.SequenceChromaSubsamplingFormat = 1;
        }
    }
    else if (picParams->m_seqInfoFlags.m_fields.m_subsamplingX == 1 && picParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0)
    {
        //4:2:2
        cmd.DW2.SequenceChromaSubsamplingFormat = 2;
    }
    else if (picParams->m_seqInfoFlags.m_fields.m_subsamplingX == 0 && picParams->m_seqInfoFlags.m_fields.m_subsamplingY == 0)
    {
        //4:4:4
        cmd.DW2.SequenceChromaSubsamplingFormat = 3;
    }

    cmd.DW2.SequencePixelBitDepthIdc                = picParams->m_bitDepthIdx;
    cmd.DW2.SequenceSuperblockSizeUsed              = picParams->m_seqInfoFlags.m_fields.m_use128x128Superblock;
    cmd.DW2.SequenceEnableOrderHintFlag             = picParams->m_seqInfoFlags.m_fields.m_enableOrderHint;
    cmd.DW2.SequenceOrderHintBitsMinus1             = (picParams->m_seqInfoFlags.m_fields.m_enableOrderHint)? picParams->m_orderHintBitsMinus1 : 0;
    cmd.DW2.SequenceEnableFilterIntraFlag           = picParams->m_seqInfoFlags.m_fields.m_enableFilterIntra;
    cmd.DW2.SequenceEnableIntraEdgeFilterFlag       = picParams->m_seqInfoFlags.m_fields.m_enableIntraEdgeFilter;
    cmd.DW2.SequenceEnableDualFilterFlag            = picParams->m_seqInfoFlags.m_fields.m_enableDualFilter;
    cmd.DW2.SequenceEnableInterIntraCompoundFlag    = picParams->m_seqInfoFlags.m_fields.m_enableInterintraCompound;
    cmd.DW2.SequenceEnableMaskedCompoundFlag        = picParams->m_seqInfoFlags.m_fields.m_enableMaskedCompound;
    cmd.DW2.SequenceEnableJointCompoundFlag         = picParams->m_seqInfoFlags.m_fields.m_enableJntComp;

    cmd.DW3.AllowScreenContentToolsFlag             = picParams->m_picInfoFlags.m_fields.m_allowScreenContentTools;
    cmd.DW3.ForceIntegerMvFlag                      = picParams->m_picInfoFlags.m_fields.m_forceIntegerMv;
    cmd.DW3.AllowWarpedMotionFlag                   = picParams->m_picInfoFlags.m_fields.m_allowWarpedMotion;
    cmd.DW3.UseCdefFilterFlag                       = !(picParams->m_losslessMode || picParams->m_picInfoFlags.m_fields.m_allowIntrabc || !picParams->m_seqInfoFlags.m_fields.m_enableCdef);//coded lossless is used here
    cmd.DW3.UseSuperResFlag                         = picParams->m_picInfoFlags.m_fields.m_useSuperres;
    cmd.DW3.FrameLevelLoopRestorationFilterEnableFlag = params->m_picParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType != 0 ||
                                                        params->m_picParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType != 0 ||
                                                        params->m_picParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType != 0;

    cmd.DW3.LargeScaleTileEnableFlag    = picParams->m_picInfoFlags.m_fields.m_largeScaleTile;
    cmd.DW3.FrameType                   = picParams->m_picInfoFlags.m_fields.m_frameType;
    cmd.DW3.IntraonlyFlag               = (picParams->m_picInfoFlags.m_fields.m_frameType == keyFrame) || (picParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame);
    cmd.DW3.ErrorResilientModeFlag      = picParams->m_picInfoFlags.m_fields.m_errorResilientMode;
    cmd.DW3.AllowIntrabcFlag            = picParams->m_picInfoFlags.m_fields.m_allowIntrabc;
    cmd.DW3.PrimaryReferenceFrameIdx    = picParams->m_primaryRefFrame;

    cmd.DW4.SegmentationEnableFlag = picParams->m_av1SegData.m_enabled;
    cmd.DW4.SegmentationUpdateMapFlag = picParams->m_av1SegData.m_updateMap;
    if (picParams->m_picInfoFlags.m_fields.m_frameType == keyFrame || picParams->m_picInfoFlags.m_fields.m_frameType == intraOnlyFrame)
    {
        cmd.DW4.SegmentationTemporalUpdateFlag = 0;//override this flag to "0" in KEY_FRAME or INTRA_ONLY frame even if this bit decoded from bitstream is different
    }
    else
    {
        cmd.DW4.SegmentationTemporalUpdateFlag = picParams->m_av1SegData.m_temporalUpdate;
    }
    cmd.DW4.PreSkipSegmentIdFlag                = picParams->m_av1SegData.m_preSkipSegmentIdFlag;
    cmd.DW4.LastActiveSegmentId                 = picParams->m_av1SegData.m_lastActiveSegmentId;
    cmd.DW4.DeltaQPresentFlag                   = picParams->m_modeControlFlags.m_fields.m_deltaQPresentFlag;
    cmd.DW4.DeltaQRes                           = picParams->m_modeControlFlags.m_fields.m_log2DeltaQRes;
    cmd.DW4.FrameCodedLosslessMode              = picParams->m_losslessMode;
    cmd.DW4.SegmentIdBufferStreamInEnableFlag   = picParams->m_av1SegData.m_segIdBufStreamInEnable;
    cmd.DW4.SegmentIdBufferStreamOutEnableFlag  = picParams->m_av1SegData.m_segIdBufStreamOutEnable;
    cmd.DW4.SegmentMapIsZeroFlag                = picParams->m_av1SegData.m_segmentMapIsZeroFlag;
    cmd.DW4.BaseQindex                          = picParams->m_baseQindex;
    cmd.DW4.YDcDeltaQ                           = picParams->m_yDcDeltaQ;

    cmd.DW5.UDcDeltaQ = picParams->m_uDcDeltaQ;
    cmd.DW5.UAcDeltaQ = picParams->m_uAcDeltaQ;
    cmd.DW5.VDcDeltaQ = picParams->m_vDcDeltaQ;
    cmd.DW5.VAcDeltaQ = picParams->m_vAcDeltaQ;

    cmd.DW6.AllowHighPrecisionMv            = picParams->m_picInfoFlags.m_fields.m_allowHighPrecisionMv;
    cmd.DW6.FrameLevelReferenceModeSelect   = !(picParams->m_modeControlFlags.m_fields.m_referenceMode == singleReference);
    cmd.DW6.McompFilterType                 = picParams->m_interpFilter;
    cmd.DW6.MotionModeSwitchableFlag        = picParams->m_picInfoFlags.m_fields.m_isMotionModeSwitchable;
    cmd.DW6.UseReferenceFrameMvSetFlag      = picParams->m_picInfoFlags.m_fields.m_useRefFrameMvs;
    cmd.DW6.ReferenceFrameSignBiasI0To7     = (params->m_referenceFrameSignBias[1] << 1) |
                                              (params->m_referenceFrameSignBias[2] << 2) |
                                              (params->m_referenceFrameSignBias[3] << 3) |
                                              (params->m_referenceFrameSignBias[4] << 4) |
                                              (params->m_referenceFrameSignBias[5] << 5) |
                                              (params->m_referenceFrameSignBias[6] << 6) |
                                              (params->m_referenceFrameSignBias[7] << 7);
    cmd.DW6.CurrentFrameOrderHint = picParams->m_orderHint;

    cmd.DW7.ReducedTxSetUsed    = picParams->m_modeControlFlags.m_fields.m_reducedTxSetUsed;
    cmd.DW7.FrameTransformMode  = picParams->m_modeControlFlags.m_fields.m_txMode;
    cmd.DW7.SkipModePresentFlag = picParams->m_modeControlFlags.m_fields.m_skipModePresent;
    cmd.DW7.SkipModeFrame0      = params->m_skipModeFrame[0] + lastFrame;
    cmd.DW7.SkipModeFrame1      = params->m_skipModeFrame[1] + lastFrame;
    cmd.DW7.RefFrameSide        = (params->m_picParams->m_refFrameSide[0]) |
                                  (params->m_picParams->m_refFrameSide[1] << 1) |
                                  (params->m_picParams->m_refFrameSide[2] << 2) |
                                  (params->m_picParams->m_refFrameSide[3] << 3) |
                                  (params->m_picParams->m_refFrameSide[4] << 4) |
                                  (params->m_picParams->m_refFrameSide[5] << 5) |
                                  (params->m_picParams->m_refFrameSide[6] << 6) |
                                  (params->m_picParams->m_refFrameSide[7] << 7);

    //inter frame uses 1..7 instead of 0..6 for LAST_FRAME->ALTREF_FRAME
    cmd.DW8.GlobalMotionType1 = picParams->m_wm[0].m_wmtype;
    cmd.DW8.GlobalMotionType2 = picParams->m_wm[1].m_wmtype;
    cmd.DW8.GlobalMotionType3 = picParams->m_wm[2].m_wmtype;
    cmd.DW8.GlobalMotionType4 = picParams->m_wm[3].m_wmtype;
    cmd.DW8.GlobalMotionType5 = picParams->m_wm[4].m_wmtype;
    cmd.DW8.GlobalMotionType6 = picParams->m_wm[5].m_wmtype;
    cmd.DW8.GlobalMotionType7 = picParams->m_wm[6].m_wmtype;
    cmd.DW8.FrameLevelGlobalMotionInvalidFlags = (picParams->m_wm[0].m_invalid << 1) |
                                                 (picParams->m_wm[1].m_invalid << 2) |
                                                 (picParams->m_wm[2].m_invalid << 3) |
                                                 (picParams->m_wm[3].m_invalid << 4) |
                                                 (picParams->m_wm[4].m_invalid << 5) |
                                                 (picParams->m_wm[5].m_invalid << 6) |
                                                 (picParams->m_wm[6].m_invalid << 7);

    //DW9..DW29
    //It specifies the Warp Parameter set for each of the 7reference frames [LAST_FRAME .. ALTREF_FRAME]
    uint8_t idx = 0;
    for (uint32_t frame = (uint32_t)lastFrame; frame <= (uint32_t)altRefFrame; frame++)
    {
        cmd.WarpParametersArrayReference1To7Projectioncoeff0To5[idx++] = CAT2SHORTS(picParams->m_wm[frame - lastFrame].m_wmmat[0], picParams->m_wm[frame - lastFrame].m_wmmat[1]);
        cmd.WarpParametersArrayReference1To7Projectioncoeff0To5[idx++] = CAT2SHORTS(picParams->m_wm[frame - lastFrame].m_wmmat[2], picParams->m_wm[frame - lastFrame].m_wmmat[3]);
        cmd.WarpParametersArrayReference1To7Projectioncoeff0To5[idx++] = CAT2SHORTS(picParams->m_wm[frame - lastFrame].m_wmmat[4], picParams->m_wm[frame - lastFrame].m_wmmat[5]);
    }

    cmd.DW30.ReferenceFrameIdx1 = lastFrame;
    cmd.DW30.ReferenceFrameIdx2 = last2Frame;
    cmd.DW30.ReferenceFrameIdx3 = last3Frame;
    cmd.DW30.ReferenceFrameIdx4 = goldenFrame;
    cmd.DW30.ReferenceFrameIdx5 = bwdRefFrame;
    cmd.DW30.ReferenceFrameIdx6 = altRef2Frame;
    cmd.DW30.ReferenceFrameIdx7 = altRefFrame;

    //Setup reference frame width/height and scale factors
    if (picParams->m_picInfoFlags.m_fields.m_frameType != keyFrame && picParams->m_picInfoFlags.m_fields.m_frameType != intraOnlyFrame)
    {
        PCODEC_PICTURE  refFrameList = &(picParams->m_refFrameMap[0]);
        uint32_t        refFrameWidth[7], refFrameHeight[7];
        uint8_t         refPicIndex;
        uint32_t        av1ScalingFactorMax = (1 << 15);  //!< AV1 Scaling factor range [1/16, 2]
        uint32_t        av1ScalingFactorMin = (1 << 10);  //!< AV1 Scaling factor range [1/16, 2]

        union
        {
            struct
            {
                uint32_t  m_verticalScaleFactor : 16; // Vertical Scale Factor
                uint32_t  m_horizontalScaleFactor : 16; // Horizontal Scale Factor
            };
            uint32_t      m_value;
        } refScaleFactor[7];
        union
        {
            struct
            {
                uint32_t  m_widthInPixelMinus1 : 16; // Ref Frame Width In Pixel Minus 1
                uint32_t  m_heightInPixelMinus1 : 16; // Ref Frame Height In Pixel Minus 1
            };
            uint32_t      m_value;
        } refFrameRes[7];

        memset(refScaleFactor, 0, sizeof(refScaleFactor));
        memset(refFrameRes, 0, sizeof(refFrameRes));

        for (auto i = 0; i < av1NumInterRefFrames; i++)  //i=0 corresponds to LAST_FRAME
        {
            refPicIndex = 0xFF;
            if (picParams->m_refFrameIdx[i] < av1TotalRefsPerFrame &&
                refFrameList[picParams->m_refFrameIdx[i]].FrameIdx < CODECHAL_MAX_DPB_NUM_AV1)
            {
                refPicIndex = refFrameList[picParams->m_refFrameIdx[i]].FrameIdx;
            }
            else
            {
                refPicIndex = params->m_validRefPicIdx;
            }
            MHW_ASSERT(refPicIndex < CODECHAL_MAX_DPB_NUM_AV1);

            refFrameWidth[i]    = params->m_refList[refPicIndex]->m_frameWidth;
            refFrameHeight[i]   = params->m_refList[refPicIndex]->m_frameHeight;

            uint32_t curFrameWidth  = picParams->m_frameWidthMinus1 + 1;
            uint32_t curFrameHeight = picParams->m_frameHeightMinus1 + 1;

            refScaleFactor[i].m_horizontalScaleFactor   = (refFrameWidth[i] * m_av1ScalingFactor + (curFrameWidth >> 1)) / curFrameWidth;
            refScaleFactor[i].m_verticalScaleFactor     = (refFrameHeight[i] * m_av1ScalingFactor + (curFrameHeight >> 1)) / curFrameHeight;

            refFrameRes[i].m_widthInPixelMinus1     = refFrameWidth[i] - 1;
            refFrameRes[i].m_heightInPixelMinus1    = refFrameHeight[i] - 1;
            MHW_CHK_COND(refScaleFactor[i].m_horizontalScaleFactor > av1ScalingFactorMax, "Invalid parameter");
            MHW_CHK_COND(refScaleFactor[i].m_verticalScaleFactor   > av1ScalingFactorMax, "Invalid parameter");
            MHW_CHK_COND(refScaleFactor[i].m_horizontalScaleFactor < av1ScalingFactorMin, "Invalid parameter");
            MHW_CHK_COND(refScaleFactor[i].m_verticalScaleFactor   < av1ScalingFactorMin, "Invalid parameter");
        }

        cmd.DW31.Value = CAT2SHORTS(picParams->m_frameWidthMinus1, picParams->m_frameHeightMinus1);
        cmd.DW32.Value = refFrameRes[0].m_value;//LAST
        cmd.DW33.Value = refFrameRes[1].m_value;//LAST2
        cmd.DW34.Value = refFrameRes[2].m_value;//LAST3
        cmd.DW35.Value = refFrameRes[3].m_value;//GOLD
        cmd.DW36.Value = refFrameRes[4].m_value;//BWD
        cmd.DW37.Value = refFrameRes[5].m_value;//ALT2
        cmd.DW38.Value = refFrameRes[6].m_value;//ALT

        cmd.DW39.Value = CAT2SHORTS(m_av1ScalingFactor, m_av1ScalingFactor);
        cmd.DW40.Value = refScaleFactor[0].m_value;//LAST
        cmd.DW41.Value = refScaleFactor[1].m_value;//LAST2
        cmd.DW42.Value = refScaleFactor[2].m_value;//LAST3
        cmd.DW43.Value = refScaleFactor[3].m_value;//GOLD
        cmd.DW44.Value = refScaleFactor[4].m_value;//BWD
        cmd.DW45.Value = refScaleFactor[5].m_value;//ALT2
        cmd.DW46.Value = refScaleFactor[6].m_value;//ALT
    }

    cmd.DW47.ReferenceFrameOrderHint0ForIntraFrame      = picParams->m_orderHint;
    cmd.DW47.ReferenceFrameOrderHint1ForLastFrame       = params->m_refOrderHints[0];
    cmd.DW47.ReferenceFrameOrderHint2ForLast2Frame      = params->m_refOrderHints[1];
    cmd.DW47.ReferenceFrameOrderHint3ForLast3Frame      = params->m_refOrderHints[2];
    cmd.DW48.ReferenceFrameOrderHint4ForGoldenFrame     = params->m_refOrderHints[3];
    cmd.DW48.ReferenceFrameOrderHint5ForBwdrefFrame     = params->m_refOrderHints[4];
    cmd.DW48.ReferenceFrameOrderHint6ForAltref2Frame    = params->m_refOrderHints[5];
    cmd.DW48.ReferenceFrameOrderHint7ForAltrefFrame     = params->m_refOrderHints[6];

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpSegmentStateCmd(
    PMOS_COMMAND_BUFFER                 cmdBuffer,
    MhwVdboxAvpSegmentStateParams       *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->m_av1SegmentParams);

    mhw_vdbox_avp_g12_X::AVP_SEGMENT_STATE_CMD cmd;
    uint8_t seg = params->m_currentSegmentId;
    cmd.DW1.SegmentId = seg;

    cmd.DW2.SegmentFeatureMask                          = params->m_av1SegmentParams->m_featureMask[seg];
    cmd.DW2.SegmentDeltaQindex                          = params->m_av1SegmentParams->m_featureData[seg][segLvlAltQ];
    cmd.DW2.SegmentBlockSkipFlag                        = params->m_av1SegmentParams->m_featureData[seg][segLvlSkip];
    cmd.DW2.SegmentBlockGlobalmvFlag                    = params->m_av1SegmentParams->m_featureData[seg][segLvlGlobalMv];
    cmd.DW2.SegmentLosslessFlag                         = params->m_av1SegmentParams->m_losslessFlag[seg];
    cmd.DW2.SegmentLumaYQmLevel                         = params->m_av1SegmentParams->m_qmLevelY[seg];
    cmd.DW2.SegmentChromaUQmLevel                       = params->m_av1SegmentParams->m_qmLevelU[seg];
    cmd.DW2.SegmentChromaVQmLevel                       = params->m_av1SegmentParams->m_qmLevelV[seg];

    cmd.DW3.SegmentDeltaLoopFilterLevelLumaVertical     = params->m_av1SegmentParams->m_featureData[seg][segLvlAltLfYv];
    cmd.DW3.SegmentDeltaLoopFilterLevelLumaHorizontal   = params->m_av1SegmentParams->m_featureData[seg][segLvlAltLfYh];
    cmd.DW3.SegmentDeltaLoopFilterLevelChromaU          = params->m_av1SegmentParams->m_featureData[seg][segLvlAltLfU];
    cmd.DW3.SegmentDeltaLoopFilterLevelChromaV          = params->m_av1SegmentParams->m_featureData[seg][segLvlAltLfV];
    cmd.DW3.SegmentReferenceFrame                       = params->m_av1SegmentParams->m_featureData[seg][segLvlRefFrame];

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpDecodeTileCodingCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    MhwVdboxAvpTileCodingParams     *params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_avp_g12_X::AVP_TILE_CODING_CMD cmd;
    MHW_RESOURCE_PARAMS     resourceParams;
    MEDIA_SYSTEM_INFO       *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    uint8_t                 numVdbox = (uint8_t)gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled;

    cmd.DW1.FrameTileId = params->m_tileId;
    cmd.DW1.TgTileNum   = params->m_tgTileNum;
    cmd.DW1.TileGroupId = params->m_tileGroupId;

    cmd.DW2.TileColumnPositionInSbUnit  = params->m_tileColPositionInSb;
    cmd.DW2.TileRowPositionInSbUnit     = params->m_tileRowPositionInSb;

    cmd.DW3.TileWidthInSuperblockUnitMinus1     = params->m_tileWidthInSbMinus1;
    cmd.DW3.TileHeightInSuperblockUnitMinus1    = params->m_tileHeightInSbMinus1;

    cmd.DW4.FilmGrainSampleTemplateWriteReadControl = 0;
    cmd.DW4.IslasttileofcolumnFlag          = params->m_isLastTileOfColumn;
    cmd.DW4.IslasttileofrowFlag             = params->m_isLastTileOfRow;
    cmd.DW4.IsstarttileoftilegroupFlag      = params->m_isFirstTileOfTileGroup;
    cmd.DW4.IsendtileoftilegroupFlag        = params->m_isLastTileOfTileGroup;
    cmd.DW4.IslasttileofframeFlag           = params->m_isLastTileOfFrame;
    cmd.DW4.DisableCdfUpdateFlag            = params->m_disableCdfUpdateFlag;
    cmd.DW4.DisableFrameContextUpdateFlag   = params->m_disableFrameContextUpdateFlag;

    cmd.DW5.NumberOfActiveBePipes           = params->m_numOfActiveBePipes;
    cmd.DW5.NumOfTileColumnsMinus1InAFrame  = params->m_numOfTileColumnsInFrame - 1;
    cmd.DW5.NumOfTileRowsMinus1InAFrame     = params->m_numOfTileRowsInFrame - 1;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpDecodeTileCodingCmdLst(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    MhwVdboxAvpTileCodingParams     *params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_avp_g12_X::AVP_TILE_CODING_CMD_LST cmd;
    MHW_RESOURCE_PARAMS     resourceParams;
    MEDIA_SYSTEM_INFO       *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    uint8_t                 numVdbox = (uint8_t)gtSystemInfo->VDBoxInfo.NumberOfVDBoxEnabled;

    cmd.DW1.FrameTileId = params->m_tileId;
    cmd.DW1.TgTileNum   = params->m_tgTileNum;
    cmd.DW1.TileGroupId = params->m_tileGroupId;

    cmd.DW2.TileColumnPositionInSbUnit  = params->m_tileColPositionInSb;
    cmd.DW2.TileRowPositionInSbUnit     = params->m_tileRowPositionInSb;

    cmd.DW3.TileWidthInSuperblockUnitMinus1     = params->m_tileWidthInSbMinus1;
    cmd.DW3.TileHeightInSuperblockUnitMinus1    = params->m_tileHeightInSbMinus1;

    cmd.DW4.IslasttileofcolumnFlag          = params->m_isLastTileOfColumn;
    cmd.DW4.IslasttileofrowFlag             = params->m_isLastTileOfRow;
    cmd.DW4.IsstarttileoftilegroupFlag      = params->m_isFirstTileOfTileGroup;
    cmd.DW4.IsendtileoftilegroupFlag        = params->m_isLastTileOfTileGroup;
    cmd.DW4.IslasttileofframeFlag           = params->m_isLastTileOfFrame;
    cmd.DW4.DisableCdfUpdateFlag            = params->m_disableCdfUpdateFlag;
    cmd.DW4.DisableFrameContextUpdateFlag   = params->m_disableFrameContextUpdateFlag;

    cmd.DW5.NumberOfActiveBePipes           = params->m_numOfActiveBePipes;
    cmd.DW5.NumOfTileColumnsMinus1InAFrame  = params->m_numOfTileColumnsInFrame - 1;
    cmd.DW5.NumOfTileRowsMinus1InAFrame     = params->m_numOfTileRowsInFrame - 1;

    cmd.DW6.OutputDecodedTileColumnPositionInSbUnit = params->m_outputDecodedTileColumnPositionInSBUnit;
    cmd.DW6.OutputDecodedTileRowPositionInSbUnit    = params->m_outputDecodedTileRowPositionInSBUnit;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpTileCodingCmd(
    PMOS_COMMAND_BUFFER                   cmdBuffer,
    PMHW_BATCH_BUFFER                     batchBuffer,
    MhwVdboxAvpTileCodingParams           *params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    if (m_decodeInUse)
    {
        if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrAV1VLDLSTDecoding) && !m_disableLstCmd)
        {
            MHW_MI_CHK_STATUS(AddAvpDecodeTileCodingCmdLst(cmdBuffer, batchBuffer, params));
        }
        else
        {
            MHW_MI_CHK_STATUS(AddAvpDecodeTileCodingCmd(cmdBuffer, batchBuffer, params));
        }
    }
    else
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpBsdObjectCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_BATCH_BUFFER                batchBuffer,
    MhwVdboxAvpBsdParams             *params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_avp_g12_X::AVP_BSD_OBJECT_CMD cmd;

    cmd.DW1.TileIndirectBsdDataLength       = params->m_bsdDataLength;
    cmd.DW2.TileIndirectDataStartAddress    = params->m_bsdDataStartOffset;

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpInloopFilterStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    MhwVdboxAvpPicStateParams       *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(params);
    mhw_vdbox_avp_g12_X::AVP_INLOOP_FILTER_STATE_CMD cmd;

    cmd.DW1.LumaYDeblockerFilterLevelVertical       = params->m_picParams->m_filterLevel[0];
    cmd.DW1.LumaYDeblockerFilterLevelHorizontal     = params->m_picParams->m_filterLevel[1];
    cmd.DW1.ChromaUDeblockerFilterLevel             = params->m_picParams->m_filterLevelU;
    cmd.DW1.ChromaVDeblockerFilterLevel             = params->m_picParams->m_filterLevelV;
    cmd.DW1.DeblockerFilterSharpnessLevel           = params->m_picParams->m_loopFilterInfoFlags.m_fields.m_sharpnessLevel;
    cmd.DW1.DeblockerFilterModeRefDeltaEnableFlag   = params->m_picParams->m_loopFilterInfoFlags.m_fields.m_modeRefDeltaEnabled;
    cmd.DW1.DeblockerDeltaLfResolution              = params->m_picParams->m_modeControlFlags.m_fields.m_log2DeltaLfRes;
    cmd.DW1.DeblockerFilterDeltaLfMultiFlag         = params->m_picParams->m_modeControlFlags.m_fields.m_deltaLfMulti;
    cmd.DW1.DeblockerFilterDeltaLfPresentFlag       = params->m_picParams->m_modeControlFlags.m_fields.m_deltaLfPresentFlag;

    //ref_deltas[0..7]
    cmd.DW2.DeblockerFilterRefDeltas0 = params->m_picParams->m_refDeltas[0];
    cmd.DW2.DeblockerFilterRefDeltas1 = params->m_picParams->m_refDeltas[1];
    cmd.DW2.DeblockerFilterRefDeltas2 = params->m_picParams->m_refDeltas[2];
    cmd.DW2.DeblockerFilterRefDeltas3 = params->m_picParams->m_refDeltas[3];

    cmd.DW3.DeblockerFilterRefDeltas4 = params->m_picParams->m_refDeltas[4];
    cmd.DW3.DeblockerFilterRefDeltas5 = params->m_picParams->m_refDeltas[5];
    cmd.DW3.DeblockerFilterRefDeltas6 = params->m_picParams->m_refDeltas[6];
    cmd.DW3.DeblockerFilterRefDeltas7 = params->m_picParams->m_refDeltas[7];

    //mode_deltas[0..1]
    cmd.DW4.DeblockerFilterModeDeltas0 = params->m_picParams->m_modeDeltas[0];
    cmd.DW4.DeblockerFilterModeDeltas1 = params->m_picParams->m_modeDeltas[1];

    //cdef strength
    cmd.DW5.CdefYStrength0 = params->m_picParams->m_cdefYStrengths[0];
    cmd.DW5.CdefYStrength1 = params->m_picParams->m_cdefYStrengths[1];
    cmd.DW5.CdefYStrength2 = params->m_picParams->m_cdefYStrengths[2];
    cmd.DW5.CdefYStrength3 = params->m_picParams->m_cdefYStrengths[3];
    cmd.DW5.CdefBits       = params->m_picParams->m_cdefBits;
    cmd.DW5.CdefFilterDampingFactorMinus3 = params->m_picParams->m_cdefDampingMinus3;

    cmd.DW6.CdefYStrength4 = params->m_picParams->m_cdefYStrengths[4];
    cmd.DW6.CdefYStrength5 = params->m_picParams->m_cdefYStrengths[5];
    cmd.DW6.CdefYStrength6 = params->m_picParams->m_cdefYStrengths[6];
    cmd.DW6.CdefYStrength7 = params->m_picParams->m_cdefYStrengths[7];

    cmd.DW7.CdefUvStrength0 = params->m_picParams->m_cdefUvStrengths[0];
    cmd.DW7.CdefUvStrength1 = params->m_picParams->m_cdefUvStrengths[1];
    cmd.DW7.CdefUvStrength2 = params->m_picParams->m_cdefUvStrengths[2];
    cmd.DW7.CdefUvStrength3 = params->m_picParams->m_cdefUvStrengths[3];

    cmd.DW8.CdefUvStrength4 = params->m_picParams->m_cdefUvStrengths[4];
    cmd.DW8.CdefUvStrength5 = params->m_picParams->m_cdefUvStrengths[5];
    cmd.DW8.CdefUvStrength6 = params->m_picParams->m_cdefUvStrengths[6];
    cmd.DW8.CdefUvStrength7 = params->m_picParams->m_cdefUvStrengths[7];

    //super-resolution
    cmd.DW9.SuperResUpscaledFrameWidthMinus1 = params->m_picParams->m_superResUpscaledWidthMinus1;
    cmd.DW9.SuperResDenom = params->m_picParams->m_picInfoFlags.m_fields.m_useSuperres ? params->m_picParams->m_superresScaleDenominator : 8;

    //loop restoration
    cmd.DW10.FrameLoopRestorationFilterTypeForLumaY = params->m_picParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType;
    cmd.DW10.FrameLoopRestorationFilterTypeForChromaU = params->m_picParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType;
    cmd.DW10.FrameLoopRestorationFilterTypeForChromaV = params->m_picParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType;

    //LRU size for Y
    if (params->m_picParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType    == 0 &&
        params->m_picParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType   == 0 &&
        params->m_picParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType   == 0)
    {
        cmd.DW10.LoopRestorationUnitSizeForLumaY    = 0;
    }
    else
    {
        cmd.DW10.LoopRestorationUnitSizeForLumaY    = params->m_picParams->m_loopRestorationFlags.m_fields.m_lrUnitShift + 1;
    }

    //LRU size for UV
    if (params->m_picParams->m_loopRestorationFlags.m_fields.m_yframeRestorationType == 0 &&
        params->m_picParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType == 0 &&
        params->m_picParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType == 0)
    {
        cmd.DW10.UseSameLoopRestorationUnitSizeForChromasUvFlag = 0;
    }
    else if(params->m_picParams->m_loopRestorationFlags.m_fields.m_cbframeRestorationType != 0 ||
            params->m_picParams->m_loopRestorationFlags.m_fields.m_crframeRestorationType != 0)
    {
        cmd.DW10.UseSameLoopRestorationUnitSizeForChromasUvFlag = (params->m_picParams->m_loopRestorationFlags.m_fields.m_lrUvShift == 0) ? 1 : 0;
    }

    //super-res
    cmd.DW11.LumaPlaneXStepQn   = params->m_lumaPlaneXStepQn;
    cmd.DW12.LumaPlaneX0Qn      = params->m_lumaPlaneX0Qn;
    cmd.DW13.ChromaPlaneXStepQn = params->m_chromaPlaneXStepQn;
    cmd.DW14.ChromaPlaneX0Qn    = params->m_chromaPlaneX0Qn;

    if (params->m_picParams->m_picInfoFlags.m_fields.m_largeScaleTile)
    {
        //set to 0 to disable
        cmd.DW1.ChromaUDeblockerFilterLevel = 0;
        cmd.DW1.ChromaVDeblockerFilterLevel = 0;

        //ref_deltas[0..7]
        cmd.DW2.DeblockerFilterRefDeltas0 = 1;
        cmd.DW2.DeblockerFilterRefDeltas1 = 0;
        cmd.DW2.DeblockerFilterRefDeltas2 = 0;
        cmd.DW2.DeblockerFilterRefDeltas3 = 0;
        cmd.DW3.DeblockerFilterRefDeltas4 = 0;
        cmd.DW3.DeblockerFilterRefDeltas5 = -1;
        cmd.DW3.DeblockerFilterRefDeltas6 = -1;
        cmd.DW3.DeblockerFilterRefDeltas7 = -1;
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}

MOS_STATUS MhwVdboxAvpInterfaceG12::AddAvpInterPredStateCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    MhwVdboxAvpPicStateParams       *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(params);
    mhw_vdbox_avp_g12_X::AVP_INTER_PRED_STATE_CMD cmd;

    //LAST
    cmd.DW1.SavedOrderHintsForAllReferences00 = params->m_savedRefOrderHints[0][0];
    cmd.DW1.SavedOrderHintsForAllReferences01 = params->m_savedRefOrderHints[0][1];
    cmd.DW1.SavedOrderHintsForAllReferences02 = params->m_savedRefOrderHints[0][2];
    cmd.DW1.SavedOrderHintsForAllReferences03 = params->m_savedRefOrderHints[0][3];
    cmd.DW2.SavedOrderHintsForAllReferences04 = params->m_savedRefOrderHints[0][4];
    cmd.DW2.SavedOrderHintsForAllReferences05 = params->m_savedRefOrderHints[0][5];
    cmd.DW2.SavedOrderHintsForAllReferences06 = params->m_savedRefOrderHints[0][6];
    cmd.DW2.ActiveReferenceBitmaskForMotionFieldProjection = params->m_refMaskMfProj;

    //LAST2
    cmd.DW3.SavedOrderHintsForAllReferences10 = params->m_savedRefOrderHints[1][0];
    cmd.DW3.SavedOrderHintsForAllReferences11 = params->m_savedRefOrderHints[1][1];
    cmd.DW3.SavedOrderHintsForAllReferences12 = params->m_savedRefOrderHints[1][2];
    cmd.DW3.SavedOrderHintsForAllReferences13 = params->m_savedRefOrderHints[1][3];
    cmd.DW4.SavedOrderHintsForAllReferences14 = params->m_savedRefOrderHints[1][4];
    cmd.DW4.SavedOrderHintsForAllReferences15 = params->m_savedRefOrderHints[1][5];
    cmd.DW4.SavedOrderHintsForAllReferences16 = params->m_savedRefOrderHints[1][6];

    //LAST3
    cmd.DW5.SavedOrderHintsForAllReferences20 = params->m_savedRefOrderHints[2][0];
    cmd.DW5.SavedOrderHintsForAllReferences21 = params->m_savedRefOrderHints[2][1];
    cmd.DW5.SavedOrderHintsForAllReferences22 = params->m_savedRefOrderHints[2][2];
    cmd.DW5.SavedOrderHintsForAllReferences23 = params->m_savedRefOrderHints[2][3];
    cmd.DW6.SavedOrderHintsForAllReferences24 = params->m_savedRefOrderHints[2][4];
    cmd.DW6.SavedOrderHintsForAllReferences25 = params->m_savedRefOrderHints[2][5];
    cmd.DW6.SavedOrderHintsForAllReferences26 = params->m_savedRefOrderHints[2][6];

    //GOLDEN_FRAME
    cmd.DW7.SavedOrderHintsForAllReferences30 = params->m_savedRefOrderHints[3][0];
    cmd.DW7.SavedOrderHintsForAllReferences31 = params->m_savedRefOrderHints[3][1];
    cmd.DW7.SavedOrderHintsForAllReferences32 = params->m_savedRefOrderHints[3][2];
    cmd.DW7.SavedOrderHintsForAllReferences33 = params->m_savedRefOrderHints[3][3];
    cmd.DW8.SavedOrderHintsForAllReferences34 = params->m_savedRefOrderHints[3][4];
    cmd.DW8.SavedOrderHintsForAllReferences35 = params->m_savedRefOrderHints[3][5];
    cmd.DW8.SavedOrderHintsForAllReferences36 = params->m_savedRefOrderHints[3][6];

    //BWDREF_FRAME
    cmd.DW9.SavedOrderHintsForAllReferences40 = params->m_savedRefOrderHints[4][0];
    cmd.DW9.SavedOrderHintsForAllReferences41 = params->m_savedRefOrderHints[4][1];
    cmd.DW9.SavedOrderHintsForAllReferences42 = params->m_savedRefOrderHints[4][2];
    cmd.DW9.SavedOrderHintsForAllReferences43 = params->m_savedRefOrderHints[4][3];
    cmd.DW10.SavedOrderHintsForAllReferences44 = params->m_savedRefOrderHints[4][4];
    cmd.DW10.SavedOrderHintsForAllReferences45 = params->m_savedRefOrderHints[4][5];
    cmd.DW10.SavedOrderHintsForAllReferences46 = params->m_savedRefOrderHints[4][6];

    //ALTREF2_FRAME
    cmd.DW11.SavedOrderHintsForAllReferences50 = params->m_savedRefOrderHints[5][0];
    cmd.DW11.SavedOrderHintsForAllReferences51 = params->m_savedRefOrderHints[5][1];
    cmd.DW11.SavedOrderHintsForAllReferences52 = params->m_savedRefOrderHints[5][2];
    cmd.DW11.SavedOrderHintsForAllReferences53 = params->m_savedRefOrderHints[5][3];
    cmd.DW12.SavedOrderHintsForAllReferences54 = params->m_savedRefOrderHints[5][4];
    cmd.DW12.SavedOrderHintsForAllReferences55 = params->m_savedRefOrderHints[5][5];
    cmd.DW12.SavedOrderHintsForAllReferences56 = params->m_savedRefOrderHints[5][6];

    //ALTREF_FRAME
    cmd.DW13.SavedOrderHintsForAllReferences60 = params->m_savedRefOrderHints[6][0];
    cmd.DW13.SavedOrderHintsForAllReferences61 = params->m_savedRefOrderHints[6][1];
    cmd.DW13.SavedOrderHintsForAllReferences62 = params->m_savedRefOrderHints[6][2];
    cmd.DW13.SavedOrderHintsForAllReferences63 = params->m_savedRefOrderHints[6][3];
    cmd.DW14.SavedOrderHintsForAllReferences64 = params->m_savedRefOrderHints[6][4];
    cmd.DW14.SavedOrderHintsForAllReferences65 = params->m_savedRefOrderHints[6][5];
    cmd.DW14.SavedOrderHintsForAllReferences66 = params->m_savedRefOrderHints[6][6];

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return eStatus;
}
