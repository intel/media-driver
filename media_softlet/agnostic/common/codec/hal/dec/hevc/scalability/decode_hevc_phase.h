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
//! \file     decode_hevc_phase.h
//! \brief    Defines the common interface for HEVC decode phase.
//!

#ifndef __DECODE_HEVC_PHASE_H__
#define __DECODE_HEVC_PHASE_H__

#include "decode_phase.h"
#include "decode_utils.h"
#include "decode_hevc_pipeline.h"

namespace decode
{

class HevcPhase : public DecodePhase
{
public:
    //!
    //! \brief  HEVC phase constructor
    //! \param  [in] pipeline
    //!         Decode pipeline
    //! \param  [in] scalabOption
    //!         Decode scalability option
    //!
    HevcPhase(DecodePipeline &pipeline, DecodeScalabilityOption &scalabOption)
        : m_scalabOption(scalabOption)
    {
        m_pipeline = dynamic_cast<HevcPipeline *>(&pipeline);
    }

    //!
    //! \brief  HEVC phase destructor
    //!
    virtual ~HevcPhase() {};

    //!
    //! \brief   Initialize the decode scalability phase
    //! \param  [in] pass
    //!         Pass for current phase
    //! \param  [in] pipe
    //!         Pipe for current phase
    //! \param  [in] activePipeNum
    //!         Number of active pipes for current pass
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(uint8_t pass = 0, uint8_t pipe = 0, uint8_t activePipeNum = 1) override
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_pipeline);
        m_pass = pass;
        m_pipe = pipe;
        m_activePipeNum = activePipeNum;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief   Scalability option for this phase if requires context switch
    //! \return  DecodeScalabilityOption *
    //!          Scalability option for this phase
    //!
    virtual DecodeScalabilityOption* GetDecodeScalabilityOption() override
    {
        DECODE_FUNC_CALL();
        return &m_scalabOption;
    }

protected:
    HevcPipeline            *m_pipeline = nullptr;
    DecodeScalabilityOption &m_scalabOption;

MEDIA_CLASS_DEFINE_END(decode__HevcPhase)
};

}
#endif // !__DECODE_HEVC_PHASE_H__

