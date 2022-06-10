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
//! \file     decode_scalability_option.h
//! \brief    Defines the decode scalability option

#ifndef __DECODE_SCALABILITY_OPTION_H__
#define __DECODE_SCALABILITY_OPTION_H__
#include "media_scalability_option.h"
#include "decode_scalability_defs.h"
#include "media_class_trace.h"
#include "mos_defs.h"
#include "mos_resource_defs.h"
struct ScalabilityPars;

namespace decode {
class DecodeScalabilityOption : public MediaScalabilityOption
{
public:
    //!
    //! \brief  decode scalability option constructor
    //! \param  [in] pOption
    //!         Pointer to input scalability option
    //!
    DecodeScalabilityOption() {};

    //!
    //! \brief  decode scalability option constructor
    //! \param  [in] option
    //!         Pointer to input scalability option
    //!
    DecodeScalabilityOption(const DecodeScalabilityOption &option);

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
    //! \return bool
    //!         Ture if matched, else false
    //!
    virtual bool IsScalabilityOptionMatched(ScalabilityPars *params);
    //!
    //! \brief  check if scalability option matched with current option
    //! \param  [in] scalabOption
    //!         Input scalability option for compare
    //! \return bool
    //!         Ture if matched, else false
    //!
    virtual bool IsScalabilityOptionMatched(MediaScalabilityOption &scalabOption);

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

    //! \brief  Get decode scalability mode
    //! \return ScalabilityMode
    //!         Return decode scalability mode
    //!
    ScalabilityMode GetMode() { return m_mode; }

    //! \brief  Get max pipe number of multipe pipe mode
    //! \return uint8_t
    //!         Return decode scalability mode
    //!
    uint8_t GetMaxMultiPipeNum() { return m_maxNumMultiPipe; }

    //! \brief  Get FE separate submission flag
    //! \return bool
    //!         Return true if FE separate submission, else return false
    //!
    bool IsFESeparateSubmission() { return m_FESeparateSubmission; }

    //! \brief  Get LRCA count
    //! \return uint32_t
    //!         Return LRCA count
    //!
    uint32_t GetLRCACount();

private:
    inline static bool IsRextFormat(MOS_FORMAT format)
    {
        return ((format != Format_NV12) && (format != Format_P010));
    }

    virtual bool IsSinglePipeDecode(DecodeScalabilityPars &params);

    virtual bool IsRealTileDecode(DecodeScalabilityPars &params);

    virtual bool IsResolutionMatchMultiPipeThreshold1(
        uint32_t frameWidth, uint32_t frameHeight, MOS_FORMAT surfaceFormat);
    virtual bool IsResolutionMatchMultiPipeThreshold2(
        uint32_t frameWidth, uint32_t frameHeight);

#if (_DEBUG || _RELEASE_INTERNAL)
    inline static uint8_t GetUserPipeNum(uint8_t numVdbox, uint8_t userPipeNum);
#endif

protected:
    static const uint8_t m_typicalNumMultiPipe = 2;
    static const uint8_t m_maxNumMultiPipe = 3;

    bool m_usingSFC = false;
    bool m_usingSlimVdbox = false;

    bool            m_FESeparateSubmission = false;
    ScalabilityMode m_mode                 = scalabilitySingleMode;

MEDIA_CLASS_DEFINE_END(decode__DecodeScalabilityOption)
};
}
#endif // !__DECODE_SCALABILITY_OPTION_H__
