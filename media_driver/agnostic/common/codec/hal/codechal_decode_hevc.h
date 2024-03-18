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

    // Flag to indicate if create dummy HCP_REF_IDX_STATE or not
    uint32_t                    DummyRefIdxState;

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
    //! \brief    Copy constructor
    //!
    CodechalDecodeHevc(const CodechalDecodeHevc&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeHevc& operator=(const CodechalDecodeHevc&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeHevc ();

    //!
    //! \brief    Allocate and initialize HEVC decoder standard
    //! \param    [in] settings
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard (
        CodechalSetting *settings) override;

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

    MOS_STATUS DumpHucS2l(PMHW_VDBOX_HUC_DMEM_STATE_PARAMS hucDmemStateParams);

#endif

    static const uint32_t  m_hucS2lKernelId = 1;             //!< VDBox Huc decode S2L kernel descriptoer

    // Parameters passed by application
    uint32_t                        m_minCtbSize;                                    //!< Min Luma Coding Block Size
    bool                            m_is10BitHevc;                                   //!< Indicate it is 10 bit HEVC
    bool                            m_is12BitHevc;                                   //!< Indicate it is 12 bit HEVC
    uint8_t                         m_chromaFormatinProfile;                         //!< Chrama format in profile
    bool                            m_shortFormatInUse;                              //!< Indicate short format is inuse
    uint32_t                        m_dataSize;                                      //!< Size of bitstream
    uint32_t                        m_dataOffset;                                    //!< Offset of bitstream
    uint32_t                        m_numSlices;                                     //!< Num of slices
    PCODEC_HEVC_PIC_PARAMS          m_hevcPicParams;                                 //!< Pointer to HEVC picture parameter
    PCODEC_HEVC_SLICE_PARAMS        m_hevcSliceParams;                               //!< Pointer to HEVC slice parameter
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS m_hevcIqMatrixParams;                            //!< Pointer to HEVC IQ matrix parameter
    MOS_SURFACE                     m_destSurface;                                   //!< Handle of render surface
    PMOS_RESOURCE                   m_presReferences[CODEC_MAX_NUM_REF_FRAME_HEVC];  //!< Pointer to Handle of Reference Frames
    MOS_RESOURCE                    m_resDataBuffer;                                 //!< Handle of bitstream data surface

    bool        m_is8BitFrameIn10BitHevc;                                          //!< Indicate 8bit frame in 10bit HEVC stream
    MOS_SURFACE m_internalNv12RtSurfaces[CODECHAL_NUM_INTERNAL_NV12_RT_HEVC];      //!< Handles of internal NV12 render target
    uint32_t    m_internalNv12RtIndexMap[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];  //!< Index map for internal NV12 render targets
    bool        m_internalNv12RtIndexMapInitilized;                                //!< Indicate the index map of internal NV12 render targets is intialized
    //! \brief Context for NV12ToP010 Kernel
    CodechalDecodeNV12ToP010        *m_decodeNV12ToP010 = nullptr;

    //Track for several row store buffer's max picture width in MB used for buffer allocation in past frames
    uint32_t m_mfdDeblockingFilterRowStoreScratchBufferPicWidth;  //!< max picture width for deblocking filter row store scratch buffer
    uint32_t m_metadataLineBufferPicWidth;                        //!< max picture width for meta data line buffer
    uint32_t m_saoLineBufferPicWidth;                             //!< max picture width for SAO line buffer

    // Internally maintained
    MOS_RESOURCE     m_resMfdDeblockingFilterRowStoreScratchBuffer;     //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
    MOS_RESOURCE     m_resDeblockingFilterTileRowStoreScratchBuffer;    //!< Handle of Deblocking Filter Tile Row Store Scratch data surface
    MOS_RESOURCE     m_resDeblockingFilterColumnRowStoreScratchBuffer;  //!< Handle of Deblocking Filter Column Row Store Scratch data surface
    MOS_RESOURCE     m_resMetadataLineBuffer;                           //!< Handle of Metadata Line data buffer
    MOS_RESOURCE     m_resMetadataTileLineBuffer;                       //!< Handle of Metadata Tile Line data buffer
    MOS_RESOURCE     m_resMetadataTileColumnBuffer;                     //!< Handle of Metadata Tile Column data buffer
    MOS_RESOURCE     m_resSaoLineBuffer;                                //!< Handle of SAO Line data buffer
    MOS_RESOURCE     m_resSaoTileLineBuffer;                            //!< Handle of SAO Tile Line data buffer
    MOS_RESOURCE     m_resSaoTileColumnBuffer;                          //!< Handle of SAO Tile Column data buffer
    bool             m_mvBufferProgrammed;                              //!< Indicate mv buffer is programmed
    MOS_RESOURCE     m_resMvTemporalBuffer[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];  //!< Handles of MV Temporal data buffer, only use CODEC_NUM_HEVC_MV_BUFFERS normally
    uint32_t         m_secondLevelBatchBufferIndex;
    MHW_BATCH_BUFFER m_secondLevelBatchBuffer[CODEC_HEVC_NUM_SECOND_BB];//!< Handle of second level batch buffer
    uint32_t         m_dmemBufferIdx;                                   //!< Indicate current idx of DMEM buffer to program
    MOS_RESOURCE     m_resDmemBuffer[CODECHAL_HEVC_NUM_DMEM_BUFFERS];   //!< Handles of DMEM buffer
    uint32_t         m_dmemBufferSize;                                  //!< Size of DMEM buffer
    uint32_t         m_dmemTransferSize;                                //!< Transfer size of DMEM data
    bool             m_dmemBufferProgrammed;                            //!< Indicate DMEM buffer is programmed
    MOS_RESOURCE     m_resCopyDataBuffer;                               //!< Handle of copied bitstream buffer
    uint32_t         m_copyDataBufferSize;                              //!< Size of copied bitstream buffer
    uint32_t         m_copyDataOffset;                                  //!< Offset of copied bitstream
    bool             m_copyDataBufferInUse;                             //!< Indicate copied bistream is inuse
    uint32_t         m_estiBytesInBitstream;                            //!< Estimated size of bitstream

    MOS_RESOURCE m_resSyncObjectWaContextInUse;  //!< signals on the video WA context

    bool     m_curPicIntra;   //!< Indicate current picture is intra
    uint32_t m_mvBufferSize;  //!< Size of MV buffer

    CODEC_PICTURE                m_currPic;                                              //!< Current Picture Struct
    PCODEC_REF_LIST              m_hevcRefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_HEVC];  //!< Pointer to reference list
    uint8_t                      m_hevcMvBufferIndex;                                    //!< Index of MV buffer for current picture
    CODECHAL_DECODE_HEVC_MV_LIST m_hevcMvList[CODEC_NUM_HEVC_MV_BUFFERS];                //!< Status table of MV buffers
    bool                         m_frameUsedAsCurRef[CODEC_MAX_NUM_REF_FRAME_HEVC];      //!< Indicate frames used as reference of current picture
    int8_t                       m_refIdxMapping[CODEC_MAX_NUM_REF_FRAME_HEVC];          //!< Map table of indices of references
    uint32_t                     m_frameIdx;                                             //!< Decode order index of current frame
    bool                         m_enableSf2DmaSubmits;                                  //!< Indicate two DMA submits is enabled on short format

    uint16_t m_tileColWidth[HEVC_NUM_MAX_TILE_COLUMN];  //!< Table of tile column width
    uint16_t m_tileRowHeight[HEVC_NUM_MAX_TILE_ROW];    //!< Table of tile row height

    uint32_t m_widthLastMaxAlloced;   //!< Max Picture Width used for buffer allocation in past frames
    uint32_t m_heightLastMaxAlloced;  //!< Max Picture Height used for buffer allocation in past frames
    uint32_t m_ctbLog2SizeYMax;       //!< Max CtbLog2 size used for buffer allocation in past frames

    uint32_t m_hcpDecPhase;  //!< HCP decode phase

#ifdef _DECODE_PROCESSING_SUPPORTED
    // SFC
    CodechalHevcSfcState         *m_sfcState = nullptr;                                            //!< HEVC SFC State
#endif
    PIC_LONG_FORMAT_MHW_PARAMS      m_picMhwParams;                                         //!< picture parameters

    bool                         m_dummyReferenceSlot[CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC];

    bool m_reportHucStatus = false;
    bool m_reportHucCriticalError = false;
};

#endif  // __CODECHAL_DECODER_HEVC_H__
