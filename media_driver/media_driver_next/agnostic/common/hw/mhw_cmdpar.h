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
//! \file     mhw_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_CMDPAR_H__
#define __MHW_CMDPAR_H__

#include <cstdint>

#define _MHW_CMD_PAR_T(cmd)  cmd##_Params   // MHW command parameter type
#define __MHW_CMD_PAR_T(cmd) cmd##_Params_  // MHW command parameter type alias, to avoid compile error

#define __MHW_CMD_PAR_SET_F(cmd) SetCmdParams_##cmd  // function name to set MHW command parameters

#define __MHW_CMD_PAR_SET_COMMON_DECL(cmd) __MHW_CMD_PAR_SET_F(cmd)(mhw::Pointer<__MHW_CMD_PAR_T(cmd)> par) const

// CodecHal uses it to define set MHW params functions in header file
#define MHW_CMD_PAR_SET_DECL_HDR(cmd) MOS_STATUS __MHW_CMD_PAR_SET_COMMON_DECL(cmd) override

// CodecHal uses it to define set MHW params functions in source file
#define MHW_CMD_PAR_SET_DECL_SRC(cmd, CLASS) MOS_STATUS CLASS::__MHW_CMD_PAR_SET_COMMON_DECL(cmd)

#define _MHW_CMD_PAR_SET_DEF(cmd)                         \
    using __MHW_CMD_PAR_T(cmd) = _MHW_CMD_PAR_T(cmd);     \
    virtual MOS_STATUS __MHW_CMD_PAR_SET_COMMON_DECL(cmd) \
    {                                                     \
        return MOS_STATUS_SUCCESS;                        \
    }

namespace mhw
{
template <typename T>
using Pointer = std::shared_ptr<T>;

template <typename T, typename U>
Pointer<T> StaticPointerCast(const Pointer<U> &r)
{
    return std::static_pointer_cast<T>(r);
}

template <typename T, typename U>
Pointer<T> DynamicPointerCast(const Pointer<U> &r)
{
    return std::dynamic_pointer_cast<T>(r);
}

template <typename T, typename U>
Pointer<T> ConstPointerCast(const Pointer<U> &r)
{
    return std::const_pointer_cast<T>(r);
}

template <typename T>
Pointer<T> MakePointer()
{
    return std::make_shared<T>();
}

template <typename T, typename... Args>
Pointer<T> MakePointer(Args &&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}
}  // namespace mhw

#endif  // __MHW_CMDPAR_H__
