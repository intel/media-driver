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
//! \file     codechal_decode_histogram_vebox.h
//! \brief    defines the decode histogram through vebox.
//! \details  decode histogram through vebox.
//!
#ifndef __CODECHAL_DECODE_HISTOGRAM_VEBOX_H__
#define __CODECHAL_DECODE_HISTOGRAM_VEBOX_H__
#include "codechal_decode_histogram.h"

//!
//! \class   CodechalDecodeHistogramVebox
//! \brief   Decode histogram through Vebox
//! \details This class defines the member fields, functions for 
//!          decode histogram through vebox
//!
class CodechalDecodeHistogramVebox: public CodechalDecodeHistogram
{
public:
    //!
    //! \brief  Constructor of decode histogram through vebox
    //! \param  [in] hwInterface
    //!         Hardware interface
    //!         [in] osInterface
    //!         OS interface
    //! \return No return
    //!
    CodechalDecodeHistogramVebox(
        CodechalHwInterface *hwInterface,
        MOS_INTERFACE *osInterface);
    //!
    //! \brief  Decode histogram destructor
    //!
    virtual ~CodechalDecodeHistogramVebox();
    //!
    //! \brief  Render and output the histogram
    //! \param  [in] codechalDecoder
    //!         Pointer of codechal decoder
    //! \param  [in] inputSurface
    //!         Input surface to generate histogram
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RenderHistogram(
        CodechalDecode *codechalDecoder,
        MOS_SURFACE *inputSurface);

protected:
    //! \brief  Vebox histogram slice0 offset
    uint32_t            m_veboxHistogramOffset        = 0;

private:
    MhwVeboxInterface   *m_veboxInterface             = nullptr;    //!<  Pointer of vebox interface
    MOS_RESOURCE        m_resSyncObject;                            //!<  Sync object MOS resource
    MOS_RESOURCE        m_resStatisticsOutput;                      //!<  Statistics output MOS resource
    MOS_SURFACE         m_sOutputSurface;                           //!<  Vebox output surface
    uint32_t            m_preWidth                    = 0;          //!<  Previous width, considering about resolution change
    uint32_t            m_preHeight                   = 0;          //!<  Previous height, considering about resolution change

    //!
    //! \brief  Allocate resource
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateResources();
    //!
    //! \brief  Set vebox iecp parameters
    //! \param  [in/out] veboxIecpParams
    //!         Vebox iecp parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVeboxIecpParams(
        PMHW_VEBOX_IECP_PARAMS veboxIecpParams);
    //!
    //! \brief  Set vebox state parameters
    //! \param  [in/out] veboxCmdParams
    //!         Vebox state parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVeboxStateParams(
        PMHW_VEBOX_STATE_CMD_PARAMS veboxCmdParams);
    //!
    //! \brief  Set vebox surface state parameters
    //! \param  [in/out] veboxSurfParams
    //!         Vebox surface state parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVeboxSurfaceStateParams(
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS veboxSurfParams);
    //!
    //! \brief  Set vebox surface di iecp parameters
    //! \param  [in/out] veboxDiIecpParams
    //!         Vebox di iecp parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetVeboxDiIecpParams(
        PMHW_VEBOX_DI_IECP_CMD_PARAMS veboxDiIecpParams);
};

#endif