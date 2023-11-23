/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021-2023, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_avp_xe_hpm.cpp
//! \brief    Constructs VdBox AVP commands on DG2 platforms

#include "mhw_vdbox_avp_xe_hpm.h"
#include "mhw_vdbox_avp_hwcmd_xe_hpm.h"
#include "mhw_mi_hwcmd_xe_xpm_base.h"
#include "mhw_utilities_xe_hpm.h"

MhwVdboxAvpInterfaceXe_Hpm::MhwVdboxAvpInterfaceXe_Hpm(
    PMOS_INTERFACE  osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface,
    bool            decodeInUse)
    : MhwVdboxAvpInterfaceG12(osInterface, miInterface, cpInterface, decodeInUse)
{
    MHW_FUNCTION_ENTER;
}

MhwVdboxAvpInterfaceXe_Hpm::~MhwVdboxAvpInterfaceXe_Hpm()
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

void MhwVdboxAvpPipeBufAddrParamsXe_Hpm::Initialize()
{
    MhwVdboxAvpPipeBufAddrParams::Initialize();

    m_originalUncompressedPictureSourceBuffer   = nullptr;
    m_downscaledUncompressedPictureSourceBuffer = nullptr;
    m_tileSizeStreamoutBuffer                   = nullptr;
    m_tileStatisticsPakStreamoutBuffer          = nullptr;
    m_cuStreamoutBuffer                         = nullptr;
    m_sseLineReadWriteBuffer                    = nullptr;
    m_sseTileLineReadWriteBuffer                = nullptr;
    m_postCDEFpixelsBuffer                      = nullptr;
}

