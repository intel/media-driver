/*
* Copyright (c) 2017-2020, Intel Corporation
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
//! \file     mhw_vdbox_mfx_g8_X.h
//! \brief    Defines functions for constructing Vdbox MFX commands on Gen8-based platforms
//!

#ifndef __MHW_VDBOX_MFX_G8_X_H__
#define __MHW_VDBOX_MFX_G8_X_H__

#include "mhw_vdbox_mfx_generic.h"
#include "mhw_mi_hwcmd_g8_X.h"
#include "mhw_mmio_g8.h"

//!  MHW Vdbox Mfx interface for Gen8
/*!
This class defines the Mfx command construction functions for Gen8 platforms as template
*/
template <class TMfxCmds>
class MhwVdboxMfxInterfaceG8 : public MhwVdboxMfxInterfaceGeneric<TMfxCmds, mhw_mi_g8_X>
{
protected:
    // Value found by experiments to allow all MBs to be converted to IPCM
    // if lower values are used as threshold not all MBs are converted to IPCM.
    static const uint32_t m_avcInterMbMaxSize = 4095; //! AVC inter macroblock max size
    static const uint32_t m_avcIntraMbMaxSize = 2700; //! AVC intra macroblock max size

    #define PATCH_LIST_COMMAND(x)  (x##_NUMBER_OF_ADDRESSES)

    enum CommandsNumberOfAddresses
    {
        // Render Engine Commands
        PIPE_CONTROL_CMD_NUMBER_OF_ADDRESSES                    =  1, //  2 DW for  1 address field
        PIPELINE_SELECT_CMD_NUMBER_OF_ADDRESSES                 =  0, //  0 DW for    address fields
        STATE_BASE_ADDRESS_CMD_NUMBER_OF_ADDRESSES              =  5, // 10 DW for  5 address fields
        MEDIA_VFE_STATE_CMD_NUMBER_OF_ADDRESSES                 =  1, //  2 DW for  1 address field
        MEDIA_CURBE_LOAD_CMD_NUMBER_OF_ADDRESSES                =  0, //  0 DW for    address fields
        MEDIA_INTERFACE_DESCRIPTOR_LOAD_CMD_NUMBER_OF_ADDRESSES =  0, //  0 DW for    address fields
        MEDIA_OBJECT_WALKER_CMD_NUMBER_OF_ADDRESSES             =  1, //  2 DW for  1 address field
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES           =  1, //  2 DW for  1 address field
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               =  1, //  2 DW for  1 address field
        MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                 =  4, //  4 DW for  2 address fields

        // MFX Engine Commands
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     =  1, //  2 DW for  1 address field
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES =  1, //  2 DW for  1 address field
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           =  1, //  2 DW for  1 address field
        MI_ATOMIC_CMD_NUMBER_OF_ADDRESSES                       =  1, //  2 DW for  1 address field
        MFX_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES            =  0, //  0 DW for    address fields
        MFX_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        MFX_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES         = 25, // 50 DW for 25 address fields
        MFX_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     =  5, // 10 DW for  5 address fields
        MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                        =  0, //  0 DW for    address fields
        MFX_BSP_BUF_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     =  3, //  2 DW for  3 address fields
        MFD_AVC_PICID_STATE_CMD_NUMBER_OF_ADDRESSES             =  0, //  0 DW for    address fields
        MFX_AVC_DIRECTMODE_STATE_CMD_NUMBER_OF_ADDRESSES        = 17, // 50 DW for 17 address fields
        MFX_AVC_IMG_STATE_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        MFX_QM_STATE_CMD_NUMBER_OF_ADDRESSES                    =  0, //  0 DW for    address fields

        MFX_FQM_STATE_CMD_NUMBER_OF_ADDRESSES                   =  0, //  0 DW for    address fields
        MFD_VC1_LONG_PIC_STATE_CMD_NUMBER_OF_ADDRESSES          =  0, //  0 DW for    address fields
        MFD_VC1_SHORT_PIC_STATE_CMD_NUMBER_OF_ADDRESSES         =  0, //  0 DW for    address fields
        MFX_VC1_PRED_PIPE_STATE_CMD_NUMBER_OF_ADDRESSES         =  0, //  0 DW for    address fields
        MFX_VC1_DIRECTMODE_STATE_CMD_NUMBER_OF_ADDRESSES        =  2, //  2 DW for  2 address fields
        MFX_MPEG2_PIC_STATE_CMD_NUMBER_OF_ADDRESSES             =  0, //  0 DW for    address fields
        MFX_DBK_OBJECT_CMD_NUMBER_OF_ADDRESSES                  =  4, //  2 DW for  4 address fields
        MFX_VP8_PIC_STATE_CMD_NUMBER_OF_ADDRESSES               =  2, //  2 DW for  2 address fields
        MFX_AVC_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES             =  0, //  0 DW for    address fields
        MFD_AVC_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              =  0, //  0 DW for    address fields
        MFD_AVC_DPB_STATE_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        MFD_AVC_SLICEADDR_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        MFX_AVC_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES           =  0, //  0 DW for    address fields
        MFX_AVC_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES      =  0, //  0 DW for    address fields
        MFC_AVC_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES       =  0, //  0 DW for    address fields
        MFD_VC1_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              =  0, //  0 DW for    address fields
        MFD_VC1_IT_OBJECT_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        MFD_MPEG2_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES            =  0, //  0 DW for    address fields
        MFD_MPEG2_IT_OBJECT_CMD_NUMBER_OF_ADDRESSES             =  0, //  0 DW for    address fields
        MFD_VP8_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES              =  0, //  0 DW for    address fields

