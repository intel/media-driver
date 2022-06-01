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
//! \file     decode_phase.h
//! \brief    Defines the common interface for decode phase.
//! \details  The decode phase interface is further sub-divided by codecs,
//!           this file is for the base interface which is shared by all codecs.
//!

#ifndef __DECODE_PHASE_H__
#define __DECODE_PHASE_H__

#include "decode_pipeline.h"
#include "decode_scalability_option.h"

namespace decode
{

class DecodePhase : public ComponentState
{
public:
    //!
    //! \brief  decode phase constructor
    //!
    DecodePhase() {};

    //!
    //! \brief  decode scalability phase destructor
    //!
    virtual ~DecodePhase() {};

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
    virtual MOS_STATUS Initialize(uint8_t pass = 0,
                                  uint8_t pipe = 0,
                                  uint8_t activePipeNum = 1) = 0;

    //!
    //! \brief   Get command buffer index for current phase, primary command buffer index is zero,
    //!          secondary command buffer index start from one.
    //! \return  uint32_t
    //!          Command buffer index for current phase
    //!
    virtual uint32_t   GetCmdBufIndex() = 0;

    //!
    //! \brief   Get command buffer submission type
    //! \return  uint32_t
    //!          Command buffer submission type
    //!
    virtual uint32_t   GetSubmissionType() = 0;

    //!
    //! \brief   Get pipe work mode and engine mode
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMode(uint32_t &pipeWorkMode, uint32_t &multiEngineMode) = 0;

    //!
    //! \brief   Get packet id for this phase
    //! \return  uint32_t
    //!          Packet id for this phase
    //!
    virtual uint32_t   GetPktId() = 0;

    //!
    //! \brief   Indicator if this phase submit immediately
    //! \return  bool
    //!          True if this phase submit immediately, else fasle
    //!
    virtual bool       ImmediateSubmit() = 0;

    //!
    //! \brief   Indicator if this phase reuquires context switch
    //! \return  bool
    //!          True if this phase reuquires context switch, else fasle
    //!
    virtual bool       RequiresContextSwitch() = 0;

    //!
    //! \brief   Scalability option for this phase if requires context switch
    //! \return  DecodeScalabilityOption *
    //!          Scalability option for this phase
    //!
    virtual DecodeScalabilityOption* GetDecodeScalabilityOption() = 0;

    //!
    //! \brief   Get pass number for current phase
    //! \return  uint8_t
    //!          Pass number for current phase
    //!
    virtual uint8_t   GetPass() { return m_pass; }

    //!
    //! \brief   Get pipe number for current phase
    //! \return  uint8_t
    //!          Pipe number for current phase
    //!
    virtual uint8_t   GetPipe() { return m_pipe; }

    //!
    //! \brief   Get active pipe number for current phase
    //! \return  uint8_t
    //!          Active pipe number for current phase
    //!
    virtual uint8_t   GetActivePipeNum() { return m_activePipeNum; }

    static constexpr uint32_t m_primaryCmdBufIdx = 0;
    static constexpr uint32_t m_secondaryCmdBufIdxBase = m_primaryCmdBufIdx + 1;

protected:
    uint8_t m_pass = 0;          //!< Pass index for current phase
    uint8_t m_pipe = 0;          //!< Pipe index for current phase
    uint8_t m_activePipeNum = 1; //!< Number of active pipe number for current pass

MEDIA_CLASS_DEFINE_END(decode__DecodePhase)
};

}
#endif // !__DECODE_PHASE_H__

