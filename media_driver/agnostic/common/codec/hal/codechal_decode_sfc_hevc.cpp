/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     codechal_decode_sfc_hevc.cpp
//! \brief    Implements the decode interface extension for CSC and scaling via SFC for HEVC decoder.
//! \details  Downsampling in this case is supported by the SFC fixed function HW unit.
//!

#include "codechal_decode_sfc_hevc.h"

MOS_STATUS CodechalHevcSfcState::CheckAndInitialize(
    PCODECHAL_DECODE_PROCESSING_PARAMS  decProcessingParams,
    PCODEC_HEVC_PIC_PARAMS              hevcPicParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    if (decProcessingParams)
    {
        if (IsSfcOutputSupported(decProcessingParams,  MhwSfcInterface::SFC_PIPE_MODE_VEBOX))
        {
            bSfcPipeOut = true;

            // Set the input region as the HCP output frame region
            dwInputFrameWidth = hevcPicParams->PicWidthInMinCbsY << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
            dwInputFrameHeight = hevcPicParams->PicHeightInMinCbsY << (hevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
            decProcessingParams->rcInputSurfaceRegion.X = 0;
            decProcessingParams->rcInputSurfaceRegion.Y = 0;
            decProcessingParams->rcInputSurfaceRegion.Width = dwInputFrameWidth;
            decProcessingParams->rcInputSurfaceRegion.Height = dwInputFrameHeight;

            // SFC Initialization. 
            // Initialize once for most of the resources
            // Destroy them in CodecHalHevc_Destroy()
            CODECHAL_HW_CHK_STATUS_RETURN(Initialize(
                decProcessingParams,
                 MhwSfcInterface::SFC_PIPE_MODE_VEBOX));
        }
        else
        {
            CODECHAL_HW_ASSERTMESSAGE("Downsampling parameters are NOT supported by SFC!");
            return MOS_STATUS_UNKNOWN;
        }
    }

    return eStatus;
}

MOS_STATUS CodechalHevcSfcState::UpdateInputInfo(
    PMHW_SFC_STATE_PARAMS   sfcStateParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_HW_FUNCTION_ENTER;

    uint16_t widthAlignUnit, heightAlignUnit;

    sfcStateParams->sfcPipeMode                = MEDIASTATE_SFC_PIPE_VE_TO_SFC;
    sfcStateParams->dwAVSFilterMode            = MEDIASTATE_SFC_AVS_FILTER_8x8;
    sfcStateParams->dwVDVEInputOrderingMode    = MEDIASTATE_SFC_INPUT_ORDERING_VE_4x8;
    sfcStateParams->dwInputChromaSubSampling   = MEDIASTATE_SFC_CHROMA_SUBSAMPLING_420; //IN: NV12 - How about P010?

    // Adjust SFC input surface alignment.
    // As VEBOX doesn't do scaling, input size equals to output size
    // For the VEBOX output to SFC, width is multiple of 16 and height is multiple of 4
    widthAlignUnit                             = pSfcInterface->m_veWidthAlignment;
    heightAlignUnit                            = pSfcInterface->m_veHeightAlignment;

    sfcStateParams->dwInputFrameWidth          = MOS_ALIGN_CEIL(pInputSurface->dwWidth, widthAlignUnit);
    sfcStateParams->dwInputFrameHeight         = MOS_ALIGN_CEIL(pInputSurface->dwHeight, heightAlignUnit);

    return eStatus;
}
