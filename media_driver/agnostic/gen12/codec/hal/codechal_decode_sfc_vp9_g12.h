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
//! \file     codechal_decode_sfc_vp9_g12.h
//! \brief    Defines the SFC interface extension for VP9 decode in G12+ platform.
//! \details  Defines all types, macros, and functions required by CodecHal SFC for VP9 decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODE_SFC_VP9_G12_H__
#define __CODECHAL_DECODE_SFC_VP9_G12_H__

#include "mhw_sfc_g12_X.h"
#include "codechal_decode_sfc.h"
#include "codechal_decode_scalability_g12.h"

class CodechalVp9SfcStateG12 : public CodechalSfcState
{
public:
    //!
    //! \brief    Constructor
    //!
    CodechalVp9SfcStateG12();
    //!
    //! \brief    Destructor
    //!
    ~CodechalVp9SfcStateG12();

    //!
    //! \brief    Check if SFC output is supported and Initialize SFC
    //! \param    [in] decodeProcParams
    //!           Pointer to decode processing params
    //! \param    [in] vp9PicParams
    //!           Pointer to VP9 picture paramters
    //! \param    [in] scalabilityState
    //!           Pointer to CODECHAL_DECODE_SCALABILITY_STATE structure
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckAndInitialize(
        PCODECHAL_DECODE_PROCESSING_PARAMS  decProcessingParams,
        PCODEC_VP9_PIC_PARAMS               vp9PicParams,
        PCODECHAL_DECODE_SCALABILITY_STATE  scalabilityState);

    virtual bool IsSfcFormatSupported(
        MOS_FORMAT                          inputFormat,
        MOS_FORMAT                          outputFormat);

    virtual MOS_STATUS UpdateInputInfo(
        PMHW_SFC_STATE_PARAMS               sfcStateParams);

    virtual MOS_STATUS AddSfcCommands(
        PMOS_COMMAND_BUFFER                 cmdBuffer);

    virtual MOS_STATUS SetSfcStateParams(
        PMHW_SFC_STATE_PARAMS               sfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS         outSurfaceParams);

    virtual MOS_STATUS SetSfcAvsStateParams();

protected:
    MOS_STATUS AllocateResources();

protected:
    int                                     m_numPipe = 1;          //!< Number of pipes for scalability
    int                                     m_curPipe = 0;          //!< Current pipe index
    int                                     m_numBuffersAllocated = 0; //!< Number line buffer allocated
    PCODEC_VP9_PIC_PARAMS                   m_vp9PicParams;         //!< VP9 picture parameters
    PCODECHAL_DECODE_SCALABILITY_STATE_G12  m_scalabilityState;     //!< Decode scalability state
    PMOS_SURFACE                            m_histogramSurface = nullptr;   //!< Histogram stream out buffer
    MOS_RESOURCE                           *m_resAvsLineBuffers = nullptr;   //!< AVS Line Buffer, one for each pipe
    MOS_RESOURCE                           *m_resSfdLineBuffers = nullptr;   //!< SFD Line Buffer, one for each pipe
    MOS_RESOURCE                            m_resAvsLineTileBuffer = { 0 };  //!< Avs Line Tile Buffer MOS Resource
    MOS_RESOURCE                            m_resSfdLineTileBuffer = { 0 };  //!< SFD Line Tile Buffer MOS Resource
};

#endif
