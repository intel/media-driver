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
//! \file     null_hardware.h
//! \brief    Defines interfaces for null hardware.


#ifndef __NULL_HARDWARE_H__
#define __NULL_HARDWARE_H__

#include "mos_os_specific.h"

class MhwMiInterface;
class NullHW
{
public:
    NullHW() = delete;
    ~NullHW() = delete;

    //!
    //! \brief    Start predicate.
    //! \details  Add predicate command to skip the following commands with NOOP.
    //! \param    [in] miInterface
    //!           Pointer to MhwMiInterface.
    //! \param    [out] cmdBuffer
    //!           command buffer for adding predicate command.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS StartPredicate(PMOS_INTERFACE pOsInterface, MhwMiInterface* miInterface, PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Start predicate. - Refactor Version
    //! \details  Add predicate command to skip the following commands with NOOP.
    //! \param    [in] miInterface
    //!           Pointer to MhwMiInterface.
    //! \param    [out] cmdBuffer
    //!           command buffer for adding predicate command.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS StartPredicateNext(PMOS_INTERFACE pOsInterface, std::shared_ptr<void> pMiItf, PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Stop predicate.
    //! \details  Add predicate command to resume to normal commands execution.
    //! \param    [in] miInterface
    //!           Pointer to MhwMiInterface.
    //! \param    [out] cmdBuffer
    //!           command buffer for adding predicate command.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS StopPredicate(PMOS_INTERFACE pOsInterface, MhwMiInterface* miInterface, PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Stop predicate. - Refactor
    //! \details  Add predicate command to resume to normal commands execution.
    //! \param    [in] miInterface
    //!           Pointer to MhwMiInterface.
    //! \param    [out] cmdBuffer
    //!           command buffer for adding predicate command.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS StopPredicateNext(PMOS_INTERFACE pOsInterface, std::shared_ptr<void> pMiItf, PMOS_COMMAND_BUFFER cmdBuffer);

    //!
    //! \brief    Overwrite status report.
    //! \details  Overwrite the status of status report and bit-stream size.
    //! \param    [out] status
    //!           Status of status report.
    //! \param    [out] streamSize
    //!           The size of bit-stream.
    //! \return   void
    //!
    static void StatusReport(PMOS_INTERFACE pOsInterface, uint32_t &status, uint32_t &streamSize);
};
#endif