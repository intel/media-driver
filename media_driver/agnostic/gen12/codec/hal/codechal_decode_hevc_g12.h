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
//! \file     codechal_decode_hevc_g12.h
//! \brief    Defines the decode interface extension for HEVC.
//! \details  Defines all types, macros, and functions required by CodecHal for GEN11 HEVC decoding. Definitions are not externally facing.
//!            This file should not be included except in codechal_decode_hevc_*.c/h.
//!

#ifndef __CODECHAL_DECODER_HEVC__G12_H__
#define __CODECHAL_DECODER_HEVC__G12_H__

#include "codechal_decode_hevc.h"
#include "codechal_decode_scalability_g12.h"
#include "codechal_decode_singlepipe_virtualengine.h"
#include "mhw_vdbox_g12_X.h"
#include "codec_def_decode_hevc.h"

// HEVC SCC Intra Block Copy Mode
#define CodecHalDecodeIsSCCIBCMode(hevcSccPicParams) \
    (hevcSccPicParams ? hevcSccPicParams->PicSCCExtensionFlags.fields.pps_curr_pic_ref_enabled_flag : false)

// HEVC SCC Palette Mode
#define CodecHalDecodeIsSCCPLTMode(hevcSccPicParams) \
    (hevcSccPicParams ? hevcSccPicParams->PicSCCExtensionFlags.fields.palette_mode_enabled_flag : false)

// HEVC SCC Adaptive Colour Transform Mode
#define CodecHalDecodeIsSCCACTMode(hevcSccPicParams) \
    (hevcSccPicParams ? hevcSccPicParams->PicSCCExtensionFlags.fields.residual_adaptive_colour_transform_enabled_flag : false)

// HEVC Wave front Parallel Processing
#define CodecHalDecodeIsWPPMode(hevcPicParams) \
    (hevcPicParams ? hevcPicParams->entropy_coding_sync_enabled_flag : false)

#define CodecHalDecodeNeedsTileDecoding(hevcPicParams, hevcSccPicParams)  \
    (hevcPicParams ? (hevcPicParams->tiles_enabled_flag                  \
                        && (CodecHalDecodeIsSCCIBCMode(hevcSccPicParams) \
                        || CodecHalDecodeIsSCCPLTMode(hevcSccPicParams)  \
                        || CodecHalDecodeIsWPPMode(hevcPicParams))) : false)

struct HUC_HEVC_S2L_PIC_BSS_G12 : public HUC_HEVC_S2L_PIC_BSS
{
    uint8_t     IsRealTileEnable;
    uint8_t     NumScalablePipes;
    uint8_t     IsSCCIBCMode;
    uint8_t     IsSCCPLTMode;
    uint8_t     MVRControlIdc;
    uint8_t     UseSliceACTOffset;
    int8_t      pps_act_y_qp_offset;
    int8_t      pps_act_cb_qp_offset;
    int8_t      pps_act_cr_qp_offset;
    uint8_t     PredictorPaletteSize;
    uint16_t    PredictorPaletteEntries[3][128];
    uint32_t    BatchBufferSize;
};
typedef struct HUC_HEVC_S2L_PIC_BSS_G12 *PHUC_HEVC_S2L_PIC_BSS_G12;

struct HUC_HEVC_S2L_BSS_G12
{
    // Platfrom information
    uint32_t                    ProductFamily;
    uint16_t                    RevId;

    // Flag to indicate if create dummy HCP_REF_IDX_STATE or not
    uint32_t                    DummyRefIdxState;

    // Picture level DMEM data
    HUC_HEVC_S2L_PIC_BSS_G12    PictureBss;

    // Slice level DMEM data
    HUC_HEVC_S2L_SLICE_BSS      SliceBss[CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6];
};
typedef struct HUC_HEVC_S2L_BSS_G12 *PHUC_HEVC_S2L_BSS_G12;

class HevcDecodeSliceLongG12;