MOS_STATUS MhwVdboxAvpInterfaceXe_Hpm::GetAvpStateCommandSize(
    uint32_t *                      commandsSize,
    uint32_t *                      patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(commandsSize);
    MHW_MI_CHK_NULL(patchListSize);

    uint32_t maxSize          = 0;
    uint32_t patchListMaxSize = 0;

    maxSize =
        8 +
        mhw::mi::xe_xpm_base::Cmd::MI_FLUSH_DW_CMD::byteSize +
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_MODE_SELECT_CMD::byteSize +
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_SURFACE_STATE_CMD::byteSize * 11 +
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_SEGMENT_STATE_CMD::byteSize * 8 +
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_INLOOP_FILTER_STATE_CMD::byteSize +
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_INTER_PRED_STATE_CMD::byteSize;

    patchListMaxSize =
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::VD_PIPELINE_FLUSH_CMD) +
        PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_MODE_SELECT_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SURFACE_STATE_CMD) * 11 +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SEGMENT_STATE_CMD) * 8 +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INTER_PRED_STATE_CMD) +
        PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INLOOP_FILTER_STATE_CMD);

    if (m_decodeInUse)
    {
        maxSize +=
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIC_STATE_CMD::byteSize +
            mhw::mi::xe_xpm_base::Cmd::VD_CONTROL_STATE_CMD::byteSize * 2;

        patchListMaxSize += PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIC_STATE_CMD);

        MHW_CHK_NULL_RETURN(params);
        auto paramsG12 = dynamic_cast<PMHW_VDBOX_STATE_CMDSIZE_PARAMS_G12>(params);
        MHW_CHK_NULL_RETURN(paramsG12);
        if (paramsG12->bScalableMode)
        {
            // VD_CONTROL_STATE AVP lock and unlock
            maxSize += 2 * mhw::mi::xe_xpm_base::Cmd::VD_CONTROL_STATE_CMD::byteSize;
        }
    }

    *commandsSize  = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceXe_Hpm::GetAvpPrimitiveCommandSize(
    uint32_t *commandsSize,
    uint32_t *patchListSize)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(commandsSize);
    MHW_MI_CHK_NULL(patchListSize);

    uint32_t maxSize          = 0;
    uint32_t patchListMaxSize = 0;

    if (m_decodeInUse)
    {
        if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrAV1VLDLSTDecoding) && !m_disableLstCmd)
        {
            maxSize =
                mhw_vdbox_avp_g12_X::AVP_TILE_CODING_CMD_LST::byteSize +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_BSD_OBJECT_CMD::byteSize +
                mhw::mi::xe_xpm_base::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_TILE_CODING_CMD_LST) +
                PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_BSD_OBJECT_CMD);
        }
        else
        {
            maxSize =
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_TILE_CODING_CMD::byteSize +
                mhw::vdbox::avp::xe_hpm::Cmd::AVP_BSD_OBJECT_CMD::byteSize +
                mhw::mi::xe_xpm_base::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_TILE_CODING_CMD) +
                PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_BSD_OBJECT_CMD);
        }
    }
    else
    {
        maxSize = mhw::mi::xe_xpm_base::Cmd::MI_BATCH_BUFFER_START_CMD::byteSize * 5 +
            mhw::mi::xe_xpm_base::Cmd::VD_CONTROL_STATE_CMD::byteSize +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_MODE_SELECT_CMD::byteSize * 2 +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_SURFACE_STATE_CMD::byteSize * 11 +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIC_STATE_CMD::byteSize +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_INTER_PRED_STATE_CMD::byteSize +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_SEGMENT_STATE_CMD::byteSize * 8 +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_INLOOP_FILTER_STATE_CMD::byteSize +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_TILE_CODING_CMD::byteSize +
            mhw::vdbox::avp::xe_hpm::Cmd::AVP_PAK_INSERT_OBJECT_CMD::byteSize * 9 +
            mhw::mi::xe_xpm_base::Cmd::MI_BATCH_BUFFER_END_CMD::byteSize;

        patchListMaxSize =
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::VD_PIPELINE_FLUSH_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SURFACE_STATE_CMD) * 11 +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_IND_OBJ_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PIC_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INTER_PRED_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_SEGMENT_STATE_CMD) * 8 +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_INLOOP_FILTER_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_TILE_CODING_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::avp::Itf::AVP_PAK_INSERT_OBJECT_CMD);
    }

    *commandsSize  = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceXe_Hpm::AddAvpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
    {
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw::vdbox::avp::xe_hpm::Cmd::AVP_SURFACE_STATE_CMD  *cmd =
        (mhw::vdbox::avp::xe_hpm::Cmd::AVP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxAvpInterfaceG12::AddAvpDecodeSurfaceStateCmd(cmdBuffer, params));

    cmd->DW4.CompressionFormat = params->dwCompressionFormat;

    return eStatus;
}

