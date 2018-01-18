/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mhw_vdbox_hcp_g10_X.h
//! \brief    Defines functions for constructing Vdbox HCP commands on Gen10-based platforms
//!

#ifndef __MHW_VDBOX_HCP_G10_X_H__
#define __MHW_VDBOX_HCP_G10_X_H__

#include "mhw_vdbox_hcp_generic.h"
#include "mhw_vdbox_hcp_hwcmd_g10_X.h"

// CU Record structure
struct EncodeHevcCuDataG10
{
    //DWORD 0
    union
    {
        struct
        {
            uint32_t DW0_L0_Mv0x : MOS_BITFIELD_RANGE(0, 15);
            uint32_t DW0_L0_Mv0y : MOS_BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t DW0_LumaIntraModeSecondBest : MOS_BITFIELD_RANGE(0, 5);
            uint32_t DW0_LumaIntraModeSecondBest4x4_1 : MOS_BITFIELD_RANGE(6, 11);
            uint32_t DW0_LumaIntraModeSecondBest4x4_2 : MOS_BITFIELD_RANGE(12, 17);
            uint32_t DW0_LumaIntraModeSecondBest4x4_3 : MOS_BITFIELD_RANGE(18, 23);
            uint32_t DW0_ChromaIntraModeSecondBest_o3a_p : MOS_BITFIELD_RANGE(24, 26);
            uint32_t DW0_Reserved0 : MOS_BITFIELD_RANGE(27, 31);
        };
    };

    //DWORD 1
    uint32_t DW1_L0_Mv1x : MOS_BITFIELD_RANGE(0, 15);
    uint32_t DW1_L0_Mv1y : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 2
    uint32_t DW2_L1_Mv0x : MOS_BITFIELD_RANGE(0, 15);
    uint32_t DW2_L1_Mv0y : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 3
    uint32_t DW3_L1_Mv1x : MOS_BITFIELD_RANGE(0, 15);
    uint32_t DW3_L1_Mv1y : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 4
    uint32_t DW4_L0Mv0RefIdx : MOS_BITFIELD_RANGE(0, 3);
    uint32_t DW4_L0Mv1RefIdx_ChromaIntraMode : MOS_BITFIELD_RANGE(4, 7);
    uint32_t DW4_L1Mv0RefIdx_ChromaIntraMode2 : MOS_BITFIELD_RANGE(8, 11);
    uint32_t DW4_L1Mv1RefIdx_ChromaIntraMode1 : MOS_BITFIELD_RANGE(12, 15);
    uint32_t DW4_Tu_Yuv_TransformSkip : MOS_BITFIELD_RANGE(16, 31);

    //DWORD 5
    uint32_t DW5_TuSize : MOS_BITFIELD_RANGE(0, 31);

    //DWORD 6
    uint32_t DW6_LumaIntraMode4x4_1 : MOS_BITFIELD_RANGE(0, 5);
    uint32_t DW6_LumaIntraMode4x4_2 : MOS_BITFIELD_RANGE(6, 11);
    uint32_t DW6_LumaIntraMode4x4_3 : MOS_BITFIELD_RANGE(12, 17);
    uint32_t DW6_RoundingSelect : MOS_BITFIELD_RANGE(18, 21);
    uint32_t DW6_Reserved0 : MOS_BITFIELD_RANGE(22, 23);
    uint32_t DW6_TuCountMinus1 : MOS_BITFIELD_RANGE(24, 27);
    uint32_t DW6_Reserved1 : MOS_BITFIELD_RANGE(28, 31);