        MFC_MPEG2_SLICEGROUP_STATE_CMD_NUMBER_OF_ADDRESSES      =  0,  //  0 DW for    address fields
        MFC_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES           =  0,  //  0 DW for    address fields
    };

protected:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxMfxInterfaceG8(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse) :
        MhwVdboxMfxInterfaceGeneric<TMfxCmds, mhw_mi_g8_X>(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;

        InitMmioRegisters();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxMfxInterfaceG8() { MHW_FUNCTION_ENTER; }

    //!
    //! \brief    Check whether interview prediction is used for MVC 
    //! 
    //! \param    [in] currPic
    //!           Current picture
    //! \param    [in] currPoc
    //!           Current POC
    //! \param    [in] refListIdx
    //!           Reference list index
    //! \param    [in] refList
    //!           Pointer to the reference list
    //!
    //! \return   bool
    //!           True if interview prediction is used, false otherwise
    //!
    bool IsMvcInterviewPred(
        const CODEC_PICTURE &currPic,
        int32_t currPoc[CODEC_NUM_FIELDS_PER_FRAME],
        uint8_t refListIdx,
        PCODEC_REF_LIST *refList)
    {
        return (((refListIdx) != (currPic).FrameIdx) &&
            (!CodecHal_PictureIsTopField(currPic) && (refList[refListIdx]->iFieldOrderCnt[1] == (currPoc)[1]) ||
                !CodecHal_PictureIsBottomField(currPic) && (refList[refListIdx]->iFieldOrderCnt[0] == (currPoc)[0])) &&
                ((currPic).FrameIdx != 0x7f));
    }

    void InitMmioRegisters()
    {
        MmioRegistersMfx *mmioRegisters = &this->m_mmioRegisters[MHW_VDBOX_NODE_1];

        mmioRegisters->generalPurposeRegister0LoOffset            = GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->generalPurposeRegister0HiOffset            = GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->generalPurposeRegister4LoOffset            = GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->generalPurposeRegister4HiOffset            = GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->generalPurposeRegister11LoOffset           = GENERAL_PURPOSE_REGISTER11_LO_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->generalPurposeRegister11HiOffset           = GENERAL_PURPOSE_REGISTER11_HI_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->generalPurposeRegister12LoOffset           = GENERAL_PURPOSE_REGISTER12_LO_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->generalPurposeRegister12HiOffset           = GENERAL_PURPOSE_REGISTER12_HI_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcImageStatusMaskRegOffset                = MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcImageStatusCtrlRegOffset                = MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcAvcNumSlicesRegOffset                   = MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcQPStatusCountOffset                     = MFC_QP_STATUS_COUNT_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxErrorFlagsRegOffset                     = MFX_ERROR_FLAG_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxFrameCrcRegOffset                       = MFX_FRAME_CRC_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxMBCountRegOffset                        = MFX_MB_COUNT_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcBitstreamBytecountFrameRegOffset        = MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset       = MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcBitstreamBytecountSliceRegOffset        = MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcVP8BitstreamBytecountFrameRegOffset     = MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcVP8ImageStatusMaskRegOffset             = MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfcVP8ImageStatusCtrlRegOffset             = MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxVP8BrcDQIndexRegOffset                  = MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxVP8BrcDLoopFilterRegOffset              = MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDQIndex01RegOffset      = MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDQIndex23RegOffset      = MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDLoopFilter01RegOffset  = MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDLoopFilter23RegOffset  = MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxVP8BrcConvergenceStatusRegOffset        = MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxLra0RegOffset                           = MFX_LRA0_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxLra1RegOffset                           = MFX_LRA1_REG_OFFSET_NODE_1_INIT_G8;
        mmioRegisters->mfxLra2RegOffset                           = MFX_LRA2_REG_OFFSET_NODE_1_INIT_G8;

        mmioRegisters = &this->m_mmioRegisters[MHW_VDBOX_NODE_2];

        mmioRegisters->generalPurposeRegister0LoOffset            = GENERAL_PURPOSE_REGISTER0_LO_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->generalPurposeRegister0HiOffset            = GENERAL_PURPOSE_REGISTER0_HI_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->generalPurposeRegister4LoOffset            = GENERAL_PURPOSE_REGISTER4_LO_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->generalPurposeRegister4HiOffset            = GENERAL_PURPOSE_REGISTER4_HI_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->generalPurposeRegister11LoOffset           = GENERAL_PURPOSE_REGISTER11_LO_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->generalPurposeRegister11HiOffset           = GENERAL_PURPOSE_REGISTER11_HI_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->generalPurposeRegister12LoOffset           = GENERAL_PURPOSE_REGISTER12_LO_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->generalPurposeRegister12HiOffset           = GENERAL_PURPOSE_REGISTER12_HI_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcImageStatusMaskRegOffset                = MFC_IMAGE_STATUS_MASK_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcImageStatusCtrlRegOffset                = MFC_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcAvcNumSlicesRegOffset                   = MFC_AVC_NUM_SLICES_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcQPStatusCountOffset                     = MFC_QP_STATUS_COUNT_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxErrorFlagsRegOffset                     = MFX_ERROR_FLAG_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxFrameCrcRegOffset                       = MFX_FRAME_CRC_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxMBCountRegOffset                        = MFX_MB_COUNT_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcBitstreamBytecountFrameRegOffset        = MFC_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcBitstreamSeBitcountFrameRegOffset       = MFC_BITSTREAM_SE_BITCOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcBitstreamBytecountSliceRegOffset        = MFC_BITSTREAM_BYTECOUNT_SLICE_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcVP8BitstreamBytecountFrameRegOffset     = MFC_VP8_BITSTREAM_BYTECOUNT_FRAME_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcVP8ImageStatusMaskRegOffset             = MFC_VP8_IMAGE_STATUS_MASK_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfcVP8ImageStatusCtrlRegOffset             = MFC_VP8_IMAGE_STATUS_CTRL_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxVP8BrcDQIndexRegOffset                  = MFX_VP8_BRC_DQ_INDEX_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxVP8BrcDLoopFilterRegOffset              = MFX_VP8_BRC_LOOP_FILTER_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDQIndex01RegOffset      = MFX_VP8_BRC_CUMULATIVE_DQ_INDEX01_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDQIndex23RegOffset      = MFX_VP8_BRC_CUMULATIVE_DQ_INDEX23_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDLoopFilter01RegOffset  = MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER01_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxVP8BrcCumulativeDLoopFilter23RegOffset  = MFX_VP8_BRC_CUMULATIVE_LOOP_FILTER23_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxVP8BrcConvergenceStatusRegOffset        = MFX_VP8_BRC_CONVERGENCE_STATUS_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxLra0RegOffset                           = MFX_LRA0_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxLra1RegOffset                           = MFX_LRA1_REG_OFFSET_NODE_2_INIT_G8;
        mmioRegisters->mfxLra2RegOffset                           = MFX_LRA2_REG_OFFSET_NODE_2_INIT_G8;
    }

    MOS_STATUS FindGpuNodeToUse(
        PMHW_VDBOX_GPUNODE_LIMIT       gpuNodeLimit)
    {
        MOS_GPU_NODE            videoGpuNode = MOS_GPU_NODE_VIDEO;
        MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

        if (MEDIA_IS_SKU(this->m_skuTable, FtrVcs2))
        {
            // BDW only
            bool setVideoNode = false;

            //If 2VDBox feature check for Content Protection, default to Master VDBox.
            if (this->m_osInterface->osCpInterface->IsCpEnabled())
            {
                setVideoNode = true;
                videoGpuNode = MOS_GPU_NODE_VIDEO;
            }

            MHW_MI_CHK_STATUS(this->m_osInterface->pfnCreateVideoNodeAssociation(
                this->m_osInterface,
                setVideoNode,
                &videoGpuNode));
        }

        gpuNodeLimit->dwGpuNodeToUse = videoGpuNode;

        return eStatus;
    }

    MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool isShortFormat)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);

        uint32_t maxSize =
            mhw_mi_g8_X::MI_FLUSH_DW_CMD::byteSize +
            TMfxCmds::MFX_PIPE_MODE_SELECT_CMD::byteSize +
            TMfxCmds::MFX_SURFACE_STATE_CMD::byteSize +
            TMfxCmds::MFX_PIPE_BUF_ADDR_STATE_CMD::byteSize +
            TMfxCmds::MFX_IND_OBJ_BASE_ADDR_STATE_CMD::byteSize +
            2 * mhw_mi_g8_X::MI_STORE_DATA_IMM_CMD::byteSize +
            2 * mhw_mi_g8_X::MI_STORE_REGISTER_MEM_CMD::byteSize;

        uint32_t patchListMaxSize =
            PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
            PATCH_LIST_COMMAND(MFX_PIPE_MODE_SELECT_CMD) +
            PATCH_LIST_COMMAND(MFX_SURFACE_STATE_CMD) +
            PATCH_LIST_COMMAND(MFX_PIPE_BUF_ADDR_STATE_CMD) +
            PATCH_LIST_COMMAND(MFX_IND_OBJ_BASE_ADDR_STATE_CMD) +
            (2 * PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD)) +
            (2 * PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD));

        uint32_t standard = CodecHal_GetStandardFromMode(mode);

        if (standard == CODECHAL_AVC)
        {
            maxSize +=
                TMfxCmds::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
                TMfxCmds::MFD_AVC_PICID_STATE_CMD::byteSize +
                TMfxCmds::MFX_AVC_DIRECTMODE_STATE_CMD::byteSize +
                TMfxCmds::MFX_AVC_IMG_STATE_CMD::byteSize +
                TMfxCmds::MFX_QM_STATE_CMD::byteSize * 4 + // QM_State sent 4 times
                TMfxCmds::MFX_FQM_STATE_CMD::byteSize * 4;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(MFD_AVC_PICID_STATE_CMD) +
                PATCH_LIST_COMMAND(MFX_AVC_DIRECTMODE_STATE_CMD) +
                PATCH_LIST_COMMAND(MFX_AVC_IMG_STATE_CMD) +
                (PATCH_LIST_COMMAND(MFX_QM_STATE_CMD) * 4) +
                (PATCH_LIST_COMMAND(MFX_FQM_STATE_CMD) * 4);

            if (mode == CODECHAL_ENCODE_MODE_AVC)
            {
                maxSize +=
                    mhw_mi_g8_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::byteSize +
                    mhw_mi_g8_X::MI_FLUSH_DW_CMD::byteSize * 3 +           // 3 extra MI_FLUSH_DWs for encode
                    TMfxCmds::MFX_FQM_STATE_CMD::byteSize * 4 +            // FQM_State sent 4 times
                    mhw_mi_g8_X::MI_STORE_REGISTER_MEM_CMD::byteSize * 8 + // 5 extra register queries for encode, 3 extra slice level commands for BrcPakStatistics
                    mhw_mi_g8_X::MI_STORE_DATA_IMM_CMD::byteSize * 3 +     // slice level commands for StatusReport, BrcPakStatistics
                    MHW_VDBOX_PAK_BITSTREAM_OVERFLOW_SIZE +                // accounting for the max DW payload for PAK_INSERT_OBJECT, for frame header payload
                    TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize * 4;     // for inserting AU, SPS, PSP, SEI headers before first slice header

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MI_CONDITIONAL_BATCH_BUFFER_END_CMD) +
                    (PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 3) +
                    (PATCH_LIST_COMMAND(MFX_FQM_STATE_CMD) * 4) +
                    (PATCH_LIST_COMMAND(MI_STORE_REGISTER_MEM_CMD) * 8) +    // 3 extra slice level commands for BrcPakStatistics
                    (PATCH_LIST_COMMAND(MI_STORE_DATA_IMM_CMD) * 3) +        // slice level commands for StatusReport, BrcPakStatistics
                    (PATCH_LIST_COMMAND(MFC_AVC_PAK_INSERT_OBJECT_CMD) * 4);
            }
            else
            {
                // this must be AVC decode case
                // add MI_BATCH_BUFFER_START_CMD for special case
                maxSize +=
                    mhw_mi_g8_X::MI_BATCH_BUFFER_START_CMD::byteSize;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD);
            }
        }
        else if (standard == CODECHAL_VC1)
        {
            maxSize +=
                mhw_mi_g8_X::MI_FLUSH_DW_CMD::byteSize +
                TMfxCmds::MFX_VC1_PRED_PIPE_STATE_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) +
                PATCH_LIST_COMMAND(MFX_VC1_PRED_PIPE_STATE_CMD);

            if (mode == CODECHAL_DECODE_MODE_VC1VLD)
            {
                maxSize +=
                    TMfxCmds::MFX_VC1_DIRECTMODE_STATE_CMD::byteSize +
                    TMfxCmds::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize;

                maxSize += isShortFormat ?
                    TMfxCmds::MFD_VC1_SHORT_PIC_STATE_CMD::byteSize :
                    TMfxCmds::MFD_VC1_LONG_PIC_STATE_CMD::byteSize;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MFX_VC1_DIRECTMODE_STATE_CMD) +
                    PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD);

                patchListMaxSize += isShortFormat ?
                    PATCH_LIST_COMMAND(MFD_VC1_SHORT_PIC_STATE_CMD) :
                    PATCH_LIST_COMMAND(MFD_VC1_LONG_PIC_STATE_CMD);
            }
            else if (mode == CODECHAL_DECODE_MODE_VC1IT)
            {
                maxSize +=
                    TMfxCmds::MFD_VC1_LONG_PIC_STATE_CMD::byteSize +
                    mhw_mi_g8_X::MI_FLUSH_DW_CMD::byteSize * 2;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MFD_VC1_LONG_PIC_STATE_CMD) +
                    (PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2);
            }
        }
        else if (standard == CODECHAL_MPEG2)
        {
            maxSize += TMfxCmds::MFX_MPEG2_PIC_STATE_CMD::byteSize;
            patchListMaxSize += PATCH_LIST_COMMAND(MFX_MPEG2_PIC_STATE_CMD);

            if (mode == CODECHAL_DECODE_MODE_MPEG2VLD)
            {
                maxSize +=
                    TMfxCmds::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
                    ((TMfxCmds::MFX_QM_STATE_CMD::byteSize + (MhwVdboxMfxInterface::m_mpeg2WeightScaleSize * sizeof(uint32_t))) * 2);

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                    (PATCH_LIST_COMMAND(MFX_QM_STATE_CMD) * 2);
            }
            else if (mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
            {
                maxSize +=
                    mhw_mi_g8_X::MI_FLUSH_DW_CMD::byteSize * 2;

                patchListMaxSize +=
                    (PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD) * 2);
            }
        }
        else if (standard == CODECHAL_VP8)
        {
            maxSize +=
                TMfxCmds::MFX_BSP_BUF_BASE_ADDR_STATE_CMD::byteSize +
                TMfxCmds::MFX_VP8_PIC_STATE_CMD::byteSize;

            patchListMaxSize +=
                PATCH_LIST_COMMAND(MFX_BSP_BUF_BASE_ADDR_STATE_CMD) +
                PATCH_LIST_COMMAND(MFX_VP8_PIC_STATE_CMD);
        }
        else if (standard == CODECHAL_JPEG)
        {
            // Adding so that error does not occur for JPEG
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported decode mode.");
            maxSize = 0;
            patchListMaxSize = 0;
            eStatus = MOS_STATUS_UNKNOWN;
        }

        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return eStatus;
    }

    MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool isModeSpecific)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(commandsSize);
        MHW_MI_CHK_NULL(patchListSize);

        uint32_t maxSize = 0, patchListMaxSize = 0;
        uint32_t standard = CodecHal_GetStandardFromMode(mode);

        if (standard == CODECHAL_AVC)
        {
            if (mode == CODECHAL_DECODE_MODE_AVCVLD)
            {
                maxSize =
                    TMfxCmds::MFX_AVC_SLICE_STATE_CMD::byteSize +
                    TMfxCmds::MFD_AVC_BSD_OBJECT_CMD::byteSize +
                    TMfxCmds::MFD_AVC_DPB_STATE_CMD::byteSize +
                    mhw_mi_g8_X::MI_FLUSH_DW_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(MFX_AVC_SLICE_STATE_CMD) +
                    PATCH_LIST_COMMAND(MFD_AVC_BSD_OBJECT_CMD) +
                    PATCH_LIST_COMMAND(MFD_AVC_DPB_STATE_CMD) +
                    PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);

                if (isModeSpecific)
                {
                    // isModeSpecific = bShortFormat for AVC decode
                    maxSize +=
                        TMfxCmds::MFD_AVC_DPB_STATE_CMD::byteSize +
                        TMfxCmds::MFD_AVC_SLICEADDR_CMD::byteSize;

                    patchListMaxSize +=
                        PATCH_LIST_COMMAND(MFD_AVC_DPB_STATE_CMD) +
                        PATCH_LIST_COMMAND(MFD_AVC_SLICEADDR_CMD);
                }
                else
                {
                    maxSize +=
                        (2 * TMfxCmds::MFX_AVC_REF_IDX_STATE_CMD::byteSize) +
                        (2 * TMfxCmds::MFX_AVC_WEIGHTOFFSET_STATE_CMD::byteSize);

                    patchListMaxSize +=
                        (2 * PATCH_LIST_COMMAND(MFX_AVC_REF_IDX_STATE_CMD)) +
                        (2 * PATCH_LIST_COMMAND(MFX_AVC_WEIGHTOFFSET_STATE_CMD));
                }
            }
            else // CODECHAL_ENCODE_MODE_AVC
            {
                // 1 PAK_INSERT_OBJECT inserted for every end of frame/stream with 1 DW payload
                maxSize = TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize + sizeof(uint32_t);
                patchListMaxSize = PATCH_LIST_COMMAND(MFC_AVC_PAK_INSERT_OBJECT_CMD);

                if (isModeSpecific)
                {
                    // isModeSpecific = bSingleTaskPhaseSupported for AVC encode
                    maxSize += (2 * mhw_mi_g8_X::MI_BATCH_BUFFER_START_CMD::byteSize);
                    patchListMaxSize += (2 * PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD));
                }
                else
                {
                    maxSize +=
                        (2 * TMfxCmds::MFX_AVC_REF_IDX_STATE_CMD::byteSize) +
                        (2 * TMfxCmds::MFX_AVC_WEIGHTOFFSET_STATE_CMD::byteSize) +
                        TMfxCmds::MFX_AVC_SLICE_STATE_CMD::byteSize +
                        MHW_VDBOX_PAK_SLICE_HEADER_OVERFLOW_SIZE + // slice header payload
                        (2 * TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize) +
                        mhw_mi_g8_X::MI_BATCH_BUFFER_START_CMD::byteSize +
                        mhw_mi_g8_X::MI_FLUSH_DW_CMD::byteSize;

                    patchListMaxSize +=
                        (2 * PATCH_LIST_COMMAND(MFX_AVC_REF_IDX_STATE_CMD)) +
                        (2 * PATCH_LIST_COMMAND(MFX_AVC_WEIGHTOFFSET_STATE_CMD)) +
                        PATCH_LIST_COMMAND(MFX_AVC_SLICE_STATE_CMD) +
                        (2 * PATCH_LIST_COMMAND(MFC_AVC_PAK_INSERT_OBJECT_CMD)) +
                        PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD) +
                        PATCH_LIST_COMMAND(MI_FLUSH_DW_CMD);
                }
            }
        }
        else if (standard == CODECHAL_VC1)
        {
            if (mode == CODECHAL_DECODE_MODE_VC1VLD)
            {
                maxSize =
                    TMfxCmds::MFD_VC1_BSD_OBJECT_CMD::byteSize;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MFD_VC1_BSD_OBJECT_CMD);
            }
            else if (mode == CODECHAL_DECODE_MODE_VC1IT)
            {
                maxSize = TMfxCmds::MFD_IT_OBJECT_CMD::byteSize +
                          TMfxCmds::MFD_IT_OBJECT_VC1_INLINE_DATA_CMD::byteSize;

                patchListMaxSize +=
                    PATCH_LIST_COMMAND(MFD_VC1_IT_OBJECT_CMD);
            }
        }
        else if (standard == CODECHAL_MPEG2)
        {
            if (mode == CODECHAL_DECODE_MODE_MPEG2VLD)
            {
                maxSize =
                    TMfxCmds::MFD_MPEG2_BSD_OBJECT_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(MFD_MPEG2_BSD_OBJECT_CMD);

            }
            else if (mode == CODECHAL_DECODE_MODE_MPEG2IDCT)
            {
                maxSize = TMfxCmds::MFD_IT_OBJECT_CMD::byteSize +
                          TMfxCmds::MFD_IT_OBJECT_MPEG2_INLINE_DATA_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(MFD_MPEG2_IT_OBJECT_CMD);
            }
            else if (mode == CODECHAL_ENCODE_MODE_MPEG2)
            {
                maxSize =
                    TMfxCmds::MFC_MPEG2_SLICEGROUP_STATE_CMD::byteSize +
                    TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::byteSize +
                    mhw_mi_g8_X::MI_BATCH_BUFFER_START_CMD::byteSize;

                patchListMaxSize =
                    PATCH_LIST_COMMAND(MFC_MPEG2_SLICEGROUP_STATE_CMD) +
                    PATCH_LIST_COMMAND(MFC_PAK_INSERT_OBJECT_CMD) +
                    PATCH_LIST_COMMAND(MI_BATCH_BUFFER_START_CMD);
            }
            else
            {
                MHW_ASSERTMESSAGE("Unsupported decode/encode mode.");
                eStatus = MOS_STATUS_UNKNOWN;
            }
        }
        else if (standard == CODECHAL_VP8)
        {
            maxSize =
                TMfxCmds::MFD_VP8_BSD_OBJECT_CMD::byteSize;

            patchListMaxSize =
                PATCH_LIST_COMMAND(MFD_VP8_BSD_OBJECT_CMD);
        }
        else
        {
            MHW_ASSERTMESSAGE("Unsupported decode mode.");
            eStatus = MOS_STATUS_UNKNOWN;
        }

        *commandsSize = maxSize;
        *patchListSize = patchListMaxSize;

        return eStatus;
    }

    MOS_STATUS AddMfxPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        typename TMfxCmds::MFX_PIPE_MODE_SELECT_CMD cmd;

        MHW_MI_CHK_STATUS(this->m_cpInterface->SetProtectionSettingsForMfxPipeModeSelect((uint32_t *)&cmd));

        cmd.DW1.StreamOutEnable = params->bStreamOutEnabled;
        cmd.DW1.DeblockerStreamOutEnable = params->bDeblockerStreamOutEnable;

        if (this->m_decodeInUse)
        {
            cmd.DW1.PreDeblockingOutputEnablePredeblockoutenable = params->bPreDeblockOutEnable;
            cmd.DW1.PostDeblockingOutputEnablePostdeblockoutenable = params->bPostDeblockOutEnable;
            cmd.DW1.CodecSelect = MhwVdboxMfxInterface::decoderCodec;
            cmd.DW1.DecoderShortFormatMode = !params->bShortFormatInUse;

            if (CodecHalIsDecodeModeVLD(params->Mode))
            {
                cmd.DW1.DecoderModeSelect = MhwVdboxMfxInterface::mfxDecoderModeVld;
            }
            else if (CodecHalIsDecodeModeIT(params->Mode))
            {
                cmd.DW1.DecoderModeSelect = MhwVdboxMfxInterface::mfxDecoderModeIt;
            }
        }
        else
        {
            cmd.DW1.PostDeblockingOutputEnablePostdeblockoutenable = params->bPostDeblockOutEnable;
            cmd.DW1.PreDeblockingOutputEnablePredeblockoutenable = params->bPreDeblockOutEnable;
            cmd.DW1.CodecSelect = MhwVdboxMfxInterface::encoderCodec;
            cmd.DW1.DecoderShortFormatMode = !params->bShortFormatInUse;  // This bit is set to be long format in order for HW to not change next slice X and Y position in encoder mode
        }

        cmd.DW1.StandardSelect = CodecHal_GetStandardFromMode(params->Mode);

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxSurfaceCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->psSurface);
        MHW_ASSERT(params->Mode != CODECHAL_UNSUPPORTED_MODE);

        typename TMfxCmds::MFX_SURFACE_STATE_CMD cmd;

        cmd.DW1.SurfaceId = params->ucSurfaceStateId;

        cmd.DW2.Height = params->psSurface->dwHeight - 1;
        cmd.DW2.Width = params->psSurface->dwWidth - 1;

        cmd.DW3.TileWalk = TMfxCmds::MFX_SURFACE_STATE_CMD::TILE_WALK_YMAJOR;
        cmd.DW3.TiledSurface = 1;
        cmd.DW3.InterleaveChroma = 1;
        cmd.DW3.SurfacePitch = params->psSurface->dwPitch - 1;
        cmd.DW3.SurfaceFormat = this->MosToMediaStateFormat(params->psSurface->Format);

        cmd.DW4.YOffsetForUCb =
            MOS_ALIGN_CEIL(params->psSurface->UPlaneOffset.iYOffset, MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY);

        if (params->Mode == CODECHAL_DECODE_MODE_JPEG)
        {
            // this parameter must always be 0 for JPEG regardless of the YUV format
            cmd.DW3.InterleaveChroma = 0;

            // Separate function for JPEG decode because this surface format should match with that programmed
            // in JPEG Picture State
            cmd.DW3.SurfaceFormat = this->GetJpegDecodeFormat(params->psSurface->Format);
        }

        if (this->IsVPlanePresent(params->psSurface->Format))
        {
            cmd.DW5.YOffsetForVCr =
                MOS_ALIGN_CEIL(params->psSurface->VPlaneOffset.iYOffset, MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY);
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_MFX_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_INDIRECT_OBJ_BASE_ADDR;

        typename TMfxCmds::MFX_IND_OBJ_BASE_ADDR_STATE_CMD cmd;

        // mode specific settings
        if (CodecHalIsDecodeModeVLD(params->Mode) || (params->Mode == CODECHAL_ENCODE_MODE_VP8))
        {
            MHW_MI_CHK_NULL(params->presDataBuffer);

            cmd.DW3.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_BITSTREAM_OBJECT_DECODE].Value;

            resourceParams.presResource = params->presDataBuffer;
            resourceParams.dwOffset = params->dwDataOffset;
            resourceParams.pdwCmd = &(cmd.DW1.Value);
            resourceParams.dwLocationInCmd = 1;
            resourceParams.dwSize = params->dwDataSize;
            resourceParams.bIsWritable = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }
        else if (CodecHalIsDecodeModeIT(params->Mode))
        {
            MHW_MI_CHK_NULL(params->presDataBuffer);

            cmd.DW13.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFD_INDIRECT_IT_COEF_OBJECT_DECODE].Value;

            resourceParams.presResource = params->presDataBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = &(cmd.DW11.Value);
            resourceParams.dwLocationInCmd = 11;
            resourceParams.dwSize = params->dwDataSize;
            resourceParams.bIsWritable = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presMvObjectBuffer)
        {
            cmd.DW8.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFX_INDIRECT_MV_OBJECT_CODEC].Value;

            resourceParams.presResource = params->presMvObjectBuffer;
            resourceParams.dwOffset = params->dwMvObjectOffset;
            resourceParams.pdwCmd = &(cmd.DW6.Value);
            resourceParams.dwLocationInCmd = 6;
            resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwMvObjectSize, 0x1000);
            resourceParams.bIsWritable = false;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        if (params->presPakBaseObjectBuffer)
        {
            cmd.DW23.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_MFC_INDIRECT_PAKBASE_OBJECT_CODEC].Value;

            resourceParams.presResource = params->presPakBaseObjectBuffer;
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = &(cmd.DW21.Value);
            resourceParams.dwLocationInCmd = 21;
            resourceParams.dwSize = MOS_ALIGN_CEIL(params->dwPakBaseObjectSize, 0x1000);
            resourceParams.bIsWritable = true;

            // upper bound of the allocated resource will be set at 3 DW apart from address location
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxDecodeAvcImgCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MOS_UNUSED(batchBuffer);

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pAvcPicParams);

        auto avcPicParams = params->pAvcPicParams;

        typename TMfxCmds::MFX_AVC_IMG_STATE_CMD cmd;

        uint32_t numMBs =
            (avcPicParams->pic_height_in_mbs_minus1 + 1) *
            (avcPicParams->pic_width_in_mbs_minus1 + 1);

        cmd.DW1.FrameSize = numMBs;
        cmd.DW2.FrameHeight = avcPicParams->pic_height_in_mbs_minus1;
        cmd.DW2.FrameWidth = avcPicParams->pic_width_in_mbs_minus1;
        cmd.DW3.SecondChromaQpOffset = avcPicParams->second_chroma_qp_index_offset;
        cmd.DW3.FirstChromaQpOffset = avcPicParams->chroma_qp_index_offset;
        cmd.DW3.WeightedPredFlag = avcPicParams->pic_fields.weighted_pred_flag;
        cmd.DW3.WeightedBipredIdc = avcPicParams->pic_fields.weighted_bipred_idc;
        cmd.DW3.ImgstructImageStructureImgStructure10 = ((avcPicParams->CurrPic.PicFlags == PICTURE_FRAME) ?
            MhwVdboxMfxInterface::avcFrame : (CodecHal_PictureIsTopField(avcPicParams->CurrPic) ?
                MhwVdboxMfxInterface::avcTopField : MhwVdboxMfxInterface::avcBottomField));
        cmd.DW4.Chromaformatidc = avcPicParams->seq_fields.chroma_format_idc;
        cmd.DW4.Entropycodingflag = avcPicParams->pic_fields.entropy_coding_mode_flag;
        cmd.DW4.Imgdisposableflag = !avcPicParams->pic_fields.reference_pic_flag;
        cmd.DW4.Constrainedipredflag = avcPicParams->pic_fields.constrained_intra_pred_flag;
        cmd.DW4.Direct8X8Infflag = avcPicParams->seq_fields.direct_8x8_inference_flag;
        cmd.DW4.Transform8X8Flag = avcPicParams->pic_fields.transform_8x8_mode_flag;
        cmd.DW4.Framembonlyflag = avcPicParams->seq_fields.frame_mbs_only_flag;
        cmd.DW4.Mbaffflameflag =
            avcPicParams->seq_fields.mb_adaptive_frame_field_flag && !avcPicParams->pic_fields.field_pic_flag;
        cmd.DW4.Fieldpicflag = avcPicParams->pic_fields.field_pic_flag;

        cmd.DW5.TrellisQuantizationRoundingTqr = 0;
        cmd.DW5.TrellisQuantizationChromaDisableTqchromadisable = true;

        cmd.DW13.CurrentPictureHasPerformedMmco5 = 0;
        cmd.DW13.NumberOfReferenceFrames = params->ucActiveFrameCnt;
        // These fields to be NumRefIdx(L0 or L1) Active - 1 at picture level
        // But hardware expects the data without the minus 1 to keep this field consistent with
        // NumRefIdxL0 and NumRefIdxL1 which appears at slice level. Thus the addition of 1
        cmd.DW13.NumberOfActiveReferencePicturesFromL0 = avcPicParams->num_ref_idx_l0_active_minus1 + 1;
        cmd.DW13.NumberOfActiveReferencePicturesFromL1 = avcPicParams->num_ref_idx_l1_active_minus1 + 1;

        cmd.DW13.InitialQpValue = avcPicParams->pic_init_qp_minus26;
        cmd.DW14.Log2MaxFrameNumMinus4 = avcPicParams->seq_fields.log2_max_frame_num_minus4;
        cmd.DW14.Log2MaxPicOrderCntLsbMinus4 = avcPicParams->seq_fields.log2_max_pic_order_cnt_lsb_minus4;
        cmd.DW14.DeblockingFilterControlPresentFlag = avcPicParams->pic_fields.deblocking_filter_control_present_flag;
        cmd.DW14.NumSliceGroupsMinus1 = avcPicParams->num_slice_groups_minus1;
        cmd.DW14.RedundantPicCntPresentFlag = avcPicParams->pic_fields.redundant_pic_cnt_present_flag;
        cmd.DW14.PicOrderPresentFlag = avcPicParams->pic_fields.pic_order_present_flag;
        cmd.DW14.SliceGroupMapType = avcPicParams->slice_group_map_type;
        cmd.DW14.PicOrderCntType = avcPicParams->seq_fields.pic_order_cnt_type;
        cmd.DW14.DeltaPicOrderAlwaysZeroFlag = avcPicParams->seq_fields.delta_pic_order_always_zero_flag;
        cmd.DW15.CurrPicFrameNum = avcPicParams->frame_num;
        cmd.DW15.SliceGroupChangeRate = avcPicParams->slice_group_change_rate_minus1;

        auto mvcExtPicParams = params->pMvcExtPicParams;
        if (mvcExtPicParams)
        {
            cmd.DW16.CurrentFrameViewId = mvcExtPicParams->CurrViewID;
            cmd.DW16.MaxViewIdxl0 = mvcExtPicParams->NumInterViewRefsL0;
            cmd.DW16.MaxViewIdxl1 = mvcExtPicParams->NumInterViewRefsL1;
            cmd.DW16.InterViewOrderDisable = 0;
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxEncodeAvcImgCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);
        MHW_MI_CHK_NULL(params->pEncodeAvcSeqParams);
        MHW_MI_CHK_NULL(params->pEncodeAvcPicParams);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        auto avcSeqParams = params->pEncodeAvcSeqParams;
        auto avcPicParams = params->pEncodeAvcPicParams;

        typename TMfxCmds::MFX_AVC_IMG_STATE_CMD cmd;

        uint32_t numMBs = params->wPicWidthInMb * params->wPicHeightInMb;

        cmd.DW1.FrameSize = (numMBs > 65535) ? 65535 : numMBs;

        cmd.DW2.FrameHeight = params->wPicHeightInMb - 1;
        cmd.DW2.FrameWidth = params->wPicWidthInMb - 1;

        cmd.DW3.ImgstructImageStructureImgStructure10 = (CodecHal_PictureIsFrame(avcPicParams->CurrOriginalPic) ?
            MhwVdboxMfxInterface::avcFrame : (CodecHal_PictureIsTopField(avcPicParams->CurrOriginalPic) ?
                MhwVdboxMfxInterface::avcTopField : MhwVdboxMfxInterface::avcBottomField));

        cmd.DW3.WeightedBipredIdc = avcPicParams->weighted_bipred_idc;
        cmd.DW3.WeightedPredFlag = avcPicParams->weighted_pred_flag;

        cmd.DW3.FirstChromaQpOffset = avcPicParams->chroma_qp_index_offset;
        cmd.DW3.SecondChromaQpOffset = avcPicParams->second_chroma_qp_index_offset;

        cmd.DW4.Fieldpicflag = CodecHal_PictureIsField(avcPicParams->CurrOriginalPic);
        cmd.DW4.Mbaffflameflag = avcSeqParams->mb_adaptive_frame_field_flag;
        cmd.DW4.Framembonlyflag = avcSeqParams->frame_mbs_only_flag;
        cmd.DW4.Transform8X8Flag = avcPicParams->transform_8x8_mode_flag;
        cmd.DW4.Direct8X8Infflag = avcSeqParams->direct_8x8_inference_flag;
        cmd.DW4.Constrainedipredflag = avcPicParams->constrained_intra_pred_flag;
        cmd.DW4.Entropycodingflag = avcPicParams->entropy_coding_mode_flag;
        cmd.DW4.Chromaformatidc = avcSeqParams->chroma_format_idc;

        cmd.DW4.Mbmvformatflag = 1;
        cmd.DW4.Mvunpackedflag = 1;

        cmd.DW4.Loadslicepointerflag = 0;
        cmd.DW4.Mbstatenabled = 0; // Disable for the first pass
        if (params->dwMaxFrameSize > 0 && params->pDeltaQp && params->currPass)
        {
            cmd.DW4.Mbstatenabled = 1;
        }
        cmd.DW4.Minframewsize = 0;

        cmd.DW5.IntrambmaxbitflagIntrambmaxsizereportmask = 1;
        cmd.DW5.IntermbmaxbitflagIntermbmaxsizereportmask = 1;
        cmd.DW5.FrameszoverflagFramebitratemaxreportmask = 1;
        cmd.DW5.FrameszunderflagFramebitrateminreportmask = 1;
        cmd.DW5.IntraIntermbipcmflagForceipcmcontrolmask = 1;
        cmd.DW5.MbratectrlflagMbLevelRateControlEnablingFlag = 0;
        cmd.DW5.Nonfirstpassflag = 0;
        cmd.DW5.TrellisQuantizationChromaDisableTqchromadisable = true;

        if (params->dwMaxFrameSize && params->currPass)
        {
            cmd.DW5.Nonfirstpassflag = 1;
        }

        if (params->dwTqEnabled && cmd.DW4.Entropycodingflag)
        {
            cmd.DW5.TrellisQuantizationEnabledTqenb = params->dwTqEnabled;
            cmd.DW5.TrellisQuantizationRoundingTqr = params->dwTqRounding;

        }
        else
        {
            cmd.DW5.TrellisQuantizationEnabledTqenb = cmd.DW5.TrellisQuantizationRoundingTqr = 0;
        }

        //DW6
        cmd.DW6.Intrambmaxsz = m_avcIntraMbMaxSize;
        cmd.DW6.Intermbmaxsz = m_avcInterMbMaxSize;

        //DW8
        cmd.DW8.Slicedeltaqppmax0 =
            cmd.DW8.Slicedeltaqpmax1 =
            cmd.DW8.Slicedeltaqpmax2 =
            cmd.DW8.Slicedeltaqpmax3 = 0;

        //DW9
        cmd.DW9.Slicedeltaqpmin0 =
            cmd.DW9.Slicedeltaqpmin1 =
            cmd.DW9.Slicedeltaqpmin2 =
            cmd.DW9.Slicedeltaqpmin3 = 0;

        //DW10
        MHW_VDBOX_AVC_IMG_BITRATE_PARAMS bitrateParams;
        this->CalcAvcImgStateMinMaxBitrate(bitrateParams);
        cmd.DW10.Framebitratemin = bitrateParams.frameBitRateMin;
        cmd.DW10.Framebitrateminunit = bitrateParams.frameBitRateMinUnit;
        cmd.DW10.Framebitrateminunitmode = bitrateParams.frameBitRateMinUnitMode;
        cmd.DW10.Framebitratemax = bitrateParams.frameBitRateMax;
        cmd.DW10.Framebitratemaxunit = bitrateParams.frameBitRateMaxUnit;
        cmd.DW10.Framebitratemaxunitmode = bitrateParams.frameBitRateMaxUnitMode;

        //DW11
        cmd.DW11.Framebitratemindelta = bitrateParams.frameBitRateMinDelta;
        cmd.DW11.Framebitratemaxdelta = bitrateParams.frameBitRateMaxDelta;
        //add for multiple pass
        if (params->dwMaxFrameSize > 0 && params->pDeltaQp && (!params->bIPCMPass))
        {
            cmd.DW8.Slicedeltaqppmax0 =
                cmd.DW8.Slicedeltaqpmax1 =
                cmd.DW8.Slicedeltaqpmax2 =
                cmd.DW8.Slicedeltaqpmax3 = params->pDeltaQp[params->currPass];
            cmd.DW10.Framebitratemaxunit = 0;
            cmd.DW10.Framebitratemaxunitmode = 0;
            //when FrameBitrateMaxUnit & FrameBitrateMaxUnitMode both are 0, the frame size unit is 128bytes.
            cmd.DW10.Framebitratemax = params->dwMaxFrameSize >> 7;
            cmd.DW11.Framebitratemaxdelta = params->dwMaxFrameSize >> 8;

            // In compatibility mode (DW10.FrameBitrateMaxUnitMode = 0), only 12 bits is used.
            // If the calulated value of max frame size exceeded 12 bits, need change unit from 128 bytes to 16K bytes.
            if (params->dwMaxFrameSize >= (0x1 << 12) * 128)
            {
                // use 16K bytes unit mode in compatibility mode.
                cmd.DW10.Framebitratemaxunit = 1;
                cmd.DW10.Framebitratemaxunitmode = 0;
                cmd.DW10.Framebitratemax = params->dwMaxFrameSize >> 14;
                cmd.DW11.Framebitratemaxdelta = params->dwMaxFrameSize >> 15;
            }
        }

        if (params->bIPCMPass)
        {
            // InterMbConfFlag, IntraMbConfFlag: not being used in HW
            cmd.DW4.Mbstatenabled = true;
            cmd.DW5.IntraIntermbipcmflagForceipcmcontrolmask = true;
        }

        MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxAvcDirectmodeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_DIRECTMODE_PARAMS params)
    {
        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(params);

        MHW_RESOURCE_PARAMS resourceParams;
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum = MHW_VDBOX_MFX_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_MFX_AVC_DIRECT_MODE;

        typename TMfxCmds::MFX_AVC_DIRECTMODE_STATE_CMD cmd;

        if (!params->bDisableDmvBuffers)
        {
            MHW_MI_CHK_NULL(params->presAvcDmvBuffers);
            MHW_MI_CHK_NULL(params->pAvcDmvList);

            cmd.DW36.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC].Value;

            // current picture
            resourceParams.presResource = &params->presAvcDmvBuffers[params->ucAvcDmvIdx];
            resourceParams.dwOffset = 0;
            resourceParams.pdwCmd = &(cmd.DirectMvBufferBaseAddressForWrite[0].DW0_1.Value[0]);
            resourceParams.dwLocationInCmd = 34;
            resourceParams.bIsWritable = true;

            MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                this->m_osInterface,
                cmdBuffer,
                &resourceParams));
        }

        CODEC_REF_LIST** refList;
        MHW_MI_CHK_NULL(refList = (CODEC_REF_LIST**)params->avcRefList);

        if (CodecHal_PictureIsBottomField(params->CurrPic))
        {
            cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_TOP] = 0;
            cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] =
                refList[params->CurrPic.FrameIdx]->iFieldOrderCnt[1];
        }
        else
        {
            cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_TOP] = cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] =
                refList[params->CurrPic.FrameIdx]->iFieldOrderCnt[0];
            if (CodecHal_PictureIsFrame(params->CurrPic))
            {
                cmd.PocList[MHW_VDBOX_AVC_DMV_DEST_BOTTOM] =
                    refList[params->CurrPic.FrameIdx]->iFieldOrderCnt[1];
            }
        }

        if (!params->bDisableDmvBuffers)
        {
            cmd.DW33.MemoryObjectControlState =
                this->m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_DIRECTMV_BUFFER_CODEC].Value;
        }

        bool dmvPresent[CODEC_MAX_NUM_REF_FRAME] = { false };
        for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
        {
            if (params->pAvcPicIdx[i].bValid)
            {
                uint8_t idx = params->pAvcPicIdx[i].ucPicIdx;
                uint8_t picID = params->bPicIdRemappingInUse ? i : refList[idx]->ucFrameId;
                uint8_t mvIdx = refList[idx]->ucDMVIdx[0];

                bool useMvcDummyDmvBuf = 0;
                //bool useMvcDummyDmvBuf = IsMvcInterviewPred(params->CurrPic, params->ppAvcRefList[params->CurrPic.FrameIdx]->iFieldOrderCnt, idx, params->ppAvcRefList);

                uint8_t validRef = ((params->uiUsedForReferenceFlags >> (i * 2)) >> 0) & 1;
                uint8_t frameID = picID << 1;

                if (frameID < CODEC_AVC_NUM_REF_DMV_BUFFERS * 2)
                {
                    if (!params->bDisableDmvBuffers)
                    {
                        dmvPresent[picID] = true;

                        resourceParams.presResource = useMvcDummyDmvBuf ?
                            params->presMvcDummyDmvBuffer : &params->presAvcDmvBuffers[mvIdx];
                        resourceParams.dwOffset = 0;
                        resourceParams.pdwCmd = &(cmd.DirectMvBufferBaseAddress[picID].DW0_1.Value[0]);
                        resourceParams.dwLocationInCmd = picID * 2 + 1;
                        resourceParams.bIsWritable = false;

                        resourceParams.dwSharedMocsOffset = 33 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW33

                        MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                            this->m_osInterface,
                            cmdBuffer,
                            &resourceParams));
                    }

                    cmd.PocList[frameID] = refList[idx]->iFieldOrderCnt[0] * validRef;
                }
                else
                {
                    return MOS_STATUS_UNKNOWN;
                }

                validRef = ((params->uiUsedForReferenceFlags >> (i * 2)) >> 1) & 1;
                frameID = (picID << 1) + 1;
                if (frameID < CODEC_AVC_NUM_REF_DMV_BUFFERS * 2)
                {
                    cmd.PocList[frameID] = refList[idx]->iFieldOrderCnt[1] * validRef;
                }
                else
                {
                    return MOS_STATUS_UNKNOWN;
                }
            }
        }

        if (!params->bDisableDmvBuffers)
        {
            // Use a valid address for remaining DMV buffers
            for (auto i = 0; i < CODEC_MAX_NUM_REF_FRAME; i++)
            {
                if (dmvPresent[i] == false)
                {
                    //Give default buffer to the MV
                    resourceParams.presResource = &params->presAvcDmvBuffers[CODEC_AVC_NUM_REF_DMV_BUFFERS];
                    resourceParams.dwOffset = 0;
                    resourceParams.pdwCmd = &(cmd.DirectMvBufferBaseAddress[i].DW0_1.Value[0]);
                    resourceParams.dwLocationInCmd = i * 2 + 1;
                    resourceParams.bIsWritable = false;

                    resourceParams.dwSharedMocsOffset = 33 - resourceParams.dwLocationInCmd; // Common Prodected Data bit is in DW33

                    MHW_MI_CHK_STATUS(this->AddResourceToCmd(
                        this->m_osInterface,
                        cmdBuffer,
                        &resourceParams));
                }
            }
        }

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMfdAvcSliceAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(avcSliceState);

        typename TMfxCmds::MFD_AVC_SLICEADDR_CMD cmd;

        if (avcSliceState->bFullFrameData)
        {
            cmd.DW1.IndirectBsdDataLength       = avcSliceState->dwNextLength;
            cmd.DW2.IndirectBsdDataStartAddress = avcSliceState->dwNextOffset;
        }
        else
        {
            cmd.DW1.IndirectBsdDataLength       = (avcSliceState->dwNextLength + 1 - this->m_osInterface->dwNumNalUnitBytesIncluded);
            cmd.DW2.IndirectBsdDataStartAddress = (avcSliceState->dwNextOffset - 1 + this->m_osInterface->dwNumNalUnitBytesIncluded);
        }

        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer = avcSliceState->presDataBuffer;
        sliceInfoParam.dwSliceIndex = avcSliceState->dwSliceIndex;
        sliceInfoParam.dwTotalBytesConsumed = avcSliceState->dwTotalBytesConsumed;
        sliceInfoParam.dwDataStartOffset[0] = cmd.DW2.IndirectBsdDataStartAddress;
        sliceInfoParam.dwDataStartOffset[1] = avcSliceState->pAvcSliceParams->slice_data_offset;

        MHW_MI_CHK_STATUS(this->m_cpInterface->SetMfxProtectionState(
            this->m_decodeInUse,
            cmdBuffer,
            nullptr,
            &sliceInfoParam));

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfdAvcBsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(this->m_osInterface);
        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(avcSliceState);
        MHW_MI_CHK_NULL(avcSliceState->pAvcSliceParams);

        typename TMfxCmds::MFD_AVC_BSD_OBJECT_CMD cmd;
        auto sliceParams = avcSliceState->pAvcSliceParams;

        cmd.DW4.LastsliceFlag = avcSliceState->bLastSlice;

        cmd.DW3.IntraPredmode4X48X8LumaErrorControlBit = 1;
        cmd.DW5.IntraPredictionErrorControlBitAppliedToIntra16X16Intra8X8Intra4X4LumaAndChroma = 1;
        cmd.DW5.Intra8X84X4PredictionErrorConcealmentControlBit = 1;
        cmd.DW5.ISliceConcealmentMode = 1;

        if (avcSliceState->bShortFormatInUse)
        {
            if (avcSliceState->bFullFrameData)
            {
                cmd.DW1.IndirectBsdDataLength       = avcSliceState->dwLength;
                cmd.DW2.IndirectBsdDataStartAddress = sliceParams->slice_data_offset;
            }
            else
            {
                cmd.DW1.IndirectBsdDataLength = avcSliceState->dwLength + 1 - this->m_osInterface->dwNumNalUnitBytesIncluded;
                cmd.DW2.IndirectBsdDataStartAddress =
                    sliceParams->slice_data_offset - 1 + this->m_osInterface->dwNumNalUnitBytesIncluded;
            }
            cmd.DW4.FirstMbByteOffsetOfSliceDataOrSliceHeader = 0;
        }
        else
        {
            // Long format
            cmd.DW1.IndirectBsdDataLength = avcSliceState->dwLength;
            cmd.DW2.IndirectBsdDataStartAddress = sliceParams->slice_data_offset + avcSliceState->dwOffset;
            cmd.DW4.FirstMacroblockMbBitOffset = sliceParams->slice_data_bit_offset;

            if (!avcSliceState->bIntelEntrypointInUse)
            {
                // NAL Header Unit must be passed to HW in the compressed bitstream buffer
                avcSliceState->dwOffset -= (this->m_osInterface->dwNumNalUnitBytesIncluded - 1);
                cmd.DW1.IndirectBsdDataLength += avcSliceState->dwOffset;
                cmd.DW2.IndirectBsdDataStartAddress -= avcSliceState->dwOffset;
                cmd.DW4.FirstMbByteOffsetOfSliceDataOrSliceHeader = avcSliceState->dwOffset;
            }

        }

        cmd.DW4.FixPrevMbSkipped = 1;

        MHW_CP_SLICE_INFO_PARAMS sliceInfoParam;
        sliceInfoParam.presDataBuffer = avcSliceState->presDataBuffer;
        sliceInfoParam.dwSliceIndex = avcSliceState->dwSliceIndex;
        sliceInfoParam.dwTotalBytesConsumed = avcSliceState->dwTotalBytesConsumed;
        sliceInfoParam.dwDataStartOffset[0] = cmd.DW2.IndirectBsdDataStartAddress;
        sliceInfoParam.dwDataStartOffset[1] = sliceParams->slice_data_offset;
        sliceInfoParam.dwDataLength[1] = sliceParams->slice_data_size;

        MHW_MI_CHK_STATUS(this->m_cpInterface->SetMfxProtectionState(
            this->m_decodeInUse,
            cmdBuffer,
            nullptr,
            &sliceInfoParam));

        MHW_MI_CHK_STATUS(this->m_osInterface->pfnAddCommand(cmdBuffer, &cmd, sizeof(cmd)));

        return eStatus;
    }

    MOS_STATUS AddMfxPakInsertObject(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_MI_CHK_NULL(params);

        if (cmdBuffer == nullptr && batchBuffer == nullptr)
        {
            MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        typename TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD cmd;
        uint32_t dwordsUsed = TMfxCmds::MFX_PAK_INSERT_OBJECT_CMD::dwSize;

        cmd.DW1.SliceHeaderIndicator = params->bSliceHeaderIndicator;

        if (params->bLastPicInSeq || params->bLastPicInStream)    // used by AVC, MPEG2
        {
            dwordsUsed += params->bLastPicInSeq + params->bLastPicInStream;

            cmd.DW0.DwordLength = TMfxCmds::GetOpLength(dwordsUsed);
            cmd.DW1.BitstreamstartresetResetbitstreamstartingpos = 0;
            cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = 1;
            cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = 1;
            cmd.DW1.EmulationflagEmulationbytebitsinsertenable = 0;
            cmd.DW1.SkipemulbytecntSkipEmulationByteCount = 0;
            // use dwBitSize to pass SrcDataEndingBitInclusion
            cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = params->dwBitSize;
            cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;
            cmd.DW1.Headerlengthexcludefrmsize = cmd.DW1.EmulationflagEmulationbytebitsinsertenable ? false
                : params->bHeaderLengthExcludeFrmSize; // Cannot be set to true if emulation byte bit insertion is enabled

            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

            if (params->bLastPicInSeq) // only used by AVC, not used by MPEG2
            {
                uint32_t lastPicInSeqData = params->dwLastPicInSeqData;

                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
                    this->m_osInterface,
                    cmdBuffer,
                    batchBuffer,
                    &lastPicInSeqData,
                    sizeof(lastPicInSeqData)));
            }

            if (params->bLastPicInStream)  // used by AVC,MPEG2
            {
                uint32_t lastPicInStreamData = params->dwLastPicInStreamData;

                MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(
                    this->m_osInterface,
                    cmdBuffer,
                    batchBuffer,
                    &lastPicInStreamData,
                    sizeof(lastPicInStreamData)));
            }
        }
        else // used by AVC, MPEG2, JPEG
        {
            uint32_t byteSize = (params->dwBitSize + 7) >> 3;
            uint32_t dataBitsInLastDw = params->dwBitSize % 32;

            if (dataBitsInLastDw == 0)
            {
                dataBitsInLastDw = 32;
            }

            dwordsUsed += ((byteSize + 3) >> 2);
            cmd.DW0.DwordLength = TMfxCmds::GetOpLength(dwordsUsed);
            cmd.DW1.BitstreamstartresetResetbitstreamstartingpos = params->bResetBitstreamStartingPos;
            cmd.DW1.EndofsliceflagLastdstdatainsertcommandflag = params->bEndOfSlice;
            cmd.DW1.LastheaderflagLastsrcheaderdatainsertcommandflag = params->bLastHeader;
            cmd.DW1.EmulationflagEmulationbytebitsinsertenable = params->bEmulationByteBitsInsert;
            cmd.DW1.SkipemulbytecntSkipEmulationByteCount = params->uiSkipEmulationCheckCount;
            cmd.DW1.DatabitsinlastdwSrcdataendingbitinclusion50 = dataBitsInLastDw;
            cmd.DW1.DatabyteoffsetSrcdatastartingbyteoffset10 = 0;
            cmd.DW1.Headerlengthexcludefrmsize = cmd.DW1.EmulationflagEmulationbytebitsinsertenable ? false
                : params->bHeaderLengthExcludeFrmSize; // Cannot be set to true if emulation byte bit insertion is enabled
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, batchBuffer, &cmd, sizeof(cmd)));

            // Add actual data
            uint8_t* data = (uint8_t*)(params->pBsBuffer->pBase + params->dwOffset);
            MHW_MI_CHK_STATUS(Mhw_AddCommandCmdOrBB(this->m_osInterface, cmdBuffer, batchBuffer, data, byteSize));
        }

        return eStatus;
    }

    MOS_STATUS AddMfdVp8BsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_BSD_PARAMS params)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        typename TMfxCmds::MFD_VP8_BSD_OBJECT_CMD cmd;

        eStatus = MhwVdboxMfxInterfaceGeneric<TMfxCmds, mhw_mi_g8_X>::AddMfdVp8BsdObjectCmd(cmdBuffer, params);
        MHW_MI_CHK_STATUS(eStatus);

        return eStatus;
    }

};

#endif
