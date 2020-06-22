/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vp_scalability_option.h
//! \brief    Defines the vp scalability option

#ifndef __VP_SCALABILITY_OPTION_H__
#define __VP_SCALABILITY_OPTION_H__
#include "media_scalability_option.h"

namespace vp {
class VpScalabilityOption : public MediaScalabilityOption
{
public:
    //!
    //! \brief  Encode scalability mulitipipe copy constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    VpScalabilityOption() {};

    //!
    //! \brief  Encode scalability mulitipipe copy constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    VpScalabilityOption(const VpScalabilityOption &pOption);

    //!
    //! \brief  Encode scalability option destructor
    //
    ~VpScalabilityOption() {};

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
};
}
#endif // !__VP_SCALABILITY_OPTION_H__
