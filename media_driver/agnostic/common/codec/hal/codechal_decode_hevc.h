/*
* Copyright (c) 2012-2017, Intel Corporation
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
//! \file     codechal_decode_hevc.h
//! \brief    Defines the decode interface extension for HEVC.
//! \details  Defines all types, macros, and functions required by CodecHal for HEVC decoding. Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_HEVC_H__
#define __CODECHAL_DECODER_HEVC_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_decode_sfc_hevc.h"
#include "codechal_decoder.h"

class CodechalDecodeNV12ToP010;

const uint8_t CODECHAL_DECODE_HEVC_Qmatrix_Scan_4x4[16] = { 0, 4, 1, 8, 5, 2, 12, 9, 6, 3, 13, 10, 7, 14, 11, 15 };
const uint8_t CODECHAL_DECODE_HEVC_Qmatrix_Scan_8x8[64] =
{ 0, 8, 1, 16, 9, 2, 24, 17, 10, 3, 32, 25, 18, 11, 4, 40,
33, 26, 19, 12, 5, 48, 41, 34, 27, 20, 13, 6, 56, 49, 42, 35,
28, 21, 14, 7, 57, 50, 43, 36, 29, 22, 15, 58, 51, 44, 37, 30,
23, 59, 52, 45, 38, 31, 60, 53, 46, 39, 61, 54, 47, 62, 55, 63 };
const uint8_t CODECHAL_DECODE_HEVC_Default_4x4[16] = { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 };
const uint8_t CODECHAL_DECODE_HEVC_Default_8x8_Intra[64] =
{ 16, 16, 16, 16, 17, 18, 21, 24, 16, 16, 16, 16, 17, 19, 22, 25,
16, 16, 17, 18, 20, 22, 25, 29, 16, 16, 18, 21, 24, 27, 31, 36,
17, 17, 20, 24, 30, 35, 41, 47, 18, 19, 22, 27, 35, 44, 54, 65,
21, 22, 25, 31, 41, 54, 70, 88, 24, 25, 29, 36, 47, 65, 88, 115 };
const uint8_t CODECHAL_DECODE_HEVC_Default_8x8_Inter[64] =
{ 16, 16, 16, 16, 17, 18, 20, 24, 16, 16, 16, 17, 18, 20, 24, 25,
16, 16, 17, 18, 20, 24, 25, 28, 16, 17, 18, 20, 24, 25, 28, 33,
17, 18, 20, 24, 25, 28, 33, 41, 18, 20, 24, 25, 28, 33, 41, 54,
20, 24, 25, 28, 33, 41, 54, 71, 24, 25, 28, 33, 41, 54, 71, 91 };

typedef class CodechalDecodeHevc *PCODECHAL_DECODE_HEVC_STATE;

typedef struct _CODECHAL_DECODE_HEVC_MV_LIST
{
    uint8_t             u8FrameId;
    bool                bInUse;
    bool                bReUse;
} CODECHAL_DECODE_HEVC_MV_LIST, *PCODECHAL_DECODE_HEVC_MV_LIST;

typedef struct
{
    uint32_t    pic_width_in_min_cbs_y;
    uint32_t    pic_height_in_min_cbs_y;
    uint8_t     log2_min_luma_coding_block_size_minus3;
    uint8_t     log2_diff_max_min_luma_coding_block_size;
    uint16_t    chroma_format_idc                           : 2;  //!< range 0..3
    uint16_t    separate_colour_plane_flag                  : 1;
    uint16_t    bit_depth_luma_minus8                       : 4;
    uint16_t    bit_depth_chroma_minus8                     : 4;
    uint16_t    log2_max_pic_order_cnt_lsb_minus4           : 4;  //!< range 0..12
    uint16_t    sample_adaptive_offset_enabled_flag         : 1;
    uint8_t     num_short_term_ref_pic_sets;                      //!< range 0..64
    uint8_t     long_term_ref_pics_present_flag             : 1;
    uint8_t     num_long_term_ref_pics_sps                  : 6;  //!< range 0..32
    uint8_t     sps_temporal_mvp_enable_flag                : 1;

    uint8_t     num_ref_idx_l0_default_active_minus1        : 4;  //!< range 0..15
    uint8_t     num_ref_idx_l1_default_active_minus1        : 4;  //!< range 0..15
    int8_t      pic_init_qp_minus26;                              //!< range -62..25
    uint8_t     dependent_slice_segments_enabled_flag       : 1;
    uint8_t     cabac_init_present_flag                     : 1;
    uint8_t     pps_slice_chroma_qp_offsets_present_flag    : 1;
    uint8_t     weighted_pred_flag                          : 1;
    uint8_t     weighted_bipred_flag                        : 1;
    uint8_t     output_flag_present_flag                    : 1;
    uint8_t     tiles_enabled_flag                          : 1;
    uint8_t     entropy_coding_sync_enabled_flag            : 1;
    uint8_t     loop_filter_across_slices_enabled_flag      : 1;
    uint8_t     deblocking_filter_override_enabled_flag     : 1;
    uint8_t     pic_disable_deblocking_filter_flag          : 1;
    uint8_t     lists_modification_present_flag             : 1;
    uint8_t     slice_segment_header_extension_present_flag : 1;
    uint8_t     high_precision_offsets_enabled_flag         : 1;
    uint8_t     chroma_qp_offset_list_enabled_flag          : 1;
    uint8_t                                                 : 1;

    int32_t     CurrPicOrderCntVal;
    int32_t     PicOrderCntValList[CODEC_MAX_NUM_REF_FRAME_HEVC];
    uint8_t     RefPicSetStCurrBefore[8];
    uint8_t     RefPicSetStCurrAfter[8];
    uint8_t     RefPicSetLtCurr[8];
    uint16_t    RefFieldPicFlag;
    uint16_t    RefBottomFieldFlag;
    int8_t      pps_beta_offset_div2;
    int8_t      pps_tc_offset_div2;
    uint16_t    StRPSBits;

    uint8_t     num_tile_columns_minus1;
    uint8_t     num_tile_rows_minus1;
    uint16_t    column_width[HEVC_NUM_MAX_TILE_COLUMN];
    uint16_t    row_height[HEVC_NUM_MAX_TILE_ROW];

    uint16_t    NumSlices;
    uint8_t     num_extra_slice_header_bits;
    int8_t      RefIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC];

    struct
    {
        uint8_t     reserve_0;
        uint16_t    reserve_1;
        uint32_t    reserve_2;
        uint32_t    reserve_3;
    } reserve;
} HUC_HEVC_S2L_PIC_BSS, *PHUC_HEVC_S2L_PIC_BSS;

typedef struct {
    uint32_t    BSNALunitDataLocation;
    uint32_t    SliceBytesInBuffer;

    struct
    {
        uint32_t    reserve_0;
        uint32_t    reserve_1;
        uint32_t    reserve_2;
        uint32_t    reserve_3;
    } reserve;
} HUC_HEVC_S2L_SLICE_BSS, *PHUC_HEVC_S2L_SLICE_BSS;

typedef struct {
    // Platfrom information
    uint32_t                    ProductFamily;
    uint16_t                    RevId;

    // Picture level DMEM data
    HUC_HEVC_S2L_PIC_BSS        PictureBss;

    // Slice level DMEM data
    HUC_HEVC_S2L_SLICE_BSS      SliceBss[CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6];

} HUC_HEVC_S2L_BSS, *PHUC_HEVC_S2L_BSS;

//!
//! \class    CodechalDecodeHevc
//! \brief    Codechal decode HEVC
//!
class CodechalDecodeHevc : public CodechalDecode
{
public:

    typedef struct
    {
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   PipeModeSelectParams;
        PMHW_VDBOX_SURFACE_PARAMS            SurfaceParams;
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS      PipeBufAddrParams;
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  IndObjBaseAddrParams;
        PMHW_VDBOX_QM_PARAMS                 QmParams;
        PMHW_VDBOX_HEVC_PIC_STATE            HevcPicState;
        PMHW_VDBOX_HEVC_TILE_STATE           HevcTileState;
    } PIC_LONG_FORMAT_MHW_PARAMS;

    //!
    //! \brief    Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeHevc(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeHevc ();

    //!
    //! \brief    Allocate and initialize HEVC decoder standard
    //! \param    [in] settings
    //!           Pointer to CODECHAL_SETTINGS
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard (
        PCODECHAL_SETTINGS          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for HEVC decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates () override;

    //!
    //! \brief    HEVC decoder state level function
    //! \details  State level function for HEVC decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel () override;

    //!
    //! \brief    HEVC decoder primitive level function
    //! \details  Primitive level function for GEN specific HEVC decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodePrimitiveLevel () override;

    //!
    //! \brief    HEVC decoder downsampling calc function
    //! \details  calc downsample param for GEN specific HEVC decoder
    //!
    //! \param    [in] picParams
    //!           Pointer to picture parameters
    //! \param    [out] refSurfWidth
    //!           Pointer to reference surface width
    //! \param    [out] refSurfHeight
    //!           Pointer to reference surface height
    //! \param    [out] format
    //!           Pointer to MOS_FORMAT returned
    //! \param    [out] frameIdx
    //!           Pointer to frame index
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  CalcDownsamplingParams (
        void                        *picParams,
        uint32_t                    *refSurfWidth,
        uint32_t                    *refSurfHeight,
        MOS_FORMAT                  *format,
        uint8_t                     *frameIdx) override;

    MOS_STATUS  InitMmcState() override;

    //!
    //! \brief    Add S2L picture level commands to command buffer
    //! \details  Add S2L picture level commands to command buffer in HEVC decode driver
    //!
    //! \param    [out] cmdBufferInUse
    //!           Pointer to Command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AddPictureS2LCmds(
        PMOS_COMMAND_BUFFER             cmdBufferInUse);

    //!
    //! \brief    Send S2L picture level commands
    //! \details  Send S2L picture level commands in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  SendPictureS2L ();

    //!
    //! \brief    Send S2L Slice level commands
    //! \details  Send S2L Slice level commands in HEVC decode driver
    //!
    //! \param    [out] cmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] hevcSliceState
    //!           Pointer to HEVC Slice State
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  SendSliceS2L (
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState);

    //!
    //! \brief    Get the Huc Dmem resource size
    //! \details  Return the Huc Dmem resource size in bytes.
    //!
    //! \return   uint32_t
    //!           the size of Dmem resource
    //!
    virtual uint32_t   GetDmemBufferSize();

    //!
    //! \brief    Set S2L picture level Dmem parameters
    //! \details  Set S2L HuC Dmem picture level paramters
    //!
    //! \param    [out] hucHevcS2LPicBss
    //!           Pointer to S2L Dmem picture Bss paramters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetHucDmemS2LPictureBss(
        PHUC_HEVC_S2L_PIC_BSS           hucHevcS2LPicBss);

    //!
    //! \brief    Set S2L Slice level Dmem parameters
    //! \details  Set S2L HuC Dmem slice related paramters
    //!
    //! \param    [out] hucHevcS2LSliceBss
    //!           Pointer to S2L Dmem Slice Bss paramters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetHucDmemS2LSliceBss(
        PHUC_HEVC_S2L_SLICE_BSS         hucHevcS2LSliceBss);

    //!
    //! \brief    Setup HuC DMEM buffer
    //! \details  Setup HuC DMEM buffer in HEVC decode driver
    //!
    //! \param    [in] dmemBuffer
    //!           Pointer to HuC DMEM resource buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  SetHucDmemParams(
        PMOS_RESOURCE               dmemBuffer);

    //!
    //! \brief    Initialize the picture level MHW parameters for long format
    //! \details  Initialize the picture level MHW parameters for long format in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  InitPicLongFormatMhwParams();

    //!
    //! \brief    Add long format picture level commands to command buffer
    //! \details  Add long format picture level commands to command buffer in HEVC decode driver
    //!
    //! \param    [out] cmdBufferInUse
    //!           Pointer to Command buffer
    //! \param    [in] picMhwParams
    //!           Pointer to the picture level MHW parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPictureLongFormatCmds(
        PMOS_COMMAND_BUFFER             cmdBufferInUse,
        PIC_LONG_FORMAT_MHW_PARAMS      *picMhwParams);

    //!
    //! \brief    Send long format picture level commands
    //! \details  Send long format picture level commands in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  SendPictureLongFormat ();

    //!
    //! \brief    Send long format Slice level commands
    //! \details  Send long format Slice level commands in HEVC decode driver
    //!
    //! \param    [out] cmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] hevcSliceState
    //!           Pointer to HEVC Slice State
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          SendSliceLongFormat (
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState);

    //!
    //! \brief    Determine Decode Phase
    //! \details  Determine decode phase in hevc decode
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          DetermineDecodePhase();

    //!
    //! \brief    Set Picture Struct
    //! \details  Set Picture Struct in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          SetPictureStructs();

    //!
    //! \brief    Concatenate the bitstreams and save them to locl buffer if multiple execution call
    //! \details  Concatenate the bitstreams and save them to locl buffer if multiple execution call in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          CheckAndCopyBitstream();

    //!
    //! \brief    Copy bitstream to local buffer
    //! \details  Copy bitstream to local buffer in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          CopyDataSurface();

    //!
    //! \brief    Initialize status for bitstream concatenating
    //! \details  Initialize status for bitstream concatenating in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          InitializeBitstreamCat ();

    //!
    //! \brief    Get all tile information
    //! \details  Get all tile information in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          GetAllTileInfo();

    //!
    //! \brief    Allocate variable sized resources
    //! \details  Allocate variable sized resources in HEVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AllocateResourcesVariableSizes();

    //!
    //! \brief    Allocate the MV Temporal buffer
    //!
    //! \param    [in] hevcMvBuffIndex
    //!           buff index for unused mv buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AllocateMvTemporalBuffer(
        uint8_t                       hevcMvBuffIndex);

    //!
    //! \brief    Allocate fixed sized resources
    //! \details  Allocate fixed sized resources HEVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          AllocateResourcesFixedSizes();

    //!
    //! \brief    Get mv buffer index
    //! \details  Find out an unused MvBuffer and get hte mv buffer index in HEVC decode driver
    //!
    //! \param    [in] frameIdx
    //!           frame idx associated with unused mv buffer
    //!
    //! \return   uint8_t
    //!           MV buffer index if success, else return CODEC_NUM_HEVC_MV_BUFFERS
    //!
    uint8_t             GetMvBufferIndex(
        uint8_t                         frameIdx);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        PCODEC_HEVC_PIC_PARAMS     picParams,
        void*                      extPicParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_HEVC_SLICE_PARAMS     sliceParams,
        void*                        extSliceParams,
        uint32_t                     numSlices,
        bool                         shortFormatInUse);

    MOS_STATUS DumpIQParams(
        PCODECHAL_HEVC_IQ_MATRIX_PARAMS matrixData);

#endif

    static const uint32_t  m_hucS2lKernelId = 1;             //!< VDBox Huc decode S2L kernel descriptoer

    // Parameters passed by application
    uint32_t                        u32MinCtbSize;                              //!< Min Luma Coding Block Size
    bool                            bIs10bitHEVC;                               //!< Indicate it is 10 bit HEVC
    uint8_t                         u8ChromaFormatinProfile;                    //!< Chrama format in profile
    bool                            bShortFormatInUse;                          //!< Indicate short format is inuse
    uint32_t                        u32DataSize;                                //!< Size of bitstream
    uint32_t                        u32DataOffset;                              //!< Offset of bitstream
    uint32_t                        u32NumSlices;                               //!< Num of slices
    PCODEC_HEVC_PIC_PARAMS          pHevcPicParams;                             //!< Pointer to HEVC picture parameter
    PCODEC_HEVC_SLICE_PARAMS        pHevcSliceParams;                           //!< Pointer to HEVC slice parameter
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS pHevcIqMatrixParams;                        //!< Pointer to HEVC IQ matrix parameter
    MOS_SURFACE                     sDestSurface;                               //!< Handle of render surface
    PMOS_RESOURCE                   presReferences[CODEC_MAX_NUM_REF_FRAME_HEVC]; //!< Pointer to Handle of Reference Frames
    MOS_RESOURCE                    resDataBuffer;                              //!< Handle of bitstream data surface

    bool                            bIs8bitFrameIn10bitHevc;                    //!< Indicate 8bit frame in 10bit HEVC stream
    MOS_SURFACE                     sInternalNV12RTSurfaces[CODECHAL_NUM_INTERNAL_NV12_RT_HEVC]; //!< Handles of internal NV12 render target
    uint32_t                        u32InternalNV12RTIndexMap[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC]; //!< Index map for internal NV12 render targets
    bool                            bInternalNV12RTIndexMapInitilized;          //!< Indicate the index map of internal NV12 render targets is intialized
    //! \brief Context for NV12ToP010 Kernel
    CodechalDecodeNV12ToP010        *m_decodeNV12ToP010 = nullptr;

    //Track for several row store buffer's max picture width in MB used for buffer allocation in past frames
    uint32_t                        u32MfdDeblockingFilterRowStoreScratchBufferPicWidth;    //!< max picture width for deblocking filter row store scratch buffer
    uint32_t                        u32MetadataLineBufferPicWidth;                          //!< max picture width for meta data line buffer
    uint32_t                        u32SaoLineBufferPicWidth;                               //!< max picture width for SAO line buffer

    // Internally maintained
    MOS_RESOURCE                    resMfdDeblockingFilterRowStoreScratchBuffer;            //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
    MOS_RESOURCE                    resDeblockingFilterTileRowStoreScratchBuffer;           //!< Handle of Deblocking Filter Tile Row Store Scratch data surface
    MOS_RESOURCE                    resDeblockingFilterColumnRowStoreScratchBuffer;         //!< Handle of Deblocking Filter Column Row Store Scratch data surface
    MOS_RESOURCE                    resMetadataLineBuffer;                                  //!< Handle of Metadata Line data buffer
    MOS_RESOURCE                    resMetadataTileLineBuffer;                              //!< Handle of Metadata Tile Line data buffer
    MOS_RESOURCE                    resMetadataTileColumnBuffer;                            //!< Handle of Metadata Tile Column data buffer
    MOS_RESOURCE                    resSaoLineBuffer;                                       //!< Handle of SAO Line data buffer
    MOS_RESOURCE                    resSaoTileLineBuffer;                                   //!< Handle of SAO Tile Line data buffer
    MOS_RESOURCE                    resSaoTileColumnBuffer;                                 //!< Handle of SAO Tile Column data buffer
    MOS_RESOURCE                    resMvTemporalBuffer[CODEC_NUM_HEVC_MV_BUFFERS];         //!< Handles of MV Temporal data buffer
    MHW_BATCH_BUFFER                secondLevelBatchBuffer;                                 //!< Handle of second level batch buffer
    uint32_t                        u32DmemBufferIdx;                                       //!< Indicate current idx of DMEM buffer to program
    MOS_RESOURCE                    resDmemBuffer[CODECHAL_HEVC_NUM_DMEM_BUFFERS];          //!< Handles of DMEM buffer
    uint32_t                        u32DmemBufferSize;                                      //!< Size of DMEM buffer
    uint32_t                        u32DmemTransferSize;                                    //!< Transfer size of DMEM data
    bool                            bDmemBufferProgrammed;                                  //!< Indicate DMEM buffer is programmed
    MOS_RESOURCE                    resCopyDataBuffer;                                      //!< Handle of copied bitstream buffer
    uint32_t                        u32CopyDataBufferSize;                                  //!< Size of copied bitstream buffer
    uint32_t                        u32CopyDataOffset;                                      //!< Offset of copied bitstream
    bool                            bCopyDataBufferInUse;                                   //!< Indicate copied bistream is inuse
    uint32_t                        u32EstiBytesInBitstream;                                //!< Estimated size of bitstream

    MOS_RESOURCE                    resSyncObjectWaContextInUse;                            //!< signals on the video WA context

    bool                            bCurPicIntra;                                           //!< Indicate current picture is intra
    uint32_t                        u32MVBufferSize;                                        //!< Size of MV buffer

    CODEC_PICTURE                   CurrPic;                                                //!< Current Picture Struct
    PCODEC_REF_LIST                 pHevcRefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];   //!< Pointer to reference list
    uint8_t                         hevcMvBufferIndex;                                      //!< Index of MV buffer for current picture
    CODECHAL_DECODE_HEVC_MV_LIST    hevcMvList[CODEC_NUM_HEVC_MV_BUFFERS];                  //!< Status table of MV buffers
    bool                            bFrameUsedAsCurRef[CODEC_MAX_NUM_REF_FRAME_HEVC];       //!< Indicate frames used as reference of current picture
    int8_t                          RefIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC];            //!< Map table of indices of references
    uint32_t                        u32FrameIdx;                                            //!< Decode order index of current frame
    bool                            bEnableSF2DMASubmits;                                   //!< Indicate two DMA submits is enabled on short format

    uint16_t                        u16TileColWidth[HEVC_NUM_MAX_TILE_COLUMN];              //!< Table of tile column width
    uint16_t                        u16TileRowHeight[HEVC_NUM_MAX_TILE_ROW];                //!< Table of tile row height

    uint32_t                        u32WidthLastMaxAlloced;                                 //!< Max Picture Width used for buffer allocation in past frames
    uint32_t                        u32HeightLastMaxAlloced;                                //!< Max Picture Height used for buffer allocation in past frames
    uint32_t                        u32CtbLog2SizeYMax;                                     //!< Max CtbLog2 size used for buffer allocation in past frames

    uint32_t                        HcpDecPhase;                                            //!< HCP decode phase

#ifdef _DECODE_PROCESSING_SUPPORTED
    // SFC
    CodechalHevcSfcState         *m_sfcState;                                            //!< HEVC SFC State
#endif
    PIC_LONG_FORMAT_MHW_PARAMS      m_picMhwParams;                                         //!< picture parameters
};

#endif  // __CODECHAL_DECODER_HEVC_H__