MOS_STATUS MhwVdboxAvpInterfaceXe_Hpm::AddAvpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    MhwVdboxAvpPipeBufAddrParams    *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->m_decodedPic);

    MHW_RESOURCE_PARAMS resourceParams;
    MOS_SURFACE details;
    mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_BUF_ADDR_STATE_CMD cmd;

    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));

    resourceParams.dwLsbNum = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
    resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

    bool firstRefPic = true;
    MOS_MEMCOMP_STATE   mmcMode = MOS_MEMCOMP_DISABLED;

    InitMocsParams(resourceParams, &cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.Value, 1, 6);
    // m_references[8] follows the order INTRA_FRAME->LAST_FRAME->LAST2_FRAME->LAST3_FRAME->GOLDEN_FRAME->
    // BWDREF_FRAME->ALTREF2_FRAME->ALTREF_FRAME.
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
                cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnable(params->m_preDeblockSurfMmcState);
                cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.CompressionType                    = MmcIsRc(params->m_preDeblockSurfMmcState);
                cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.TileMode                           = MosGetHWTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);
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
    // Reference Picture Base Address. Only one control DW17 for all references
    cmd.ReferenceFrameBufferBaseAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
        m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_REFERENCE_PICTURE_CODEC].Gen12_5.Index;

    //Decoded Output Frame Buffer
    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnable(params->m_preDeblockSurfMmcState);
    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.CompressionType                    = MmcIsRc(params->m_preDeblockSurfMmcState);
    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.TileMode                           = MosGetHWTileType(params->m_decodedPic->TileType, params->m_decodedPic->TileModeGMM, params->m_decodedPic->bGMMTileEnabled);

    resourceParams.presResource = &(params->m_decodedPic->OsResource);
    resourceParams.dwOffset = params->m_decodedPic->dwOffset;
    resourceParams.pdwCmd = (cmd.DecodedOutputFrameBufferAddress.DW0_1.Value);
    resourceParams.dwLocationInCmd = 18;
    resourceParams.bIsWritable = true;
    InitMocsParams(resourceParams, &cmd.DecodedOutputFrameBufferAddressAttributes.DW0.Value, 1, 6);
    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));
    cmd.DecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Gen12_5.Index;

    //IntraBC Decoded Output Frame buffer
    if (params->m_intrabcDecodedOutputFrameBuffer != nullptr)
    {
        resourceParams.presResource = params->m_intrabcDecodedOutputFrameBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntrabcDecodedOutputFrameBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 24;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        //This surface should not have memory compression turned on
        cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = 0;
        cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.CompressionType = 0;
        cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.TileMode = MosGetHWTileType(params->m_decodedPic->TileType, params->m_decodedPic->TileModeGMM, params->m_decodedPic->bGMMTileEnabled);
        cmd.IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // CDF Table Initialization Buffer
    if (params->m_cdfTableInitializationBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdfTableInitializationBuffer;
        resourceParams.dwOffset = params->m_cdfTableInitializationBufferOffset;
        resourceParams.pdwCmd = (cmd.CdfTablesInitializationBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 27;
        resourceParams.bIsWritable = false;
        InitMocsParams(resourceParams, &cmd.CdfTablesInitializationBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdfTablesInitializationBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // CDF Tables Backward Adaptation Buffer
    if (params->m_cdfTableBwdAdaptationBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdfTableBwdAdaptationBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdfTablesBackwardAdaptationBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 30;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CdfTablesBackwardAdaptationBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdfTablesBackwardAdaptationBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Reset dwSharedMocsOffset
    //resourceParams.dwSharedMocsOffset = 0;

    // AV1 Segment Id Read Buffer
    if (params->m_segmentIdReadBuffer != nullptr)
    {
        resourceParams.presResource = params->m_segmentIdReadBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.Av1SegmentIdReadBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 33;
        resourceParams.bIsWritable = true;
        resourceParams.dwSharedMocsOffset = 35 - resourceParams.dwLocationInCmd;
        InitMocsParams(resourceParams, &cmd.Av1SegmentIdReadBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.Av1SegmentIdReadBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // AV1 Segment Id Write Buffer
    if (params->m_segmentIdWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_segmentIdWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.Av1SegmentIdWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 36;
        resourceParams.bIsWritable = true;
        resourceParams.dwSharedMocsOffset = 38 - resourceParams.dwLocationInCmd;
        InitMocsParams(resourceParams, &cmd.Av1SegmentIdWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.Av1SegmentIdWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }


    InitMocsParams(resourceParams, &cmd.CollocatedMotionVectorTemporalBufferBaseAddressAttributes.DW0.Value, 1, 6);
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
    //Collocated MV Temporal buffers
    cmd.CollocatedMotionVectorTemporalBufferBaseAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;

    // Current Motion Vector Temporal Buffer
    if (params->m_curMvTemporalBuffer != nullptr)
    {
        resourceParams.presResource = params->m_curMvTemporalBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CurrentFrameMotionVectorWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 56;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CurrentFrameMotionVectorWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.CurrentFrameMotionVectorWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
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
        resourceParams.presResource = params->m_bitstreamDecoderEncoderLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 62;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);

        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.BitstreamDecoderEncoderLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Bitstream Decode Tile Line Rowstore Buffer
    if (params->m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_bitstreamDecoderEncoderTileLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 65;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.BitstreamDecoderEncoderTileLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
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
        resourceParams.presResource = params->m_intraPredictionLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntraPredictionLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 68;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.IntraPredictionLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Intra Prediction Tile Line Rowstore Buffer
    if (params->m_intraPredictionTileLineRowstoreReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_intraPredictionTileLineRowstoreReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.IntraPredictionTileLineRowstoreReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 71;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.IntraPredictionTileLineRowstoreReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.IntraPredictionTileLineRowstoreReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Spatial Motion Vector Line Buffer
    if (m_smvlRowstoreCache.bEnabled)
    {
        cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.SpatialMotionVectorLineReadWriteBufferAddress.DW0_1.BaseAddress = m_smvlRowstoreCache.dwAddress;
    }
    else if (params->m_spatialMotionVectorLineReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_spatialMotionVectorLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SpatialMotionVectorLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 74;
        resourceParams.bIsWritable = true;

        InitMocsParams(resourceParams, &cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.SpatialMotionVectorLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Spatial Motion Vector Tile Line Buffer
    if (params->m_spatialMotionVectorCodingTileLineReadWriteBuffer != nullptr)
    {

        resourceParams.presResource = params->m_spatialMotionVectorCodingTileLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SpatialMotionVectorCodingTileLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 77;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.SpatialMotionVectorTileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.SpatialMotionVectorTileLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    //Loop Restoration Meta Tile Column Read/Write Buffer
    if (params->m_loopRestorationMetaTileColumnReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_loopRestorationMetaTileColumnReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationMetaTileColumnReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 80;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.LoopRestorationMetaTileColumnReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.LoopRestorationMetaTileColumnReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    //Deblocker Filter Control Parameters Line Read Write Buffer
    if (params->m_loopRestorationFilterTileReadWriteLineYBuffer != nullptr)
    {
        resourceParams.presResource = params->m_loopRestorationFilterTileReadWriteLineYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileReadWriteLineYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 83;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileReadWriteLineYBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.LoopRestorationFilterTileReadWriteLineYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    //Deblocker Filter Control Parameters Tile Line Read Write Buffer
    if (params->m_loopRestorationFilterTileReadWriteLineUBuffer != nullptr)
    {
        resourceParams.presResource = params->m_loopRestorationFilterTileReadWriteLineUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileReadWriteLineUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 86;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileReadWriteLineUBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.LoopRestorationFilterTileReadWriteLineUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    //Deblocker Filter Control Parameters Tile Column Read Write Buffer
    if (params->m_loopRestorationFilterTileReadWriteLineVBuffer != nullptr)
    {
        resourceParams.presResource = params->m_loopRestorationFilterTileReadWriteLineVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileReadWriteLineVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 89;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileReadWriteLineVBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.LoopRestorationFilterTileReadWriteLineVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Line Read Write Y Buffer
    if (m_dflyRowstoreCache.bEnabled)
    {
        cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockerFilterLineReadWriteYBufferAddress.DW0_1.BaseAddress = m_dflyRowstoreCache.dwAddress;
    }
    else if (params->m_deblockerFilterLineReadWriteYBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterLineReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterLineReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 92;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterLineReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Line Read Write U Buffer
    if (m_dfluRowstoreCache.bEnabled)
    {
        cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockerFilterLineReadWriteUBufferAddress.DW0_1.BaseAddress = m_dfluRowstoreCache.dwAddress;
    }
    else if (params->m_deblockerFilterLineReadWriteUBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterLineReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterLineReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 95;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterLineReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }
    // Deblocker Filter Line Read Write V Buffer
    if (m_dflvRowstoreCache.bEnabled)
    {
        cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.DeblockerFilterLineReadWriteVBufferAddress.DW0_1.BaseAddress = m_dflvRowstoreCache.dwAddress;
    }
    else if (params->m_deblockerFilterLineReadWriteVBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterLineReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterLineReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 98;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterLineReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Tile Line Read Write Y Buffer
    if (params->m_deblockerFilterTileLineReadWriteYBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterTileLineReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileLineReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 101;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterTileLineReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterTileLineReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Tile Line Read Write V Buffer
    if (params->m_deblockerFilterTileLineReadWriteVBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterTileLineReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileLineReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 104;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterTileLineReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterTileLineReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Tile Line Read Write U Buffer
    if (params->m_deblockerFilterTileLineReadWriteUBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterTileLineReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileLineReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 107;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterTileLineReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterTileLineReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Tile Column Read Write Y Buffer
    if (params->m_deblockerFilterTileColumnReadWriteYBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterTileColumnReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileColumnReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 110;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterTileColumnReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterTileColumnReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Tile Column Read Write U Buffer
    if (params->m_deblockerFilterTileColumnReadWriteUBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterTileColumnReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileColumnReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 113;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterTileColumnReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterTileColumnReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Deblocker Filter Tile Column Read Write V Buffer
    if (params->m_deblockerFilterTileColumnReadWriteVBuffer != nullptr)
    {
        resourceParams.presResource = params->m_deblockerFilterTileColumnReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DeblockerFilterTileColumnReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 116;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DeblockerFilterTileColumnReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));

        cmd.DeblockerFilterTileColumnReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DEBLOCKINGFILTER_ROWSTORE_SCRATCH_BUFFER_CODEC].Gen12_5.Index;
    }

    // Cdef Filter Line Read Write Y Buffer
    if (m_cdefRowstoreCache.bEnabled)
    {
        cmd.CdefFilterLineReadWriteBufferAddressAttributes.DW0.BaseAddressRowStoreScratchBufferCacheSelect = cmd.CdefFilterLineReadWriteBufferAddressAttributes.BASE_ADDRESS_ROW_STORE_SCRATCH_BUFFER_CACHE_SELECT_UNNAMED1;
        cmd.CdefFilterLineReadWriteBufferAddress.DW0_1.BaseAddress = m_cdefRowstoreCache.dwAddress;
    }
    else if (params->m_cdefFilterLineReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdefFilterLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 119;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CdefFilterLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdefFilterLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Cdef Filter Tile Line Read Write Y Buffer
    if (params->m_cdefFilterTileLineReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdefFilterTileLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterTileLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 128;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CdefFilterTileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdefFilterTileLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Cdef Filter Tile Line Read Write U Buffer
    if (params->m_cdefFilterTileColumnReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdefFilterTileColumnReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterTileColumnReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 137;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CdefFilterTileColumnReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdefFilterTileColumnReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Cdef Filter Tile Line Read Write V Buffer
    if (params->m_cdefFilterMetaTileLineReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdefFilterMetaTileLineReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterMetaTileLineReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 140;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CdefFilterMetaTileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdefFilterMetaTileLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Cdef Filter Tile Column Read Write Y Buffer
    if (params->m_cdefFilterMetaTileColumnReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdefFilterMetaTileColumnReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterMetaTileColumnReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 143;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CdefFilterMetaTileColumnReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdefFilterMetaTileColumnReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Cdef Filter Top Left Corner Read Write Buffer
    if (params->m_cdefFilterTopLeftCornerReadWriteBuffer != nullptr)
    {
        resourceParams.presResource = params->m_cdefFilterTopLeftCornerReadWriteBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.CdefFilterTopLeftCornerReadWriteBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 146;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.CdefFilterTopLeftCornerReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.CdefFilterTopLeftCornerReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Super-Res Tile Column Read Write Y Buffer
    if (params->m_superResTileColumnReadWriteYBuffer != nullptr)
    {
        resourceParams.presResource = params->m_superResTileColumnReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SuperResTileColumnReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 149;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.SuperResTileColumnReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.SuperResTileColumnReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Super-Res Tile Column Read Write U Buffer
    if (params->m_superResTileColumnReadWriteUBuffer != nullptr)
    {
        resourceParams.presResource = params->m_superResTileColumnReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SuperResTileColumnReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 152;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.SuperResTileColumnReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.SuperResTileColumnReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Super-Res Tile Column Read Write V Buffer
    if (params->m_superResTileColumnReadWriteVBuffer != nullptr)
    {
        resourceParams.presResource = params->m_superResTileColumnReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.SuperResTileColumnReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 155;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.SuperResTileColumnReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.SuperResTileColumnReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Loop Restoration Filter Tile Column Read Write Y Buffer
    if (params->m_loopRestorationFilterTileColumnReadWriteYBuffer != nullptr)
    {
        resourceParams.presResource = params->m_loopRestorationFilterTileColumnReadWriteYBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileColumnReadWriteYBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 158;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileColumnReadWriteYBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.LoopRestorationFilterTileColumnReadWriteYBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Loop Restoration Filter Tile Column Read Write U Buffer
    if (params->m_loopRestorationFilterTileColumnReadWriteUBuffer != nullptr)
    {
        resourceParams.presResource = params->m_loopRestorationFilterTileColumnReadWriteUBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileColumnReadWriteUBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 161;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileColumnReadWriteUBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.LoopRestorationFilterTileColumnReadWriteUBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Loop Restoration Filter Tile Column Read Write V Buffer
    if (params->m_loopRestorationFilterTileColumnReadWriteVBuffer != nullptr)
    {
        resourceParams.presResource = params->m_loopRestorationFilterTileColumnReadWriteVBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.LoopRestorationFilterTileColumnReadWriteVBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 164;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.LoopRestorationFilterTileColumnReadWriteVBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.LoopRestorationFilterTileColumnReadWriteVBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Decoded Frame Status Error Buffer
    if (params->m_decodedFrameStatusErrorBuffer != nullptr)
    {
        resourceParams.presResource = params->m_decodedFrameStatusErrorBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DecodedFrameStatusErrorBufferBaseAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 176;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DecodedFrameStatusErrorBufferBaseAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.DecodedFrameStatusErrorBufferBaseAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    // Decoded Block Data Streamout Buffer
    if (params->m_decodedBlockDataStreamoutBuffer != nullptr)
    {
        resourceParams.presResource = params->m_decodedBlockDataStreamoutBuffer;
        resourceParams.dwOffset = 0;
        resourceParams.pdwCmd = (cmd.DecodedBlockDataStreamoutBufferAddress.DW0_1.Value);
        resourceParams.dwLocationInCmd = 179;
        resourceParams.bIsWritable = true;
        InitMocsParams(resourceParams, &cmd.DecodedBlockDataStreamoutBufferAddressAttributes.DW0.Value, 1, 6);
        MHW_MI_CHK_STATUS(AddResourceToCmd(
            m_osInterface,
            cmdBuffer,
            &resourceParams));
        cmd.DecodedBlockDataStreamoutBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
    }

    if (this->m_decodeInUse == false)
    {
        MhwVdboxAvpPipeBufAddrParamsXe_Hpm* dg2Params = static_cast<MhwVdboxAvpPipeBufAddrParamsXe_Hpm*> (params);
        // Original Uncompressed Picture Source Buffer
        if (dg2Params->m_originalUncompressedPictureSourceBuffer != nullptr)
        {
            resourceParams.presResource = dg2Params->m_originalUncompressedPictureSourceBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.OriginalUncompressedPictureSourceBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 188;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.OriginalUncompressedPictureSourceBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.OriginalUncompressedPictureSourceBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }

        // Downscaled Uncompressed Picture Source Buffer
        if (dg2Params->m_downscaledUncompressedPictureSourceBuffer != nullptr)
        {
            resourceParams.presResource = dg2Params->m_downscaledUncompressedPictureSourceBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.DownscaledUncompressedPictureSourceBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 191;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.DownscaledUncompressedPictureSourceBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.DownscaledUncompressedPictureSourceBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }

        // Tile Size Streamout Buffer
        if (dg2Params->m_tileSizeStreamoutBuffer)
        {
            resourceParams.presResource = dg2Params->m_tileSizeStreamoutBuffer;
            resourceParams.dwOffset = dg2Params->m_tileSizeStreamoutBufferOffset;
            resourceParams.pdwCmd = (cmd.TileSizeStreamoutBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 194;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.TileSizeStreamoutBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.TileSizeStreamoutBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }

        // Tile Statistics Streamout Buffer
        if (dg2Params->m_tileStatisticsPakStreamoutBuffer != nullptr)
        {
            resourceParams.presResource = dg2Params->m_tileStatisticsPakStreamoutBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.TileStatisticsStreamoutBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 197;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.TileStatisticsStreamoutBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.TileStatisticsStreamoutBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }

        // CU Streamout Buffer
        if (dg2Params->m_cuStreamoutBuffer != nullptr)
        {
            resourceParams.presResource = dg2Params->m_cuStreamoutBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.CUStreamoutBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 200;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.CUStreamoutBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.CUStreamoutBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }

        // SSE Line Read / Write Buffer
        if (dg2Params->m_sseLineReadWriteBuffer != nullptr)
        {
            resourceParams.presResource = dg2Params->m_sseLineReadWriteBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.SSELineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 203;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.SSELineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.SSELineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }

        // SSE Tile Line Read/Write Buffer
        if (dg2Params->m_sseTileLineReadWriteBuffer != nullptr)
        {
            resourceParams.presResource = dg2Params->m_sseTileLineReadWriteBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.SSETileLineReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 206;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.SSETileLineReadWriteBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.SSETileLineReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }

        // PostCDEF pixels Buffer
        if (dg2Params->m_postCDEFpixelsBuffer != nullptr)
        {
            cmd.PostCDEFpixelsBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable                   = MmcEnable(dg2Params->m_postCdefSurfMmcState);
            cmd.PostCDEFpixelsBufferAddressAttributes.DW0.CompressionType                                      = MmcIsRc(dg2Params->m_postCdefSurfMmcState);
            cmd.PostCDEFpixelsBufferAddressAttributes.DW0.TileMode                                             = MosGetHWTileType(dg2Params->m_postCDEFpixelsBuffer->TileType,
                                                                                                                                        dg2Params->m_postCDEFpixelsBuffer->TileModeGMM,
                                                                                                                                        dg2Params->m_postCDEFpixelsBuffer->bGMMTileEnabled);

            resourceParams.presResource = &dg2Params->m_postCDEFpixelsBuffer->OsResource;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = (cmd.PostCDEFpixelsBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = 209;
            resourceParams.bIsWritable = true;
            InitMocsParams(resourceParams, &cmd.PostCDEFpixelsBufferAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.PostCDEFpixelsBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_UNCACHED].Gen12_5.Index;
        }
    }

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxAvpInterfaceXe_Hpm::AddAvpPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    auto paramsG12 = dynamic_cast<PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12>(params);
    MHW_MI_CHK_NULL(paramsG12);
    mhw::vdbox::avp::xe_hpm::Cmd::AVP_PIPE_MODE_SELECT_CMD   cmd;

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

    cmd.DW1.CdefOutputStreamoutEnableFlag = false;
    cmd.DW1.LoopRestorationOutputStreamoutEnableFlag = false;
    cmd.DW1.PicStatusErrorReportEnable = false;
    cmd.DW1.CodecStandardSelect = 2;
    cmd.DW1.MultiEngineMode = paramsG12->MultiEngineMode;
    cmd.DW1.PipeWorkingMode = paramsG12->PipeWorkMode;
    cmd.DW1.TileBasedEngine = paramsG12->bTileBasedReplayMode;
    cmd.DW3.PicStatusErrorReportId = false;
    cmd.DW5.PhaseIndicator = paramsG12->ucPhaseIndicator;
    if (paramsG12->bVdencEnabled)
    {
        cmd.DW1.FrameReconstructionDisable = paramsG12->frameReconDisable;
        cmd.DW1.VdencMode = true;
        cmd.DW1.TileStatsStreamoutEnable = paramsG12->bBRCEnabled;
        cmd.DW1.MotionCompMemTrackerCounterEnable = false;
        cmd.DW6.PAKFrameLevelStreamOutEnable = paramsG12->bBRCEnabled;
        cmd.DW6.MotionCompMemoryTrackerCntEnable = false;
        cmd.DW6.SourcePixelPrefetchLen = 4;
        cmd.DW6.SourcePixelPrefetchEnable = true;
        cmd.DW6.SSEEnable = false;
    }

    MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(m_osInterface, cmdBuffer, params->pBatchBuffer, &cmd, sizeof(cmd)));

    // for Gen11+, we need to add MFX wait for both KIN and VRT before and after AVP Pipemode select...
    MHW_MI_CHK_STATUS(m_miInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    return MOS_STATUS_SUCCESS;
}

    MOS_STATUS MhwVdboxAvpInterfaceXe_Hpm::AddAvpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(m_osInterface);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS resourceParams;
        mhw::vdbox::avp::xe_hpm::Cmd::AVP_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum         = MHW_VDBOX_HCP_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType    = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        // mode specific settings
        if(m_decodeInUse)
        {
            MHW_MI_CHK_NULL(params->presDataBuffer);

            resourceParams.presResource     = params->presDataBuffer;
            resourceParams.dwOffset         = params->dwDataOffset;
            resourceParams.pdwCmd           = &(cmd.AvpIndirectBitstreamObjectBaseAddress.DW0_1.Value[0]);
            resourceParams.dwLocationInCmd  = 1;
            resourceParams.dwSize           = params->dwDataSize;
            resourceParams.bIsWritable      = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;
            InitMocsParams(resourceParams, &cmd.AvpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value, 1, 6);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                m_osInterface,
                cmdBuffer,
                &resourceParams));
            cmd.AvpIndirectBitstreamObjectMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Gen12_5.Index;

            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
        }
        else
        {
            if (params->presPakBaseObjectBuffer)
            {
                resourceParams.presResource     = params->presPakBaseObjectBuffer;
                resourceParams.dwOffset         = params->dwPakBaseObjectOffset;
                resourceParams.pdwCmd           = &(cmd.AvpIndirectBitstreamObjectBaseAddress.DW0_1.Value[0]);
                resourceParams.dwLocationInCmd  = 1;
                resourceParams.dwSize           = MOS_ALIGN_FLOOR(params->dwPakBaseObjectSize, PAGE_SIZE);
                resourceParams.bIsWritable      = true;

                // upper bound of the allocated resource will be set at 3 DW apart from address location
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;
                InitMocsParams(resourceParams, &cmd.AvpIndirectBitstreamObjectMemoryAddressAttributes.DW0.Value, 1, 6);
                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    m_osInterface,
                    cmdBuffer,
                    &resourceParams));
                cmd.AvpIndirectBitstreamObjectMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                    m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Gen12_5.Index;
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
            }

            if (params->presMvObjectBuffer)
            {
                resourceParams.presResource = params->presMvObjectBuffer;
                resourceParams.dwOffset = params->dwMvObjectOffset;
                resourceParams.pdwCmd = &(cmd.AvpIndirectCuObjectBaseAddress.DW0_1.Value[0]);
                resourceParams.dwLocationInCmd = 6;
                resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwMvObjectSize, PAGE_SIZE);
                resourceParams.bIsWritable = false;

                // no upper bound for indirect CU object 
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;
                InitMocsParams(resourceParams, &cmd.AvpIndirectCuObjectMemoryAddressAttributes.DW0.Value, 1, 6);
                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    m_osInterface,
                    cmdBuffer,
                    &resourceParams));
                cmd.AvpIndirectCuObjectMemoryAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables =
                    m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC].Gen12_5.Index;
            }
        }

        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

        return eStatus;
    }
