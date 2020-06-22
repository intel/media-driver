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
//! \file     codechal_decode_histogram.h
//! \brief    defines the decode histogram.
//! \details  decode histogram.
//!
#ifndef __CODECHAL_DECODE_HISTOGRAM_H__
#define __CODECHAL_DECODE_HISTOGRAM_H__
#include "mos_os.h"
#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_decoder.h"

#define HISTOGRAM_BINCOUNT  256

//!
//! \class   CodechalDecodeHistogram
//! \brief   Decode histogram base class
//! \details This class defines the base class for decode histogram, it includes
//!          common member fields, functions, interfaces etc. Specific definitions,
//!          features should be put into their corresponding classes. To create a
//!          decode histogram instance, client needs to new the instance in media interfaces
//!
class CodechalDecodeHistogram
{
public:
    //!
    //! \brief  Decode histogram constructor
    //! \param  [in] hwInterface
    //!         Hardware interface
    //! \param  [in] osInterface
    //!         OS interface
    //! \return No return
    //!
    CodechalDecodeHistogram(
        CodechalHwInterface *hwInterface,
        MOS_INTERFACE *osInterface);
    //!
    //! \brief  Decode histogram destructor
    //!
    virtual ~CodechalDecodeHistogram();
    //!
    //! \brief  Set histogram component
    //! \param  [in] component
    //!         Specify the histogram component
    //! \return No return
    //!
    virtual void setHistogramComponent(uint8_t component);

    //!
    //! \brief  Get surface to hold input histogram buffer
    //! \return PMOS_SURFACE
    //!
    virtual PMOS_SURFACE GetHistogramSurface();

    //!
    //! \brief  Set source surface to hold internal histogram buffer
    //! \return No Return
    //!
    virtual void SetSrcHistogramSurface(PMOS_SURFACE refSurface) {};

    //!
    //! \brief  Render and output the histogram
    //! \param  [in] codechalDecoder
    //!         Pointer of codechal decoder
    //! \param  [in] inputSurface
    //!         Input surface to generate histogram
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS RenderHistogram(
        CodechalDecode *codechalDecoder,
        MOS_SURFACE *inputSurface);

protected:
    CodechalDecode      *m_decoder                      = nullptr;  //!< Pointer of codechal deocder
    MOS_INTERFACE       *m_osInterface                  = nullptr;  //!< Pointer of OS interface
    CodechalHwInterface *m_hwInterface                  = nullptr;  //!< Pointer of hardware interface
    PMOS_SURFACE         m_inputSurface                 = nullptr;  //!< Pointer of input surface
    uint8_t              m_histogramComponent           = 0;        //!< Histogram component
    MOS_RESOURCE         m_resHistogram;                            //!< Internal histogram resource
    MOS_SURFACE          m_inputHistogramSurfaces[4];               //!< Histogram surfaces to hold input buffers
};

#endif
