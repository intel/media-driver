/*
* Copyright (c) 2013-2022, Intel Corporation
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
//! \file     mos_util_debug_specific.cpp
//! \brief    This module implments Linux MOS debug functionalities
//!

#include "mos_util_debug.h"

#if MOS_MESSAGES_ENABLED

#include "mos_utilities_specific.h"
#include "mos_utilities.h"
#include "mos_util_debug_specific_next.h"

//!
//! \brief    Prints debug messages when enabled
//! \details  Prints debug messages if prints are enabled and the level of the comp and sub-comp is
//!           set to less than the message level.
//! \param    MOS_MESSAGE_LEVEL level
//!           [in] Level of the message
//! \param    const PCCHAR logtag
//!           [in] For Linux only, used for tagging the message.
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \param    const char  *functionName
//!           [in] pointer to the function name
//! \param    int32_t lineNum
//!           [in] Indicates which line the message locate, -1 for no line output
//! \param    const char  *message
//!           [in] pointer to the message format string
//! \param    var_args
//!           [in] variable list of arguments for the message
//! \return   void
//!
void MOS_Message(
    MOS_MESSAGE_LEVEL level,
    const PCCHAR      logtag,
    MOS_COMPONENT_ID  compID,
    uint8_t           subCompID,
    const PCCHAR      functionName,
    int32_t           lineNum,
    const PCCHAR      message,
    ...)
{
    va_list var_args;
    va_start(var_args, message);
    MosUtilDebugSpecific::MosMessageInternal(level, logtag, compID, subCompID, functionName, lineNum, message, var_args);
    va_end(var_args);
    return;
}

#if MOS_ASSERT_ENABLED
//!
//! \brief    MOS assert function for MOS internal use
//! \details  Halts the cpu in debug mode when expression resolves to zero
//!           and only if assert enabled for both comp and sub-comp.
//!           Nop in release version
//!           Called by MOS_ASSERT macro only
//! \param    MOS_COMPONENT_ID compID
//!           [in] Indicates which component
//! \param    uint8_t subCompID
//!           [in] Indicates which sub-component
//! \return   void
//!
void _MOS_Assert(MOS_COMPONENT_ID compID, uint8_t subCompID)
{
    return MosUtilDebug::MosAssert(compID,subCompID);
}

#endif // MOS_ASSERT_ENABLED

#endif // MOS_MESSAGES_ENABLED
