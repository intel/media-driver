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
//! \file     mhw_def.h
//! \brief    MHW interface common defines
//! \details
//!

#ifndef __MHW_DEF_H__
#define __MHW_DEF_H__

#include <memory>
#include <vector>
#include "mhw_utilities.h"

//   [Macro Prefixes]                 |   [Macro Suffixes]
//   No Prefix: for external use      |   _T   : type
//   _        : for internal use      |   _F   : function name
//   __       : intermediate macros   |   _M   : class member name
//              only used by other    |   _DECL: declaration
//              macros                |   _DEF : definition

#define MHW_HWCMDPARSER_ENABLED (_MHW_HWCMDPARSER_SUPPORTED && (_DEBUG || _RELEASE_INTERNAL))
#if MHW_HWCMDPARSER_ENABLED
#include "mhw_hwcmd_parser.h"
#else
#define MHW_HWCMDPARSER_INIT(osInterface)
#define MHW_HWCMDPARSER_DESTROY()
#define MHW_HWCMDPARSER_PARSEFIELDSLAYOUTEN() false
#define MHW_HWCMDPARSER_PARSEFIELDLAYOUT(dw, field)
#define MHW_HWCMDPARSER_FRAMEINFOUPDATE(frameType)
#define MHW_HWCMDPARSER_PARSECMD(cmdName, cmdData, dwLen)
#define MHW_HWCMDPARSER_PARSECMDBUF(cmdBuf, dwLen)
#define MHW_HWCMDPARSER_PARSECMDBUFGFX(cmdBufGfx)
#endif

#define _MHW_CMD_T(cmd)      cmd##_CMD         // MHW command type
#define __MHW_CMD_PAR_M(cmd) m_##cmd##_Params  // member which is a pointer to MHW command parameters

#define __MHW_CMD_PAR_GET_F(cmd)       GetCmdPar_##cmd       // function name to get the pointer to MHW command parameter
#define __MHW_CMD_BYTE_SIZE_GET_F(cmd) GetCmdByteSize_##cmd  // function name to get MHW command size in byte
#define __MHW_CMD_ADD_F(cmd)           AddCmd_##cmd          // function name to add command
#define __MHW_CMD_CREATE_F(cmd)        CreateCmd_##cmd       // function name to create command data
#define __MHW_CMD_SET_F(cmd)           SetCmd_##cmd          // function name to set command data

#define __MHW_CMD_PAR_GET_DECL(cmd)       mhw::Pointer<_MHW_CMD_PAR_T(cmd)> __MHW_CMD_PAR_GET_F(cmd)(bool reset)
#define __MHW_CMD_BYTE_SIZE_GET_DECL(cmd) size_t __MHW_CMD_BYTE_SIZE_GET_F(cmd)() const

#define __MHW_CMD_ADD_DECL(cmd) MOS_STATUS __MHW_CMD_ADD_F(cmd)(PMOS_COMMAND_BUFFER cmdBuf,                  \
                                                                PMHW_BATCH_BUFFER   batchBuf      = nullptr, \
                                                                const uint8_t *     extraData     = nullptr, \
                                                                size_t              extraDataSize = 0)

#define __MHW_CMD_CREATE_DECL(cmd) mhw::Pointer<std::vector<uint8_t>> __MHW_CMD_CREATE_F(cmd)() const

#if MHW_HWCMDPARSER_ENABLED
#define __MHW_CMD_SET_DECL(cmd) MOS_STATUS __MHW_CMD_SET_F(cmd)(uint8_t * cmdDataPtr, const std::string &cmdName = #cmd)
#else
#define __MHW_CMD_SET_DECL(cmd) MOS_STATUS __MHW_CMD_SET_F(cmd)(uint8_t * cmdDataPtr)
#endif

#define _MHW_CMD_SET_DECL_OVERRIDE(cmd) __MHW_CMD_SET_DECL(cmd) override

#define __MHW_CMD_PAR_GET_COMMON_DEF(cmd, par_t)                                      \
    __MHW_CMD_PAR_GET_DECL(cmd) override                                              \
    {                                                                                 \
        MHW_FUNCTION_ENTER;                                                           \
        if (this->__MHW_CMD_PAR_M(cmd) == nullptr)                                    \
        {                                                                             \
            this->__MHW_CMD_PAR_M(cmd) = mhw::MakePointer<par_t(cmd)>();              \
        }                                                                             \
        if (reset)                                                                    \
        {                                                                             \
            auto p = mhw::DynamicPointerCast<par_t(cmd)>(this->__MHW_CMD_PAR_M(cmd)); \
            *p     = par_t(cmd)();                                                    \
        }                                                                             \
        return this->__MHW_CMD_PAR_M(cmd);                                            \
    }

#define __MHW_CMD_BYTE_SIZE_GET_DEF(cmd)                \
    __MHW_CMD_BYTE_SIZE_GET_DECL(cmd) override          \
    {                                                   \
        return sizeof(typename cmd_t::_MHW_CMD_T(cmd)); \
    }

