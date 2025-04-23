/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

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
//! \file     codechal_decode_vc1_xe_xpm.cpp
//! \brief    Implements the decode interface extension for Xe_XPM VC1.
//! \details  Implements all functions required by CodecHal for Xe_XPM VC1 decoding.
//!

#include "codechal_decode_vc1_xe_xpm.h"

MOS_STATUS CodechalDecodeVc1Xe_Xpm::AllocateStandard(
    CodechalSetting *          settings)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_STATUS_RETURN(CodechalDecodeVc1G12::AllocateStandard(settings));

    m_olpMdfKernel = MOS_New(CodechalKernelOlpMdf);
    CODECHAL_DECODE_CHK_NULL_RETURN(m_olpMdfKernel);
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_olpMdfKernel->Init(m_osInterface));

    return eStatus;
}

MOS_STATUS CodechalDecodeVc1Xe_Xpm::PerformVc1Olp()
{
    CODECHAL_DECODE_FUNCTION_ENTER;
    CODECHAL_DECODE_CHK_NULL_RETURN(m_olpMdfKernel);

    MOS_SYNC_PARAMS syncParams;
    uint16_t        srcMemory_object_control;
    uint16_t        dstMemory_object_control;
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_videoContext;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineSignal(m_osInterface, &syncParams));

    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_renderContext;
    syncParams.presSyncResource = &m_resSyncObject;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnEngineWait(m_osInterface, &syncParams));

    // Check if destination surface needs to be synchronized, before command buffer submission
    syncParams = g_cInitSyncParams;
    syncParams.GpuContext = m_renderContext;
    syncParams.presSyncResource         = &m_deblockSurface.OsResource;
    syncParams.bReadOnly = false;
    syncParams.bDisableDecodeSyncLock = m_disableDecodeSyncLock;
    syncParams.bDisableLockForTranscode = m_disableLockForTranscode;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnPerformOverlaySync(m_osInterface, &syncParams));
    CODECHAL_DECODE_CHK_STATUS_RETURN(m_osInterface->pfnResourceWait(m_osInterface, &syncParams));

    // execute OLP kernel
    OLPFlags olpFlags;
    olpFlags.Interlace            = CodecHal_PictureIsField(m_vc1PicParams->CurrPic);
    olpFlags.HorizontalUpscaling  = m_vc1PicParams->UpsamplingFlag;
    olpFlags.VerticalUpscaling    = m_vc1PicParams->UpsamplingFlag;
    olpFlags.Profile              = m_vc1PicParams->sequence_fields.AdvancedProfileFlag;
    olpFlags.RangeExpansion       = (m_vc1PicParams->range_mapping_fields.range_mapping_enabled != 0);
    if (m_vc1PicParams->sequence_fields.AdvancedProfileFlag)
    {
        olpFlags.RangeMapUV           = m_vc1PicParams->range_mapping_fields.chroma;
        olpFlags.RangeMapUVFlag       = m_vc1PicParams->range_mapping_fields.chroma_flag;
        olpFlags.RangeMapY            = m_vc1PicParams->range_mapping_fields.luma;
        olpFlags.RangeMapYFlag        = m_vc1PicParams->range_mapping_fields.luma_flag;
    }

    srcMemory_object_control = (uint16_t)m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_PRE_DEBLOCKING_CODEC].Value;
    dstMemory_object_control = (uint16_t)m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_POST_DEBLOCKING_CODEC].Value;

    CODECHAL_DECODE_CHK_STATUS_RETURN(m_olpMdfKernel->Execute(&m_destSurface, &srcMemory_object_control, &m_deblockSurface, &dstMemory_object_control, olpFlags.value));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS CodechalDecodeVc1Xe_Xpm::HandleSkipFrame()
{
    MOS_COMMAND_BUFFER                  cmdBuffer;
    MHW_MI_FLUSH_DW_PARAMS              flushDwParams;
    MHW_GENERIC_PROLOG_PARAMS           genericPrologParams;
    MOS_SURFACE                         srcSurface;
    uint8_t                             fwdRefIdx;
    uint32_t                            surfaceSize;
    MOS_SYNC_PARAMS                     syncParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    fwdRefIdx = (uint8_t)m_vc1PicParams->ForwardRefIdx;

    MOS_ZeroMemory(&srcSurface, sizeof(MOS_SURFACE));
    srcSurface.Format = Format_NV12;
    srcSurface.OsResource = m_vc1RefList[fwdRefIdx]->resRefPic;
    CODECHAL_DECODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface, &srcSurface));

    CODECHAL_DECODE_CHK_NULL_RETURN(srcSurface.OsResource.pGmmResInfo);

    //Keep driver copy for debug purpose
    //surfaceSize = ((srcSurface.OsResource.pGmmResInfo->GetArraySize()) > 1) ?
    //              ((uint32_t)(srcSurface.OsResource.pGmmResInfo->GetQPitchPlanar(GMM_PLANE_Y) *
    //                          srcSurface.OsResource.pGmmResInfo->GetRenderPitch())) :
    //               (uint32_t)(srcSurface.OsResource.pGmmResInfo->GetSizeMainSurface());
    //CodechalDataCopyParams dataCopyParams;
    //MOS_ZeroMemory(&dataCopyParams, sizeof(CodechalDataCopyParams));
    //dataCopyParams.srcResource = &srcSurface.OsResource;
    //dataCopyParams.srcSize     = surfaceSize;
    //dataCopyParams.srcOffset = srcSurface.dwOffset;
    //dataCopyParams.dstResource = &m_destSurface.OsResource;
    //dataCopyParams.dstSize     = surfaceSize;
    //dataCopyParams.dstOffset   = m_destSurface.dwOffset;
    //CODECHAL_DECODE_CHK_STATUS_RETURN(m_hwInterface->CopyDataSourceWithDrv(&dataCopyParams));

    // HuC copy doesn't support CCS mapping. Using Vebox copy instead
    MOS_MEMCOMP_STATE mmcState = MOS_MEMCOMP_DISABLED;

