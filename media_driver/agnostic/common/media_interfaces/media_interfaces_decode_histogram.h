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
//! \file     media_interfaces_decode_histogram.h
//! \brief    Gen-specific factory creation of the decode histogram interfaces
//!

#ifndef __MEDIA_INTERFACES_DECODE_HISTOGRAM_H__
#define __MEDIA_INTERFACES_DECODE_HISTOGRAM_H__

// forward declaration
class CodechalDecodeHistogram;

class DecodeHistogramDevice
{
public:
    virtual ~DecodeHistogramDevice() {};

    //!
    //! \brief    Create decode histogram instance
    //! \details  Entry point to create Gen specific decode histogram instance
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   Pointer to Gen specific decode histogram instance if
    //!           successful, otherwise return nullptr
    //!
    static CodechalDecodeHistogram* CreateFactory(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface);

    //!
    //! \brief    Initializes platform specific decode histogram states
    //! \param    [in] osInterface
    //!           OS interface
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    virtual MOS_STATUS Initialize(
        CodechalHwInterface *hwInterface,
        PMOS_INTERFACE osInterface) = 0;

    CodechalDecodeHistogram *m_decodeHistogramDevice = nullptr; //!< decode histogram device

};

extern template class MediaFactory<uint32_t, DecodeHistogramDevice>;
#endif // __MEDIA_INTERFACES_DECODE_HISTOGRAM_H__