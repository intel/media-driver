/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     codechal_decode_sfc.h
//! \brief    Defines the decode interface extension for CSC and scaling via SFC.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#ifndef __CODECHAL_DECODE_SFC_H__
#define __CODECHAL_DECODE_SFC_H__

#include "codechal.h"
#include "codechal_hw.h"

#define CODECHAL_SFC_ALIGNMENT_16        16
#define CODECHAL_SFC_ALIGNMENT_8         8

#define CODECHAL_SFC_VEBOX_STATISTICS_SIZE                        (32 * 4)
#define CODECHAL_SFC_VEBOX_LACE_HISTOGRAM_256_BIN_PER_BLOCK       (256 * 2)
#define CODECHAL_SFC_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE (256 * 4)
#define CODECHAL_SFC_NUM_FRAME_PREVIOUS_CURRENT                    2
#define CODECHAL_SFC_VEBOX_MAX_SLICES                              2

#define CODECHAL_SFC_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE           (256 * 4)
#define CODECHAL_SFC_NUM_RGB_CHANNEL                               3
#define CODECHAL_SFC_VEBOX_RGB_HISTOGRAM_SIZE                     (CODECHAL_SFC_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                                   CODECHAL_SFC_NUM_RGB_CHANNEL                    * \
                                                                   CODECHAL_SFC_VEBOX_MAX_SLICES)

//!
//! \struct CODECHAL_RECTANGLE
//! \brief Parameters to describe a surface region
//!
typedef struct _CODECHAL_RECTANGLE
{
    uint32_t           X;
    uint32_t           Y;
    uint32_t           Width;
    uint32_t           Height;
} CODECHAL_RECTANGLE, *PCODECHAL_RECTANGLE;

//!
//! \struct CODECHAL_DECODE_PROCESSING_PARAMS
//! \brief Parameters needed for the processing of the decode render target.
//!
typedef struct _CODECHAL_DECODE_PROCESSING_PARAMS
{
    // Input
    PMOS_SURFACE                 pInputSurface;
    CODECHAL_RECTANGLE           rcInputSurfaceRegion;
    uint32_t                     uiInputColorStandard;
    uint32_t                     uiInputColorRange;
    uint32_t                     uiChromaSitingType;

    // Output
    PMOS_SURFACE                 pOutputSurface;
    CODECHAL_RECTANGLE           rcOutputSurfaceRegion;
    uint32_t                     uiOutputColorStandard;

    PMOS_SURFACE                 pHistogramSurface;

    // Processing state
    uint32_t                     uiRotationState;
    uint32_t                     uiBlendState;
    uint32_t                     uiMirrorState;
    bool                         bIsSourceSurfAllocated;
    bool                         bIsReferenceOnlyPattern;
}CODECHAL_DECODE_PROCESSING_PARAMS, *PCODECHAL_DECODE_PROCESSING_PARAMS;

//!
//! \class CodechalSfcState
//! \brief This class defines the member fields, functions etc used by SFC State.
//!
class CodechalSfcState
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalSfcState() {};

    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalSfcState();

    //!
    //! \brief    Initialize Sfc State
    //! \details  Initialize Sfc State
    //! \param    [in] inDecoder
    //!           Pointer to Decode interface
    //! \param    [in] hwInterface
    //!           Pointer to hardware interface
    //! \param    [in] osInterface
    //!           Pointer to OS interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitializeSfcState(
        CodechalDecode                      *inDecoder,
        CodechalHwInterface                 *hwInterface,
        PMOS_INTERFACE                      osInterface);

    //!
    //! \brief    Initialize Sfc variables
    //! \details  Initialize Sfc variables
    //! \param    [in] decodeProcParams
    //!           Pointer to Decode Processing Params
    //! \param    [in] sfcPipeMode
    //!           Indicate which media pipe using SFC
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize(
        PCODECHAL_DECODE_PROCESSING_PARAMS  decodeProcParams,
        uint8_t                             sfcPipeMode);

    //!
    //! \brief    Send Vebox and SFC Cmd
    //! \details  Send Vebox and SFC Cmd to HW
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RenderStart();

    //!
    //! \brief    Send SFC Avs State / Ief State Cmd
    //! \details  Send SFC Avs State / Ief State Cmd
    //! \param    [in] cmdBuffer
    //!           Pointer to Command buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddSfcCommands(
        PMOS_COMMAND_BUFFER                 cmdBuffer);

    //!
    //! \brief    Check if specified input and output format supported by SFC
    //! \details  Check if a specified input/output pair can be supported by SFC
    //! \param    [in] inputFormat
    //!           The SFC input format
    //! \param    [in] outputFormat
    //!           The SFC output format
    //! \return   bool
    //!           true if supported, else false
    //!
    virtual bool IsSfcFormatSupported(
        MOS_FORMAT                          inputFormat,
        MOS_FORMAT                          outputFormat);

    //!
    //! \brief    Check if SFC output is supported
    //! \details  Check if SFC output is supported according to Decode Processing Params
    //! \param    [in] decodeProcParams
    //!           Pointer to decode processing params
    //! \param    [in] sfcPipeMode
    //!           Indicate which media pipe using SFC
    //! \return   bool
    //!           true if supported, else false
    //!
    bool IsSfcOutputSupported(
        PCODECHAL_DECODE_PROCESSING_PARAMS  decodeProcParams,
        uint8_t                             sfcPipeMode);

    CodechalDecode *     m_decoder        = nullptr;  //!< Decoder
    PMOS_INTERFACE       m_osInterface    = nullptr;  //!< OS Interface
    CodechalHwInterface *m_hwInterface    = nullptr;  //!< HW Interface
    MhwVeboxInterface *  m_veboxInterface = nullptr;  //!< Vebox Interface
    PMHW_SFC_INTERFACE   m_sfcInterface   = nullptr;  //!< Sfc Interface

    bool         m_deblockingEnabled = false;    //!< Indicate if Deblocking is enabled
    uint32_t     m_inputFrameWidth   = 0;        //!< Input Frame Width
    uint32_t     m_inputFrameHeight  = 0;        //!< Input Frame Height
    bool         m_sfcPipeOut        = false;    //!< Indicate Sfc Pipe Out is enabled
    PMOS_SURFACE m_sfcOutputSurface  = nullptr;  //!< Pointer of Sfc Output Surface

    bool         m_jpegInUse      = false;  //!< Indicate if Jpeg is in use
    uint8_t      m_jpegChromaType = 0;      //!< Jpeg Chroma Type

