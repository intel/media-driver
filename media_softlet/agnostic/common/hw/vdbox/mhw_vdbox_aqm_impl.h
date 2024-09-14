/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     mhw_vdbox_aqm_impl.h
//! \brief    MHW VDBOX AQM interface common base
//! \details
//!

#ifndef __MHW_VDBOX_AQM_IMPL_H__
#define __MHW_VDBOX_AQM_IMPL_H__

#include "mhw_vdbox_aqm_itf.h"
#include "mhw_impl.h"

#ifdef IGFX_AQM_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_aqm_hwcmd_ext.h"
#include "mhw_vdbox_aqm_cmdpar_ext.h"
#include "mhw_vdbox_aqm_impl_ext.h"
#endif

namespace mhw
{
namespace vdbox
{
namespace aqm
{
template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _AQM_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);
#if _MEDIA_RESERVED
    _AQM_CMD_DEF_EXT(_MHW_CMD_ALL_DEF_FOR_IMPL);
#endif

public:
    MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(settings);

        size_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

        return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
    }

protected:
    using base_t = Itf;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};

    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf){};

    _MHW_SETCMD_OVERRIDE_DECL(AQM_FRAME_START)
    {
        _MHW_SETCMD_CALLBASE(AQM_FRAME_START);

#define DO_FIELDS() \
    DO_FIELD(DW1, AqmFrameStart, params.aqmFrameStart)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AQM_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(AQM_PIC_STATE);

#define DO_FIELDS()                                                           \
    DO_FIELD(DW1, FrameWidthInPixelMinus1, params.frameWidthInPixelMinus1);   \
    DO_FIELD(DW1, FrameHeightInPixelMinus1, params.FrameHeightInPixelMinus1); \
    DO_FIELD(DW2, VdaqmEnable, params.vdaqmEnable);                           \
    DO_FIELD(DW2, TileBasedEngine, params.tileBasedEngine);                   \
    DO_FIELD(DW2, LcuSize, params.lcuSize);                                   \
    DO_FIELD(DW2, Pixelbitdepth, params.pixelbitdepth);                       \
    DO_FIELD(DW2, Chromasubsampling, params.chromasubsampling);               \
    DO_FIELD(DW2, AqmMode, params.aqmMode);                                   \
    DO_FIELD(DW2, Codectype, params.codectype);                               \
    DO_FIELD(DW18, SseEnable, params.sseEnable)

#define DO_FIELDS_EXT() \
    __MHW_VDBOX_AQM_WRAPPER_EXT(AQM_PIC_STATE_IMPL_EXT)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AQM_SURFACE_STATE)
    {
        _MHW_SETCMD_CALLBASE(AQM_SURFACE_STATE);

#define DO_FIELDS()                                                                                    \
    DO_FIELD(DW1, SurfaceId, params.surfaceStateId);                                                   \
    DO_FIELD(DW1, SurfacePitchMinus1, params.pitch - 1);                                               \
    DO_FIELD(DW2, SurfaceFormat, static_cast<uint32_t>(params.surfaceFormat));                         \
    DO_FIELD(DW2, YOffsetForUCbInPixel, params.uOffset);                                               \
    DO_FIELD(DW3, YOffsetForVCr, params.vOffset);                                                      \
    DO_FIELD(DW4, CompressionTypeForSourceFrame, MmcEnabled(params.mmcStateRawSurf) ? 1 : 0);          \
    DO_FIELD(DW4, CompressionTypeForReconstructedFrame, MmcEnabled(params.mmcStateReconSurf) ? 1 : 0); \
    DO_FIELD(DW4, CompressionFormat, params.compressionFormat)
#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AQM_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(AQM_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};
        resourceParams.dwLsbNum            = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType       = MOS_MFX_PIPE_BUF_ADDR;

        if (!Mos_ResourceIsNull(params.surfaceRawBuffer))
        {
            MOS_SURFACE details = {};
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.surfaceRawBuffer, &details));

            cmd.SourcePixelsFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.mmcStateRawSurf);
            cmd.SourcePixelsFrameBufferAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.mmcStateRawSurf);
            cmd.SourcePixelsFrameBufferAddressAttributes.DW0.Tilemode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

            resourceParams.presResource    = params.surfaceRawBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.SourcePixelsFrameBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(SourcePixelsFrameBufferAddress);
            resourceParams.bIsWritable     = false;

            MOS_GPU_CONTEXT gpuContext = m_osItf->pfnGetGpuContext(m_osItf);
            m_osItf->pfnSyncOnResource(
                m_osItf,
                params.surfaceRawBuffer,
                gpuContext,
                false);

            InitMocsParams(resourceParams, &cmd.SourcePixelsFrameBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.surfaceReconBuffer))
        {
            MOS_SURFACE details = {};
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.surfaceReconBuffer, &details));

            cmd.ReconstructedPixelsFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.mmcStateReconSurf);
            cmd.ReconstructedPixelsFrameBufferAddressAttributes.DW0.CompressionType                    = MmcRcEnabled(params.mmcStateReconSurf);
            cmd.ReconstructedPixelsFrameBufferAddressAttributes.DW0.Tilemode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

            resourceParams.presResource    = params.surfaceReconBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.ReconstructedPixelsFrameBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(ReconstructedPixelsFrameBufferAddress);
            resourceParams.bIsWritable     = false;

            MOS_GPU_CONTEXT gpuContext = m_osItf->pfnGetGpuContext(m_osItf);
            m_osItf->pfnSyncOnResource(
                m_osItf,
                params.surfaceReconBuffer,
                gpuContext,
                false);

            InitMocsParams(resourceParams, &cmd.ReconstructedPixelsFrameBufferAddressAttributes.DW0.Value, 1, 6);

            MHW_CHK_STATUS_RETURN(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        typename cmd_t::SPLITBASEADDRESS64BYTEALIGNED_CMD *AqmCmdArr1[] =
            {&cmd.AQM_PIPE_BUF_ADDR_STATE_DW10, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW13, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW16, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW19, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW22};
        typename cmd_t::MEMORYADDRESSATTRIBUTES_CMD * AqmCmdArr1Attributes[] =
            {&cmd.AQM_PIPE_BUF_ADDR_STATE_DW12, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW15, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW18, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW21, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW24};
        uint32_t AqmCmdArr1DwLoaction[] =
            {_MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW10), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW13), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW16), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW16), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW22)};

        for (uint8_t depth = 0; depth < 5; depth++)
        {
            if (!Mos_ResourceIsNull(params.AqmPipeBufAddrStatePar0[depth]))
            {
                AqmCmdArr1Attributes[depth]->DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;  //TODO: change to below line when enabling GMM Cacheability for HW resources; this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_AVP_CDF_INIT_CODEC].Value;

                resourceParams.presResource    = params.AqmPipeBufAddrStatePar0[depth];
                resourceParams.dwOffset        = params.AqmPipeBufAddrStatePar1[depth];
                resourceParams.pdwCmd          = (AqmCmdArr1[depth]->DW0_1.Value);
                resourceParams.dwLocationInCmd = AqmCmdArr1DwLoaction[depth];
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        if (!Mos_ResourceIsNull(params.AqmPipeBufAddrStatePar2))
        {
            cmd.AQM_PIPE_BUF_ADDR_STATE_DW30.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;  //TODO: change to below line when enabling GMM Cacheability for HW resources; this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_AVP_CDF_INIT_CODEC].Value;

            MOS_SURFACE details = {};
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.AqmPipeBufAddrStatePar2, &details));

            cmd.AQM_PIPE_BUF_ADDR_STATE_DW30.DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.AqmPipeBufAddrStatePar3);
            cmd.AQM_PIPE_BUF_ADDR_STATE_DW30.DW0.CompressionType                    = MmcRcEnabled(params.AqmPipeBufAddrStatePar3);
            cmd.AQM_PIPE_BUF_ADDR_STATE_DW30.DW0.Tilemode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

            resourceParams.presResource    = params.AqmPipeBufAddrStatePar2;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.AQM_PIPE_BUF_ADDR_STATE_DW28.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW28);
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        typename cmd_t::SPLITBASEADDRESS64BYTEALIGNED_CMD *AqmCmdArr2[] =
            {&cmd.AQM_PIPE_BUF_ADDR_STATE_DW34, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW37, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW40, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW43, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW46};
        typename cmd_t::MEMORYADDRESSATTRIBUTES_CMD *AqmCmdArr2Attributes[] =
            {&cmd.AQM_PIPE_BUF_ADDR_STATE_DW36, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW39, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW42, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW45, &cmd.AQM_PIPE_BUF_ADDR_STATE_DW48};
        uint32_t AqmCmdArr2DwLoaction[] =
            {_MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW34), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW37), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW40), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW43), _MHW_CMD_DW_LOCATION(AQM_PIPE_BUF_ADDR_STATE_DW46)};

        for (uint8_t depth = 0; depth < 5; depth++)
        {
            if (!Mos_ResourceIsNull(params.AqmPipeBufAddrStatePar4[depth]))
            {
                AqmCmdArr2Attributes[depth]->DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;  //TODO: change to below line when enabling GMM Cacheability for HW resources; this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_AVP_CDF_INIT_CODEC].Value;

                MOS_SURFACE details = {};
                MOS_ZeroMemory(&details, sizeof(details));
                details.Format = Format_Invalid;
                MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.AqmPipeBufAddrStatePar4[depth], &details));

                AqmCmdArr2Attributes[depth]->DW0.BaseAddressMemoryCompressionEnable = MmcEnabled(params.AqmPipeBufAddrStatePar5[depth]);
                AqmCmdArr2Attributes[depth]->DW0.CompressionType                    = MmcRcEnabled(params.AqmPipeBufAddrStatePar5[depth]);
                AqmCmdArr2Attributes[depth]->DW0.Tilemode                           = GetHwTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);

                resourceParams.presResource    = params.AqmPipeBufAddrStatePar4[depth];
                resourceParams.dwOffset        = 0;
                resourceParams.pdwCmd          = (AqmCmdArr2[depth]->DW0_1.Value);
                resourceParams.dwLocationInCmd = AqmCmdArr2DwLoaction[depth];
                resourceParams.bIsWritable     = true;

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(AQM_TILE_CODING)
    {
        _MHW_SETCMD_CALLBASE(AQM_TILE_CODING);

#define DO_FIELDS()                                                               \
    DO_FIELD(DW1, FrameTileId, params.tileId);                                    \
    DO_FIELD(DW1, TileGroupId, params.tileGroupId);                               \
    DO_FIELD(DW2, TileColumnPositionInSbUnit, params.tileColPositionInSb);        \
    DO_FIELD(DW2, TileRowPositionInSbUnit, params.tileRowPositionInSb);           \
    DO_FIELD(DW3, TileWidthInSuperblockUnitMinus1, params.tileWidthInSbMinus1);   \
    DO_FIELD(DW3, TileHeightInSuperblockUnitMinus1, params.tileHeightInSbMinus1); \
    DO_FIELD(DW4, TileNumber, params.tileNum)

#include "mhw_hwcmd_process_cmdfields.h"
    }


    _MHW_SETCMD_OVERRIDE_DECL(AQM_SLICE_STATE)
    {
        _MHW_SETCMD_CALLBASE(AQM_SLICE_STATE);

#define DO_FIELDS()                                                               \
    DO_FIELD(DW1, FirstSuperSlice, params.firstSuperSlice);                       \
    DO_FIELD(DW1, MbLcuStartXPosition, params.tileSliceStartLcuMbX);              \
    DO_FIELD(DW1, MbLcuStartYPosition, params.tileSliceStartLcuMbY);              \
                                                                                  \
    DO_FIELD(DW2, NextsliceMbLcuStartXPosition, params.nextTileSliceStartLcuMbX); \
    DO_FIELD(DW2, NextsliceMbStartYPosition, params.nextTileSliceStartLcuMbY)

#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__aqm__Impl)
};
}  // namespace aqm
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AQM_IMPL_H__
