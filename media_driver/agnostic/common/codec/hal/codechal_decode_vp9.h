/*
* Copyright (c) 2012-2020, Intel Corporation
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
//! \file     codechal_decode_vp9.h
//! \brief    Defines the decode interface extension for VP9.
//! \details  Defines all types, macros, and functions required by CodecHal for VP9 decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODE_VP9_H__
#define __CODECHAL_DECODE_VP9_H__

#include "codechal_decoder.h"
#include "codec_def_vp9_probs.h"

//!
//! \struct _CODECHAL_DECODE_VP9_PROB_UPDATE
//! \brief  Define variables for VP9 decode probabilty updated
//!
typedef struct _CODECHAL_DECODE_VP9_PROB_UPDATE
{

    int32_t      bSegProbCopy;     //!< seg tree and pred prob update with values from App.
    int32_t      bProbSave;        //!< Save prob buffer
    int32_t      bProbRestore;     //!< Restore prob buffer
    int32_t      bProbReset;       //!<  1: reset; 0: not reset
    int32_t      bResetFull;       //!< full reset or partial (section A) reset
    int32_t      bResetKeyDefault; //!<  reset to key or inter default
    uint8_t      SegTreeProbs[7];  //!< Segment tree prob buffers
    uint8_t      SegPredProbs[3];  //!< Segment predict prob buffers
} CODECHAL_DECODE_VP9_PROB_UPDATE, *PCODECHAL_DECODE_VP9_PROB_UPDATE;

//!
//! \class CodechalDecodeVp9
//! \brief This class defines the member fields, functions etc used by VP9 decoder.
//!
class CodechalDecodeVp9 : public CodechalDecode
{
public:
    //!
    //! \brief    Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeVp9(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeVp9();

    //!
    //! \brief    Allocate and initialize VP9 decoder standard
    //! \param    [in] settings
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard (
        CodechalSetting *          settings) override;

    //!
    //! \brief  Set states for each frame to prepare for VP9 decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates () override;

    //!
    //! \brief    VP9 decoder state level function
    //! \details  State level function for VP9 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodeStateLevel () override;

    //!
    //! \brief    VP9 decoder primitive level function
    //! \details  Primitive level function for GEN specific VP9 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DecodePrimitiveLevel () override;

    MOS_STATUS  InitMmcState() override;

    //!
    //! \brief    Init context buffer 
    //! \details
    //! \param    [in,out] ctxBuffer 
    //!           Pointer to context buffer 
    //!
    //! \param    [in] setToKey 
    //!           Specify if it's key frame 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ContextBufferInit(
            uint8_t *ctxBuffer,
            bool setToKey);

    //!
    //! \brief    Populate prob values which are different between Key and Non-Key frame 
    //! \details
    //! \param    [in,out] ctxBuffer 
    //!           Pointer to context buffer 
    //!
    //! \param    [in] setToKey 
    //!           Specify if it's key frame 
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CtxBufDiffInit(
            uint8_t *ctxBuffer,
            bool setToKey);

    //!
    //! \brief    Intialize VP9 decode mode
    //! \details  Do nothing for base class, will be overloaded by inheritted class for dynamic mode switch support
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  InitializeDecodeMode ();

    //!
    //! \brief    Check and initialize SFC support and state
    //! \details  Check whether SFC can be used and initialize SFC if it is supported in VP9 decode
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitSfcState();

    //!
    //! \brief    Allocate fixed sized resources
    //! \details  Allocate fixed sized resources VP9 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResourcesFixedSizes();

    //!
    //! \brief    Allocate variable sized resources
    //! \details  Allocate variable sized resources in VP9 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResourcesVariableSizes();

    //!
    //! \brief    Determine Decode Phase
    //! \details  Determine decode phase in VP9 decode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DetermineDecodePhase();

    // Parameters passed by application
    uint16_t                  m_usFrameWidthAlignedMinBlk;   //!< Picture Width aligned to minBlock
    uint16_t                  m_usFrameHeightAlignedMinBlk;  //!< Picture Height aligned to minBlock
    uint8_t                   m_vp9DepthIndicator;           //!< Indicate it is 8/10/12 bit VP9
    uint8_t                   m_chromaFormatinProfile;       //!< Chroma format with current profilce
    uint32_t                  m_dataSize;                    //!< Data size
    uint32_t                  m_dataOffset;                  //!< Date offset
    PCODEC_VP9_PIC_PARAMS     m_vp9PicParams = nullptr;                //!< Pointer to VP9 picture parameter
    PCODEC_VP9_SEGMENT_PARAMS m_vp9SegmentParams = nullptr;            //!< Pointer to VP9 segment parameter
    PCODEC_VP9_SLICE_PARAMS   m_vp9SliceParams = nullptr;              //!< Pointer to VP9 slice parameter
    MOS_SURFACE               m_destSurface;                 //!< MOS_SURFACE of render surface
    PMOS_RESOURCE             m_presLastRefSurface = nullptr;          //!< Pointer to last reference surface
    PMOS_RESOURCE             m_presGoldenRefSurface = nullptr;        //!< Pointer to golden reference surface
    PMOS_RESOURCE             m_presAltRefSurface = nullptr;           //!< Pointer to alternate reference surface
    MOS_SURFACE               m_lastRefSurface;              //!< MOS_SURFACE of last reference surface
    MOS_SURFACE               m_goldenRefSurface;            //!< MOS_SURFACE of golden reference surface
    MOS_SURFACE               m_altRefSurface;               //!< MOS_SURFACE of alternate reference surface
    MOS_RESOURCE              m_resDataBuffer;               //!< Handle of bitstream data surface
    MOS_RESOURCE              m_resCoefProbBuffer;           //!< Handle of Coefficient Probability Data surface

    // Internally maintained
    uint8_t         m_frameCtxIdx;
    MOS_RESOURCE    m_resDeblockingFilterLineRowStoreScratchBuffer;         //!< Handle of Deblocking Filter Line Row Store Scratch data surface
    MOS_RESOURCE    m_resDeblockingFilterTileRowStoreScratchBuffer;         //!< Handle of Deblocking Filter Tile Row Store Scratch data surface
    MOS_RESOURCE    m_resDeblockingFilterColumnRowStoreScratchBuffer;       //!< Handle of Deblocking Filter Column Row Store Scratch data surface
    MOS_RESOURCE    m_resMetadataLineBuffer;                                //!< Handle of Metadata Line data buffer
    MOS_RESOURCE    m_resMetadataTileLineBuffer;                            //!< Handle of Metadata Tile Line data buffer
    MOS_RESOURCE    m_resMetadataTileColumnBuffer;                          //!< Handle of Metadata Tile Column data buffer
    MOS_RESOURCE    m_resHvcLineRowstoreBuffer;                             //!< Handle of HVC Line Row Store surface
    MOS_RESOURCE    m_resHvcTileRowstoreBuffer;                             //!< Handle of HVC Tile Row Store surface
    MOS_RESOURCE    m_resVp9ProbBuffer[CODEC_VP9_NUM_CONTEXTS + 1];         //!< Handle of VP9 Probability surface
    MOS_RESOURCE    m_resVp9SegmentIdBuffer;                                //!< Handle of VP9 Segment ID surface
    MOS_RESOURCE    m_resVp9MvTemporalBuffer[CODECHAL_VP9_NUM_MV_BUFFERS];  //!< Handle of VP9 MV Temporal buffer
    PCODEC_REF_LIST m_vp9RefList[CODECHAL_NUM_UNCOMPRESSED_SURFACE_VP9];    //!< Pointer to reference list
    MOS_RESOURCE    m_resSyncObject;                                        //!< Handle of synce object
    uint8_t         m_curMvTempBufIdx;                                      //!< Current mv temporal buffer index
    uint8_t         m_colMvTempBufIdx;                                      //!< Colocated mv temporal buffer index
    MOS_RESOURCE    m_resCopyDataBuffer;                                    //!< Handle of buffer to store copy data
    uint32_t        m_copyDataBufferSize;                                   //!< Buffer size of copy data
    uint32_t        m_copyDataOffset;                                       //!< Data offset of copy data
    bool            m_copyDataBufferInUse;                                  //!< Indicate if copy data buffer is in use
    MOS_RESOURCE    m_resSyncObjectWaContextInUse;                          //!< signals on the video WA context
    MOS_RESOURCE    m_resSyncObjectVideoContextInUse;                       //!< signals on the video contex
    uint32_t        m_hcpDecPhase;                                          //!< Hcp Decode phase

    union
    {
        struct
        {
            uint8_t                 KeyFrame            : 1;                    //!< [0..1]
            uint8_t                 IntraOnly           : 1;                    //!< [0..1]
            uint8_t                 Display             : 1;                    //!< [0..1]
            uint8_t                 ReservedField       : 5;                    //!< [0]
        } fields;
        uint8_t                     value;
    } m_prevFrameParams;  //!< Previous frame parameters

    uint32_t m_prevFrmWidth;         //!< Frame width of the previous frame
    uint32_t m_prevFrmHeight;        //!< Frame height of the previous frame
    uint32_t m_allocatedWidthInSb;   //!< Frame width in super block
    uint32_t m_allocatedHeightInSb;  //!< Frame height in super block
    uint32_t m_mvBufferSize;         //!< MV buffer size

    //for Internal buffer upating
    bool    m_resetSegIdBuffer;                                //!< if segment id buffer need to do reset
    bool    m_pendingResetPartial;                             //!< indicating if there is pending partial reset operation on prob buffer 0.
    bool    m_saveInterProbs;                                  //!< indicating if inter probs is saved for prob buffer 0.
    bool    m_pendingResetFullTables[CODEC_VP9_NUM_CONTEXTS];  //!< indicating if there is pending full frame context table reset operation on each prob buffers except 0.
    bool    m_pendingCopySegProbs[CODEC_VP9_NUM_CONTEXTS];     //!< indicating if there is pending seg probs copy operation on each prob buffers.
    uint8_t m_segTreeProbs[7];                                 //!< saved seg tree probs for pending seg probs copy operation to use
    uint8_t m_segPredProbs[3];                                 //!< saved seg pred probs for pending seg probs copy operation to use
    bool    m_fullProbBufferUpdate;                            //!< indicating if prob buffer is a full buffer update
    uint8_t m_interProbSaved[CODECHAL_VP9_INTER_PROB_SIZE];    //!< indicating if internal prob buffer saved

    uint32_t                        m_dmemBufferSize;          //!< Dmem buffer size
    MOS_RESOURCE                    m_resDmemBuffer;           //!< Handle of Dmem buffer
    MOS_RESOURCE                    m_resInterProbSaveBuffer;  //!< Handle of inter prob saved buffer
    CODECHAL_DECODE_VP9_PROB_UPDATE m_probUpdateFlags;         //!< Prob update flags
    MOS_RESOURCE                    m_resSegmentIdBuffReset;   //!< Handle of segment Id reset buffer
    MOS_RESOURCE                    m_resHucSharedBuffer;      //!< Handle of Huc shared buffer

protected:

    //!
    //! \struct CodechalDecodeVp9:: PIC_STATE_MHW_PARAMS
    //! \brief  Define MHW parameters for picture state
    //!
    typedef struct
    {
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   PipeModeSelectParams;
        PMHW_VDBOX_SURFACE_PARAMS            SurfaceParams[4];
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS      PipeBufAddrParams;
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS  IndObjBaseAddrParams;
        PMHW_VDBOX_VP9_PIC_STATE             Vp9PicState;
        PMHW_VDBOX_VP9_SEGMENT_STATE         Vp9SegmentState;
    } PIC_STATE_MHW_PARAMS;

    PIC_STATE_MHW_PARAMS            m_picMhwParams;   //!< Picture state params

    //!
    //! \brief    VP9 Prob buffer partial update with driver
    //! \details  using driver to do the parital prob buffer update
    //!           for clear decode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ProbBufferPartialUpdatewithDrv();

    //!
    //! \brief    VP9 Prob buffer full update with driver
    //! \details  full prob buffer udpate in clear decode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ProbBufFullUpdatewithDrv();

    //!
    //! \brief    VP9 Segment id buffer reset with driver
    //! \details  VP9 Segment id buffer reset with driver in clear decode
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResetSegIdBufferwithDrv();

    //!
    //! \brief    VP9 Prob buffer full update with Huc Streamout command
    //! \details  VP9 prob buffer full update with Huc Streamout
    //! \param    cmdBuffer
    //!           [in] command buffer to hold HW commands
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ProbBufFullUpdatewithHucStreamout(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    VP9 Segment Id buffer reset with HucStreamout command
    //! \details  vp9 segment id buffer reset using Huc Streamout command
    //! \param    cmdBuffer
    //!           [in] command buffer to hold HW commands
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ResetSegIdBufferwithHucStreamout(
        PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Determine the internal bufer update
    //! \details  Decide if vp9 prob buffer or segment id buffer need to do
    //!           update and what updat type it is
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DetermineInternalBufferUpdate();

    //!
    //! \brief    Initialize during begin frame
    //! \details  Initialize during begin frame in VP9 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializeBeginFrame();

    //!
    //! \brief    Copy data surface
    //! \details  Copy data surface in VP9 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CopyDataSurface();

    //!
    //! \brief    Check and copy bit stream
    //! \details  Check and copy bit stream in VP9 decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckAndCopyBitStream();

    //!
    //! \brief    Initialize picture state mhw parameters
    //! \details  Initialize picture state mhw parameters for VP9 decoder
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS          InitPicStateMhwParams();

    //!
    //! \brief    Add picture state mhw parameters
    //! \details  Add picture state mhw parameters to command buffer for VP9 decoder
    //! \param    [in] cmdBuffer
    //!           Command buffer to hold HW commands
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS  AddPicStateMhwCmds(
        PMOS_COMMAND_BUFFER       cmdBuffer
    );

    //!
    //! \brief    Update picture state buffers
    //! \details  Update picture state buffers for VP9 decoder
    //! \param    [in] cmdBuffer
    //!           Command buffer to hold HW commands
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdatePicStateBuffers(
        PMOS_COMMAND_BUFFER       cmdBuffer);

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpDecodePicParams(
        PCODEC_VP9_PIC_PARAMS picParams);

    MOS_STATUS DumpDecodeSegmentParams(
        PCODEC_VP9_SEGMENT_PARAMS segmentParams);

    MOS_STATUS DumpDecodeSliceParams(
        CODEC_VP9_SLICE_PARAMS *slcParams);
#endif
};

#endif  // __CODECHAL_DECODE_VP9_H__
