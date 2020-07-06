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
//! \file     decode_scalability_option.h
//! \brief    Defines the decode scalability option

#ifndef __DECODE_SCALABILITY_OPTION_H__
#define __DECODE_SCALABILITY_OPTION_H__
#include "media_scalability_option.h"
#include "decode_scalability_defs.h"

namespace decode {
class DecodeScalabilityOption : public MediaScalabilityOption
{
public:
    //!
    //! \brief  decode scalability mulitipipe copy constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    DecodeScalabilityOption() {};

    //!
    //! \brief  decode scalability mulitipipe copy constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    DecodeScalabilityOption(const DecodeScalabilityOption &pOption);

    //!
    //! \brief  decode scalability option destructor
    //
    ~DecodeScalabilityOption() {};

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

    //!
    //! \brief  check if Slim vdbox flag is set
    //! \return bool
    //!         Ture for using slim vdbox
    //!
    bool IsUsingSlimVdbox() { return m_usingSlimVdbox; };

private:
    inline static bool IsRextFormat(MOS_FORMAT format)
    {
        return ((format != Format_NV12) && (format != Format_P010));
    }

protected:
    bool m_usingSFC = false;
    bool m_usingSlimVdbox = false;
};
}
#endif // !__DECODE_SCALABILITY_OPTION_H__
