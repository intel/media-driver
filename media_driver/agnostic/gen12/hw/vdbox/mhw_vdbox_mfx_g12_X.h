/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     mhw_vdbox_mfx_g12_X.h
//! \brief    Defines functions for constructing Vdbox MFX commands on Gen12-based platforms
//!

#ifndef __MHW_VDBOX_MFX_G12_X_H__
#define __MHW_VDBOX_MFX_G12_X_H__

#include "mhw_vdbox_mfx_generic.h"
#include "mhw_vdbox_mfx_hwcmd_g12_X.h"
#include "mhw_mi_hwcmd_g12_X.h"


//!  MHW Vdbox Mfx interface for Gen12
/*!
This class defines the Mfx command construction functions for Gen12 platform
Right now, we still use mhw_vdbox_mfx_g11_X as template, will switch to mfx_g12 once the command is done
*/
class MhwVdboxMfxInterfaceG12 : public MhwVdboxMfxInterfaceGeneric<mhw_vdbox_mfx_g12_X, mhw_mi_g12_X>
{
protected:
    static const uint32_t m_avcInterMbMaxSize = 4095; //! AVC inter macroblock max size
    static const uint32_t m_avcIntraMbMaxSize = 2700; //! AVC intra macroblock max size

    bool m_scalabilitySupported = false; //!< Indicate if scalability supported

#if (_DEBUG || _RELEASE_INTERNAL)
    #define MHW_VDBOX_IS_VDBOX_SPECIFIED(keyval, vdid, shift, mask, useVD) \
    do\
    {\
        int32_t TmpVal = keyval;\
        while (TmpVal != 0) \
        {\
            if (((TmpVal) & (mask)) == (vdid))\
            {\
                useVD = true;\
                break;\
            }\
            TmpVal >>= (shift);\
        };\
    }while(0)
#endif

    #define PATCH_LIST_COMMAND(x)  (x##_NUMBER_OF_ADDRESSES)

    enum CommandsNumberOfAddresses 
    {
        // MFX Engine Commands
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES           =  1, //  2 DW for  1 address field
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES               =  1, //  2 DW for  1 address field
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                     =  1, //  2 DW for  1 address field
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES =  1, //  2 DW for  1 address field
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES           =  1, //  2 DW for  1 address field
        MFX_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES            =  0, //  0 DW for    address fields
        MFX_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        MFX_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES         = 27, // 50 DW for 25 address fields, added 2 for DownScaledReconPicAddr
        MFX_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     =  5, // 10 DW for  5 address fields
        MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                        =  0, //  0 DW for    address fields
        MFX_BSP_BUF_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES     =  3, //  2 DW for  3 address fields
        MFD_AVC_PICID_STATE_CMD_NUMBER_OF_ADDRESSES             =  0, //  0 DW for    address fields
        MFX_AVC_DIRECTMODE_STATE_CMD_NUMBER_OF_ADDRESSES        = 17, // 50 DW for 17 address fields
        MFX_AVC_IMG_STATE_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        MFX_QM_STATE_CMD_NUMBER_OF_ADDRESSES                    =  0, //  0 DW for    address fields
        MFX_FQM_STATE_CMD_NUMBER_OF_ADDRESSES                   =  0, //  0 DW for    address fields
        MFD_VC1_LONG_PIC_STATE_CMD_NUMBER_OF_ADDRESSES          =  0, //  0 DW for    address fields
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
    };

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxMfxInterfaceG12(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse)
        : MhwVdboxMfxInterfaceGeneric(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;

        m_osInterface = osInterface;
        if (m_numVdbox > 1
#if (_DEBUG || _RELEASE_INTERNAL)
            && m_osInterface != nullptr
            && m_osInterface->bHcpDecScalabilityMode
#endif
            )
        {
            m_scalabilitySupported = true;
        }

        m_rhoDomainStatsEnabled = true;

        InitRowstoreUserFeatureSettings();
        InitMmioRegisters();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxMfxInterfaceG12() { MHW_FUNCTION_ENTER; }

public:
    //!
    //! \brief    Judge if scalability is supported
    //!
    //! \return   bool
    //!           true if supported, else false
    //!
    inline bool IsScalabilitySupported()
    {
        return m_scalabilitySupported;
    }

    //!
    //! \brief    Disable scalability support
    //!
    //! \return   void
    //!
    inline void DisableScalabilitySupport()
    {
        m_scalabilitySupported = false;
    }

protected:

    void InitMmioRegisters();

    void InitRowstoreUserFeatureSettings();

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams);

#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS CheckScalabilityOverrideValidity();
#endif

    MOS_STATUS FindGpuNodeToUse(
        PMHW_VDBOX_GPUNODE_LIMIT gpuNodeLimit);

    MOS_STATUS GetMfxStateCommandsDataSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool isShortFormat);

    MOS_STATUS GetMfxPrimitiveCommandsDataSize(
        uint32_t mode,
        uint32_t *commandsSize,
        uint32_t *patchListSize,
        bool  isModeSpecific);

    MOS_STATUS AddMfxPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params);

    MOS_STATUS AddMfxSurfaceCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params);

    MOS_STATUS AddMfxPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params);

    MOS_STATUS AddMfxIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS params);

    MOS_STATUS AddMfxBspBufBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS params);

    MOS_STATUS AddMfxDecodeAvcImgCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params);

    MOS_STATUS AddMfxEncodeAvcImgCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params);

    MOS_STATUS AddMfxAvcDirectmodeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_DIRECTMODE_PARAMS params);

    MOS_STATUS AddMfdAvcSliceAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState);

    MOS_STATUS AddMfdAvcBsdObjectCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE avcSliceState);

    MOS_STATUS AddMfxPakInsertObject(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER batchBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS params);

    MOS_STATUS AddMfxJpegPicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_JPEG_PIC_STATE params);

    MOS_STATUS AddMfxJpegEncodePicStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MhwVdboxJpegEncodePicState *params);

    MOS_STATUS AddMfxJpegFqmCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_QM_PARAMS params,
        uint32_t numQuantTables);

    MOS_STATUS AddMfcJpegHuffTableStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS params);

    MOS_STATUS AddMfcJpegScanObjCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MhwVdboxJpegScanParams *params);

    MOS_STATUS AddMfxDecodeVp8PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_PIC_STATE params);

    MOS_STATUS AddMfxEncodeVp8PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_PIC_STATE params);

    MOS_STATUS InitMfxVp8EncoderCfgCmd(
        PMOS_RESOURCE cfgCmdBuffer,
        PMHW_VDBOX_VP8_ENCODER_CFG_PARAMS params);

    MOS_STATUS AddMfxVp8BspBufBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS params);

    virtual uint32_t GetScaledReferenceSurfaceCachePolicy()
    {
        return m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_SURFACE_HME_DOWNSAMPLED_ENCODE].Value >> 1;
    }
};

#endif
