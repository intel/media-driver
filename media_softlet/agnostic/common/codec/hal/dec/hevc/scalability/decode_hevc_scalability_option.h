/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_hevc_scalability_option.h
//! \brief    Defines the Hevc decode scalability option

#ifndef __DECODE_HEVC_SCALABILITY_OPTION_H__
#define __DECODE_HEVC_SCALABILITY_OPTION_H__
#include "decode_scalability_option.h"
#include "media_class_trace.h"
#include "mos_defs.h"

struct ScalabilityPars;

namespace decode
{

class DecodeHevcScalabilityOption : public DecodeScalabilityOption
{
public:
    //!
    //! \brief  Hevc decode scalability mulitipipe copy constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    DecodeHevcScalabilityOption() {};

    //!
    //! \brief  Hevc decode scalability option destructor
    //
    ~DecodeHevcScalabilityOption() {};

    //!
    //! \brief  Hevc decode scalability option constructor
    //! \param  [in] option
    //!         Input scalability option
    //!
    DecodeHevcScalabilityOption(const DecodeHevcScalabilityOption &option);

    //!
    //! \brief  Set scalability option
    //! \param  [in] params
    //!         Pointer to the input parameters to set scalability option
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetScalabilityOption(ScalabilityPars *params) override;

MEDIA_CLASS_DEFINE_END(decode__DecodeHevcScalabilityOption)
};

}
#endif // !__DECODE_HEVC_SCALABILITY_OPTION_H__
