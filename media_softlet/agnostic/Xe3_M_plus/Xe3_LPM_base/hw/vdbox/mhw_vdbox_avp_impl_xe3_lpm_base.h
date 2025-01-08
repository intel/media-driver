/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     mhw_vdbox_avp_impl_xe3_lpm_base.h
//! \brief    MHW VDBOX AVP interface common base for Xe3_LPM+ platforms
//! \details
//!

#ifndef __MHW_VDBOX_AVP_IMPL_XE3_LPM_BASE_H__
#define __MHW_VDBOX_AVP_IMPL_XE3_LPM_BASE_H__

#include "mhw_vdbox_avp_impl.h"
#include "mhw_vdbox_avp_hwcmd_xe3_lpm.h"

#ifdef IGFX_AVP_INTERFACE_EXT_SUPPORT
#include "mhw_vdbox_avp_impl_xe3_lpm_base_ext.h"
#endif

namespace mhw
{
namespace vdbox
{
namespace avp
{
namespace xe3_lpm_base
{
template <typename cmd_t>
class BaseImpl : public avp::Impl<cmd_t>
{

protected:
    using base_t = avp::Impl<cmd_t>;

    BaseImpl(PMOS_INTERFACE osItf) : base_t(osItf){};

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_MODE_SELECT);

#define DO_FIELDS()                                                                            \
    DO_FIELD(DW1, TileStatisticsStreamoutEnable, params.tileStatsStreamoutEnable ? 1 : 0);     \
    DO_FIELD(DW6, DownscaledSourcePixelPrefetchLength, params.srcPixelPrefetchLen);            \
    DO_FIELD(DW6, DownscaledSourcePixelPrefetchEnable, params.srcPixelPrefetchEnable ? 1 : 0); \
    DO_FIELD(DW6, OriginalSourcePixelPrefetchLength, params.srcPixelPrefetchLen);              \
    DO_FIELD(DW6, OriginalSourcePixelPrefetchEnable, params.srcPixelPrefetchEnable ? 1 : 0)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_FILM_GRAIN_STATE)  // corresponding to AVP_CMD1
    {
        _MHW_SETCMD_CALLBASE(AVP_FILM_GRAIN_STATE);

        MOS_SecureMemcpy(cmd.PointLumaValueI0To13,
            sizeof(params.pointYValue),
            params.pointYValue,
            sizeof(params.pointYValue));

        MOS_SecureMemcpy(cmd.PointLumaScalingI0To13,
            sizeof(params.pointYScaling),
            params.pointYScaling,
            sizeof(params.pointYScaling));

        MOS_SecureMemcpy(cmd.PointCbValueI0To9,
            sizeof(params.pointCbValue),
            params.pointCbValue,
            sizeof(params.pointCbValue));

        MOS_SecureMemcpy(cmd.PointCbScalingI0To9,
            sizeof(params.pointCbScaling),
            params.pointCbScaling,
            sizeof(params.pointCbScaling));

        MOS_SecureMemcpy(cmd.PointCrValueI0To9,
            sizeof(params.pointCrValue),
            params.pointCrValue,
            sizeof(params.pointCrValue));

        MOS_SecureMemcpy(cmd.PointCrScalingI0To9,
            sizeof(params.pointCrScaling),
            params.pointCrScaling,
            sizeof(params.pointCrScaling));

        MOS_SecureMemcpy(cmd.ArCoeffLumaPlus128I023,
            sizeof(params.arCoeffsY),
            params.arCoeffsY,
            sizeof(params.arCoeffsY));

        MOS_SecureMemcpy(cmd.ArCoeffChromaCbPlus128I024,
            sizeof(params.arCoeffsCb),
            params.arCoeffsCb,
            sizeof(params.arCoeffsCb));

        MOS_SecureMemcpy(cmd.ArCoeffChromaCrPlus128I024,
            sizeof(params.arCoeffsCr),
            params.arCoeffsCr,
            sizeof(params.arCoeffsCr));

#define DO_FIELDS()                                                                                 \
        DO_FIELD(DW1, GrainRandomSeed, params.grainRandomSeed);                                     \
        DO_FIELD(DW1, ClipToRestrictedRangeFlag, params.clipToRestrictedRange);                     \
        DO_FIELD(DW1, NumberOfLumaPoints, params.numOfYPoints);                                     \
        DO_FIELD(DW1, NumberOfChromaCbPoints, params.numOfCbPoints);                                \
        DO_FIELD(DW1, NumberOfChromaCrPoints, params.numOfCrPoints);                                \
        DO_FIELD(DW1, McIdentityFlag, params.matrixCoefficients);                                   \
        DO_FIELD(DW2, GrainScalingMinus8, params.grainScalingMinus8);                               \
        DO_FIELD(DW2, ArCoeffLag, params.arCoeffLag);                                               \
        DO_FIELD(DW2, ArCoeffShiftMinus6, params.arCoeffShiftMinus6);                               \
        DO_FIELD(DW2, GrainScaleShift, params.grainScaleShift);                                     \
        DO_FIELD(DW2, ChromaScalingFromLumaFlag, params.chromaScalingFromLuma);                     \
        DO_FIELD(DW2, GrainNoiseOverlapFlag, params.grainNoiseOverlap);                             \
                                                                                                    \
        DO_FIELD(DW43, CbMult, params.cbMult);                                                      \
        DO_FIELD(DW43, CbLumaMult, params.cbLumaMult);                                              \
        DO_FIELD(DW43, CbOffset, params.cbOffset);                                                  \
        DO_FIELD(DW44, CrMult, params.crMult);                                                      \
        DO_FIELD(DW44, CrLumaMult, params.crLumaMult);                                              \
        DO_FIELD(DW44, CrOffset, params.crOffset)

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIPE_BUF_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIPE_BUF_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        resourceParams.dwLsbNum      = MHW_VDBOX_HCP_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_PIPE_BUF_ADDR;

        // Film grain related
        if (!Mos_ResourceIsNull(params.filmGrainOutputSurface))
        {
            MOS_SURFACE details = {};
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetResourceInfo(this->m_osItf, params.filmGrainOutputSurface, &details));

            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;
            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.TileMode                                             = this->GetHwTileType(params.filmGrainOutputSurface->TileType,
                params.filmGrainOutputSurface->TileModeGMM,
                params.filmGrainOutputSurface->bGMMTileEnabled);

            MOS_MEMCOMP_STATE mmcState = MOS_MEMCOMP_DISABLED;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetMemoryCompressionMode(this->m_osItf,
                params.filmGrainOutputSurface,
                &mmcState));
            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.BaseAddressMemoryCompressionEnable = this->MmcEnabled(mmcState);
            cmd.FilmGrainInjectedOutputFrameBufferAddressAttributes.DW0.CompressionType                    = this->MmcRcEnabled(mmcState);