class CodechalDecodeHevcG12 : public CodechalDecodeHevc
{
    friend class HevcDecodeSliceLongG12;

public:
    //!
    //! \brief    Allocate and initialize HEVC decoder standard
    //! \param    [in] settings
    //!           Pointer to CodechalSetting
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateStandard (
        CodechalSetting *  settings) override;

    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeHevcG12(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalDecodeHevcG12(const CodechalDecodeHevcG12&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalDecodeHevcG12& operator=(const CodechalDecodeHevcG12&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalDecodeHevcG12 ();

    //!
    //! \brief    Intialize HEVC decode mode
    //! \details  For dynamic mode switch support
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializeDecodeMode();

    //!
    //! \brief  Set states for each frame to prepare for GEN12 HEVC decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetFrameStates () override;

    //!
    //! \brief    HEVC decoder state level function
    //! \details  State level function for GEN12 HEVC decoder
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
    //! \brief    Allocate variable sized resources
    //! \details  Allocate variable sized resources in HEVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  AllocateResourcesVariableSizes ();

    //!
    //! \brief    Get the Huc Dmem resource size
    //! \details  Return the Huc Dmem resource size in bytes.
    //!
    //! \param    None
    //!
    //! \return   uint32_t
    //!           the size of Dmem resource
    //!
    uint32_t GetDmemBufferSize() override;

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
    MOS_STATUS SetHucDmemS2LPictureBss(
        PHUC_HEVC_S2L_PIC_BSS           hucHevcS2LPicBss) override;

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
    MOS_STATUS  SetHucDmemParams(
        PMOS_RESOURCE               dmemBuffer) override;

    //!
    //! \brief    Determine Decode Phase
    //! \details  Determine decode phase in hevc decode
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DetermineDecodePhase ();

    //!
    //! \brief    Send S2L picture level commands
    //! \details  Send S2L picture level commands in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SendPictureS2L () override;

    //!
    //! \brief    Initialize the picture level MHW parameters for long format
    //! \details  Initialize the picture level MHW parameters for long format in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  InitPicLongFormatMhwParams() override;

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
    MOS_STATUS AddPictureLongFormatCmds(
        PMOS_COMMAND_BUFFER             cmdBufferInUse,
        PIC_LONG_FORMAT_MHW_PARAMS      *picMhwParams) override;

    //!
    //! \brief    Send long format picture level commands
    //! \details  Send long format picture level commands in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SendPictureLongFormat () override;


    //!
    //! \brief    Send short format HuCStart Command for slices
    //! \details  Send short format HuCStart Command for slices in HEVC decode driver
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendShortSlices(PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Send vdbox flush cmds for S2L HuC cmds
    //! \details  Send vdbox flush cmds for S2L HuC cmds
    //!
    //! \param    [out] cmdBuffer
    //!           Pointer to Command buffer
    //! \param    [in] primCmdBuffer
    //!           Reference to Primary Command buffer
    //! \param    [in] scdryCmdBuffer
    //!           Reference to Secondary Command buffer
    //! \param    [in] renderingFlags
    //!           Rendering flags
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SendHucFlush(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MOS_COMMAND_BUFFER  &primCmdBuffer,
        MOS_COMMAND_BUFFER  &scdryCmdBuffer,
        uint32_t            renderingFlags);

    //!
    //! \brief    Send flush pipe commands
    //! \details  Send flush pipe commands for each pipe in HEVC decode driver
    //!
    //! \param    [in] cmdBufferInUse
    //!           Pointer to current using command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddPipeEpilog(
        PMOS_COMMAND_BUFFER cmdBufferInUse,
        MOS_COMMAND_BUFFER &scdryCmdBuffer);

    MOS_STATUS CalcDownsamplingParams(
        void                        *picParams,
        uint32_t                    *refSurfWidth,
        uint32_t                    *refSurfHeight,
        MOS_FORMAT                  *format,
        uint8_t                     *frameIdx) override;

    MOS_STATUS InitMmcState() override;

    PCODEC_HEVC_EXT_PIC_PARAMS   m_hevcExtPicParams;    // Extended pic params for Rext
    PCODEC_HEVC_EXT_SLICE_PARAMS m_hevcExtSliceParams;  // Extended slice params for Rext
    PCODEC_HEVC_SCC_PIC_PARAMS   m_hevcSccPicParams;    // Pic params for SCC
    PCODEC_HEVC_SUBSET_PARAMS    m_hevcSubsetParams;    //!<  Hevc subset params for tile entrypoint offset

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpPicParams(
        PCODEC_HEVC_PIC_PARAMS     picParams,
        PCODEC_HEVC_EXT_PIC_PARAMS extPicParams,
        PCODEC_HEVC_SCC_PIC_PARAMS sccPicParams);

    MOS_STATUS DumpSliceParams(
        PCODEC_HEVC_SLICE_PARAMS     sliceParams,
        PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
        uint32_t                     numSlices,
        bool                         shortFormatInUse);

    MOS_STATUS DumpSubsetsParams(
        PCODEC_HEVC_SUBSET_PARAMS     subsetsParams);
#endif

protected:
    //!
    //! \brief    Set And Populate VE Hint parameters
    //! \details  Set Virtual Engine hint parameter and populate it to primary cmd buffer attributes
    //! \param    [in] primCmdBuf
    //!               Pointer to primary cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetAndPopulateVEHintParams(
        PMOS_COMMAND_BUFFER        primCmdBuf);

    //!
    //! \brief    Determine if need to send prolog with frame tracking
    //! \param    [in] sendPrologWithFrameTracking
    //!               Pointer to bool type flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  DetermineSendProlgwithFrmTracking(
        bool                        *sendPrologWithFrameTracking);

    //!
    //! \brief    Allocate internal buffer as reference
    //! \details  Allocate reference resource before loop filter for SCC IBC mode
    //!
    MOS_STATUS  AllocateResourceRefBefLoopFilter();

    //!
    //! \brief    Check LCU Size
    //! \details  Check LCU Size for HEVC decode driver
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  CheckLCUSize();

    MOS_STATUS SetGpuCtxCreatOption(CodechalSetting *settings) override;

private:
    //!
    //! \brief  Calculate command buffer size needed for picture level and slice level commands
    //! \param    [out] requestedSize
    //!           Return command buffer size for picture level and slice level command
    //! \param    [out] additionalSizeNeeded
    //!           Return additianl size needed
    //! \param    [out] requestedPatchListSize
    //!           return patch list size used in this command buffer
    //! \return None
    //!
    void CalcRequestedSpace(
        uint32_t       &requestedSize,
        uint32_t       &additionalSizeNeeded,
        uint32_t       &requestedPatchListSize) override;

    //!
    //! \brief  The virtual function for decode standard to override the requested space size
    //! \param    [in] requestedSize
    //!           The intial request size computed by picture level and slice level
    //! \return The final requested space size
    //!
    uint32_t RequestedSpaceSize(uint32_t requestedSize) override;

    //!
    //! \brief  The virtual function for decode standard to override the extra requested space size
    //! \param    [in] requestedSize
    //!           The intial request size computed by picture level and slice level
    //! \param    [in] additionalSizeNeeded
    //!           The additional request size for command buffer
    //! \return The extra requested space size
    //!
    virtual MOS_STATUS VerifyExtraSpace(
        uint32_t requestedSize,
        uint32_t additionalSizeNeeded) override;

    //!
    //! \brief  Utility function to allocate internal histogram surface
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateHistogramSurface();

protected:
    bool               m_isRealTile = false;                                //!< If real tile decode mode enabled
    uint16_t           m_ctbSize = 0;                                       //!< Max coding tree block size
    uint32_t           m_frameSizeMaxAlloced = 0;                           //!< Max Frame size used for buffer allocation in past frames

    bool               m_isSeparateTileDecoding = false;                    //!< Single pipe tile decoding for IBC/PAL
    MOS_RESOURCE       m_resRefBeforeLoopFilter;                            //!< For SCC IBC
    bool               m_twoVersionsOfCurrDecPicFlag = false;               //!< Flag for SCC IBC mode
    uint8_t            m_IBCRefIdx = 0;                                     //!< Reference ID for IBC mode

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t m_rtFrameCount;  //!< frame count for real tile decoding
    uint32_t m_vtFrameCount;  //!< frame count for virtual tile decoding
    uint32_t m_spFrameCount;  //!< frame count for single pipe decoding

    bool         m_histogramDebug = true;       //!< Internal histogram buffer debug
#endif

    PMOS_SURFACE m_histogramSurface = nullptr;  //!< Internal histogram buffer
    PCODECHAL_DECODE_SINGLEPIPE_VIRTUALENGINE_STATE m_sinlgePipeVeState;  //!< single pipe virtual engine state
    PCODECHAL_DECODE_SCALABILITY_STATE_G12           m_scalabilityState;   //!< Scalability state
};
#endif  // __CODECHAL_DECODER_HEVC__G12_H__
