/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     mhw_vdbox_hcp_g11_X.h
//! \brief    Defines functions for constructing Vdbox HCP commands on Gen11-based platforms
//!

#ifndef __MHW_VDBOX_HCP_G11_X_H__
#define __MHW_VDBOX_HCP_G11_X_H__

#include "mhw_vdbox_hcp_generic.h"
#include "mhw_vdbox_hcp_hwcmd_g11_X.h"
#include "mhw_vdbox_g11_X.h"
#include "mos_util_user_interface.h"

#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO        (26 * MHW_CACHELINE_SIZE) // 18+4+4
#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO_REXT   (35 * MHW_CACHELINE_SIZE) // 27+4+4

#define MHW_HCP_WORST_CASE_CU_TU_INFO            (4 * MHW_CACHELINE_SIZE) // 2+1+1
#define MHW_HCP_WORST_CASE_CU_TU_INFO_REXT       (6 * MHW_CACHELINE_SIZE) // 4+1+1


// ICL rowstore Cache values
#define MHW_VDBOX_PICWIDTH_1920                                                 1920
#define MHW_VDBOX_PICWIDTH_3840                                                 3840
#define VP9DATROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920            30
#define VP9DATROWSTORE_BASEADDRESS_PICWIDTH_BETWEEN_1920_AND_3840               60
#define VP9DFROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_1920             30
#define HEVCDFROWSTORE_BASEADDRESS_NON444_PICWIDTH_LESS_THAN_OR_EQU_TO_1920     120
#define HEVCSAOROWSTORE_BASEADDRESS_8BIT_PICWIDTH_LESS_THAN_OR_EQU_TO_1920      360
#define HEVCSAOROWSTORE_BASEADDRESS_444_10BIT_PICWIDTH_LESS_THAN_OR_EQU_TO_1920 120

// CU Record structure
struct EncodeHevcCuDataG11
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

struct MHW_VDBOX_HEVC_PIC_STATE_G11 : public MHW_VDBOX_HEVC_PIC_STATE
{
    PCODEC_HEVC_EXT_PIC_PARAMS              pHevcExtPicParams = nullptr;
};
using PMHW_VDBOX_HEVC_PIC_STATE_G11 = MHW_VDBOX_HEVC_PIC_STATE_G11 *;


struct MHW_VDBOX_HEVC_SLICE_STATE_G11 : public MHW_VDBOX_HEVC_SLICE_STATE
{
    // GEN11+ Tile coding params
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11   pTileCodingParams = nullptr;
    uint32_t                                dwTileID = 0;
    uint32_t                                dwNumPipe = 0;

    PCODEC_HEVC_EXT_SLICE_PARAMS    pHevcExtSliceParams = nullptr;
    PCODEC_HEVC_EXT_PIC_PARAMS      pHevcExtPicParam = nullptr;
};
using PMHW_VDBOX_HEVC_SLICE_STATE_G11 = MHW_VDBOX_HEVC_SLICE_STATE_G11 *;

struct MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11 : public MHW_VDBOX_PIPE_BUF_ADDR_PARAMS
{
    //Scalable
    PMOS_RESOURCE               presSliceStateStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presMvUpRightColStoreBuffer = nullptr;
    PMOS_RESOURCE               presIntraPredUpRightColStoreBuffer = nullptr;
    PMOS_RESOURCE               presIntraPredLeftReconColStoreBuffer = nullptr;
    PMOS_RESOURCE               presCABACSyntaxStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presCABACSyntaxStreamOutMaxAddr = nullptr;
};
using PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11 = MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G11 *;

// the tile size record is streamed out serving 2 purposes
// in vp9 for back annotation of tile size into the bitstream
struct HCPPakHWTileSizeRecord_G11
{
    //DW0
    uint32_t
        Address_31_0;

    //DW1
    uint32_t
        Address_63_32;

    //DW2
    uint32_t
        Length; // Bitstream length per tile; includes header len in first tile, and tail len in last tile

                //DW3
    uint32_t
        TileSize; // In Vp9, it is used for back annotation, In Hevc, it is the mmio register bytecountNoHeader

                  //DW4
    uint32_t
        AddressOffset; // Cacheline offset

                       //DW5
    uint32_t
        ByteOffset : 6, //[5:0] // Byte offset within cacheline
        Res_95_70 : 26; //[31:6]

                        //DW6
    uint32_t
        Hcp_Bs_SE_Bitcount_Tile; // bitstream size for syntax element per tile

                                 //DW7
    uint32_t
        Hcp_Cabac_BinCnt_Tile; // bitstream size for syntax element per tile

                               //DW8
    uint32_t
        Res_DW8_31_0;

    //DW9
    uint32_t
        Hcp_Image_Status_Ctrl; // image status control per tile

                               //DW10
    uint32_t
        Hcp_Qp_Status_Count; // Qp status count per tile

                             //DW11
    uint32_t
        Hcp_Slice_Count_Tile; // number of slices per tile

                              //DW12-15
    uint32_t
        Res_DW12_DW15[4]; // reserved bits added so that QwordDisables are set correctly
};

