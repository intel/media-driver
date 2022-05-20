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
//! \file     mhw_itf.h
//! \brief    MHW itf common defines
//! \details
//!

#ifndef __MHW_ITF_H__
#define __MHW_ITF_H__

#include "mhw_cmdpar.h"
#include "media_class_trace.h"

#if _MHW_HWCMDPARSER_SUPPORTED
#include "mhw_hwcmd_parser.h"
#endif

//   [Macro Prefixes]                 |   [Macro Suffixes]
//   No Prefix: for external use      |   _T   : type
//   _        : for internal use      |   _F   : function name
//   __       : intermediate macros   |   _M   : class member name
//              only used by other    |   _DECL: declaration
//              macros                |   _DEF : definition

#define __MHW_GETPAR_F(CMD) GETPAR_##CMD         // function name to get the pointer to MHW command parameter
#define MHW_GETPAR_F(CMD) __MHW_GETPAR_F(CMD)    // to support 2-level macro expanding
#define __MHW_GETSIZE_F(CMD) GETSIZE_##CMD       // function name to get MHW command size in byte
#define MHW_GETSIZE_F(CMD) __MHW_GETSIZE_F(CMD)  // to support 2-level macro expanding
#define __MHW_ADDCMD_F(CMD) ADDCMD_##CMD         // function name to add command
#define MHW_ADDCMD_F(CMD) __MHW_ADDCMD_F(CMD)    // to support 2-level macro expanding
#define __MHW_SETCMD_F(CMD) SETCMD_##CMD         // function name to set command data

#define __MHW_GETPAR_DECL(CMD) _MHW_PAR_T(CMD) & MHW_GETPAR_F(CMD)()
#define __MHW_GETSIZE_DECL(CMD) size_t MHW_GETSIZE_F(CMD)() const
#define __MHW_ADDCMD_DECL(CMD) MOS_STATUS MHW_ADDCMD_F(CMD)(PMOS_COMMAND_BUFFER cmdBuf, PMHW_BATCH_BUFFER batchBuf = nullptr)
#define __MHW_SETCMD_DECL(CMD) MOS_STATUS __MHW_SETCMD_F(CMD)()

#define _MHW_CMD_ALL_DEF_FOR_ITF(CMD)    \
public:                                  \
    virtual __MHW_GETPAR_DECL(CMD)  = 0; \
    virtual __MHW_GETSIZE_DECL(CMD) = 0; \
    virtual __MHW_ADDCMD_DECL(CMD)  = 0; \
                                         \
protected:                               \
    virtual __MHW_SETCMD_DECL(CMD) { return MOS_STATUS_SUCCESS; }

#endif  // __MHW_DEF_H__
