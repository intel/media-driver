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

//!
//! \struct    CODECHAL_ENCODE_SFC_STATE
//! \brief     Codechal encode sfc state
//!
struct CODECHAL_ENCODE_SFC_STATE
{
    CODECHAL_ENCODE_SFC_STATE()
    {
        MOS_ZeroMemory(this, sizeof(*this));
    }

    PMOS_SURFACE    pInputSurface;
    MHW_CSPACE      InputSurfaceColorSpace;
    PMOS_SURFACE    pVeboxOutputSurface;
    PMOS_SURFACE    pSfcOutputSurface;
    MHW_CSPACE      OutputSurfaceColorSpace;

    MOS_RESOURCE    resAvsLineBuffer;
    MOS_RESOURCE    resLaceOrAceOrRgbHistogram;
    MOS_RESOURCE    resStatisticsOutput;

    bool            bScaling;
    bool            bColorFill;
    bool            bIEF;
    bool            bCSC;

    float           fScaleX;
    float           fScaleY;

    uint16_t        wIefFactor;
    uint32_t        uiRotationMode;
    uint32_t        uiChromaSiting;
    uint32_t        dwInputFrameWidth;
    uint32_t        dwInputFrameHeight;

    CODECHAL_ENCODE_RECTANGLE       rcInputSurfaceRegion;
    CODECHAL_ENCODE_RECTANGLE       rcOutputSurfaceRegion;

    // CSC in VEBOX params
    bool                            bVeboxCsc;
    MHW_CSPACE                      CscOutputCspace;                            //!< Cspace of Output Frame
    MHW_CSPACE                      CscInputCspace;                             //!< Cspace of Input frame
    float                           fCscCoeff[9];
    float                           fCscInOffset[3];
    float                           fCscOutOffset[3];


    MHW_AVS_PARAMS                  AvsParams;
    MHW_SFC_AVS_LUMA_TABLE          LumaTable;
    MHW_SFC_AVS_CHROMA_TABLE        ChromaTable;
    MHW_SFC_AVS_STATE               AvsState;
};
using PCODECHAL_ENCODE_SFC_STATE = CODECHAL_ENCODE_SFC_STATE*;

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
MOS_STATUS CodecHalEncodeSfc_Initialize(
    CodechalHwInterface                *hwInterface,
    PMOS_INTERFACE                      osInterface);

//!
//! \brief    Set parameters
//! \details    call every frame.  get the input/output surface and  color space...
//!
//! \param    [in] osInterface
//!           Pointer to MOS interface
//! \param    [in] sfcState
//!           Pointer to codechal encode sfc state
//! \param    [in] params
//!           Pointer to codechal encode sfc parameters
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHalEncodeSfc_SetParams(
    PMOS_INTERFACE                      osInterface,
    PCODECHAL_ENCODE_SFC_STATE          sfcState,
    CODECHAL_ENCODE_SFC_PARAMS*         params);

//!
//! \brief    Destroy
//!
//! \param    [in] hwInterface
//!           Codechal hardware interface
//! \param    [in] osInterface
//!           Pointer to MOS interface
//! \param    [in] sfcState
//!           Pointer to codechal encode sfc state
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHalEncodeSfc_Destroy(
    CodechalHwInterface            *hwInterface,
    PMOS_INTERFACE                  osInterface,
    PCODECHAL_ENCODE_SFC_STATE      sfcState);

//!
//! \brief    Render start
//!
//! \param    [in] hwInterface
//!           Codechal hardware interface
//! \param    [in] osInterface
//!           Pointer to MOS interface
//! \param    [in] encoder
//!           Pointer to codechal encoder state
//! \param    [in] sfcState
//!           Pointer to codechal encode sfc state
//!
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if call success, else fail reason
//!
MOS_STATUS CodecHalEncodeSfc_RenderStart(
    CodechalHwInterface            *hwInterface,
    PMOS_INTERFACE                  osInterface,
    CodechalEncoderState*           encoder,
    PCODECHAL_ENCODE_SFC_STATE      sfcState);

#endif  // __CODECHAL_ENCODE_SFC_H__