//!  MHW Vdbox Hcp interface for Gen11
/*!
This class defines the Hcp command construction functions for Gen11 platform
*/
class MhwVdboxHcpInterfaceG11 : public MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g11_X>
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
        MI_ATOMIC_CMD_NUMBER_OF_ADDRESSES                          =  1, //  2 DW for  1 address field

        MFX_WAIT_CMD_NUMBER_OF_ADDRESSES                           =  0, //  0 DW for    address fields
    
        HCP_PIPE_MODE_SELECT_CMD_NUMBER_OF_ADDRESSES               =  0, //  0 DW for    address fields
        HCP_SURFACE_STATE_CMD_NUMBER_OF_ADDRESSES                  =  0, //  0 DW for    address fields
        HCP_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES            = 45, //           45 address fields
        HCP_IND_OBJ_BASE_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES        = 11, // 22 DW for 11 address field
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
        HCP_TILE_CODING_COMMAND_NUMBER_OF_ADDRESSES                =  1, //  0 DW for    address fields
    
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES          = 12, // 12 DW for 12 address fields
        VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for  0 address fields
    };

    bool       m_hevcRDOQPerfDisabled = false; //!< Flag to indicate if HEVC RDOQ Perf is disabled
    uint32_t   m_watchDogTimerThreshold = 0; //!< For Watch Dog Timer threshold on Gen11+

    static const uint32_t m_hcpPakObjSize = (5 + 3);   //!< hcp pak object size

    static const uint32_t m_veboxRgbHistogramSizePerSlice = 256 * 4;
    static const uint32_t m_veboxNumRgbChannel = 3;
    static const uint32_t m_veboxAceHistogramSizePerFramePerSlice = 256 * 4;
    static const uint32_t m_veboxNumFramePreviousCurrent = 2;

    static const uint32_t m_veboxMaxSlices = 4;
    static const uint32_t m_veboxRgbHistogramSize = m_veboxRgbHistogramSizePerSlice * m_veboxNumRgbChannel * m_veboxMaxSlices;
    static const uint32_t m_veboxRgbAceHistogramSizeReserved = 3072 * 4;
    static const uint32_t m_veboxLaceHistogram256BinPerBlock = 256 * 2;
    static const uint32_t m_veboxStatisticsSize = 32 * 8;

public:
    static const uint32_t m_watchDogEnableCounter = 0x0;
    static const uint32_t m_watchDogDisableCounter = 0x00000001;
    static const uint32_t m_watchDogTimeoutInMs = 120; // derived from WDT threshold in KMD

    //!
    //! \brief  Constructor
    //!
    MhwVdboxHcpInterfaceG11(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse)
        : MhwVdboxHcpInterfaceGeneric(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;

        m_rhoDomainStatsEnabled = true;

        // Debug hook for HEVC RDOQ issue on Gen11
        MOS_USER_FEATURE_VALUE_DATA userFeatureData;
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_HEVC_ENCODE_RDOQ_PERF_DISABLE_ID,
            &userFeatureData);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_hevcRDOQPerfDisabled = userFeatureData.i32Data ? true : false;

        m_watchDogTimerThreshold = m_watchDogTimeoutInMs;
#if (_DEBUG || _RELEASE_INTERNAL)
        // User feature config of watchdog timer threshold
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_WATCHDOG_TIMER_THRESHOLD,
            &userFeatureData);
        if (userFeatureData.u32Data != 0)
        {
            m_watchDogTimerThreshold = userFeatureData.u32Data;
        }
#endif

        m_hevcEncCuRecordSize = sizeof(EncodeHevcCuDataG11);
        m_pakHWTileSizeRecordSize = sizeof(HCPPakHWTileSizeRecord_G11);

        InitRowstoreUserFeatureSettings();
        InitMmioRegisters();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxHcpInterfaceG11() { MHW_FUNCTION_ENTER; }

    uint32_t GetHcpPakObjSize()
    {
        return m_hcpPakObjSize;
    }

    inline uint32_t GetHcpHevcVp9RdoqStateCommandSize()
    {
        return mhw_vdbox_hcp_g11_X::HEVC_VP9_RDOQ_STATE_CMD::byteSize;
    }

    inline uint32_t GetHcpVp9PicStateCommandSize()
    {
        return mhw_vdbox_hcp_g11_X::HCP_VP9_PIC_STATE_CMD::byteSize;
    }

    inline uint32_t GetHcpVp9SegmentStateCommandSize()
    {
        return mhw_vdbox_hcp_g11_X::HCP_VP9_SEGMENT_STATE_CMD::byteSize;
    }

    inline uint32_t GetWatchDogTimerThrehold()
    {
        return m_watchDogTimerThreshold;
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

    MOS_STATUS AddHcpTileStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_HEVC_TILE_STATE       params);

    MOS_STATUS AddHcpWeightOffsetStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_HEVC_WEIGHTOFFSET_PARAMS  params);

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

    //!
    //! \brief    Adds HCP tile coding command in command buffer for decoder
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AddHcpDecodeTileCodingCmd(
        PMOS_COMMAND_BUFFER                   cmdBuffer,
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 params);

    //!
    //! \brief    Adds HCP tile coding command in command buffer for encoder
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AddHcpEncodeTileCodingCmd(
        PMOS_COMMAND_BUFFER                   cmdBuffer,
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 params);

    MOS_STATUS AddHcpHevcPicBrcBuffer(
        PMOS_RESOURCE                   hcpImgStates,
        PMHW_VDBOX_HEVC_PIC_STATE        hevcPicState);

    MOS_STATUS GetOsResLaceOrAceOrRgbHistogramBufferSize(
        uint32_t                        width,
        uint32_t                        height,
        uint32_t                       *size);

    MOS_STATUS GetOsResStatisticsOutputBufferSize(
        uint32_t                        width,
        uint32_t                        height,
        uint32_t                       *size);

    //!
    //! \brief    Adds HCP tile coding command in command buffer
    //! \details  Client facing function to add HCP tile coding command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHcpTileCodingCmd(
        PMOS_COMMAND_BUFFER                   cmdBuffer,
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G11 params);
};

#endif
