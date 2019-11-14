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
//! \file        mos_util_debug_specific_next.h 
//! \brief 
//!
//!

#ifndef __MOS_UTIL_DEBUG_SPECIFIC_NEXT_H__
#define __MOS_UTIL_DEBUG_SPECIFIC_NEXT_H__

#include "mos_defs.h"
#include "mos_util_debug_specific.h"


#if MOS_MESSAGES_ENABLED
class MosUtilDebugSpecific
{
public:
    //!
    //! \brief    Prints debug messages in debug mode when enabled
    //! \details  Prints debug messages if the level of the comp and sub-comp is
    //!           set to less than the message level. Nop in release version.
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
    static void MosMessage(
    MOS_MESSAGE_LEVEL level,
    const PCCHAR      logtag,
    MOS_COMPONENT_ID  compID,
    uint8_t           subCompID,
    const PCCHAR      functionName,
    int32_t           lineNum,
    const PCCHAR      message,
    ...);

private:
#if USE_PRETTY_FUNCTION
    //!
    //! \brief    Converts a __PRETTY_FUNCTION__ into Class::Method
    //! \details  Converts a __PRETTY_FUNCTION__ into Class::Method to allow prettier debug output
    //! \param    PCCHAR pcPrettyFunction
    //!           [in] in the form of "TYPE [CLASS::]FUNCTION(INPUT LIST)"
    //! \return   PCCHAR in the form of [CLASS::]FUNCTION
    //!
    static PCCHAR MOS_getClassMethod(PCCHAR pcPrettyFunction);
#endif

private:
    static MOS_MUTEX m_mosMsgMutex;
    static char m_functionName[256]; // 256 is an arbitrary long enough size.
};

#endif //MOS_MESSAGES_ENABLED




#endif //__MOS_UTIL_DEBUG_SPECIFIC_NEXT_H__

