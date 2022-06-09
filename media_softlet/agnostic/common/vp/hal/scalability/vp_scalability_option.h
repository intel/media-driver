/*
* Copyright (c) 2019-2020, Intel Corporation
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

#include <stdint.h>
#include "mos_defs.h"
#include "media_scalability_option.h"
#include "media_class_trace.h"

struct ScalabilityPars;

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

    //!
    //! \brief  check if SFC flag is set
    //! \return bool
    //!         Ture for using SFC
    //!
    bool IsUsingSFC() { return m_usingSFC; };

    //! \brief  Get LRCA count
    //! \return uint32_t
    //!         Return LRCA count
    //!
    uint32_t GetLRCACount();

    //! \brief  Get max pipe number of multipe pipe mode
    //! \return uint8_t
    //!         Return decode scalability mode
    //!
    uint8_t GetMaxMultiPipeNum() { return m_maxNumMultiPipe; }

    bool m_usingSFC = false;

    uint8_t m_typicalNumMultiPipe = 2;
    uint8_t m_maxNumMultiPipe     = 2;

MEDIA_CLASS_DEFINE_END(vp__VpScalabilityOption)
};
}
#endif // !__VP_SCALABILITY_OPTION_H__