protected:
    uint8_t                     m_sfcPipeMode       = MhwSfcInterface::SFC_PIPE_MODE_VDBOX; //!< which FE engine pipe used

    PMOS_SURFACE m_inputSurface       = nullptr;  //!< Pointer of Input Surface
    PMOS_SURFACE m_veboxOutputSurface = nullptr;  //!< Pointer of Vebox Output Surface

    MOS_RESOURCE m_resAvsLineBuffer           = {0};  //!< Avs Line Buffer MOS Resource
    MOS_RESOURCE m_resLaceOrAceOrRgbHistogram = {0};  //!< Lace/Ace/Rgb Histogram MOS Resource
    MOS_RESOURCE m_resStatisticsOutput        = {0};  //!< Statistics Output MOS Resource

    bool         m_scaling   = false;  //!< Indicate if scaling is needed
    bool         m_colorFill = false;  //!< Indicate if color fill is needed
    bool         m_ief       = false;  //!< Indicate if IEF is needed for Surface
    bool         m_csc       = false;  //!< Indicate if YUV->RGB/YUV->YUV CSC is enabled

    float        m_scaleX = 0;  //!< Horizontal Scaling Ratio
    float        m_scaleY = 0;  //!< Vertical Scaling Ratio

    uint32_t     m_rotationMode = 0;  //!< Rotation Mode
    uint32_t     m_chromaSiting = 0;  //!< Chroma Siting Type

    CODECHAL_RECTANGLE m_inputSurfaceRegion  = {0};  //!< Input Region Resolution and Offset
    CODECHAL_RECTANGLE m_outputSurfaceRegion = {0};  //!< Output Region Resolution and Offset

    float              m_cscCoeff[9]     = {0};  //!< Csc Coefficient
    float              m_cscInOffset[3]  = {0};  //!< Csc In Offset
    float              m_cscOutOffset[3] = {0};  //!< Csc Out Offset

    MHW_AVS_PARAMS           m_avsParams   = {Format_Any};  //!< Avs Params
    MHW_SFC_AVS_LUMA_TABLE   m_lumaTable   = {0};           //!< Avs Luma Table
    MHW_SFC_AVS_CHROMA_TABLE m_chromaTable = {0};           //!< Avs Chroma Table
    MHW_SFC_AVS_STATE        m_avsState    = {0};           //<! Avs State

    MOS_RESOURCE m_resSyncObject = {0};  //!< Sync Object

protected:
    //!
    //! \brief    Allocate Resources for SFC
    //! \details  Allocate Buffer for SFC and initialize AVS params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Set Vebox State Cmd Params
    //! \details  Set Vebox State Cmd Params
    //! \param    [in] veboxCmdParams
    //!           Pointer to Vebox State Cmd Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVeboxStateParams(
        PMHW_VEBOX_STATE_CMD_PARAMS         veboxCmdParams);

    //!
    //! \brief    Set Vebox Surface State Cmd Params
    //! \details  Set Vebox Surface State Cmd Params
    //! \param    [in] veboxSurfParams
    //!           Pointer to Vebox Surface State Cmd Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVeboxSurfaceStateParams(
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS veboxSurfParams);

    //!
    //! \brief    Set Vebox Di Iecp Cmd Params
    //! \details  Set Vebox Di Iecp Cmd Params
    //! \param    [in] veboxDiIecpParams
    //!           Pointer to Vebox Di Iecp Cmd Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVeboxDiIecpParams(
        PMHW_VEBOX_DI_IECP_CMD_PARAMS       veboxDiIecpParams);

    //!
    //! \brief    Set Sfc State Params
    //! \details  Set Sfc State Params
    //! \param    [in] sfcStateParams
    //!           Pointer to Sfc State Params
    //! \param    [in] outSurfaceParams
    //!           Pointer to Out Surface Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSfcStateParams(
        PMHW_SFC_STATE_PARAMS               sfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS         outSurfaceParams);

    //!
    //! \brief    Set Sfc Avs State Params
    //! \details  Set Sfc Avs State Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetSfcAvsStateParams();

    //!
    //! \brief    Set Sfc Ief State Params
    //! \details  Set Sfc Ief State Params
    //! \param    [in] iefStateParams
    //!           Pointer to Sfc Ief State Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSfcIefStateParams(
        PMHW_SFC_IEF_STATE_PARAMS           iefStateParams);

    //!
    //! \brief    Update Sfc State Params according to input info
    //! \details  Update Sfc State Params according to input info
    //! \param    [in] sfcStateParams
    //!           Pointer to Sfc State Params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateInputInfo(
        PMHW_SFC_STATE_PARAMS          sfcStateParams) = 0;
};

using PCODECHAL_SFC_STATE = CodechalSfcState*;

#endif  // __CODECHAL_DECODE_SFC_H__

