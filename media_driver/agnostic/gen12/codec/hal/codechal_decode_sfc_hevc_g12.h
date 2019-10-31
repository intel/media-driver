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
//! \file     codechal_decode_sfc_hevc_g12.h
//! \brief    Defines the SFC interface extension for HEVC decode in G12+ platform.
//! \details  Defines all types, macros, and functions required by CodecHal SFC for HEVC decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODE_SFC_HEVC_G12_H__
#define __CODECHAL_DECODE_SFC_HEVC_G12_H__

#include "mhw_sfc_g12_X.h"
#include "codechal_decode_sfc_hevc.h"
#include "codechal_decode_scalability_g12.h"

class CodechalHevcSfcStateG12 : public CodechalHevcSfcState
{
public:
    enum HEVC_LCU_SIZE
    {
        HEVC_SFC_LCU_16_16 = 0,
        HEVC_SFC_LCU_32_32 = 1,
        HEVC_SFC_LCU_64_64 = 2
    };
    enum HEVC_CHROMA_IDC
    {
        HEVC_CHROMA_FORMAT_400 = 0,
        HEVC_CHROMA_FORMAT_420 = 1,
        HEVC_CHROMA_FORMAT_422 = 2,
        HEVC_CHROMA_FORMAT_444 = 3
    };
    //!
    //! \brief    Constructor
    //!
    CodechalHevcSfcStateG12();
    //!
    //! \brief    Destructor
    //!
    ~CodechalHevcSfcStateG12();

    //!
    //! \brief    Check if SFC output is supported and Initialize SFC
    //! \param    [in] decodeProcParams
    //!           Pointer to decode processing params
    //! \param    [in] hevcPicParams
    //!           Pointer to HEVC picture paramters
    //! \param    [in] scalabilityState
    //!           Pointer to CODECHAL_DECODE_SCALABILITY_STATE structure
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CheckAndInitialize(
        PCODECHAL_DECODE_PROCESSING_PARAMS  decProcessingParams,
        PCODEC_HEVC_PIC_PARAMS              hevcPicParams,
        PCODECHAL_DECODE_SCALABILITY_STATE  scalabilityState,
        PMOS_SURFACE                        histogramSurface);

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
    int                                   m_numPipe = 1;                  //!< Number of pipes for scalability
    int                                   m_curPipe = 0;                  //!< Current pipe index
    int                                   m_numBuffersAllocated = 0;      //!< Number line buffer allocated
    bool                                  m_enable8TapFilter = false;     //!< Using 8 tap filter
    PCODEC_HEVC_PIC_PARAMS                m_hevcPicParams = nullptr;      //!< HEVC picture parameters
    PCODECHAL_DECODE_SCALABILITY_STATE_G12 m_scalabilityState = nullptr;   //!< Decode scalability state
    PMOS_SURFACE                          m_histogramSurface = nullptr;   //!< Histogram stream out buffer
    MOS_RESOURCE                         *m_resAvsLineBuffers = nullptr;   //!< AVS Line Buffer, one for each pipe
    MOS_RESOURCE                         *m_resSfdLineBuffers = nullptr;   //!< SFD Line Buffer, one for each pipe
    MOS_RESOURCE                          m_resAvsLineTileBuffer = { 0 }; //!< Avs Line Tile Buffer MOS Resource
    MOS_RESOURCE                          m_resSfdLineTileBuffer = { 0 }; //!< SFD Line Tile Buffer MOS Resource
};

#endif
