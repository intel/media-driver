/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \brief    MHW cmdpar common defines
//! \details
//!

#ifndef __MHW_CMDPAR_H__
#define __MHW_CMDPAR_H__

#include <memory>

//   [Macro Prefixes]                 |   [Macro Suffixes]
//   No Prefix: for external use      |   _T   : type
//   _        : for internal use      |   _F   : function name
//   __       : intermediate macros   |   _M   : class member name
//              only used by other    |   _DECL: declaration
//              macros                |   _DEF : definition

#define __MHW_PAR_T(CMD) CMD##_PAR              // MHW command parameter type
#define _MHW_PAR_T(CMD) __MHW_PAR_T(CMD)        // to support 2-level macro expanding
#define __MHW_PAR_ALIAS_T(CMD) CMD##_PAR_ALIAS  // MHW command parameter type alias, to avoid compile error

#define __MHW_SETPAR_F(CMD) SETPAR_##CMD       // function name to set MHW command parameters
#define MHW_SETPAR_F(CMD) __MHW_SETPAR_F(CMD)  // to support 2-level macro expanding

#define __MHW_SETPAR_COMMON_DECL(CMD) MHW_SETPAR_F(CMD)(__MHW_PAR_ALIAS_T(CMD) &params) const

// CodecHal uses it to define set MHW params functions in header file
#define MHW_SETPAR_DECL_HDR(CMD) MOS_STATUS __MHW_SETPAR_COMMON_DECL(CMD) override

// CodecHal uses it to define set MHW params functions in source file
#define MHW_SETPAR_DECL_SRC(CMD, CLASS) MOS_STATUS CLASS::__MHW_SETPAR_COMMON_DECL(CMD)

#define _MHW_SETPAR_DEF(CMD)                         \
    using __MHW_PAR_ALIAS_T(CMD) = _MHW_PAR_T(CMD);  \
    virtual MOS_STATUS __MHW_SETPAR_COMMON_DECL(CMD) \
    {                                                \
        return MOS_STATUS_SUCCESS;                   \
    }

#endif  // __MHW_CMDPAR_H__
