/*===================== begin_copyright_notice ==================================

Copyright (c) 2017-2019, Intel Corporation

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_hcp_g12_X.h
//! \brief    Defines functions for constructing Vdbox HCP commands on Gen11-based platforms
//!

#ifndef __MHW_VDBOX_HCP_G12_X_H__
#define __MHW_VDBOX_HCP_G12_X_H__

#include "mhw_vdbox_hcp_generic.h"
#include "mhw_vdbox_hcp_hwcmd_g12_X.h"
#include "mhw_vdbox_g12_X.h"
#include "media_user_settings_mgr_g12.h"
#include "codec_def_decode_hevc.h"

//definition for row store cache offset on GEN12
#define VDENCHEVC_RSC_OFFSET_C420OR422_DXX_LCU32OR64_4K_G12               1824
#define VDENCHEVC_RSC_OFFSET_C420OR422_DXX_LCU32OR64_8K_G12               2304
#define VDENCHEVC_RSC_OFFSET_C444_D8_LCU32OR64_4K_G12                     1568
#define VDENCHEVC_RSC_OFFSET_C444_D8_LCU32OR64_8K_G12                     2112
#define VDENCHEVC_RSC_OFFSET_C444_D10_LCU32OR64_4K_G12                    2336
#define VDENCHEVC_RSC_OFFSET_C444_D10_LCU32OR64_8K_G12                    1600
#define VDENCHEVC_RSC_OFFSET_C444_D12_LCU32OR64_4K_G12                    2336
#define VDENCHEVC_RSC_OFFSET_C444_D12_LCU32OR64_8K_G12                    1600

#define HEVCDAT_RSC_OFFSET_CXXX_DXX_LCUXX_XX_G12                          0

#define HEVCDF_RSC_OFFSET_C420OR422_DXX_LCUXX_4K_G12                      256
#define HEVCDF_RSC_OFFSET_C420OR422_DXX_LCU16_8K_G12                      512
#define HEVCDF_RSC_OFFSET_C420OR422_DXX_LCU32OR64_8K_G12                  256
#define HEVCDF_RSC_OFFSET_C444_D8_LCU16_4K_G12                            256
#define HEVCDF_RSC_OFFSET_C444_D8_LCU16_8K_G12                            512
#define HEVCDF_RSC_OFFSET_C444_D10_LCU16_4K_G12                           256
#define HEVCDF_RSC_OFFSET_C444_D12_LCU16_4K_G12                           256
#define HEVCDF_RSC_OFFSET_C444_D8_LCU32OR64_4K_G12                        256
#define HEVCDF_RSC_OFFSET_C444_D8_LCU32OR64_8K_G12                        512
#define HEVCDF_RSC_OFFSET_C444_D10_LCU32OR64_4K_G12                       256
#define HEVCDF_RSC_OFFSET_C444_D12_LCU32OR64_4K_G12                       128

#define HEVCSAO_RSC_OFFSET_C420OR422_DXX_LCUXX_4K_G12                     1280
#define HEVCSAO_RSC_OFFSET_C444_D8_LCU16_4K_G12                           1024
#define HEVCSAO_RSC_OFFSET_C444_D10_LCU16_4K_G12                          1792
#define HEVCSAO_RSC_OFFSET_C444_D10_LCU16_8K_G12                          512
#define HEVCSAO_RSC_OFFSET_C444_D12_LCU16_4K_G12                          1792
#define HEVCSAO_RSC_OFFSET_C444_D12_LCU16_8K_G12                          256
#define HEVCSAO_RSC_OFFSET_C444_D8_LCU32OR64_4K_G12                       1024
#define HEVCSAO_RSC_OFFSET_C444_D10_LCU32OR64_4K_G12                      1792
#define HEVCSAO_RSC_OFFSET_C444_D10_LCU32OR64_8K_G12                      512
#define HEVCSAO_RSC_OFFSET_C444_D12_LCU32OR64_4K_G12                      1664
#define HEVCSAO_RSC_OFFSET_C444_D12_LCU32OR64_8K_G12                      256

#define HEVCHSAO_RSC_OFFSET_C420OR422_DXX_LCU16_4K_G12                    2048
#define HEVCHSAO_RSC_OFFSET_C420OR422_DXX_LCU32OR64_4K_G12                1792
#define HEVCHSAO_RSC_OFFSET_C444_D8_LCU16_4K_G12                          1792
#define HEVCHSAO_RSC_OFFSET_C444_D8_LCU16_8K_G12                          2048
#define HEVCHSAO_RSC_OFFSET_C444_D10_LCU16_8K_G12                         2048
#define HEVCHSAO_RSC_OFFSET_C444_D12_LCU16_8K_G12                         1792
#define HEVCHSAO_RSC_OFFSET_C444_D8_LCU32OR64_4K_G12                      1536
#define HEVCHSAO_RSC_OFFSET_C444_D8_LCU32OR64_8K_G12                      2048
#define HEVCHSAO_RSC_OFFSET_C444_D10_LCU32OR64_4K_G12                     2304
#define HEVCHSAO_RSC_OFFSET_C444_D10_LCU32OR64_8K_G12                     1536
#define HEVCHSAO_RSC_OFFSET_C444_D12_LCU32OR64_4K_G12                     2304
#define HEVCHSAO_RSC_OFFSET_C444_D12_LCU32OR64_8K_G12                     1536

// TGL rowstore Cache Values
#define VP9DATROWSTORE_BASEADDRESS_PICWIDTH_LESS_THAN_OR_EQU_TO_2K        32
#define VP9DATROWSTORE_BASEADDRESS_128                                    128
#define VP9DFROWSTORE_BASEADDRESS_64                                      64
#define VP9DFROWSTORE_BASEADDRESS_192                                     192

#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO        (26 * MHW_CACHELINE_SIZE) // 18+4+4
#define MHW_HCP_WORST_CASE_LCU_CU_TU_INFO_REXT   (35 * MHW_CACHELINE_SIZE) // 27+4+4

#define MHW_HCP_WORST_CASE_CU_TU_INFO            (4 * MHW_CACHELINE_SIZE) // 2+1+1
#define MHW_HCP_WORST_CASE_CU_TU_INFO_REXT       (6 * MHW_CACHELINE_SIZE) // 4+1+1

struct HcpPakObjectG12
{
    // DW0
    struct
    {
        uint32_t DwordLength : 16;
        uint32_t SubOp : 7;
        uint32_t Opcode : 6;
        uint32_t Type : 3;
    } DW0;

    //DW1
    struct
    {
        uint32_t Split_flag_level2_level1part0 : 4;
        uint32_t Split_flag_level2_level1part1 : 4;
        uint32_t Split_flag_level2_level1part2 : 4;
        uint32_t Split_flag_level2_level1part3 : 4;
        uint32_t Split_flag_level1 : 4;
        uint32_t Split_flag_level0 : 1;
        uint32_t Reserved21_23 : 3;
        uint32_t CU_count_minus1 : 6;
        uint32_t LastCtbOfTileFlag : 1;
        uint32_t LastCtbOfSliceFlag : 1;
    } DW1;

    //DW2
    struct
    {
        uint32_t Current_LCU_X_Addr : 16;
        uint32_t Current_LCU_Y_Addr : 16;
    } DW2;

    //DW3
    struct
    {
        uint32_t Estimated_LCU_Size_In_Bytes : 32;
    } DW3;

    //DW4
    struct
    {
        uint32_t SSE_ClassID_32x32_0 : 4;
        uint32_t SSE_ClassID_32x32_1 : 4;
        uint32_t SSE_ClassID_32x32_2 : 4;
        uint32_t SSE_ClassID_32x32_3 : 4;
        uint32_t LCUForceZeroCoeff : 1;
        uint32_t Disable_SAO_On_LCU_Flag : 1;
        uint32_t Reserve18_31 : 14;
    } DW4;

    uint32_t DW5;
    uint32_t DW6;
    uint32_t DW7;
};

// CU Record structure
struct EncodeHevcCuDataG12
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

struct MHW_VDBOX_HEVC_SLICE_STATE_G12 : public MHW_VDBOX_HEVC_SLICE_STATE
{
    // GEN11+ Tile coding params
    PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12   pTileCodingParams = nullptr;
    uint32_t                                dwTileID = 0;
    uint32_t                                dwNumPipe = 0;

    // For real tile decoding across tile
    bool                            bTileInSlice = false;           //!< This slice across tiles
    bool                            bIsNotFirstTile = false;        //!< Not first tile in slice
    uint16_t                        u16SliceHeaderLength = 0;   //!< Slice header length in this entrypoint
    uint16_t                        u16TileCtbX = 0;            //!< Ctb X index for this tile
    uint16_t                        u16TileCtbY = 0;            //!< Ctb Y index for this tile
    uint16_t                        u16NextTileCtbX = 0;        //!< Ctb X index for next tile
    uint16_t                        u16NextTileCtbY = 0;        //!< Ctb Y index for next tile
    uint16_t                        u16OrigCtbX = 0;            //!< Original slice start Ctb X index
    uint16_t                        u16OrigCtbY = 0;            //!< Original slice start Ctb Y index

    PCODEC_HEVC_EXT_SLICE_PARAMS    pHevcExtSliceParams = nullptr;
    PCODEC_HEVC_EXT_PIC_PARAMS      pHevcExtPicParam = nullptr;
    PCODEC_HEVC_SCC_PIC_PARAMS      pHevcSccPicParam = nullptr;

    // For SCC
    uint8_t                         ucRecNotFilteredID = 0;
};
using PMHW_VDBOX_HEVC_SLICE_STATE_G12 = MHW_VDBOX_HEVC_SLICE_STATE_G12 *;

struct MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 : public MHW_VDBOX_HEVC_REF_IDX_PARAMS
{
    // For SCC
    bool                            bIBCEnabled  = false;;
    uint8_t                         ucRecNotFilteredID = 0;;
};
using PMHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 = MHW_VDBOX_HEVC_REF_IDX_PARAMS_G12 *;

struct MHW_VDBOX_HEVC_PIC_STATE_G12 : public MHW_VDBOX_HEVC_PIC_STATE
{
    PCODEC_HEVC_EXT_PIC_PARAMS      pHevcExtPicParams = nullptr;
    PCODEC_HEVC_SCC_PIC_PARAMS      pHevcSccPicParams = nullptr;
    
    // For SCC
    uint8_t                         ucRecNotFilteredID = 0;
    uint8_t                         IBCControl = 0;
    bool                            PartialFrameUpdateEnable = false;
};
using PMHW_VDBOX_HEVC_PIC_STATE_G12 = MHW_VDBOX_HEVC_PIC_STATE_G12 *;

struct MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12 : public MHW_VDBOX_PIPE_BUF_ADDR_PARAMS
{
    //Scalable
    PMOS_RESOURCE               presSliceStateStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presMvUpRightColStoreBuffer = nullptr;
    PMOS_RESOURCE               presIntraPredUpRightColStoreBuffer = nullptr;
    PMOS_RESOURCE               presIntraPredLeftReconColStoreBuffer = nullptr;
    PMOS_RESOURCE               presCABACSyntaxStreamOutBuffer = nullptr;
    PMOS_RESOURCE               presCABACSyntaxStreamOutMaxAddr = nullptr;

    //Reference Mmc
    bool                        bSpecificReferencedMmcRequired = false;
    MOS_MEMCOMP_STATE           ReferencesMmcState = MOS_MEMCOMP_DISABLED;
};
using PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12 = MHW_VDBOX_PIPE_BUF_ADDR_PARAMS_G12 *;

// the tile size record is streamed out serving 2 purposes
// in vp9 for back annotation of tile size into the bitstream
struct HCPPakHWTileSizeRecord_G12
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
class MhwVdboxHcpInterfaceG12 : public MhwVdboxHcpInterfaceGeneric<mhw_vdbox_hcp_g12_X>
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
        HCP_PALETTE_INITIALIZER_STATE_CMD_NUMBER_OF_ADDRESSES      =  0, //  0 DW for    address fields
    
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES          = 12, // 12 DW for 12 address fields
        VD_PIPELINE_FLUSH_CMD_NUMBER_OF_ADDRESSES                  =  0,  //  0 DW for  0 address fields
    };

    enum HcpPicStateIbcConfigurationInVdencMode
    {
        HCP_PIC_STATE_IBC_CONFIGURATION_IN_VDENC_MODE_UNNAMED0  = 0, ///< When IBC configuration in VDENC mode is set to 0, Intra block search is disabled.
        HCP_PIC_STATE_IBC_CONFIGURATION_IN_VDENC_MODE_UNNAMED1  = 1, ///< When IBC configuration in VDENC mode is set to 1, Intra block search includes only left region.
        HCP_PIC_STATE_IBC_CONFIGURATION_IN_VDENC_MODE_UNNAMED2  = 2, ///<
        HCP_PIC_STATE_IBC_CONFIGURATION_IN_VDENC_MODE_UNNAMED3  = 3, ///< When IBC configuration in VDENC mode is set to 2, Intra block search includes top and left regions.
    };

    bool       m_hevcRDOQPerfDisabled = false; //!< Flag to indicate if HEVC RDOQ Perf is disabled
    uint32_t   m_watchDogTimerThreshold = 0;   //!< For Watch Dog Timer threshold on Gen11+
    bool       m_disableTlbPrefetch = false;   //!< To disable TLB pre-fetch or not

    static const uint32_t   m_HevcSccPaletteSize = 96; //!< For HEVC SCC palette size on Gen12+
    static const uint32_t   m_hcpPakObjSize = MOS_BYTES_TO_DWORDS(sizeof(HcpPakObjectG12));   //!< hcp pak object size

    static const uint32_t   m_veboxRgbHistogramSizePerSlice = 256 * 4;
    static const uint32_t   m_veboxNumRgbChannel = 3;
    static const uint32_t   m_veboxAceHistogramSizePerFramePerSlice = 256 * 4;
    static const uint32_t   m_veboxNumFramePreviousCurrent = 2;

    static const uint32_t   m_veboxMaxSlices = 4;
    static const uint32_t   m_veboxRgbHistogramSize = m_veboxRgbHistogramSizePerSlice * m_veboxNumRgbChannel * m_veboxMaxSlices;
    static const uint32_t   m_veboxRgbAceHistogramSizeReserved = 3072 * 4;
    static const uint32_t   m_veboxLaceHistogram256BinPerBlock = 256 * 2;
    static const uint32_t   m_veboxStatisticsSize = 32 * 8;

public:
    static const uint32_t   m_watchDogEnableCounter = 0x0;
    static const uint32_t   m_watchDogDisableCounter = 0x00000001;
    static const uint32_t   m_watchDogTimeoutInMs = 120; // derived from WDT threshold in KMD

    //!
    //! \brief  Constructor
    //!
    MhwVdboxHcpInterfaceG12(
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
            &userFeatureData,
            this->m_osInterface->pOsContext);
#endif // _DEBUG || _RELEASE_INTERNAL
        m_hevcRDOQPerfDisabled = userFeatureData.i32Data ? true : false;

        m_watchDogTimerThreshold = m_watchDogTimeoutInMs;
#if (_DEBUG || _RELEASE_INTERNAL)
        // User feature config of watchdog timer threshold
        MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MOS_UserFeature_ReadValue_ID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_WATCHDOG_TIMER_THRESHOLD,
            &userFeatureData,
            this->m_osInterface->pOsContext);
        if (userFeatureData.u32Data != 0)
        {
            m_watchDogTimerThreshold = userFeatureData.u32Data;
        }
#endif

        if (MEDIA_IS_WA(m_osInterface->pfnGetWaTable(m_osInterface), Wa_14012254246))
        {
            MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_DISABLE_TLB_PREFETCH_ID,
                &userFeatureData,
                this->m_osInterface->pOsContext);
            m_disableTlbPrefetch = userFeatureData.u32Data ? true : false;
        }

        m_hevcEncCuRecordSize = sizeof(EncodeHevcCuDataG12);
        m_pakHWTileSizeRecordSize = sizeof(HCPPakHWTileSizeRecord_G12);

        InitRowstoreUserFeatureSettings();
        InitMmioRegisters();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxHcpInterfaceG12() { MHW_FUNCTION_ENTER; }

    uint32_t GetHcpPakObjSize()
    {
        return m_hcpPakObjSize;
    }

    inline uint32_t GetHcpHevcVp9RdoqStateCommandSize()
    {
        return mhw_vdbox_hcp_g12_X::HEVC_VP9_RDOQ_STATE_CMD::byteSize;
    }

    inline uint32_t GetHcpVp9PicStateCommandSize()
    {
        return mhw_vdbox_hcp_g12_X::HCP_VP9_PIC_STATE_CMD::byteSize;
    }

    inline uint32_t GetHcpVp9SegmentStateCommandSize()
    {
        return mhw_vdbox_hcp_g12_X::HCP_VP9_SEGMENT_STATE_CMD::byteSize;
    }

    inline uint32_t GetWatchDogTimerThrehold()
    {
        return m_watchDogTimerThreshold;
    }

    void InitMmioRegisters();

    void InitRowstoreUserFeatureSettings();

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams);

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

    MOS_STATUS AddHcpRefIdxStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_VDBOX_HEVC_REF_IDX_PARAMS  params);

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
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 params);

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
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 params);

    MOS_STATUS AddHcpHevcPicBrcBuffer(
        PMOS_RESOURCE                   hcpImgStates,
        PMHW_VDBOX_HEVC_PIC_STATE        hevcPicState);

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
        PMHW_VDBOX_HCP_TILE_CODING_PARAMS_G12 params);

    //!
    //! \brief    Adds Hcp palette initializer state in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddHcpPaletteInitializerStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PCODEC_HEVC_SCC_PIC_PARAMS       params);

    MOS_STATUS GetOsResLaceOrAceOrRgbHistogramBufferSize(
        uint32_t                         width,
        uint32_t                         height,
        uint32_t                        *size);

    MOS_STATUS GetOsResStatisticsOutputBufferSize(
        uint32_t                         width,
        uint32_t                         height,
        uint32_t                        *size);
};

#if MOS_EVENT_TRACE_DUMP_SUPPORTED
typedef struct _DECODE_EVENTDATA_CMD_HCPPIPEMODESELECT
{
    uint32_t DW0_Value;
    uint32_t DW1_Value;
    uint32_t DW2_Value;
    uint32_t DW3_Value;
    uint32_t DW4_Value;
    uint32_t DW5_Value;
    uint32_t DW6_Value;
    uint32_t DW1_CodecSelect;
    uint32_t DW1_DeblockerStreamoutEnable;
    uint32_t DW1_PicStatusErrorReportEnable;
    uint32_t DW1_CodecStandardSelect;
    uint32_t DW1_MultiEngineMode;
    uint32_t DW1_PipeWorkingMode;
    uint32_t DW1_PrefetchDisable;
    uint32_t DW2_MediaSoftResetCounterPer1000Clocks;
    uint32_t DW6_PhaseIndicator;
    uint32_t DW6_HevcSeparateTileProgramming;
} DECODE_EVENTDATA_CMD_HCPPIPEMODESELECT;

typedef struct _DECODE_EVENTDATA_CMD_HCPSURFACESTATE
{
    uint32_t DW0_Value;
    uint32_t DW1_Value;
    uint32_t DW2_Value;
    uint32_t DW3_Value;
    uint32_t DW4_Value;
    uint32_t DW1_SurfaceId;
    uint32_t DW1_SurfacePitchMinus1;
    uint32_t DW2_YOffsetForUCbInPixel;
    uint32_t DW2_SurfaceFormat;
    uint32_t DW3_DefaultAlphaValue;
    uint32_t DW4_MemoryCompressionEnable;
    uint32_t DW4_CompressionType;
} DECODE_EVENTDATA_CMD_HCPSURFACESTATE;

typedef struct _DECODE_EVENTDATA_CMD_RESOURCE
{
    uint32_t hAllocation;
    uint32_t Address_1;
    uint32_t Address_0;
} DECODE_EVENTDATA_CMD_RESOURCE;

typedef struct _DECODE_EVENTDATA_CMD_HCPPIPEBUFADDRSTATE
{
    uint32_t                        DW0_Value;
    uint32_t                        DecodedPictureAllocationHandle;
    uint32_t                        DecodedPictureAddress_1;
    uint32_t                        DecodedPictureAddress_0;
    uint32_t                        DecodedPictureAttribute;
    uint32_t                        DecodedPictureMemoryCompressionEnable;
    uint32_t                        DecodedPictureCompressionType;
    uint32_t                        DecodedPictureTileMode;
    uint32_t                        DeblockingFilterLineBufferAddress_1;
    uint32_t                        DeblockingFilterLineBufferAddress_0;
    uint32_t                        DeblockingFilterLineBufferAttribute;
    uint32_t                        DeblockingFilterTileLineBufferAddress_1;
    uint32_t                        DeblockingFilterTileLineBufferAddress_0;
    uint32_t                        DeblockingFilterTileLineBufferAttribute;
    uint32_t                        DeblockingFilterTileColumnBufferAddress_1;
    uint32_t                        DeblockingFilterTileColumnBufferAddress_0;
    uint32_t                        DeblockingFilterTileColumnBufferAttribute;
    uint32_t                        MetadataLineBufferAddress_1;
    uint32_t                        MetadataLineBufferAddress_0;
    uint32_t                        MetadataLineBufferAttribute;
    uint32_t                        MetadataTileLineBufferAddress_1;
    uint32_t                        MetadataTileLineBufferAddress_0;
    uint32_t                        MetadataTileLineBufferAttribute;
    uint32_t                        MetadataTileColumnBufferAddress_1;
    uint32_t                        MetadataTileColumnBufferAddress_0;
    uint32_t                        MetadataTileColumnBufferAttribute;
    uint32_t                        SaoLineBufferAddress_1;
    uint32_t                        SaoLineBufferAddress_0;
    uint32_t                        SaoLineBufferAttribute;
    uint32_t                        SaoTileLineBufferAddress_1;
    uint32_t                        SaoTileLineBufferAddress_0;
    uint32_t                        SaoTileLineBufferAttribute;
    uint32_t                        SaoTileColumnBufferAddress_1;
    uint32_t                        SaoTileColumnBufferAddress_0;
    uint32_t                        SaoTileColumnBufferAttribute;
    uint32_t                        CurrentMotionVectorTemporalBufferAddress_1;
    uint32_t                        CurrentMotionVectorTemporalBufferAddress_0;
    uint32_t                        CurrentMotionVectorTemporalBufferAttribute;
    DECODE_EVENTDATA_CMD_RESOURCE   ReferencePicture[8];
    uint32_t                        ReferencePictureAttribute;
    uint32_t                        StreamoutDataDestinationAddress_1;
    uint32_t                        StreamoutDataDestinationAddress_0;
    uint32_t                        StreamoutDataDestinationAttribute;
    DECODE_EVENTDATA_CMD_RESOURCE   CollocatedMotionVector[8];
    uint32_t                        CollocatedMotionVectorAttribute;
    uint32_t                        Vp9ProbabilityAddress_1;
    uint32_t                        Vp9ProbabilityAddress_0;
    uint32_t                        Vp9ProbabilityAttribute;
    uint32_t                        Vp9SegmentIdAddress_1;
    uint32_t                        Vp9SegmentIdAddress_0;
    uint32_t                        Vp9SegmentIdAttribute;
    uint32_t                        Vp9HvdTileRowstoreAddress_1;
    uint32_t                        Vp9HvdTileRowstoreAddress_0;
    uint32_t                        Vp9HvdTileRowstoreAttribute;
    uint32_t                        HcpScalabilitySliceStateAddress_1;
    uint32_t                        HcpScalabilitySliceStateAddress_0;
    uint32_t                        HcpScalabilitySliceStateAttribute;
    uint32_t                        HcpScalabilityCabacDecodedSyntaxAddress_1;
    uint32_t                        HcpScalabilityCabacDecodedSyntaxAddress_0;
    uint32_t                        HcpScalabilityCabacDecodedSyntaxAttribute;
    uint32_t                        MotionVectorUpperRightColumnAddress_1;
    uint32_t                        MotionVectorUpperRightColumnAddress_0;
    uint32_t                        MotionVectorUpperRightColumnAttribute;
    uint32_t                        IntraPredictionUpperRightColumnAddress_1;
    uint32_t                        IntraPredictionUpperRightColumnAddress_0;
    uint32_t                        IntraPredictionUpperRightColumnAttribute;
    uint32_t                        IntraPredictionLeftReconColumnAddress_1;
    uint32_t                        IntraPredictionLeftReconColumnAddress_0;
    uint32_t                        IntraPredictionLeftReconColumnAttribute;
    uint32_t                        HcpScalabilityCabacDecodedSyntaxMaxAddress_1;
    uint32_t                        HcpScalabilityCabacDecodedSyntaxMaxAddress_0;
} DECODE_EVENTDATA_CMD_HCPPIPEBUFADDRSTATE;

typedef struct _DECODE_EVENTDATA_CMD_HCPINDOBJBASEADDRSTATE
{
    uint32_t DW0_Value;
    uint32_t DW1_Value;
    uint32_t DW2_Value;
    uint32_t DW3_Value;
    uint32_t DW4_Value;
    uint32_t DW5_Value;
    uint32_t DW1_2_BitstreamBaseAddress_1;
    uint32_t DW1_2_BitstreamBaseAddress_0;
    uint32_t DW3_MemoryCompressionEnable;
    uint32_t DW3_CompressionType;
    uint32_t DW4_5_BitstreamUpperAddress_1;
    uint32_t DW4_5_BitstreamUpperAddress_0;
} DECODE_EVENTDATA_CMD_HCPINDOBJBASEADDRSTATE;

typedef struct _DECODE_EVENTDATA_CMD_HCPVP9SEGMENTSTATE
{
    uint32_t DW0_Value;
    uint32_t DW1_Value;
    uint32_t DW2_Value;
    uint32_t DW3_Value;
    uint32_t DW4_Value;
    uint32_t DW5_Value;
    uint32_t DW6_Value;
    uint32_t DW1_SegmentId;    
    uint32_t DW2_SegmentSkipped;
    uint32_t DW2_SegmentReference;
    uint32_t DW2_SegmentReferenceEnabled;    
    uint32_t DW3_Filterlevelref0Mode0;
    uint32_t DW3_Filterlevelref0Mode1;
    uint32_t DW3_Filterlevelref1Mode0;
    uint32_t DW3_Filterlevelref1Mode1;    
    uint32_t DW4_Filterlevelref2Mode0;
    uint32_t DW4_Filterlevelref2Mode1;
    uint32_t DW4_Filterlevelref3Mode0;
    uint32_t DW4_Filterlevelref3Mode1;    
    uint32_t DW5_LumaDcQuantScaleDecodeModeOnly;
    uint32_t DW5_LumaAcQuantScaleDecodeModeOnly;    
    uint32_t DW6_ChromaDcQuantScaleDecodeModeOnly;
    uint32_t DW6_ChromaAcQuantScaleDecodeModeOnly;
} DECODE_EVENTDATA_CMD_HCPVP9SEGMENTSTATE;

typedef struct _DECODE_EVENTDATA_CMD_HCPVP9PICSTATE
{
    uint32_t DW0_Value;
    uint32_t DW1_Value;
    uint32_t DW2_Value;
    uint32_t DW3_Value;
    uint32_t DW4_Value;
    uint32_t DW5_Value;
    uint32_t DW6_Value;
    uint32_t DW7_Value;
    uint32_t DW8_Value;
    uint32_t DW9_Value;
    uint32_t DW10_Value;
    uint32_t DW1_FrameWidthInPixelsMinus1;
    uint32_t DW1_FrameHeightInPixelsMinus1;    
    uint32_t DW2_FrameType;
    uint32_t DW2_AdaptProbabilitiesFlag;
    uint32_t DW2_IntraonlyFlag;
    uint32_t DW2_RefreshFrameContext;
    uint32_t DW2_ErrorResilientMode;
    uint32_t DW2_FrameParallelDecodingMode;
    uint32_t DW2_FilterLevel;
    uint32_t DW2_SharpnessLevel;
    uint32_t DW2_SegmentationEnabled;
    uint32_t DW2_SegmentationUpdateMap;
    uint32_t DW2_LosslessMode;
    uint32_t DW2_SegmentIdStreamoutEnable;
    uint32_t DW2_SegmentIdStreaminEnable;
    uint32_t DW2_AllowHiPrecisionMv;
    uint32_t DW2_McompFilterType;
    uint32_t DW2_SegmentationTemporalUpdate;
    uint32_t DW2_RefFrameSignBias02;
    uint32_t DW2_LastFrameType;
    uint32_t DW2_UsePrevInFindMvReferences;    
    uint32_t DW3_Log2TileRow;
    uint32_t DW3_Log2TileColumn;
    uint32_t DW3_ChromaSamplingFormat;
    uint32_t DW3_Bitdepthminus8;
    uint32_t DW3_ProfileLevel;    
    uint32_t DW4_HorizontalScaleFactorForLast;
    uint32_t DW4_VerticalScaleFactorForLast;    
    uint32_t DW5_HorizontalScaleFactorForGolden;
    uint32_t DW5_VerticalScaleFactorForGolden;    
    uint32_t DW6_HorizontalScaleFactorForAltref;
    uint32_t DW6_VerticalScaleFactorForAltref;    
    uint32_t DW7_LastFrameWidthInPixelsMinus1;
    uint32_t DW7_LastFrameHieghtInPixelsMinus1;    
    uint32_t DW8_GoldenFrameWidthInPixelsMinus1;
    uint32_t DW8_GoldenFrameHieghtInPixelsMinus1;    
    uint32_t DW9_AltrefFrameWidthInPixelsMinus1;
    uint32_t DW9_AltrefFrameHieghtInPixelsMinus1;    
    uint32_t DW10_UncompressedHeaderLengthInBytes7;
    uint32_t DW10_FirstPartitionSizeInBytes150;
} DECODE_EVENTDATA_CMD_HCPVP9PICSTATE;

typedef struct _DECODE_EVENTDATA_CMD_HCPHEVCPICSTATE
{
    uint32_t DW0_Value;
    uint32_t DW1_Value;
    uint32_t DW2_Value;
    uint32_t DW4_Value;
    uint32_t DW5_Value;    
    uint32_t DW36_Value; 
    uint32_t DW1_Framewidthinmincbminus1;              
    uint32_t DW1_Frameheightinmincbminus1;             
    uint32_t DW2_ChromaSubsampling;                   
    uint32_t DW2_Mincusize;                     
    uint32_t DW2_CtbsizeLcusize;                    
    uint32_t DW2_Maxtusize;                      
    uint32_t DW2_Mintusize;                      
    uint32_t DW2_Minpcmsize;                       
    uint32_t DW2_Maxpcmsize;                       
    uint32_t DW4_SampleAdaptiveOffsetEnabledFlag;      
    uint32_t DW4_PcmEnabledFlag;             
    uint32_t DW4_CuQpDeltaEnabledFlag;            
    uint32_t DW4_DiffCuQpDeltaDepthOrNamedAsMaxDqpDepth;
    uint32_t DW4_PcmLoopFilterDisableFlag;
    uint32_t DW4_ConstrainedIntraPredFlag;   
    uint32_t DW4_Log2ParallelMergeLevelMinus2;
    uint32_t DW4_SignDataHidingFlag;  
    uint32_t DW4_LoopFilterAcrossTilesEnabledFlag;    
    uint32_t DW4_EntropyCodingSyncEnabledFlag;    
    uint32_t DW4_TilesEnabledFlag;        
    uint32_t DW4_WeightedPredFlag;         
    uint32_t DW4_WeightedBipredFlag;         
    uint32_t DW4_Fieldpic;
    uint32_t DW4_Bottomfield;
    uint32_t DW4_TransformSkipEnabledFlag;
    uint32_t DW4_AmpEnabledFlag;
    uint32_t DW4_TransquantBypassEnableFlag;
    uint32_t DW4_StrongIntraSmoothingEnableFlag;
    uint32_t DW5_BitDepthChromaMinus8;
    uint32_t DW5_BitDepthLumaMinus8;              
    uint32_t DW5_PicCbQpOffset;                   
    uint32_t DW5_PicCrQpOffset;   
    uint32_t DW5_MaxTransformHierarchyDepthIntraOrNamedAsTuMaxDepthIntra;
    uint32_t DW5_MaxTransformHierarchyDepthInterOrNamedAsTuMaxDepthInter;
    uint32_t DW5_PcmSampleBitDepthChromaMinus1;
    uint32_t DW5_PcmSampleBitDepthLumaMinus1;
    uint32_t DW36_FrameCrcEnable;
    uint32_t DW36_FrameCrcType;
} DECODE_EVENTDATA_CMD_HCPHEVCPICSTATE;

typedef struct _DECODE_EVENTDATA_CMD_HCPHEVCREXTPICSTATE
{
    uint32_t DW0_Value;
    uint32_t DW2_Value;
    uint32_t DW3_Value;
    uint32_t DW32_Value;
    uint32_t DW33_Value;
    uint32_t DW2_ChromaQpOffsetListEnabledFlag;
    uint32_t DW2_DiffCuChromaQpOffsetDepth;
    uint32_t DW2_ChromaQpOffsetListLenMinus1;
    uint32_t DW2_Log2SaoOffsetScaleLuma;
    uint32_t DW2_Log2SaoOffsetScaleChroma;
    uint32_t DW3_Log2Maxtransformskipsize;     
    uint32_t DW3_CrossComponentPredictionEnabledFlag;
    uint32_t DW3_CabacBypassAlignmentEnabledFlag;   
    uint32_t DW3_PersistentRiceAdaptationEnabledFlag;
    uint32_t DW3_IntraSmoothingDisabledFlag;      
    uint32_t DW3_ExplicitRdpcmEnabledFlag;        
    uint32_t DW3_ImplicitRdpcmEnabledFlag;         
    uint32_t DW3_TransformSkipContextEnabledFlag;  
    uint32_t DW3_TransformSkipRotationEnabledFlag;  
    uint32_t DW3_HighPrecisionOffsetsEnableFlag;
    uint32_t DW32_CbQpOffsetList0;
    uint32_t DW32_CbQpOffsetList1;
    uint32_t DW32_CbQpOffsetList2;
    uint32_t DW32_CbQpOffsetList3;
    uint32_t DW32_CbQpOffsetList4;
    uint32_t DW32_CbQpOffsetList5;
    uint32_t DW33_CrQpOffsetList0;
    uint32_t DW33_CrQpOffsetList1;
    uint32_t DW33_CrQpOffsetList2;
    uint32_t DW33_CrQpOffsetList3;
    uint32_t DW33_CrQpOffsetList4;
    uint32_t DW33_CrQpOffsetList5;
} DECODE_EVENTDATA_CMD_HCPHEVCREXTPICSTATE;

typedef struct _DECODE_EVENTDATA_CMD_HCPHEVCSCCPICSTATE
{
    uint32_t DW0_Value;
    uint32_t DW34_Value;
    uint32_t DW35_Value;
    uint32_t DW34_IbcMotionCompensationBufferReferenceIdc;  
    uint32_t DW34_PpsActCrQpOffsetPlus3;                
    uint32_t DW34_PpsActCbQpOffsetPlus5;                  
    uint32_t DW34_PpsActYOffsetPlus5;                    
    uint32_t DW34_PpsSliceActQpOffsetsPresentFlag;         
    uint32_t DW34_ResidualAdaptiveColourTransformEnabledFlag;
    uint32_t DW34_PpsCurrPicRefEnabledFlag;        
    uint32_t DW34_MotionVectorResolutionControlIdc;       
    uint32_t DW34_IntraBoundaryFilteringDisabledFlag;
    uint32_t DW34_DeblockingFilterOverrideEnabledFlag;      
    uint32_t DW34_PpsDeblockingFilterDisabledFlag;        
    uint32_t DW35_PaletteMaxSize;              
    uint32_t DW35_DeltaPaletteMaxPredictorSize;           
    uint32_t DW35_IbcMotionVectorErrorHandlingDisable;     
    uint32_t DW35_ChromaBitDepthEntryMinus8;           
    uint32_t DW35_LumaBitDepthEntryMinus8;              
    uint32_t DW35_IbcConfiguration;                 
    uint32_t DW35_MonochromePaletteFlag;                   
    uint32_t DW35_PaletteModeEnabledFlag;
} DECODE_EVENTDATA_CMD_HCPHEVCSCCPICSTATE;

void DecodeEventDataCmdHcpPipeModeSelectInit(
    DECODE_EVENTDATA_CMD_HCPPIPEMODESELECT        *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_PIPE_MODE_SELECT_CMD *pCmd);

void DecodeEventDataCmdHcpSurfaceStateInit(
    DECODE_EVENTDATA_CMD_HCPSURFACESTATE          *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_SURFACE_STATE_CMD    *pCmd);

void DecodeEventDataCmdHcpPipeBufAddrStateInit(
    DECODE_EVENTDATA_CMD_HCPPIPEBUFADDRSTATE         *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_PIPE_BUF_ADDR_STATE_CMD *pCmd,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS pParams);

void DecodeEventDataCmdHcpIndObjBaseAddrStateInit(
    DECODE_EVENTDATA_CMD_HCPINDOBJBASEADDRSTATE          *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_IND_OBJ_BASE_ADDR_STATE_CMD *pCmd);

void DecodeEventDataCmdHcpVp9SegmentStateInit(
    DECODE_EVENTDATA_CMD_HCPVP9SEGMENTSTATE        *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_VP9_SEGMENT_STATE_CMD *pCmd);

void DecodeEventDataCmdHcpVp9PicStateInit(
    DECODE_EVENTDATA_CMD_HCPVP9PICSTATE        *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_VP9_PIC_STATE_CMD *pCmd);

void DecodeEventDataCmdHcpHevcPicStateInit(
    DECODE_EVENTDATA_CMD_HCPHEVCPICSTATE   *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_PIC_STATE_CMD *pCmd);

void DecodeEventDataCmdHcpHevcRextPicStateInit(
    DECODE_EVENTDATA_CMD_HCPHEVCREXTPICSTATE *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_PIC_STATE_CMD *  pCmd);

void DecodeEventDataCmdHcpHevcSccPicStateInit(
    DECODE_EVENTDATA_CMD_HCPHEVCSCCPICSTATE *pEventData,
    mhw_vdbox_hcp_g12_X::HCP_PIC_STATE_CMD  *pCmd);

#endif

#endif