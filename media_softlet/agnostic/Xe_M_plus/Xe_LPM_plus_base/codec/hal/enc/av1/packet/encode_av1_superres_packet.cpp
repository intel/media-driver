/*
* Copyright (c) 2021 - 2023, Intel Corporation
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
//! \file     encode_av1_superres_packet.cpp
//! \brief    Downscaling packet for super-res, it will use VE SFC to downscale raw surface.
//!

#include "encode_av1_superres_packet.h"
#include "codec_hw_next.h"

namespace encode
{
Av1SuperresPkt::Av1SuperresPkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface) : CmdPacket(task)
{
    ENCODE_FUNC_CALL();

    auto hw = dynamic_cast<CodechalHwInterfaceNext *>(hwInterface);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(hw);

    m_osInterface = hw->GetOsInterface();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_osInterface);

    m_avpItf = std::static_pointer_cast<mhw::vdbox::avp::Itf>(hwInterface->GetAvpInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_avpItf);

    m_vdencItf = std::static_pointer_cast<mhw::vdbox::vdenc::Itf>(hwInterface->GetVdencInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_vdencItf);

    m_miItf = std::static_pointer_cast<mhw::mi::Itf>(hwInterface->GetMiInterfaceNext());
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_miItf);

    m_sfcItf = hw->GetMediaSfcInterface();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_sfcItf);

    MEDIA_SFC_INTERFACE_MODE sfcMode = {};
    sfcMode.vdboxSfcEnabled          = false;
    sfcMode.veboxSfcEnabled          = true;
    m_sfcItf->Initialize(sfcMode);

    m_pipeLine = dynamic_cast<Av1VdencPipeline *>(pipeline);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_pipeLine);

    m_allocator = m_pipeLine->GetEncodeAllocator();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_allocator);

    m_featureManager = m_pipeLine->GetPacketLevelFeatureManager(Av1Pipeline::Av1Superres);
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_featureManager);

    m_basicFeature = dynamic_cast<Av1BasicFeature *>(m_featureManager->GetFeature(Av1FeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    m_superResFeature = dynamic_cast<Av1SuperRes *>(m_featureManager->GetFeature(Av1FeatureIDs::av1SuperRes));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_superResFeature);

    m_trackedBuf = m_basicFeature->m_trackedBuf;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_trackedBuf);

    m_mmcState = m_pipeLine->GetMmcState();
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_mmcState);
}

MOS_STATUS Av1SuperresPkt::Prepare()
{
    ENCODE_FUNC_CALL();

    m_basicFeature->m_ref.SetPostCdefAsEncRef(true);
    m_is10Bit      = m_basicFeature->m_is10Bit;
    m_isIntraFrame = AV1_KEY_OR_INRA_FRAME(m_basicFeature->m_av1PicParams->PicFlags.fields.frame_type);
    m_useSuperRes  = m_basicFeature->m_av1PicParams->PicFlags.fields.use_superres;
    m_widthChanged = m_prevDsWidth != m_basicFeature->m_av1PicParams->frame_width_minus1 + 1;

    ENCODE_CHK_STATUS_RETURN(PrepareRawSurface());
    ENCODE_CHK_STATUS_RETURN(PrepareRefSurface());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperresPkt::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    ENCODE_FUNC_CALL();

    bool                       switchBackContext = false;
    std::vector<PMOS_RESOURCE> resToSync;

    // Use VE-SFC to downscale raw surface
    if (m_useSuperRes)
    {
        ENCODE_CHK_STATUS_RETURN(SubmitVeSfcDownscaling(m_raw, m_rawDs));
        resToSync.push_back(&m_raw.resource->OsResource);
        resToSync.push_back(&m_rawDs.resource->OsResource);
        switchBackContext = true;
    }

    // Scale ref surfaces at transition stage
    switch (m_refDsType)
    {
    case REF_SCALE_TYPE::NO_SCALE:
        break;
    case REF_SCALE_TYPE::USE_VE_SFC:
        ENCODE_CHK_STATUS_RETURN(SubmitVeSfcDownscaling(m_ref, m_refScaled));
        ENCODE_CHK_STATUS_RETURN(SubmitVeSfcDownscaling(m_ref4x, m_ref4xScaled));
        ENCODE_CHK_STATUS_RETURN(SubmitVeSfcDownscaling(m_ref8x, m_ref8xScaled));
        resToSync.push_back(&m_ref.resource->OsResource);
        resToSync.push_back(&m_refScaled.resource->OsResource);
        resToSync.push_back(&m_ref4x.resource->OsResource);
        resToSync.push_back(&m_ref4xScaled.resource->OsResource);
        resToSync.push_back(&m_ref8x.resource->OsResource);
        resToSync.push_back(&m_ref8xScaled.resource->OsResource);
        switchBackContext = true;
        break;
    default:
        break;
    }

    m_prevDsWidth = m_basicFeature->m_av1PicParams->frame_width_minus1 + 1;

    if (switchBackContext)
    {
        m_pipeLine->ContextSwitchBack();
    }

#if MOS_MEDIASOLO_SUPPORTED
    if (!m_osInterface->bSoloInUse)
#endif
    {
        for (auto res : resToSync)
        {
            m_allocator->SyncOnResource(res, false);
        }
    }

    return MOS_STATUS_SUCCESS;
}

static MOS_STATUS SetSurfaceMMCParams(EncodeMemComp& mmcState, MOS_SURFACE& surf)
{
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcMode(&surf));
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcState(&surf));
    ENCODE_CHK_STATUS_RETURN(mmcState.SetSurfaceMmcFormat(&surf));
    surf.bIsCompressed = surf.CompressionMode != MOS_MMC_DISABLED;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperresPkt::PrepareRawSurface()
{
    ENCODE_FUNC_CALL();

    if (m_useSuperRes)
    {
        m_raw.resource        = &m_basicFeature->m_rawSurface;
        m_raw.unalignedWidth  = m_superResFeature->GetUpscaledWidth();
        m_raw.unalignedHeight = m_basicFeature->m_av1PicParams->frame_height_minus1 + 1;

        ENCODE_CHK_NULL_RETURN(m_mmcState);

        if (m_widthChanged)
        {
            if (!Mos_ResourceIsNull(&m_rawDs.resource->OsResource))
            {
                m_allocator->DestroySurface(m_rawDs.resource);
            }

            MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
            MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));

            allocParamsForBuffer2D.Type         = MOS_GFXRES_2D;
            allocParamsForBuffer2D.TileType     = MOS_TILE_Y;
            allocParamsForBuffer2D.Format       = m_raw.resource->Format;
            allocParamsForBuffer2D.dwWidth      = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, av1SuperBlockWidth);
            allocParamsForBuffer2D.dwHeight     = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, av1SuperBlockHeight);
            allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
            allocParamsForBuffer2D.pBufName     = "superResEncRawSurface";

            if (m_mmcState->IsMmcEnabled())
            {
                allocParamsForBuffer2D.CompressionMode = MOS_MMC_MC;
                allocParamsForBuffer2D.bIsCompressible = true;
            }

            m_rawDs.resource = m_allocator->AllocateSurface(allocParamsForBuffer2D, false);
            ENCODE_CHK_NULL_RETURN(m_rawDs.resource);
            ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_rawDs.resource));

            m_rawDs.unalignedWidth  = m_basicFeature->m_av1PicParams->frame_width_minus1 + 1;
            m_rawDs.unalignedHeight = m_basicFeature->m_av1PicParams->frame_height_minus1 + 1;
        }

        if (m_mmcState->IsMmcEnabled())
        {
            SetSurfaceMMCParams(*m_mmcState, *m_raw.resource);
            SetSurfaceMMCParams(*m_mmcState, *m_rawDs.resource);
        }

        m_basicFeature->m_rawSurfaceToEnc = m_rawDs.resource;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperresPkt::PrepareRefSurface()
{
    ENCODE_FUNC_CALL();

    m_basicFeature->m_ref.UpdateEncRefBufType(BufferType::postCdefReconSurface, BufferType::ds4xSurface, BufferType::ds8xSurface);
    m_refDsType = REF_SCALE_TYPE::NO_SCALE;

    if (m_widthChanged && !m_isIntraFrame)
    {
        auto scalingIdxList = m_basicFeature->m_ref.GetRefScalingIdx();
        auto scalingIdx     = scalingIdxList[0];
        auto encRef         = m_trackedBuf->GetSurface(BufferType::postCdefReconSurface, scalingIdx);
        ENCODE_CHK_NULL_RETURN(encRef);

        if (encRef->dwWidth != m_rawDs.resource->dwWidth)
        {
            auto pakRefList = m_basicFeature->m_ref.GetPakRefSurface();

            ENCODE_CHK_COND_RETURN(scalingIdxList.size() != 1 || pakRefList.size() != 1, "In super-res transition stage, only 1 reference is allowed!");

            m_ref.resource = pakRefList[0];
            ENCODE_CHK_NULL_RETURN(m_ref.resource);

            m_ref.unalignedWidth  = m_superResFeature->GetUpscaledWidth();
            m_ref.unalignedHeight = m_basicFeature->m_av1PicParams->frame_height_minus1 + 1;

            m_ref4x.resource = m_trackedBuf->GetSurface(BufferType::ds4xSurface, scalingIdx);
            ENCODE_CHK_NULL_RETURN(m_ref4x.resource);

            m_ref4x.unalignedWidth  = m_prevDsWidth >> 2;
            m_ref4x.unalignedHeight = m_rawDs.unalignedHeight >> 2;

            m_ref8x.resource = m_trackedBuf->GetSurface(BufferType::ds8xSurface, scalingIdx);
            ENCODE_CHK_NULL_RETURN(m_ref8x.resource);

            m_ref8x.unalignedWidth  = m_prevDsWidth >> 3;
            m_ref8x.unalignedHeight = m_rawDs.unalignedHeight >> 3;

            ENCODE_CHK_STATUS_RETURN(m_basicFeature->m_ref.UpdateEncRefBufType(BufferType::superResRefScaled, BufferType::superResRef4xDsScaled, BufferType::superResRef8xDsScaled));

            m_refDsType = m_is10Bit ? REF_SCALE_TYPE::USE_PAK : REF_SCALE_TYPE::USE_VE_SFC;

            ENCODE_CHK_STATUS_RETURN(AllocateScaledRefs(scalingIdx));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperresPkt::AllocateScaledRefs(uint8_t scalingIdx)
{
    ENCODE_FUNC_CALL();

    MOS_ALLOC_GFXRES_PARAMS allocParams;
    MOS_ZeroMemory(&allocParams, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
    MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));

    allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
    allocParamsForBuffer2D.TileType = MOS_TILE_Y;
    allocParamsForBuffer2D.Format   = m_is10Bit ? Format_P010 : Format_NV12;

    allocParamsForBuffer2D.dwWidth  = m_rawDs.resource->dwWidth;
    allocParamsForBuffer2D.dwHeight = m_rawDs.resource->dwHeight;
    allocParamsForBuffer2D.pBufName = "superResRefScaled";
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(BufferType::superResRefScaled, allocParamsForBuffer2D));

    allocParamsForBuffer2D.Format   = Format_NV12;  // 4x and 8x downscaled surface always use NV12
    allocParamsForBuffer2D.dwWidth  = m_basicFeature->m_4xDSSurface->dwWidth;
    allocParamsForBuffer2D.dwHeight = m_basicFeature->m_4xDSSurface->dwHeight;
    allocParamsForBuffer2D.pBufName = "superResRef4xDsScaled";
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(BufferType::superResRef4xDsScaled, allocParamsForBuffer2D));

    allocParamsForBuffer2D.dwWidth  = m_basicFeature->m_8xDSSurface->dwWidth;
    allocParamsForBuffer2D.dwHeight = m_basicFeature->m_8xDSSurface->dwHeight;
    allocParamsForBuffer2D.pBufName = "superResRef8xDsScaled";
    ENCODE_CHK_STATUS_RETURN(m_trackedBuf->RegisterParam(BufferType::superResRef8xDsScaled, allocParamsForBuffer2D));

    m_refScaled.resource = m_trackedBuf->GetSurface(BufferType::superResRefScaled, scalingIdx);
    ENCODE_CHK_NULL_RETURN(m_refScaled.resource);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_refScaled.resource));

    m_refScaled.unalignedWidth  = m_rawDs.unalignedWidth;
    m_refScaled.unalignedHeight = m_rawDs.unalignedHeight;

    m_ref4xScaled.resource = m_trackedBuf->GetSurface(BufferType::superResRef4xDsScaled, scalingIdx);
    ENCODE_CHK_NULL_RETURN(m_ref4xScaled.resource);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_ref4xScaled.resource));

    m_ref4xScaled.unalignedWidth  = m_refScaled.unalignedWidth >> 2;
    m_ref4xScaled.unalignedHeight = m_refScaled.unalignedHeight >> 2;

    m_ref8xScaled.resource = m_trackedBuf->GetSurface(BufferType::superResRef8xDsScaled, scalingIdx);
    ENCODE_CHK_NULL_RETURN(m_ref8xScaled.resource);
    ENCODE_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(m_ref8xScaled.resource));

    m_ref8xScaled.unalignedWidth  = m_refScaled.unalignedWidth >> 3;
    m_ref8xScaled.unalignedHeight = m_refScaled.unalignedHeight >> 3;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Av1SuperresPkt::SubmitVeSfcDownscaling(const SURFACE &inSurf, const SURFACE &outSurf)
{
    ENCODE_FUNC_CALL();

    VEBOX_SFC_PARAMS params = {};

    params.input.surface      = inSurf.resource;
    params.input.chromaSiting = 0;
    params.input.rcSrc        = {0, 0, (long)inSurf.unalignedWidth, (long)inSurf.unalignedHeight};
    params.input.rotation     = ROTATION_IDENTITY;

    params.output.surface      = outSurf.resource;
    params.output.chromaSiting = 0;
    params.output.rcDst        = {0, 0, (long)outSurf.unalignedWidth, (long)outSurf.unalignedHeight};

    switch (inSurf.resource->Format)
    {
        case Format_NV12:
        case Format_P010:
            params.input.colorSpace  = CSpace_Any;
            params.output.colorSpace = CSpace_Any;
            break;
        case Format_A8R8G8B8:
        case Format_A8B8G8R8:
            params.input.colorSpace  = CSpace_sRGB;
            params.output.colorSpace = CSpace_sRGB;
            break;
        default:
            params.input.colorSpace  = CSpace_Any;
            params.output.colorSpace = CSpace_Any;
            break;
    }

    ENCODE_CHK_STATUS_RETURN(m_sfcItf->Render(params));

    return MOS_STATUS_SUCCESS;
}
}  // namespace encode
