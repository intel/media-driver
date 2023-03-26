/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_mpeg2_picture_packet_xe_m_base.cpp
//! \brief    Defines the interface of mpeg2 decode picture packet for Xe_M_Base
//!

#include "codechal_utilities.h"
#include "decode_mpeg2_picture_packet_xe_m_base.h"
#include "codechal_debug.h"
#include "decode_common_feature_defs.h"

namespace decode {

    Mpeg2DecodePicPktXe_M_Base::~Mpeg2DecodePicPktXe_M_Base()
    {
        FreeResources();
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::FreeResources()
    {
        DECODE_FUNC_CALL();

        if (m_allocator != nullptr)
        {
            m_allocator->Destroy(m_resMfdDeblockingFilterRowStoreScratchBuffer);
            m_allocator->Destroy(m_resBsdMpcRowStoreScratchBuffer);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_featureManager);
        DECODE_CHK_NULL(m_hwInterface);
        DECODE_CHK_NULL(m_osInterface);
        DECODE_CHK_NULL(m_miInterface);
        DECODE_CHK_NULL(m_mpeg2Pipeline);
        DECODE_CHK_NULL(m_mfxInterface);

        m_mpeg2BasicFeature = dynamic_cast<Mpeg2BasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        DECODE_CHK_NULL(m_mpeg2BasicFeature);

        m_allocator = m_pipeline->GetDecodeAllocator();
        DECODE_CHK_NULL(m_allocator);

        DECODE_CHK_STATUS(AllocateFixedResources());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        m_mpeg2PicParams = m_mpeg2BasicFeature->m_mpeg2PicParams;
        DECODE_CHK_NULL(m_mpeg2PicParams);

#ifdef _MMC_SUPPORTED
        m_mmcState = m_mpeg2Pipeline->GetMmcState();
        DECODE_CHK_NULL(m_mmcState);
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::AllocateFixedResources()
    {
        DECODE_FUNC_CALL();

        uint16_t picWidthInMb = m_mpeg2BasicFeature->m_picWidthInMb;
        uint16_t picHeightInMb = m_mpeg2BasicFeature->m_picHeightInMb;

        // Deblocking Filter Row Store Scratch buffer
        // (Num MacroBlock Width) * (Num Cachlines) * (Cachline size)
        m_resMfdDeblockingFilterRowStoreScratchBuffer = m_allocator->AllocateBuffer(
            picWidthInMb * 7 * CODECHAL_CACHELINE_SIZE,
            "DeblockingFilterScratch",
            resourceInternalReadWriteCache,
            notLockableVideoMem);

        // MPR Row Store Scratch buffer
        // (FrameWidth in MB) * (CacheLine size per MB) * 2
        m_resBsdMpcRowStoreScratchBuffer = m_allocator->AllocateBuffer(
            ((uint32_t)(picWidthInMb * CODECHAL_CACHELINE_SIZE)) * 2,
            "MprScratchBuffer",
            resourceInternalReadWriteCache,
            notLockableVideoMem);

        return MOS_STATUS_SUCCESS;
    }

    void Mpeg2DecodePicPktXe_M_Base::SetMfxPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12& pipeModeSelectParams)
    {
        DECODE_FUNC_CALL();

        pipeModeSelectParams.Mode = m_mpeg2BasicFeature->m_mode;
        pipeModeSelectParams.bStreamOutEnabled = m_mpeg2BasicFeature->m_streamOutEnabled;
        pipeModeSelectParams.bPostDeblockOutEnable = m_mpeg2BasicFeature->m_deblockingEnabled;
        pipeModeSelectParams.bPreDeblockOutEnable = !m_mpeg2BasicFeature->m_deblockingEnabled;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::SetMfxSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& dstSurfaceParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&dstSurfaceParams, sizeof(dstSurfaceParams));
        dstSurfaceParams.Mode = m_mpeg2BasicFeature->m_mode;
        dstSurfaceParams.psSurface = &m_mpeg2BasicFeature->m_destSurface;

#ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(m_mmcState->SetSurfaceMmcState(&(m_mpeg2BasicFeature->m_destSurface)));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(dstSurfaceParams.psSurface, &dstSurfaceParams.mmcState));
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcFormat(dstSurfaceParams.psSurface, &dstSurfaceParams.dwCompressionFormat));
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::AddMfxSurfacesCmd(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_SURFACE_PARAMS dstSurfaceParams;
        DECODE_CHK_STATUS(SetMfxSurfaceParams(dstSurfaceParams));
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxSurfaceCmd(&cmdBuffer, &dstSurfaceParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::SetMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        pipeBufAddrParams.Mode = m_mpeg2BasicFeature->m_mode;

        if (m_mpeg2BasicFeature->m_deblockingEnabled)
        {
            pipeBufAddrParams.psPostDeblockSurface = &(m_mpeg2BasicFeature->m_destSurface);
        }
        else
        {
            pipeBufAddrParams.psPreDeblockSurface = &(m_mpeg2BasicFeature->m_destSurface);
        }
        pipeBufAddrParams.presMfdDeblockingFilterRowStoreScratchBuffer = &m_resMfdDeblockingFilterRowStoreScratchBuffer->OsResource;

        if (m_mpeg2BasicFeature->m_streamOutEnabled)
        {
            pipeBufAddrParams.presStreamOutBuffer = m_mpeg2BasicFeature->m_streamOutBuffer;
        }

        Mpeg2ReferenceFrames& refFrames = m_mpeg2BasicFeature->m_refFrames;

        // when there is not a forward or backward reference,
        // the index is set to the destination frame index
        pipeBufAddrParams.presReferences[CodechalDecodeFwdRefTop] =
            pipeBufAddrParams.presReferences[CodechalDecodeFwdRefBottom] =
            pipeBufAddrParams.presReferences[CodechalDecodeBwdRefTop] =
            pipeBufAddrParams.presReferences[CodechalDecodeBwdRefBottom] =
            &m_mpeg2BasicFeature->m_destSurface.OsResource;

        if (m_mpeg2BasicFeature->m_fwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            pipeBufAddrParams.presReferences[CodechalDecodeFwdRefTop] =
                pipeBufAddrParams.presReferences[CodechalDecodeFwdRefBottom] =
                &refFrames.m_refList[m_mpeg2BasicFeature->m_fwdRefIdx]->resRefPic;
        }
        if (m_mpeg2BasicFeature->m_bwdRefIdx < CODECHAL_NUM_UNCOMPRESSED_SURFACE_MPEG2)
        {
            pipeBufAddrParams.presReferences[CodechalDecodeBwdRefTop] =
                pipeBufAddrParams.presReferences[CodechalDecodeBwdRefBottom] =
                &refFrames.m_refList[m_mpeg2BasicFeature->m_bwdRefIdx]->resRefPic;
        }

        // special case for second fields
        if (m_mpeg2PicParams->m_secondField && m_mpeg2PicParams->m_pictureCodingType == P_TYPE)
        {
            if (m_mpeg2PicParams->m_topFieldFirst)
            {
                pipeBufAddrParams.presReferences[CodechalDecodeFwdRefTop] =
                    &m_mpeg2BasicFeature->m_destSurface.OsResource;
            }
            else
            {
                pipeBufAddrParams.presReferences[CodechalDecodeFwdRefBottom] =
                    &m_mpeg2BasicFeature->m_destSurface.OsResource;
            }
        }

#ifdef _MMC_SUPPORTED
        DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(pipeBufAddrParams.psPreDeblockSurface, &pipeBufAddrParams.PreDeblockSurfMmcState));
        if (m_mmcState->IsMmcEnabled())
        {
            pipeBufAddrParams.bMmcEnabled = true;
        }
#endif

        DECODE_CHK_STATUS(FixMfxPipeBufAddrParams(pipeBufAddrParams));

        CODECHAL_DEBUG_TOOL(DumpResources(pipeBufAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::FixMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        PMOS_RESOURCE dummyRef = &(m_mpeg2BasicFeature->m_dummyReference.OsResource);

        // set all ref pic addresses to valid addresses for error concealment purpose
        for (uint32_t i = 0; i < CODEC_MAX_NUM_REF_FRAME_NON_AVC; i++)
        {
            if (m_mpeg2BasicFeature->m_useDummyReference && !m_allocator->ResourceIsNull(dummyRef) &&
                pipeBufAddrParams.presReferences[i] == nullptr)
            {
                pipeBufAddrParams.presReferences[i] = dummyRef;
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    void Mpeg2DecodePicPktXe_M_Base::SetMfxIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&indObjBaseAddrParams, sizeof(indObjBaseAddrParams));
        indObjBaseAddrParams.Mode = m_mpeg2BasicFeature->m_mode;;
        indObjBaseAddrParams.dwDataSize = m_mpeg2BasicFeature->m_copiedDataBufferInUse ?
            m_mpeg2BasicFeature->m_copiedDataBufferSize :
            m_mpeg2BasicFeature->m_dataSize;
        indObjBaseAddrParams.presDataBuffer = m_mpeg2BasicFeature->m_copiedDataBufferInUse ?
            &(m_mpeg2BasicFeature->m_copiedDataBuf->OsResource) :
            &(m_mpeg2BasicFeature->m_resDataBuffer.OsResource);
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::AddMfxIndObjBaseAddrCmd(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS indObjBaseAddrParams;
        SetMfxIndObjBaseAddrParams(indObjBaseAddrParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxIndObjBaseAddrCmd(&cmdBuffer, &indObjBaseAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    void Mpeg2DecodePicPktXe_M_Base::SetMfxBspBufBaseAddrParams(MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS& bspBufBaseAddrParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&bspBufBaseAddrParams, sizeof(bspBufBaseAddrParams));
        bspBufBaseAddrParams.presBsdMpcRowStoreScratchBuffer = &m_resBsdMpcRowStoreScratchBuffer->OsResource;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::AddMfxBspBufBaseAddrCmd(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS BspBufBaseAddrParams;
        SetMfxBspBufBaseAddrParams(BspBufBaseAddrParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxBspBufBaseAddrCmd(&cmdBuffer, &BspBufBaseAddrParams));

        return MOS_STATUS_SUCCESS;
    }

    void Mpeg2DecodePicPktXe_M_Base::SetMfxMpeg2PicStateParams(MHW_VDBOX_MPEG2_PIC_STATE& mpeg2PicState)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&mpeg2PicState, sizeof(mpeg2PicState));
        mpeg2PicState.Mode = m_mpeg2BasicFeature->m_mode;
        mpeg2PicState.pMpeg2PicParams = m_mpeg2PicParams;
        mpeg2PicState.bDeblockingEnabled = m_mpeg2BasicFeature->m_deblockingEnabled;
        mpeg2PicState.dwMPEG2ISliceConcealmentMode = m_mpeg2BasicFeature->m_mpeg2ISliceConcealmentMode;
        mpeg2PicState.dwMPEG2PBSliceConcealmentMode = m_mpeg2BasicFeature->m_mpeg2PbSliceConcealmentMode;
        mpeg2PicState.dwMPEG2PBSlicePredBiDirMVTypeOverride = m_mpeg2BasicFeature->m_mpeg2PbSlicePredBiDirMvTypeOverride;
        mpeg2PicState.dwMPEG2PBSlicePredMVOverride = m_mpeg2BasicFeature->m_mpeg2PbSlicePredMvOverride;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::AddMfxMpeg2PicCmd(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_MPEG2_PIC_STATE mpeg2PicState;
        SetMfxMpeg2PicStateParams(mpeg2PicState);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxMpeg2PicCmd(&cmdBuffer, &mpeg2PicState));

        return MOS_STATUS_SUCCESS;
    }

    void Mpeg2DecodePicPktXe_M_Base::SetMfxQmParams(MHW_VDBOX_QM_PARAMS& qmParams)
    {
        DECODE_FUNC_CALL();

        MOS_ZeroMemory(&qmParams, sizeof(qmParams));
        qmParams.Standard = CODECHAL_MPEG2;
        qmParams.pMpeg2IqMatrix = m_mpeg2BasicFeature->m_mpeg2IqMatrixBuffer;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::AddMfxQmCmd(MOS_COMMAND_BUFFER& cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_QM_PARAMS qmParams;
        SetMfxQmParams(qmParams);
        DECODE_CHK_STATUS(m_mfxInterface->AddMfxQmCmd(&cmdBuffer, &qmParams));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::CalculateCommandSize(uint32_t& commandBufferSize, uint32_t& requestedPatchListSize)
    {
        DECODE_FUNC_CALL();

        commandBufferSize = m_pictureStatesSize;
        requestedPatchListSize = m_picturePatchListSize;

        return MOS_STATUS_SUCCESS;
    }

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS Mpeg2DecodePicPktXe_M_Base::DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        CodechalDebugInterface *debugInterface = m_pipeline->GetDebugInterface();
        DECODE_CHK_NULL(debugInterface);

        if (m_mpeg2PicParams->m_pictureCodingType != I_TYPE)
        {
            for (uint16_t n = 0; n <= CodechalDecodeBwdRefBottom; n++)
            {
                if (pipeBufAddrParams.presReferences[n])
                {
                    MOS_SURFACE refSurface;
                    MOS_ZeroMemory(&refSurface, sizeof(MOS_SURFACE));
                    refSurface.OsResource = *(pipeBufAddrParams.presReferences[n]);
                    DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&refSurface));

                    debugInterface->m_refIndex = n;
                    std::string refSurfName    = "RefSurf[" + std::to_string(static_cast<uint32_t>(debugInterface->m_refIndex)) + "]";
                    DECODE_CHK_STATUS(debugInterface->DumpYUVSurface(
                        &refSurface,
                        CodechalDbgAttr::attrDecodeReferenceSurfaces,
                        refSurfName.c_str()));
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }
#endif
}  // namespace decode