    //DWORD 7
    uint32_t DW7_LumaIntraMode : MOS_BITFIELD_RANGE(0, 5);
    uint32_t DW7_CuSize : MOS_BITFIELD_RANGE(6, 7);
    uint32_t DW7_ChromaIntraMode : MOS_BITFIELD_RANGE(8, 10);
    uint32_t DW7_CuTransquantBypassFlag : MOS_BITFIELD_BIT(11);
    uint32_t DW7_CuPartMode : MOS_BITFIELD_RANGE(12, 14);
    uint32_t DW7_CuPredMode : MOS_BITFIELD_BIT(15);
    uint32_t DW7_InterPredIdcMv0 : MOS_BITFIELD_RANGE(16, 17);
    uint32_t DW7_InterPredIdcMv1 : MOS_BITFIELD_RANGE(18, 19);
    uint32_t DW7_ModifiedFlag : MOS_BITFIELD_BIT(20);
    uint32_t DW7_ForceZeroCoeff : MOS_BITFIELD_BIT(21);
    uint32_t DW7_Reserved0 : MOS_BITFIELD_RANGE(22, 23);
    uint32_t DW7_CuQp : MOS_BITFIELD_RANGE(24, 30);
    uint32_t DW7_CuQpSign : MOS_BITFIELD_BIT(31);

};

//!  MHW Vdbox Hcp interface for Gen10
/*!
This class defines the Hcp command construction functions for Gen10 platform
*/
class MhwVdboxHcpInterfaceG10 : public MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g10_X>
{
protected:
    #define PATCH_LIST_COMMAND(x)  (x##_NUMBER_OF_ADDRESSES)

    enum CommandsNumberOfAddresses
    {
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES              =  1, //  2 DW for  1 address field
        MI_STORE_DATA_IMM_CMD_NUMBER_OF_ADDRESSES                  =  1, //  2 DW for  1 address field
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES                        =  1, //  2 DW for  1 address field
        MI_CONDITIONAL_BATCH_BUFFER_END_CMD_NUMBER_OF_ADDRESSES    =  1, //  2 DW for  1 address field
        MI_STORE_REGISTER_MEM_CMD_NUMBER_OF_ADDRESSES              =  1, //  2 DW for  1 address field
        MI_COPY_MEM_MEM_CMD_NUMBER_OF_ADDRESSES                    =  4, //  4 DW for  2 address fields
        MI_SEMAPHORE_WAIT_CMD_NUMBER_OF_ADDRESSES                  =  1, //  2 DW for  1 address fields

        HCP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        HCP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields
        HCP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES            = 38, //           38 address fields
        HCP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES        = 10, // 20 DW for 10 address field
        HCP_QM_STATE_CMD_NUMBER_OF_ADDRESSES                       =  0, //  0 DW for    address fields
        HCP_FQM_STATE_CMD_NUMBER_OF_ADDRESSES                      =  0, //  0 DW for    address fields
        HCP_PIC_STATE_CMD_NUMBER_OF_ADDRESSES                      =  0, //  0 DW for    address fields
        HCP_REF_IDX_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields
        HCP_WEIGHTOFFSET_STATE_CMD_NUMBER_OF_ADDRESSES             =  0, //  0 DW for    address fields
        HCP_SLICE_STATE_CMD_NUMBER_OF_ADDRESSES                    =  0, //  0 DW for    address fields
        HCP_PAK_INSERT_OBJECT_CMD_NUMBER_OF_ADDRESSES              =  0, //  0 DW for    address fields
        HCP_TILE_STATE_CMD_NUMBER_OF_ADDRESSES                     =  0, //  0 DW for    address fields
        HCP_BSD_OBJECT_CMD_NUMBER_OF_ADDRESSES                     =  0, //  0 DW for    address fields
        HCP_VP9_SEGMENT_STATE_CMD_NUMBER_OF_ADDRESSES              =  0, //  0 DW for    address fields
        HCP_VP9_PIC_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields

        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES          = 12, // 12 DW for 12 address fields
        VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for  0 address fields
    };

    static const uint32_t m_hcpPakObjSize = (5 + 3);   //!< hcp pak object size

    static const uint32_t m_veboxRgbHistogramSizePerSlice = 256 * 4;
    static const uint32_t m_veboxNumRgbChannel = 3;
    static const uint32_t m_veboxAceHistogramSizePerFramePerSlice = 256 * 4;
    static const uint32_t m_veboxNumFramePreviousCurrent = 2;

