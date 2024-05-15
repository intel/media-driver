/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_hevc_vdenc_scc.cpp
//! \brief    Implemetation for hevc scc feature
//!

#include "encode_hevc_vdenc_scc.h"
#include "encode_hevc_vdenc_feature_manager.h"
#include "encode_utils.h"
#include "encode_huc_brc_update_packet.h"

namespace encode
{
    HevcVdencScc::HevcVdencScc(
        MediaFeatureManager *featureManager,
        EncodeAllocator *allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *constSettings) :
        MediaFeature(constSettings, hwInterface ? hwInterface->GetOsInterface() : nullptr)
    {
        ENCODE_FUNC_CALL();
        auto encFeatureManager = dynamic_cast<EncodeHevcVdencFeatureManager *>(featureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(encFeatureManager);
        ENCODE_CHK_NULL_NO_STATUS_RETURN(hwInterface);
        CODEC_HW_ASSERT(hwInterface->GetOsInterface());
        m_osInterface = hwInterface->GetOsInterface();

        m_basicFeature = dynamic_cast<EncodeBasicFeature *>(encFeatureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

        m_mosCtx = hwInterface->GetOsInterface()->pOsContext;
        ENCODE_CHK_NULL_NO_STATUS_RETURN(m_mosCtx);
    }

    HevcVdencScc::~HevcVdencScc()
    {
        FreeEncResources();
    }

    MOS_STATUS HevcVdencScc::Init(void *settings)
    {
        ENCODE_FUNC_CALL();

        CodechalSetting* codecSettings = (CodechalSetting*)settings;
        m_enableSCC = codecSettings->isSCCEnabled;
        m_mmcEnabled = codecSettings->isMmcEnabled;

#if (_DEBUG || _RELEASE_INTERNAL)
        // LBC only Enable should be passed from DDI, will change later when DDI is ready
        MediaUserSetting::Value outValue;
        ReadUserSetting(
            m_userSettingPtr,
            outValue,
            "HEVC VDEnc LBC Only Enable",
            MediaUserSetting::Group::Sequence,
            (PMOS_CONTEXT)m_mosCtx);
        m_enableLBCOnly = outValue.Get<bool>();
#endif

        ENCODE_CHK_STATUS_RETURN(AllocateEncResources());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencScc::Update(void *params)
    {
        ENCODE_FUNC_CALL();

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        m_enableSCC = m_enableSCC && (hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag || hevcFeature->m_hevcSeqParams->palette_mode_enabled_flag);
        // Error concealment, disable IBC if slice coding type is I type
        if (m_enableSCC && hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            for (uint32_t slcCount = 0; slcCount < hevcFeature->m_numSlices; slcCount++)
            {
                if (hevcFeature->m_hevcSliceParams[slcCount].slice_type == encodeHevcISlice)
                {
                    hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag = false;
                    break;
                }
            }
        }
        m_enableSCC = m_enableSCC && (hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag || hevcFeature->m_hevcSeqParams->palette_mode_enabled_flag);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencScc::AllocateEncResources()
    {
        ENCODE_FUNC_CALL();

        if (!m_enableSCC)
        {
            return MOS_STATUS_SUCCESS;
        }
        MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBuffer2D;
        uint32_t                alignedWidth, alignedHeight;

        // Allocate the recon not filtered surface for IBC
        // First align to LCU size 64x64
        alignedWidth  = MOS_ALIGN_CEIL(m_basicFeature->m_frameWidth, ((HevcBasicFeature *)m_basicFeature)->m_maxLCUSize);
        alignedHeight = MOS_ALIGN_CEIL(m_basicFeature->m_frameHeight, ((HevcBasicFeature *)m_basicFeature)->m_maxLCUSize);

        MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
        allocParamsForBuffer2D.TileType = MOS_TILE_Y;
        // default setting
        allocParamsForBuffer2D.Format   = Format_NV12;
        allocParamsForBuffer2D.pBufName = "Recon not Filtered Surface";
        allocParamsForBuffer2D.dwWidth  = alignedWidth;
        allocParamsForBuffer2D.dwHeight = alignedHeight;

        // The format and size is dependent on chroma format and bit depth
        if (m_basicFeature->m_bitDepth >= 12)
        {
            ENCODE_ASSERTMESSAGE("Bit depth should be smaller than 12.");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        if (HCP_CHROMA_FORMAT_YUV420 == m_basicFeature->m_chromaFormat)
        {
            if (10 == m_basicFeature->m_bitDepth)
            {
                if (m_mmcEnabled)
                {
                    allocParamsForBuffer2D.dwWidth = alignedWidth * 2;
                }
                else
                {
                    allocParamsForBuffer2D.Format = Format_P010;
                }
            }
        }
        else if (HCP_CHROMA_FORMAT_YUV444 == m_basicFeature->m_chromaFormat)
        {
            if (8 == m_basicFeature->m_bitDepth)
            {
                allocParamsForBuffer2D.Format = Format_AYUV;
                allocParamsForBuffer2D.dwWidth  = MOS_ALIGN_CEIL(alignedWidth, 512 / 4);
                allocParamsForBuffer2D.dwHeight = MOS_ALIGN_CEIL(alignedHeight * 3 / 4, 8);
            }
            else
            {
                allocParamsForBuffer2D.Format = Format_Y410;
                allocParamsForBuffer2D.dwWidth  = MOS_ALIGN_CEIL(alignedWidth, 256 / 4);
                allocParamsForBuffer2D.dwHeight = MOS_ALIGN_CEIL(alignedHeight * 3 / 2, 8);
            }
        }
        else
        {
            ENCODE_ASSERTMESSAGE("4:2:2 is not supported for SCC feature!");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            return eStatus;
        }

        if (m_mmcEnabled && IsCompressFlagNeeded())
        {
            allocParamsForBuffer2D.bIsCompressible = true;
            allocParamsForBuffer2D.CompressionMode = MOS_MMC_MC;
        }
        allocParamsForBuffer2D.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
                    m_osInterface,
                    &allocParamsForBuffer2D,
                    &m_vdencRecNotFilteredBuffer),
                "Failed to allocate Recon not filtered surface for IBC");

        // Allocate the palette mode stream out surface here
        // Checking the size now, will add once it is clarified
        return eStatus;
    }

    MOS_STATUS HevcVdencScc::FreeEncResources()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_osInterface);
        m_osInterface->pfnFreeResource(m_osInterface, &m_vdencRecNotFilteredBuffer);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcVdencScc::SetHucBrcUpdateDmem(void* hucVdencBrcUpdateDmem)
    {
        ENCODE_FUNC_CALL();

        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        VdencHevcHucBrcUpdateDmem *phucVdencBrcUpdateDmem = (VdencHevcHucBrcUpdateDmem *)hucVdencBrcUpdateDmem;
        ENCODE_CHK_NULL_RETURN(hucVdencBrcUpdateDmem);
        // SCC is in conflict with PAK only pass
        if (m_enableSCC)
        {
            phucVdencBrcUpdateDmem->ReEncodePositiveQPDeltaThr_S8 = 0;
            phucVdencBrcUpdateDmem->ReEncodeNegativeQPDeltaThr_S8 = 0;
        }

        return eStatus;
    }

    MOS_STATUS HevcVdencScc::SetRecNotFilteredID(unsigned char &slotForRecNotFiltered)
    {
        ENCODE_FUNC_CALL();
        slotForRecNotFiltered = m_slotForRecNotFiltered;
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_MODE_SELECT, HevcVdencScc)
    {
        if (m_enableSCC)
        {
            params.pakObjCmdStreamOut = false;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_PIPE_BUF_ADDR_STATE, HevcVdencScc)
    {
        ENCODE_FUNC_CALL();

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        // Set up the recon not filtered surface for IBC
        if (m_enableSCC &&
            hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            if (m_slotForRecNotFiltered >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }

            unsigned int i;

            for (i = 0; i < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; i++)
            {
                if (params.refs[i] == nullptr)
                {
                    break;
                }
            }
            if (i >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
            {
                ENCODE_ASSERTMESSAGE("Find no available slot.");
                return MOS_STATUS_INVALID_PARAMETER;
            }

            int j;
            for (j = i; j >= 0; j--)
            {
                if (j > m_slotForRecNotFiltered)
                {
                    if (j >= 1)
                    {
                        params.refs[j]         = params.refs[j - 1];
                        params.refsDsStage2[j] = params.refsDsStage2[j - 1];
                        params.refsDsStage1[j] = params.refsDsStage1[j - 1];
                    }
                }
                else if (j == m_slotForRecNotFiltered)
                {
                    params.refs[j]         = const_cast<MOS_RESOURCE *>(& m_vdencRecNotFilteredBuffer);
                    params.refsDsStage2[j] = nullptr;
                    params.refsDsStage1[j] = nullptr;
                }
                else if (j < m_slotForRecNotFiltered)
                {
                    break;
                }
            }

            params.numActiveRefL0 += 1;
            params.mmcSkipMask = (1 << m_slotForRecNotFiltered);
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_CMD2, HevcVdencScc)
    {
        ENCODE_FUNC_CALL();

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        if (hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            if (hevcFeature->m_hevcPicParams->CodingType == I_TYPE)
            {
                params.frameIdxL0Ref0 = m_slotForRecNotFiltered;
            }
            else
            {
                if (hevcFeature->m_hevcSliceParams->num_ref_idx_l0_active_minus1 == 0)
                {
                    params.frameIdxL0Ref1 = m_slotForRecNotFiltered;
                }
                else if (hevcFeature->m_hevcSliceParams->num_ref_idx_l0_active_minus1 == 1)
                {
                    params.frameIdxL0Ref2 = m_slotForRecNotFiltered;
                }
                else if (hevcFeature->m_hevcSliceParams->num_ref_idx_l0_active_minus1 == 2)
                {
                    params.frameIdxL1Ref0 = m_slotForRecNotFiltered;
                }
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIC_STATE, HevcVdencScc)
    {
        ENCODE_FUNC_CALL();

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);
        PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS hevcSeqParams = hevcFeature->m_hevcSeqParams;
        ENCODE_CHK_NULL_RETURN(hevcSeqParams);
        PCODEC_HEVC_ENCODE_PICTURE_PARAMS hevcPicParams = hevcFeature->m_hevcPicParams;
        ENCODE_CHK_NULL_RETURN(hevcPicParams);

        params.intraBoundaryFilteringDisabledFlag      = hevcSeqParams->intra_boundary_filtering_disabled_flag;
        params.motionVectorResolutionControlIdc        = (uint8_t)hevcSeqParams->motion_vector_resolution_control_idc;
        params.ppsCurrPicRefEnabledFlag                = hevcPicParams->pps_curr_pic_ref_enabled_flag;
        params.ibcMotionCompensationBufferReferenceIdc = m_slotForRecNotFiltered;
        params.ibcConfiguration                        = hevcPicParams->pps_curr_pic_ref_enabled_flag ? (m_enableLBCOnly ? 0x2 : 0x3) : 0;
        params.paletteModeEnabledFlag                  = hevcSeqParams->palette_mode_enabled_flag;
        params.paletteMaxSize                          = hevcSeqParams->palette_mode_enabled_flag ? 64 : 0;
        params.deltaPaletteMaxPredictorSize            = hevcSeqParams->palette_mode_enabled_flag ? 32 : 0;
        params.lumaBitDepthEntryMinus8                 = hevcSeqParams->bit_depth_luma_minus8;
        params.chromaBitDepthEntryMinus8               = hevcSeqParams->bit_depth_chroma_minus8;

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_PIPE_BUF_ADDR_STATE, HevcVdencScc)
    {
        ENCODE_FUNC_CALL();

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        // Set up the recon not filtered surface for IBC
        if (m_enableSCC &&
            hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            if (m_slotForRecNotFiltered >= CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC)
            {
                return MOS_STATUS_INVALID_PARAMETER;
            }
            params.presReferences[m_slotForRecNotFiltered] = const_cast<PMOS_RESOURCE>(&m_vdencRecNotFilteredBuffer);
            params.bIBCEnabled                             = true;
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_SURFACE_STATE, HevcVdencScc)
    {
        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        if (m_enableSCC && hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag
            && CODECHAL_HCP_REF_SURFACE_ID == params.surfaceStateId)
        {
            ENCODE_CHK_STATUS_RETURN(hevcFeature->m_ref.SetSlotForRecNotFiltered(const_cast<unsigned char &> (m_slotForRecNotFiltered)));

            params.mmcSkipMask = (1 << m_slotForRecNotFiltered);
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(HCP_REF_IDX_STATE, HevcVdencScc)
    {
        ENCODE_FUNC_CALL();

        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        // For IBC
        if (hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            uint8_t ucNumRefForList = 0;
            MHW_ASSERT(m_slotForRecNotFiltered < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC);
            params.numRefIdxLRefpiclistnumActiveMinus1++;
            ucNumRefForList = params.numRefIdxLRefpiclistnumActiveMinus1;

            if ((hevcFeature->m_hevcPicParams->CodingType == I_TYPE) && (hevcFeature->m_hevcSliceParams->slice_type == mhw::vdbox::hcp::hevcSliceP))
            {
                ucNumRefForList = 0;
                params.numRefIdxLRefpiclistnumActiveMinus1 = 0;
            }

            params.listEntryLxReferencePictureFrameIdRefaddr07[ucNumRefForList] = m_slotForRecNotFiltered;
            params.referencePictureTbValue[ucNumRefForList]                     = 0;
            params.longtermreference[ucNumRefForList]                           = true;
            params.fieldPicFlag[ucNumRefForList]                                = 0;
            params.bottomFieldFlag[ucNumRefForList]                             = 0;
        }
        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(VDENC_HEVC_VP9_TILE_SLICE_STATE, HevcVdencScc)
    {
        auto hevcFeature = dynamic_cast<HevcBasicFeature *>(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(hevcFeature);

        uint32_t IbcControl = 0;
        if (hevcFeature->m_hevcPicParams->pps_curr_pic_ref_enabled_flag)
        {
            IbcControl = m_enableLBCOnly ? 1 : 3;
        }
        else
        {
            IbcControl = 0;
        }
        uint32_t PaletteModeEnable  = (hevcFeature->m_hevcSeqParams->palette_mode_enabled_flag != 0) ? 1 : 0;
        uint32_t SliceQP            = hevcFeature->m_hevcPicParams->QpY + hevcFeature->m_hevcSliceParams->slice_qp_delta;
        uint32_t BitDepthLumaMinus8 = hevcFeature->m_hevcSeqParams->bit_depth_luma_minus8;
        uint8_t  TargetUsage        = hevcFeature->m_hevcSeqParams->TargetUsage;

        params.VdencHEVCVP9TileSlicePar0  = 0;
        params.ibcControl = IbcControl;

        params.paletteModeEnable          = PaletteModeEnable;
        params.VdencHEVCVP9TileSlicePar1  = 1;

        MHW_ASSERT(params.VdencHEVCVP9TileSlicePar1 < 3);
        uint32_t tableIdx;
        if (SliceQP <= 12)
        {
            tableIdx = 0;
        }
        else if (SliceQP > 12 && SliceQP <= 17)
        {
            tableIdx = 1;
        }
        else if (SliceQP > 17 && SliceQP <= 22)
        {
            tableIdx = 2;
        }
        else if (SliceQP > 22 && SliceQP <= 27)
        {
            tableIdx = 3;
        }
        else if (SliceQP > 27 && SliceQP <= 32)
        {
            tableIdx = 4;
        }
        else if (SliceQP > 32 && SliceQP <= 37)
        {
            tableIdx = 5;
        }
        else if (SliceQP > 37 && SliceQP <= 42)
        {
            tableIdx = 6;
        }
        else if (SliceQP > 42 && SliceQP <= 47)
        {
            tableIdx = 7;
        }
        else if (SliceQP > 47 && SliceQP <= 49)
        {
            tableIdx = 8;
        }
        else
        {
            tableIdx = 9;
        }

        static const uint32_t table[3][10][11] =
        {
        {
            {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 4},
            {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 4},
            {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 4},
            {16, 16, 4, 8, 10, 12, 128, 2, 1, 1, 4},
            {32, 32, 8, 4, 10, 4, 128, 2, 2, 1, 4},
            {48, 32, 12, 6, 16, 4, 128, 2, 2, 1, 4},
            {64, 64, 12, 6, 24, 1, 128, 2, 2, 1, 4},
            {96, 64, 12, 6, 24, 1, 128, 2, 3, 1, 4},
            {128, 64, 16, 12, 32, 1, 128, 2, 6, 1, 4},
            {256, 48, 24, 6, 48, 1, 128, 3, 8, 1, 4},
        },
        {
            {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 4},
            {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 4},
            {16, 16, 2, 4, 10, 16, 128, 1, 1, 1, 4},
            {16, 16, 4, 8, 10, 12, 128, 2, 1, 1, 4},
            {32, 32, 8, 4, 10, 4, 128, 2, 2, 1, 4},
            {48, 32, 12, 6, 16, 4, 128, 2, 2, 1, 4},
            {64, 63, 12, 6, 24, 1, 128, 2, 2, 1, 4},
            {96, 63, 12, 6, 24, 1, 128, 2, 3, 1, 4},
            {128, 63, 16, 12, 32, 1, 128, 2, 6, 1, 4},
            {256, 48, 24, 6, 48, 1, 128, 3, 8, 1, 4},
        },
        {
            {256, 24, 4, 4, 12, 8, 128, 2, 1, 1, 4},
            {256, 32, 4, 4, 12, 8, 128, 2, 1, 1, 4},
            {256, 32, 4, 4, 16, 8, 128, 2, 1, 1, 4},
            {256, 32, 8, 4, 16, 8, 128, 2, 1, 1, 4},
            {256, 32, 8, 4, 32, 4, 128, 3, 1, 1, 4},
            {768, 32, 8, 4, 32, 4, 128, 3, 1, 1, 4},
            {768, 128, 32, 8, 64, 1, 128, 3, 4, 1, 4},
            {768, 128, 48, 8, 128, 1, 128, 3, 12, 1, 4},
            {768, 128, 48, 8, 128, 1, 128, 3, 24, 1, 4},
            {768, 128, 64, 8, 128, 1, 128, 4, 32, 0, 4},
        },
        };

        params.VdencHEVCVP9TileSlicePar14 = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][0];
        params.VdencHEVCVP9TileSlicePar8  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][1];
        params.VdencHEVCVP9TileSlicePar6  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][2];
        params.VdencHEVCVP9TileSlicePar9  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][3];
        params.VdencHEVCVP9TileSlicePar7  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][4];
        params.VdencHEVCVP9TileSlicePar10 = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][5];

        params.VdencHEVCVP9TileSlicePar5  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][7];
        params.VdencHEVCVP9TileSlicePar2  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][8];
        params.VdencHEVCVP9TileSlicePar3  = table[params.VdencHEVCVP9TileSlicePar1][tableIdx][9];
        params.VdencHEVCVP9TileSlicePar15 = 0;

        if (BitDepthLumaMinus8 > 0 && PaletteModeEnable)
        {
            uint32_t shift = BitDepthLumaMinus8;
            params.VdencHEVCVP9TileSlicePar5 += shift;
            params.VdencHEVCVP9TileSlicePar6 <<= shift;
            params.VdencHEVCVP9TileSlicePar7 <<= shift;
            if (params.VdencHEVCVP9TileSlicePar14 >= 256)
            {
                params.VdencHEVCVP9TileSlicePar14 = 255;
            }
            params.VdencHEVCVP9TileSlicePar14 <<= shift;
        }

        params.VdencHEVCVP9TileSlicePar4 = 0;  // Default value in C model

        params.VdencHEVCVP9TileSlicePar11 = 1;   // Default in C model
        params.VdencHEVCVP9TileSlicePar12 = 72;  // Default in C model
        params.VdencHEVCVP9TileSlicePar13 = 2;   // Default in C model

        params.VdencHEVCVP9TileSlicePar16[2] = 1;              // Default in C model
        params.VdencHEVCVP9TileSlicePar16[1] = 0;              // Default in C model
        params.VdencHEVCVP9TileSlicePar16[0] = 1;              // Default in C model
                                                  // According to HW team, disable it with setting to 0 when RTL is not ready
        params.VdencHEVCVP9TileSlicePar23 = 6;  // Default in C model

        if (TargetUsage == 7)
        {
            params.VdencHEVCVP9TileSlicePar17[2] = 49;
            params.VdencHEVCVP9TileSlicePar17[1] = 49;
            params.VdencHEVCVP9TileSlicePar17[0] = 49;
        }
        else
        {
            params.VdencHEVCVP9TileSlicePar17[2] = 63;
            params.VdencHEVCVP9TileSlicePar17[1] = 63;
            params.VdencHEVCVP9TileSlicePar17[0] = 63;
        }

        return MOS_STATUS_SUCCESS;
    }
    }  // namespace encode
