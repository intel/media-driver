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
//! \file     decode_avc_picture_packet_xe3_lpm_base.cpp
//! \brief    Defines the interface for avc decode picture packet
//!
#include "decode_avc_picture_packet_xe3_lpm_base.h"

namespace decode
{
   MOS_STATUS AvcDecodePicPktXe3_Lpm_Base::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_STATUS(AvcDecodePicPkt::Init());
        DECODE_CHK_STATUS(CalculatePictureStateCommandSize());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe3_Lpm_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        SETPAR_AND_ADDCMD(MFX_PIPE_MODE_SELECT, m_mfxItf, &cmdBuffer);

        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

#ifdef _DECODE_PROCESSING_SUPPORTED
        if (m_downSamplingFeature != nullptr && m_downSamplingPkt != nullptr)
        {
            DECODE_CHK_STATUS(m_downSamplingPkt->Execute(cmdBuffer));
        }
#endif
        SETPAR_AND_ADDCMD(MFX_SURFACE_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_PIPE_BUF_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_IND_OBJ_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_BSP_BUF_BASE_ADDR_STATE, m_mfxItf, &cmdBuffer);
        if (m_avcPipeline->IsShortFormat())
        {
            SETPAR_AND_ADDCMD(MFD_AVC_DPB_STATE, m_mfxItf, &cmdBuffer);
        }
        SETPAR_AND_ADDCMD(MFD_AVC_PICID_STATE, m_mfxItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_AVC_IMG_STATE, m_mfxItf, &cmdBuffer);
        AddAllCmds_MFX_QM_STATE(&cmdBuffer);
        SETPAR_AND_ADDCMD(MFX_AVC_DIRECTMODE_STATE, m_mfxItf, &cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe3_Lpm_Base::SetSurfaceMmcState(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS &pipeBufAddrParams)
    {
        DECODE_FUNC_CALL();

        if (m_mmcState->IsMmcEnabled())
        {
            if (m_avcBasicFeature->m_deblockingEnabled)
            {
                DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(
                    &m_avcBasicFeature->m_destSurface,
                    &pipeBufAddrParams.PostDeblockSurfMmcState));
            }
            else
            {
                DECODE_CHK_STATUS(m_mmcState->GetSurfaceMmcState(
                    &m_avcBasicFeature->m_destSurface,
                    &pipeBufAddrParams.PreDeblockSurfMmcState));
            }
        }
        else
        {
            pipeBufAddrParams.PreDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
            pipeBufAddrParams.PostDeblockSurfMmcState = MOS_MEMCOMP_DISABLED;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe3_Lpm_Base::GetAvcStateCommandsDataSize(
        uint32_t  mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool      isShortFormat)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(commandsSize);
        DECODE_CHK_NULL(patchListSize);

        uint32_t maxSize =
            m_miItf->GETSIZE_MI_FLUSH_DW() +
            m_mfxItf->GETSIZE_MFX_PIPE_MODE_SELECT() +
            m_mfxItf->GETSIZE_MFX_SURFACE_STATE() +
            m_mfxItf->GETSIZE_MFX_PIPE_BUF_ADDR_STATE() +
            m_mfxItf->GETSIZE_MFX_IND_OBJ_BASE_ADDR_STATE() +
            m_miItf->GETSIZE_MI_STORE_DATA_IMM() * 2 +
            m_miItf->GETSIZE_MI_STORE_REGISTER_MEM() * 2 +
            m_miItf->GETSIZE_MI_LOAD_REGISTER_REG() * 8;

        uint32_t patchListMaxSize =
            PATCH_LIST_COMMAND(mhw::mi::Itf::MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_IND_OBJ_BASE_ADDR_STATE_CMD) +
            (2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_DATA_IMM_CMD)) +
            (2 * PATCH_LIST_COMMAND(mhw::mi::Itf::MI_STORE_REGISTER_MEM_CMD));

        //AVC Specific
        maxSize +=
            m_mfxItf->GETSIZE_MFX_BSP_BUF_BASE_ADDR_STATE() +
            m_mfxItf->GETSIZE_MFD_AVC_PICID_STATE() +
            m_mfxItf->GETSIZE_MFX_AVC_DIRECTMODE_STATE() +
            m_mfxItf->GETSIZE_MFX_AVC_DIRECTMODE_STATE() +
            m_mfxItf->GETSIZE_MFX_AVC_IMG_STATE() +
            m_mfxItf->GETSIZE_MFX_QM_STATE() * 4;

        patchListMaxSize +=
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFD_AVC_PICID_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_DIRECTMODE_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_AVC_IMG_STATE_CMD) +
            PATCH_LIST_COMMAND(mhw::vdbox::mfx::Itf::MFX_QM_STATE_CMD) * 4;
        
        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return MOS_STATUS_SUCCESS;

    }

    MOS_STATUS AvcDecodePicPktXe3_Lpm_Base::CalculatePictureStateCommandSize()
    {
        // Picture Level Commands
        DECODE_CHK_STATUS(GetAvcStateCommandsDataSize(
                CODECHAL_DECODE_MODE_AVCVLD,
                &m_pictureStatesSize,
                &m_picturePatchListSize,
                m_avcBasicFeature->m_shortFormatInUse));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AvcDecodePicPktXe3_Lpm_Base::AllocateVariableResources()
    {
        DECODE_FUNC_CALL();

        uint16_t picWidthInMB   = m_avcPicParams->pic_width_in_mbs_minus1 + 1;
        bool     isMbaffOn      = m_avcPicParams->seq_fields.mb_adaptive_frame_field_flag;
        bool     isAVCRext      = false;

        if (m_avcPicParams->bit_depth_chroma_minus8 == 2 && m_avcPicParams->bit_depth_luma_minus8 == 2)
        {
            isAVCRext = true;
        }

        auto AllocateBuffer = [&](PMOS_BUFFER &buffer, uint32_t numCLperMB, const char *bufferName) {
            if (buffer == nullptr)
            {
                buffer = m_allocator->AllocateBuffer(picWidthInMB * numCLperMB * CODECHAL_CACHELINE_SIZE,
                    bufferName,
                    resourceInternalReadWriteCache,
                    notLockableVideoMem);
                DECODE_CHK_NULL(buffer);
            }
            else
            {
                DECODE_CHK_STATUS(m_allocator->Resize(buffer, picWidthInMB * numCLperMB * CODECHAL_CACHELINE_SIZE,
                    notLockableVideoMem));
            }
            return MOS_STATUS_SUCCESS;
        };

        if (m_mfxItf->IsBsdMpcRowstoreCacheEnabled() == false)
        {
            DECODE_CHK_STATUS(AllocateBuffer(m_resBsdMpcRowStoreScratchBuffer,
                isMbaffOn ? 2 : 1, "MpcScratchBuffer"));
        }

        if (m_mfxItf->IsMprRowstoreCacheEnabled() == false)
        {
            DECODE_CHK_STATUS(AllocateBuffer(m_resMprRowStoreScratchBuffer,
                isMbaffOn ? 2 : 1, "MprScratchBuffer"));
        }

        if (m_mfxItf->IsIntraRowstoreCacheEnabled() == false)
        {
            DECODE_CHK_STATUS(AllocateBuffer(m_resMfdIntraRowStoreScratchBuffer,
                1, "MprScratchBuffer"));
        }

        if (m_mfxItf->IsDeblockingFilterRowstoreCacheEnabled() == false)
        {
            DECODE_CHK_STATUS(AllocateBuffer(m_resMfdDeblockingFilterRowStoreScratchBuffer,
                isMbaffOn ? 4 : (isAVCRext ? 3 : 2), "DeblockingScratchBuffer"));
        }

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_AVC_IMG_STATE, AvcDecodePicPktXe3_Lpm_Base)
    {
        AvcDecodePicPkt::MHW_SETPAR_F(MFX_AVC_IMG_STATE)(params);

#ifdef IGFX_MFX_INTERFACE_EXT_SUPPORT
        params.bitDepthLumaMinus8   = m_avcPicParams->bit_depth_luma_minus8;
        params.bitDepthChromaMinus8 = m_avcPicParams->bit_depth_chroma_minus8;
#endif

        return MOS_STATUS_SUCCESS;
    }

    MHW_SETPAR_DECL_SRC(MFX_SURFACE_STATE, AvcDecodePicPktXe3_Lpm_Base)
    {
        AvcDecodePicPkt::MHW_SETPAR_F(MFX_SURFACE_STATE)(params);

        uint8_t chromaType             = m_avcPicParams->seq_fields.chroma_format_idc;
        uint8_t ucBitDepthLumaMinus8   = m_avcPicParams->bit_depth_luma_minus8;
        uint8_t ucBitDepthChromaMinus8 = m_avcPicParams->bit_depth_luma_minus8;
        PMOS_SURFACE psSurface         = &m_avcBasicFeature->m_destSurface;

        if ((chromaType == avcChromaFormatMono || chromaType == avcChromaFormat420) && psSurface->Format == Format_NV12)
        {
            if (ucBitDepthLumaMinus8 == 0 && ucBitDepthChromaMinus8 == 0)
            {
                params.surfaceFormat = SURFACE_FORMAT_PLANAR4208;  // 420 8 bit
            }
        }
#ifdef IGFX_MFX_INTERFACE_EXT_SUPPORT
        else if (chromaType == avcChromaFormat420 && psSurface->Format == Format_P010)
        {
            if (ucBitDepthLumaMinus8 == 2 && ucBitDepthChromaMinus8 == 2)
            {
                params.surfaceFormat = SURFACE_FORMAT_P010;  // 420 10 bit
            }
        }
        else if (chromaType == avcChromaFormat422 && (psSurface->Format == Format_Y210 || psSurface->Format == Format_Y216))
        {
            if (ucBitDepthLumaMinus8 == 2 && ucBitDepthChromaMinus8 == 2)
            {
                params.surfaceFormat = SURFACE_FORMAT_Y216;  // 422 10 bit (upto 16 bit)
            }
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

}