            resourceParams.presResource    = params.filmGrainOutputSurface;
            resourceParams.dwOffset        = details.RenderOffset.YUV.Y.BaseOffset;
            resourceParams.pdwCmd          = (cmd.FilmGrainInjectedOutputFrameBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(FilmGrainInjectedOutputFrameBufferAddress);  // 21;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.filmGrainSampleTemplateBuffer))
        {
            cmd.FilmGrainSampleTemplateAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;

            resourceParams.presResource    = params.filmGrainSampleTemplateBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.FilmGrainSampleTemplateAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(FilmGrainSampleTemplateAddress);  // 59;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.lrTileColumnAlignBuffer))
        {
            cmd.LoopRestorationFilterTileColumnAlignmentReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;

            resourceParams.presResource    = params.lrTileColumnAlignBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.LoopRestorationFilterTileColumnAlignmentReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(LoopRestorationFilterTileColumnAlignmentReadWriteBufferAddress);  // 170;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.filmGrainTileColumnDataBuffer))
        {
            cmd.FilmGrainTileColumnDataReadWriteBufferAddressAttributes.DW0.BaseAddressIndexToMemoryObjectControlStateMocsTables = 0;

            resourceParams.presResource    = params.filmGrainTileColumnDataBuffer;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.FilmGrainTileColumnDataReadWriteBufferAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(FilmGrainTileColumnDataReadWriteBufferAddress);  // 173;
            resourceParams.bIsWritable     = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }
        __MHW_VDBOX_AVP_WRAPPER_EXT(AVP_PIPE_BUF_ADDR_STATE_IMPL_XE3_LPM_BASE_EXT);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(AVP_PIC_STATE)
    {
        _MHW_SETCMD_CALLBASE(AVP_PIC_STATE);

#define DO_FIELDS()                                                      \
    DO_FIELD(DW64, VDAQMenable, params.VdaqmEnable);                     \

#define DO_FIELDS_EXT()                                                  \
    __MHW_VDBOX_AVP_WRAPPER_EXT(AVP_PIC_STATE_IMPL_XE3_LPM_BASE_EXT);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    MOS_STATUS GetAvpBufSize(AvpBufferType bufferType, AvpBufferSizePar *avpBufSizeParam) override
    {
        MHW_FUNCTION_ENTER;

        if (xe3_lpm::Cmd::AVP_PIC_STATE_CMD::SEQUENCE_CHROMA_SUBSAMPLING_FORMAT_420 == avpBufSizeParam->chromaFormat)
        {
            avp::Impl<cmd_t>::GetAvpBufSize(bufferType, avpBufSizeParam);
        }
        else if (xe3_lpm::Cmd::AVP_PIC_STATE_CMD::SEQUENCE_CHROMA_SUBSAMPLING_FORMAT_444 == avpBufSizeParam->chromaFormat)
        {
            uint32_t bufferSize = 0;

            MHW_MI_CHK_STATUS(this->CalculateBufferSize(bufferType, avpBufSizeParam, avpBufferSize444, avpBufferSizeExt444, bufferSize));

            avpBufSizeParam->bufferSize = bufferSize * MHW_CACHELINE_SIZE;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid Chroma format!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS GetRowstoreCachingAddrs(mhw::vdbox::avp::AvpVdboxRowStorePar rowstoreParams) override
    {
        MHW_FUNCTION_ENTER;

        if (xe3_lpm::Cmd::AVP_PIC_STATE_CMD::SEQUENCE_CHROMA_SUBSAMPLING_FORMAT_420 == rowstoreParams.chromaFormat)
        {
            avp::Impl<cmd_t>::GetRowstoreCachingAddrs(rowstoreParams);
        }
        else if (xe3_lpm::Cmd::AVP_PIC_STATE_CMD::SEQUENCE_CHROMA_SUBSAMPLING_FORMAT_444 == rowstoreParams.chromaFormat)
        {
            // BTDL
            if (this->m_btdlRowstoreCache.supported)
            {
                this->m_btdlRowstoreCache.enabled   = true;
                this->m_btdlRowstoreCache.dwAddress = 0;
            }

            // SMVL
            if (this->m_smvlRowstoreCache.supported)
            {
                this->m_smvlRowstoreCache.enabled   = true;
                this->m_smvlRowstoreCache.dwAddress = 128;
            }

            // IPDL
            if (this->m_ipdlRowstoreCache.supported)
            {
                this->m_ipdlRowstoreCache.enabled   = true;
                this->m_ipdlRowstoreCache.dwAddress = 384;
            }

            // DFLY
            if (this->m_dflyRowstoreCache.supported)
            {
                this->m_dflyRowstoreCache.enabled   = true;
                this->m_dflyRowstoreCache.dwAddress = 768;
            }

            // DFLU
            if (this->m_dfluRowstoreCache.supported)
            {
                this->m_dfluRowstoreCache.enabled   = true;
                this->m_dfluRowstoreCache.dwAddress = 1472;
            }

            // DFLV
            if (this->m_dflvRowstoreCache.supported)
            {
                this->m_dflvRowstoreCache.enabled   = true;
                this->m_dflvRowstoreCache.dwAddress = 1792;
            }

            // CDEF, 444 disable cdef storage cache
            this->m_cdefRowstoreCache.enabled   = false;
            this->m_cdefRowstoreCache.dwAddress = 0;
        }
        else
        {
            MHW_ASSERTMESSAGE("Invalid Chroma format!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        return MOS_STATUS_SUCCESS;
    }

    // AVP internal buffer size table [buffer_index][bitdepthIdc][IsSb128x128]
    const uint8_t avpBufferSize444[avpInternalBufferMax][2][2] =
    {
        { 2  ,  8  ,  2   ,  8   }, //segmentIdBuffer,
        { 4  ,  16 ,  4   ,  16  }, //mvTemporalBuffer,
        { 2  ,  4  ,  2   ,  4   }, //bsdLineBuffer,
        { 2  ,  4  ,  2   ,  4   }, //bsdTileLineBuffer,
        { 3  ,  6  ,  6   ,  12  }, //intraPredLineBuffer,
        { 2  ,  4  ,  4   ,  8   }, //intraPredTileLineBuffer,
        { 4  ,  8  ,  4   ,  8   }, //spatialMvLineBuffer,
        { 4  ,  8  ,  4   ,  8   }, //spatialMvTileLineBuffer,
        { 1  ,  1  ,  1   ,  1   }, //lrMetaTileColBuffer,
        { 7  ,  7  ,  7   ,  7   }, //lrTileLineYBuffer,
        { 5  ,  5  ,  5   ,  5   }, //lrTileLineUBuffer,
        { 5  ,  5  ,  5   ,  5   }, //lrTileLineVBuffer,
        { 9  ,  17 ,  11  ,  21  }, //deblockLineYBuffer,
        { 4  ,  8  ,  5   ,  10  }, //deblockLineUBuffer,
        { 4  ,  8  ,  5   ,  10  }, //deblockLineVBuffer,
        { 9  ,  17 ,  11  ,  21  }, //deblockTileLineYBuffer,
        { 4  ,  8  ,  5   ,  10  }, //deblockTileLineVBuffer,
        { 4  ,  8  ,  5   ,  10  }, //deblockTileLineUBuffer,
        { 8  ,  16 ,  10  ,  20  }, //deblockTileColYBuffer,
        { 4  ,  8  ,  5   ,  9   }, //deblockTileColUBuffer,
        { 4  ,  8  ,  5   ,  9   }, //deblockTileColVBuffer,
        { 20 ,  40 ,  27  ,  54  }, //cdefLineBufferBuffer,
        { 20 ,  40 ,  27  ,  54  }, //cdefTileLineBufBuffer,
        { 20 ,  40 ,  27  ,  54  }, //cdefTileColBufBuffer,
        { 1  ,  1  ,  1   ,  1   }, //cdefMetaTileLineBuffer,
        { 1  ,  1  ,  1   ,  1   }, //cdefMetaTileColBuffer,
        { 3  ,  3  ,  3   ,  3   }, //cdefTopLeftCornerBuffer,
        { 22 ,  44 ,  29  ,  58  }, //superResTileColYBuffer,
        { 22 ,  44 ,  29  ,  58  }, //superResTileColUBuffer,
        { 22 ,  44 ,  29  ,  58  }, //superResTileColVBuffer,
        { 9  ,  17 ,  11  ,  22  }, //lrTileColYBuffer,
        { 9  ,  17 ,  11  ,  22  }, //lrTileColUBuffer,
        { 9  ,  17 ,  11  ,  22  }, //lrTileColVBuffer,
        { 0  ,  0  ,  0   ,  0   }, //frameStatusErrBuffer,
        { 0  ,  0  ,  0   ,  0   }, //dbdStreamoutBuffer,
        { 2  ,  4  ,  3   ,  5   }, //fgTileColBuffer
        { 96 ,  96 ,  192 ,  192 }, //fgSampleTmpBuffer
        { 4  ,  8  ,  5   ,  10  }, //lrTileColAlignBuffer
    };

    const uint8_t avpBufferSizeExt444[avpInternalBufferMax][2][2] =
    {
        { 0  ,  0  ,  0  ,  0  }, //segmentIdBuffer,
        { 0  ,  0  ,  0  ,  0  }, //mvTemporalBuffer,
        { 0  ,  0  ,  0  ,  0  }, //bsdLineBuffer,
        { 0  ,  0  ,  0  ,  0  }, //bsdTileLineBuffer,
        { 0  ,  0  ,  0  ,  0  }, //intraPredLineBuffer,
        { 0  ,  0  ,  0  ,  0  }, //intraPredTileLineBuffer,
        { 0  ,  0  ,  0  ,  0  }, //spatialMvLineBuffer,
        { 0  ,  0  ,  0  ,  0  }, //spatialMvTileLineBuffer,
        { 1  ,  1  ,  1  ,  1  }, //lrMetaTileColBuffer,
        { 0  ,  0  ,  0  ,  0  }, //lrTileLineYBuffer,
        { 0  ,  0  ,  0  ,  0  }, //lrTileLineUBuffer,
        { 0  ,  0  ,  0  ,  0  }, //lrTileLineVBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockLineYBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockLineUBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockLineVBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockTileLineYBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockTileLineVBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockTileLineUBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockTileColYBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockTileColUBuffer,
        { 0  ,  0  ,  0  ,  0  }, //deblockTileColVBuffer,
        { 3  ,  3  ,  4  ,  4  }, //cdefLineBuffer,
        { 3  ,  3  ,  4  ,  4  }, //cdefTileLineBuffer,
        { 3  ,  3  ,  4  ,  4  }, //cdefTileColBuffer,
        { 0  ,  0  ,  0  ,  0  }, //cdefMetaTileLineBuffer,
        { 0  ,  0  ,  0  ,  0  }, //cdefMetaTileColBuffer,
        { 0  ,  0  ,  0  ,  0  }, //cdefTopLeftCornerBuffer,
        { 22 ,  44 ,  29 ,  58 }, //superResTileColYBuffer,
        { 22 ,  44 ,  29 ,  58 }, //superResTileColUBuffer,
        { 22 ,  44 ,  29 ,  58 }, //superResTileColVBuffer,
        { 2  ,  2  ,  2  ,  2  }, //lrTileColYBuffer,
        { 2  ,  2  ,  2  ,  2  }, //lrTileColUBuffer,
        { 2  ,  2  ,  2  ,  2  }, //lrTileColVBuffer,
        { 0  ,  0  ,  0  ,  0  }, //frameStatusErrBuffer,
        { 0  ,  0  ,  0  ,  0  }, //dbdStreamoutBuffer,
        { 1  ,  1  ,  1  ,  1  }, //fgTileColBuffer,
        { 0  ,  0  ,  0  ,  0  }, //fgSampleTmpBuffer,
        { 1  ,  1  ,  1  ,  1  }, //lrTileColAlignBuffer,
    };

    MEDIA_CLASS_DEFINE_END(mhw__vdbox__avp__xe3_lpm_base__BaseImpl)
};
}  // namespace xe3_lpm_base
}  // namespace avp
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_AVP_IMPL_XE3_LPM_BASE_H__
