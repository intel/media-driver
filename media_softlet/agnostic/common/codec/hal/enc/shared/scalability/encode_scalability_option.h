/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     encode_scalability_option.h
//! \brief    Defines the encode scalability option

#ifndef __ENCODE_SCALABILITY_OPTION_H__
#define __ENCODE_SCALABILITY_OPTION_H__

#include "media_class_trace.h"
#include "media_scalability_option.h"
#include "mos_defs.h"
struct ScalabilityPars;

namespace encode {
class EncodeScalabilityOption : public MediaScalabilityOption
{
public:
    //!
    //! \brief  Encode scalability mulitipipe copy constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    EncodeScalabilityOption() {};

    //!
    //! \brief  Encode scalability mulitipipe copy constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    EncodeScalabilityOption(const EncodeScalabilityOption &pOption);

    //!
    //! \brief  Encode scalability option destructor
    //
    ~EncodeScalabilityOption() {};

    //!
    //! \brief  Set scalability option
    //! \param  [in] params
    //!         Pointer to the input parameters to set scalability option
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetScalabilityOption(ScalabilityPars *params);
    //!
    //! \brief  check if scalability option matched with current option
    //! \param  [in] params
    //!         Pointer to the input parameters for compare
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual bool IsScalabilityOptionMatched(ScalabilityPars *params);
    //!
    //! \brief  check if VDEnc Enabled
    //! \return bool
    //!         true if enabled, else false
    //!
    virtual bool IsVdencEnabled() const { return m_enabledVdenc; }
private:
    bool m_enabledVdenc = false;

MEDIA_CLASS_DEFINE_END(encode__EncodeScalabilityOption)
};
}
#endif // !__ENCODE_SCALABILITY_OPTION_H__
