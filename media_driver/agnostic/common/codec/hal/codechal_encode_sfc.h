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
//! \file     codechal_encode_sfc.h
//! \brief    Defines the encode interface for CSC via SFC.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#ifndef __CODECHAL_ENCODE_SFC_H__
#define __CODECHAL_ENCODE_SFC_H__

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

typedef struct _CODECHAL_ENCODE_RECTANGLE
{
    uint32_t              X;
    uint32_t              Y;
    uint32_t              Width;
    uint32_t              Height;
} CODECHAL_ENCODE_RECTANGLE, *PCODECHAL_ENCODE_RECTANGLE;

//!
//! \struct    CODECHAL_ENCODE_SFC_PARAMS
//! \brief     Parameters needed for the processing of the encode render target
//!
struct CODECHAL_ENCODE_SFC_PARAMS
{
    CODECHAL_ENCODE_SFC_PARAMS()
    {
        MOS_ZeroMemory(this, sizeof(*this));
    }

    // Input
    PMOS_SURFACE                 pInputSurface;
    CODECHAL_ENCODE_RECTANGLE    rcInputSurfaceRegion;
    uint32_t                     uiChromaSitingType;

    // Output
    PMOS_SURFACE                 pOutputSurface;
    CODECHAL_ENCODE_RECTANGLE    rcOutputSurfaceRegion;
};

class CodecHalEncodeSfc
{
public:
    //!
    //! \brief    Constructor
    //!
    CodecHalEncodeSfc() {};

    //!
    //! \brief    Destructor
    //!
    virtual ~CodecHalEncodeSfc();

    void SetInputColorSpace(MHW_CSPACE colorSpace)
    {
        m_inputSurfaceColorSpace = colorSpace;
    }
    void SetOutputColorSpace(MHW_CSPACE colorSpace)
    {
        m_outputSurfaceColorSpace = colorSpace;
    }
    //!
    //! \brief    Initialize
    //!
    //! \param    [in] hwInterface
    //!           Codechal hardware interface
    //! \param    [in] osInterface
    //!           Pointer to MOS interface
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS Initialize(
        CodechalHwInterface                *hwInterface,
        PMOS_INTERFACE                      osInterface);

    //!
    //! \brief    Allocate resources
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS AllocateResources();

    //!
    //! \brief    Free resources
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS FreeResources();

    //!
    //! \brief    Set parameters
    //! \details    call every frame.  get the input/output surface and  color space...
    //!
    //! \param    [in] params
    //!           Pointer to codechal encode sfc parameters
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS SetParams(
        CODECHAL_ENCODE_SFC_PARAMS*         params);

    //!
    //! \brief    Render start
    //!
    //! \param    [in] encoder
    //!           Pointer to codechal encoder state
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS RenderStart(
        CodechalEncoderState*           encoder);

    //!
    //! \brief    Add sfc commands
    //!
    //! \param    [in] sfcInterface
    //!           Pointer to MHW sfc interface
    //! \param    [in] cmdBuffer
    //!           Pointer to MOS command buffer
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS AddSfcCommands(
        PMHW_SFC_INTERFACE              sfcInterface,
        PMOS_COMMAND_BUFFER             cmdBuffer);

    //!
    //! \brief    Set sfc state parameters
    //!
    //! \param    [in] sfcInterface
    //!           Pointer to MHW sfc interface
    //! \param    [in] params
    //!           Pointer to MHW sfc state parameters
    //! \param    [in] outSurfaceParams
    //!           Pointer to MHW sfc out surface parameters
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS SetSfcStateParams(
        PMHW_SFC_INTERFACE             sfcInterface,
        PMHW_SFC_STATE_PARAMS          params,
        PMHW_SFC_OUT_SURFACE_PARAMS    outSurfaceParams);

    //!
    //! \brief    Set sfc avs state parameters
    //!
    //! \param    [in] sfcInterface
    //!           Pointer to MHW sfc interface
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS SetSfcAvsStateParams(
        PMHW_SFC_INTERFACE             sfcInterface);

    //!
    //! \brief    Set sfc ief state parameters
    //!
    //! \param    [in] params
    //!           Pointer to MHW sfc ief state parameters
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS SetSfcIefStateParams(
        PMHW_SFC_IEF_STATE_PARAMS         params);