#define __MHW_CMD_ADD_DEF(cmd)                                                                            \
    __MHW_CMD_ADD_DECL(cmd) override                                                                      \
    {                                                                                                     \
        MHW_FUNCTION_ENTER;                                                                               \
        this->m_currentCmdBuf   = cmdBuf;                                                                 \
        this->m_currentBatchBuf = batchBuf;                                                               \
        auto cmdData            = this->__MHW_CMD_CREATE_F(cmd)();                                        \
        MHW_CHK_STATUS_RETURN(this->__MHW_CMD_SET_F(cmd)(cmdData->data()));                               \
        MHW_HWCMDPARSER_PARSECMD(#cmd,                                                                    \
            reinterpret_cast<uint32_t *>(cmdData->data()),                                                \
            cmdData->size() / sizeof(uint32_t));                                                          \
        MHW_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(cmdBuf, batchBuf, cmdData->data(), cmdData->size())); \
        if (extraData && extraDataSize > 0)                                                               \
        {                                                                                                 \
            MHW_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(cmdBuf, batchBuf, extraData, extraDataSize));     \
        }                                                                                                 \
        return MOS_STATUS_SUCCESS;                                                                        \
    }

#define __MHW_CMD_CREATE_DEF(cmd)                                              \
    __MHW_CMD_CREATE_DECL(cmd) override                                        \
    {                                                                          \
        typename cmd_t::_MHW_CMD_T(cmd) cmdData;                               \
        auto p = reinterpret_cast<const uint8_t *>(&cmdData);                  \
        return mhw::MakePointer<std::vector<uint8_t>>(p, p + sizeof(cmdData)); \
    }

#define _MHW_CMD_ALL_DEF_FOR_ITF(cmd)              \
public:                                            \
    virtual __MHW_CMD_PAR_GET_DECL(cmd)       = 0; \
    virtual __MHW_CMD_BYTE_SIZE_GET_DECL(cmd) = 0; \
    virtual __MHW_CMD_ADD_DECL(cmd)           = 0

#define _MHW_CMD_ALL_DEF_FOR_IMPL(cmd)                             \
public:                                                            \
    __MHW_CMD_PAR_GET_COMMON_DEF(cmd, _MHW_CMD_PAR_T);             \
    __MHW_CMD_ADD_DEF(cmd);                                        \
                                                                   \
protected:                                                         \
    virtual __MHW_CMD_CREATE_DECL(cmd) = 0;                        \
    virtual __MHW_CMD_SET_DECL(cmd) { return MOS_STATUS_SUCCESS; } \
    mhw::Pointer<_MHW_CMD_PAR_T(cmd)> __MHW_CMD_PAR_M(cmd) = nullptr

#define _MHW_CMD_ALL_DEF_FOR_IMPL_GENERIC(cmd) \
public:                                        \
    __MHW_CMD_BYTE_SIZE_GET_DEF(cmd);          \
                                               \
protected:                                     \
    __MHW_CMD_CREATE_DEF(cmd)

#define __MHW_CMDSET_GETCMDPARAMS_COMMON(cmdName, par_t)                                   \
    MHW_FUNCTION_ENTER;                                                                    \
    auto cmd    = reinterpret_cast<typename cmd_t::_MHW_CMD_T(cmdName) *>(cmdDataPtr);     \
    auto params = mhw::DynamicPointerCast<par_t(cmdName)>(this->__MHW_CMD_PAR_M(cmdName)); \
    MHW_CHK_NULL_RETURN(cmd);                                                              \
    MHW_CHK_NULL_RETURN(params)

#define __MHW_CMDSET_CALLBASE(cmd) base_t::__MHW_CMD_SET_F(cmd)(cmdDataPtr)

#define _MHW_CMDSET_GETCMDPARAMS_AND_CALLBASE(cmd)         \
    __MHW_CMDSET_GETCMDPARAMS_COMMON(cmd, _MHW_CMD_PAR_T); \
    __MHW_CMDSET_CALLBASE(cmd)

#define _MHW_SETPARAMS_AND_ADDCMD(cmd, cmdPar_t, GetPar, AddCmd, ...)           \
    {                                                                           \
        auto par = GetPar(cmd, true);                                           \
        auto p   = dynamic_cast<const cmdPar_t *>(this);                        \
        if (p)                                                                  \
        {                                                                       \
            p->__MHW_CMD_PAR_SET_F(cmd)(par);                                   \
        }                                                                       \
        LOOP_FEATURE_INTERFACE_RETURN(cmdPar_t, __MHW_CMD_PAR_SET_F(cmd), par); \
        AddCmd(cmd, __VA_ARGS__);                                               \
    }

// DWORD location of a command field
#define _MHW_CMD_DW_LOCATION(field) \
    static_cast<uint32_t>((reinterpret_cast<uint32_t *>(&(cmd->field)) - reinterpret_cast<uint32_t *>(&(*cmd))))

#define _MHW_CMD_ASSIGN_FIELD(dw, field, value) cmd->dw.field = (value)

namespace mhw
{
inline int32_t Clip3(int32_t x, int32_t y, int32_t z)
{
    int32_t ret = 0;

    if (z < x)
    {
        ret = x;
    }
    else if (z > y)
    {
        ret = y;
    }
    else
    {
        ret = z;
    }

    return ret;
}

inline bool MmcEnabled(MOS_MEMCOMP_STATE state)
{
    return state == MOS_MEMCOMP_RC || state == MOS_MEMCOMP_MC;
}

inline bool MmcRcEnabled(MOS_MEMCOMP_STATE state)
{
    return state == MOS_MEMCOMP_RC;
}

inline uint32_t GetHwTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
{
    uint32_t tileMode = 0;

    if (gmmTileEnabled)
    {
        return tileModeGMM;
    }

    switch (tileType)
    {
    case MOS_TILE_LINEAR:
        tileMode = 0;
        break;
    case MOS_TILE_YS:
        tileMode = 1;
        break;
    case MOS_TILE_X:
        tileMode = 2;
        break;
    default:
        tileMode = 3;
        break;
    }

    return tileMode;
}
}  // namespace mhw

#endif  // __MHW_DEF_H__
