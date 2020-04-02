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
//! \file        mos_util_devult_specific_next.h
//! \brief 
//!
//!

#ifndef __MOS_UTIL_DEVULT_SPECIFIC_NEXT_H__
#define __MOS_UTIL_DEVULT_SPECIFIC_NEXT_H__

extern uint8_t MosUltFlag;


class MosUtilDevUltSpecific
{
public:
    template<class _Fun, class... _Types>
    static void MOS_DEVULT_FuncCall(_Fun fun, _Types&&... ags)
    {
        if (MosUltFlag && fun)
        {
            fun(std::forward<_Types>(ags)...);
        }
    }

};
MOS_EXPORT_DECL extern void (*pfnUltGetCmdBuf)(PMOS_COMMAND_BUFFER pCmdBuffer);

#endif // __MOS_UTIL_DEVULT_SPECIFIC_NEXT_H__