    //!
    //! \brief    Set vebox state parameters
    //!
    //! \param    [in] params
    //!           Pointer to vebox state command params
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS SetVeboxStateParams(
        PMHW_VEBOX_STATE_CMD_PARAMS         params);

    //!
    //! \brief    Set vebox surface state params
    //!
    //! \param    [in] params
    //!           Pointer to MHW vebox surface state command parameters
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS SetVeboxSurfaceStateParams(
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS         params);

    //!
    //! \brief    Set vebox di iecp parameters
    //!
    //! \param    [in] params
    //!           Pointer to MHW vebox di iecp command parameters
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS SetVeboxDiIecpParams(
        PMHW_VEBOX_DI_IECP_CMD_PARAMS         params);

    //!
    //! \brief    Vebox set iecp parameters
    //! \details  input -> RGB (from app)
    //!           output -> NV12 (raw surface?)
    //!
    //! \param    [in] mhwVeboxIecpParams
    //!           Pointer t MHW vebox iecp paramters
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    MOS_STATUS VeboxSetIecpParams(
        PMHW_VEBOX_IECP_PARAMS         mhwVeboxIecpParams);

private:
    //!
    //! \brief    Check if is Cspac
    //!
    //! \param    [in] srcCspace
    //!           MHW cspace
    //! \param    [in] dstCspace
    //!           MHW cspace
    //!
    //! \return   bool
    //!           true if call success, else false
    //!
    bool IsCspace(MHW_CSPACE srcCspace, MHW_CSPACE dstCspace);

    //!
    //! \brief    Get RGB range and offset
    //!
    //! \param    [in] srcCspace
    //!           MHW cspace
    //! \param    [in] rgbOffset
    //!           RGB offset
    //! \param    [in] rgbExcursion
    //!           RGB excursion
    //!
    //! \return   bool
    //!           true if call success, else false
    //!
    bool GetRgbRangeAndOffset(
        MHW_CSPACE          srcCspace,
        float               *rgbOffset,
        float               *rgbExcursion);

    //!
    //! \brief    Get YUV range and offset
    //!
    //! \param    [in] srcCspace
    //!           MHW cspace
    //! \param    [in] lumaOffset
    //!           Luma offset
    //! \param    [in] lumaExcursion
    //!           Luma excursion
    //! \param    [in] chromaZero
    //!           Chroma zero
    //! \param    [in] chromaExcursion
    //!           Chroma excursion
    //!
    //! \return   bool
    //!           true if call success, else false
    //!
    bool GetYuvRangeAndOffset(
        MHW_CSPACE          srcCspace,
        float               *lumaOffset,
        float               *lumaExcursion,
        float               *chromaZero,
        float               *chromaExcursion);

    //!
    //! \brief    Calculate YUV To RGB matrix
    //! \details  Given the YUV->RGB transfer matrix, get the final matrix after
    //!             applying offsets and excursions.
    //!
    //! [R']     [R_o]                                 [R_e/Y_e    0       0   ]  [Y'  - Y_o]
    //! [G']  =  [R_o] + [YUVtoRGBCoeff (3x3 matrix)]. [   0    R_e/C_e    0   ]. [Cb' - C_z]
    //! [B']     [R_o]                                 [   0       0    R_e/C_e]. [Cr' - C_z]
    //!
    //! [R']  = [C0  C1   C2] [Y' ]   [C3]      {Out pMatrix}
    //! [G']  = [C4  C5   C6].[Cb'] + [C7]
    //! [B']  = [C8  C9  C10] [Cr'] + [C11]
    //!
    //! \param    [in] srcCspace
    //!           YUV Color space 
    //! \param    [in] dstCspace
    //!           RGB Color space
    //! \param    [in] transferMatrix
    //!           Transfer matrix (3x3)
    //! \param    [out] outMatrix
    //!           Conversion matrix (3x4)
    //! \return   true if success else false
    //!
    bool CalcYuvToRgbMatrix(
        MHW_CSPACE      srcCspace,
        MHW_CSPACE      dstCspace,
        float           *transferMatrix,
        float           *outMatrix);