    static const uint32_t m_veboxMaxSlices = 2;
    static const uint32_t m_veboxRgbHistogramSize = m_veboxRgbHistogramSizePerSlice * m_veboxNumRgbChannel * m_veboxMaxSlices;
    static const uint32_t m_veboxLaceHistogram256BinPerBlock = 256 * 2;
    static const uint32_t m_veboxStatisticsSize = 32 * 4;

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxHcpInterfaceG10(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse)
        : MhwVdboxHcpInterfaceGeneric(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;

        m_rhoDomainStatsEnabled = true;
        m_hevcEncCuRecordSize = sizeof(EncodeHevcCuDataG10);
        InitRowstoreUserFeatureSettings();
        InitMmioRegisters();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxHcpInterfaceG10() { MHW_FUNCTION_ENTER; }

    uint32_t GetHcpPakObjSize()
    {
        return m_hcpPakObjSize;
    }

    inline uint32_t GetHcpHevcVp9RdoqStateCommandSize()
    {
        return mhw_vdbox_hcp_g10_X::HEVC_VP9_RDOQ_STATE_CMD::byteSize;
    }

    inline uint32_t GetHcpVp9PicStateCommandSize()
    {
        return mhw_vdbox_hcp_g10_X::HCP_VP9_PIC_STATE_CMD::byteSize;
    }

    inline uint32_t GetHcpVp9SegmentStateCommandSize()
    {
        return mhw_vdbox_hcp_g10_X::HCP_VP9_SEGMENT_STATE_CMD::byteSize;
    }

    void InitMmioRegisters();

    void InitRowstoreUserFeatureSettings();

    MOS_STATUS GetHcpStateCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params);

    MOS_STATUS GetHcpPrimitiveCommandSize(
        uint32_t                        mode,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize,
        bool                            modeSpecific);

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams);

    MOS_STATUS GetHevcBufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
        PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam);

    MOS_STATUS GetVp9BufferSize(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
        PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam);

    MOS_STATUS IsHevcBufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
        PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam);

    MOS_STATUS IsVp9BufferReallocNeeded(
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
        PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam);

    MOS_STATUS AddHcpPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params);

    MOS_STATUS AddHcpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params);

    MOS_STATUS AddHcpEncodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS        params);

    MOS_STATUS AddHcpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS  params);

    MOS_STATUS AddHcpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  params);

    MOS_STATUS AddHcpDecodePicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params);

    MOS_STATUS AddHcpEncodePicStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE       params);

    MOS_STATUS AddHcpFqmStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_QM_PARAMS             params);

    MOS_STATUS AddHcpDecodeSliceStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState);

    MOS_STATUS AddHcpEncodeSliceStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState);

    MOS_STATUS AddHcpPakInsertObject(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_PAK_INSERT_PARAMS     params);

    MOS_STATUS AddHcpVp9PicStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_VP9_PIC_STATE         params);

    MOS_STATUS AddHcpVp9PicStateEncCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_VDBOX_VP9_ENCODE_PIC_STATE params);

    MOS_STATUS AddHcpVp9SegmentStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_VP9_SEGMENT_STATE     params);

    MOS_STATUS AddHcpHevcVp9RdoqStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_PIC_STATE        params);

    MOS_STATUS AddHcpHevcPicBrcBuffer(
        PMOS_RESOURCE                   hcpImgStates,
        MHW_VDBOX_HEVC_PIC_STATE        hevcPicState);

    MOS_STATUS GetOsResLaceOrAceOrRgbHistogramBufferSize(
        uint32_t                        width,
        uint32_t                        height,
        uint32_t                       *size);

    MOS_STATUS GetOsResStatisticsOutputBufferSize(
        uint32_t                        width,
        uint32_t                        height,
        uint32_t                       *size);
};

#endif
