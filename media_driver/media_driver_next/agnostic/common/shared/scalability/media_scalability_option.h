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
//! \file     media_scalability_option.h
//! \brief    Defines the common interface for media scalability option
//! \details  The media scalability interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_SCALABILITY_OPTION_H__
#define __MEDIA_SCALABILITY_OPTION_H__
#include "media_scalability_defs.h"
#include "mos_os.h"

class MediaScalabilityOption {
public:
    //!
    //! \brief  Media scalability option destructor
    //
    virtual ~MediaScalabilityOption(){};

    //!
    //! \brief  Set scalability option
    //! \param  [in] params
    //!         Pointer to the input parameters to set scalability option
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetScalabilityOption(ScalabilityPars *params) = 0;
    //!
    //! \brief  check if scalability option matched with current option
    //! \param  [in] params
    //!         Pointer to the input parameters for compare
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual bool IsScalabilityOptionMatched(ScalabilityPars *params) = 0;
    //!
    //! \brief  check if scalability option matched with current option
    //! \return uint8_t
    //!         pipe number
    //!
    uint8_t GetNumPipe() { return m_numPipe; };
protected:
    uint8_t m_numPipe = 0;

    static constexpr uint32_t m_4KFrameWdithTh = 3840;
    static constexpr uint32_t m_4KFrameHeightTh = 2160;
    static constexpr uint32_t m_5KFrameWdithTh  = 5120;
    static constexpr uint32_t m_5KFrameHeightTh = 2880;
};
#endif // !__MEDIA_SCALABILITY_OPTION_H__
