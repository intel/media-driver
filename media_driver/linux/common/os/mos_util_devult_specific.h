/*
* Copyright (c) 2013-2018, Intel Corporation
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
//! \file        mos_util_devult_specific.h
//! \brief 
//!
//!

#ifndef __MOS_UTIL_DEVULT_SPECIFIC_H__
#define __MOS_UTIL_DEVULT_SPECIFIC_H__

#include "mos_os_specific.h"
#include "mos_utilities.h"

template<class _Fun, class... _Types>
void MOS_DEVULT_FuncCall(_Fun fun, _Types&&... ags)
{
    if (MosUtilities::m_mosUltFlag && (*MosUtilities::m_mosUltFlag) && fun)
    {
        fun(std::forward<_Types>(ags)...);
    }
}

MOS_EXPORT_DECL extern void (*pfnUltGetCmdBuf)(PMOS_COMMAND_BUFFER pCmdBuffer);

#endif // __MOS_UTIL_DEVULT_SPECIFIC_H__