    //!
    //! \brief    Calculate RGB To YUV matrix
    //!
    //! \param    [in] srcCspace
    //!           RGB Color space 
    //! \param    [in] dstCspace
    //!           YUV Color space
    //! \param    [in] transferMatrix
    //!           Transfer matrix (3x3)
    //! \param    [out] outMatrix
    //!           Conversion matrix (3x4)
    //! \return   bool
    //!           true if call success, else false
    bool CalcRgbToYuvMatrix(
        MHW_CSPACE      srcCspace,
        MHW_CSPACE      dstCspace,
        float           *transferMatrix,
        float           *outMatrix);

    //!
    //! \brief    Get csc matrix
    //!
    //! \param    [in] srcCspace
    //!           Source Color space
    //! \param    [in] dstCspace
    //!           Destination Color space
    //! \param    [out] cscMatrix
    //!           CSC matrix to use
    //!
    void GetCSCMatrix(
        MHW_CSPACE          srcCspace,
        MHW_CSPACE          dstCspace,
        float               *cscMatrix);

    //!
    //! \brief    Get csc matrix
    //!
    //! \param    [in] srcCspace
    //!           Source Cspace
    //! \param    [in] dstCspace
    //!           Destination Cspace
    //! \param    [out] cscCoeff
    //!           Coefficients matrix
    //! \param    [out] cscInOffset
    //!           Input Offset matrix
    //! \param    [out] cscOutOffset
    //!           Output Offset matrix
    //!
    void GetCscMatrix(
        MHW_CSPACE             srcCspace,
        MHW_CSPACE             dstCspace,
        float                  *cscCoeff,
        float                  *cscInOffset,
        float                  *cscOutOffset);

protected:
    CodechalHwInterface            *m_hwInterface = nullptr;  //!< Pointer to CodechalHwInterface
    PMOS_INTERFACE                  m_osInterface = nullptr;  //!< Pointer to MOS_INTERFACE

    PMOS_SURFACE    m_inputSurface = nullptr;
    MHW_CSPACE      m_inputSurfaceColorSpace = MHW_CSpace_Any;
    PMOS_SURFACE    m_veboxOutputSurface = nullptr;
    PMOS_SURFACE    m_sfcOutputSurface = nullptr;
    MHW_CSPACE      m_outputSurfaceColorSpace = MHW_CSpace_Any;

    MOS_RESOURCE    m_resAvsLineBuffer = { 0, };
    MOS_RESOURCE    m_resLaceOrAceOrRgbHistogram = { 0, };
    MOS_RESOURCE    m_resStatisticsOutput = { 0, };

    bool            m_scaling = false;
    bool            m_colorFill = false;
    bool            m_IEF = false;
    bool            m_CSC = false;

    float           m_scaleX = 0.0f;
    float           m_scaleY = 0.0f;

    uint16_t        m_iefFactor = 0;
    uint32_t        m_rotationMode = 0;
    uint32_t        m_chromaSiting = 0;
    uint32_t        m_inputFrameWidth = 0;
    uint32_t        m_inputFrameHeight = 0;

    CODECHAL_ENCODE_RECTANGLE       m_inputSurfaceRegion = { 0, };
    CODECHAL_ENCODE_RECTANGLE       m_outputSurfaceRegion = { 0, };

    // CSC in VEBOX params
    bool                            m_veboxCsc = false;
    MHW_CSPACE                      m_cscOutputCspace = MHW_CSpace_Any;    //!< Cspace of Output Frame
    MHW_CSPACE                      m_cscInputCspace = MHW_CSpace_Any;     //!< Cspace of Input frame
    float                           m_cscCoeff[9] = { 0.0f, };
    float                           m_cscInOffset[3] = { 0.0f, };
    float                           m_cscOutOffset[3] = { 0.0f, };

    MHW_AVS_PARAMS                  m_avsParams = { Format_Any, };
    MHW_SFC_AVS_LUMA_TABLE          m_lumaTable = { 0, };
    MHW_SFC_AVS_CHROMA_TABLE        m_chromaTable = { 0, };
    MHW_SFC_AVS_STATE               m_avsState = { 0, };
};
#endif  // __CODECHAL_ENCODE_SFC_H__