#ifdef _MMC_SUPPORTED
    if (m_mmc && m_mmc->IsMmcEnabled())
    {
        CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnGetMemoryCompressionMode(m_osInterface,
            &srcSurface.OsResource,
            &mmcState));
    }
#endif

    CODECHAL_HW_CHK_STATUS_RETURN(m_osInterface->pfnDoubleBufferCopyResource(m_osInterface,
        &srcSurface.OsResource,
        &m_destSurface.OsResource,
        ((mmcState == MOS_MEMCOMP_MC) || (mmcState == MOS_MEMCOMP_RC)) ? true : false));

    return eStatus;
}

CodechalDecodeVc1Xe_Xpm::CodechalDecodeVc1Xe_Xpm(
    CodechalHwInterface   *hwInterface,
    CodechalDebugInterface* debugInterface,
    PCODECHAL_STANDARD_INFO standardInfo) :
    CodechalDecodeVc1G12(hwInterface, debugInterface, standardInfo)
{
    MOS_ZeroMemory(&m_olpInYSurface, sizeof(m_olpInYSurface));
    MOS_ZeroMemory(&m_olpInUVSurface, sizeof(m_olpInUVSurface));
    MOS_ZeroMemory(&m_olpOutYSurface, sizeof(m_olpOutYSurface));
    MOS_ZeroMemory(&m_olpOutUVSurface, sizeof(m_olpOutUVSurface));
}

CodechalDecodeVc1Xe_Xpm::~CodechalDecodeVc1Xe_Xpm()
{
    if(m_olpMdfKernel)
    {
        m_olpMdfKernel->UnInit();
        MOS_Delete(m_olpMdfKernel);
        m_olpMdfKernel = nullptr;
    }
}

