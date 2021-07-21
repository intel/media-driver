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

#if _MHW_HWCMDPARSER_SUPPORTED
#include "mhw_hwcmd_parser.h"
#endif

//   [Macro Prefixes]                 |   [Macro Suffixes]
//   No Prefix: for external use      |   _T   : type
//   _        : for internal use      |   _F   : function name
//   __       : intermediate macros   |   _M   : class member name
//              only used by other    |   _DECL: declaration
//              macros                |   _DEF : definition

#define __MHW_CMD_PAR_GET_F(CMD)       GetCmdPar_##CMD       // function name to get the pointer to MHW command parameter
#define __MHW_CMD_BYTE_SIZE_GET_F(CMD) GetCmdByteSize_##CMD  // function name to get MHW command size in byte
#define __MHW_CMD_ADD_F(CMD)           AddCmd_##CMD          // function name to add command
#define __MHW_CMD_SET_F(CMD)           SetCmd_##CMD          // function name to set command data

#define __MHW_CMD_PAR_GET_DECL(CMD)       _MHW_CMD_PAR_T(CMD)& __MHW_CMD_PAR_GET_F(CMD)()
#define __MHW_CMD_BYTE_SIZE_GET_DECL(CMD) size_t __MHW_CMD_BYTE_SIZE_GET_F(CMD)() const

#define __MHW_CMD_ADD_DECL(CMD) MOS_STATUS __MHW_CMD_ADD_F(CMD)(PMOS_COMMAND_BUFFER cmdBuf, PMHW_BATCH_BUFFER batchBuf = nullptr)
#define __MHW_CMD_SET_DECL(CMD) MOS_STATUS __MHW_CMD_SET_F(CMD)()

#define _MHW_CMD_ALL_DEF_FOR_ITF(CMD)              \
public:                                            \
    virtual __MHW_CMD_PAR_GET_DECL(CMD)       = 0; \
    virtual __MHW_CMD_BYTE_SIZE_GET_DECL(CMD) = 0; \
    virtual __MHW_CMD_ADD_DECL(CMD)           = 0; \
protected:                                         \
    virtual __MHW_CMD_SET_DECL(CMD) { return MOS_STATUS_SUCCESS; }

#define _MHW_SETPARAMS_AND_ADDCMD(CMD, cmdPar_t, GetPar, AddCmd, ...)           \
    {                                                                           \
        auto &par = GetPar(CMD);                                                \
        par       = {};                                                         \
        auto p    = dynamic_cast<const cmdPar_t *>(this);                       \
        if (p)                                                                  \
        {                                                                       \
            p->__MHW_CMD_PAR_SET_F(CMD)(par);                                   \
        }                                                                       \
        LOOP_FEATURE_INTERFACE_RETURN(cmdPar_t, __MHW_CMD_PAR_SET_F(CMD), par); \
        AddCmd(CMD, __VA_ARGS__);                                               \
    }

#endif  // __MHW_DEF_H__
