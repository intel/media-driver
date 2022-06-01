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
//! \file     decode_hevc_phase_long.h
//! \brief    Defines the common interface for HEVC long phase.
//!

#ifndef __DECODE_HEVC_PHASE_LONG_H__
#define __DECODE_HEVC_PHASE_LONG_H__

#include "decode_hevc_phase.h"

namespace decode
{

class HevcPhaseLong : public HevcPhase
{
public:
    //!
    //! \brief  HEVC decode scalability long phase constructor
    //! \param  [in] pipeline
    //!         Decode pipeline
    //! \param  [in] scalabOption
    //!         Decode scalability option
    //! \param  [in] params
    //!         Decode scalability params
    //!
    HevcPhaseLong(DecodePipeline &pipeline, DecodeScalabilityOption &scalabOption)
        : HevcPhase(pipeline, scalabOption)
    {}

    //!
    //! \brief  HEVC decode scalability long phase destructor
    //!
    virtual ~HevcPhaseLong() {};

    virtual uint32_t   GetCmdBufIndex() override;
    virtual uint32_t   GetSubmissionType() override;
    virtual MOS_STATUS GetMode(uint32_t &pipeWorkMode, uint32_t &multiEngineMode) override;
    virtual uint32_t   GetPktId() override;
    virtual bool       ImmediateSubmit() override;
    virtual bool       RequiresContextSwitch() override;

MEDIA_CLASS_DEFINE_END(decode__HevcPhaseLong)
};

}
#endif // !__DECODE_HEVC_PHASE_LONG_H__

