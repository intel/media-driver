/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     mhw_impl.h
//! \brief    MHW impl common defines
//! \details
//!

#ifndef __MHW_IMPL_H__
#define __MHW_IMPL_H__

#include "mhw_itf.h"

//   [Macro Prefixes]                 |   [Macro Suffixes]
//   No Prefix: for external use      |   _T   : type
//   _        : for internal use      |   _F   : function name
//   __       : intermediate macros   |   _M   : class member name
//              only used by other    |   _DECL: declaration
//              macros                |   _DEF : definition

#define _MHW_CMD_T(CMD)      CMD##_CMD         // MHW command type
#define __MHW_CMD_PAR_M(CMD) m_##CMD##_Info    // MHW command parameters and data

#define __MHW_CMD_PAR_GET_DEF(CMD)                \
    __MHW_CMD_PAR_GET_DECL(CMD) override          \
    {                                             \
        return this->__MHW_CMD_PAR_M(CMD)->first; \
    }

#define __MHW_CMD_BYTE_SIZE_GET_DEF(CMD)                \
    __MHW_CMD_BYTE_SIZE_GET_DECL(CMD) override          \
    {                                                   \
        return sizeof(typename cmd_t::_MHW_CMD_T(CMD)); \
    }

#define __MHW_CMD_ADD_DEF(CMD)                                                                            \
    __MHW_CMD_ADD_DECL(CMD) override                                                                      \
    {                                                                                                     \
        MHW_FUNCTION_ENTER;                                                                               \
                                                                                                          \
        this->m_currentCmdBuf   = cmdBuf;                                                                 \
        this->m_currentBatchBuf = batchBuf;                                                               \
                                                                                                          \
        /*set MHW cmd*/                                                                                   \
        auto &cmd = this->__MHW_CMD_PAR_M(CMD)->second;                                                   \
        cmd       = {};                                                                                   \
        MHW_CHK_STATUS_RETURN(this->__MHW_CMD_SET_F(CMD)());                                              \
                                                                                                          \
        /*call MHW cmd parser*/                                                                           \
        MHW_HWCMDPARSER_PARSECMD(#CMD, reinterpret_cast<uint32_t *>(&cmd), sizeof(cmd)/sizeof(uint32_t)); \
                                                                                                          \
        /*add cmd to cmd buffer*/                                                                         \
        MHW_CHK_STATUS_RETURN(Mhw_AddCommandCmdOrBB(cmdBuf, batchBuf, &cmd, sizeof(cmd)));                \
                                                                                                          \
        return MOS_STATUS_SUCCESS;                                                                        \
    }

#define _MHW_CMD_ALL_DEF_FOR_IMPL(CMD)                                                                    \
public:                                                                                                   \
    __MHW_CMD_PAR_GET_DEF(CMD);                                                                           \
    __MHW_CMD_BYTE_SIZE_GET_DEF(CMD);                                                                     \
    __MHW_CMD_ADD_DEF(CMD)                                                                                \
protected:                                                                                                \
    std::unique_ptr<std::pair<_MHW_CMD_PAR_T(CMD), typename cmd_t::_MHW_CMD_T(CMD)>> __MHW_CMD_PAR_M(CMD) \
        = std::unique_ptr<std::pair<_MHW_CMD_PAR_T(CMD), typename cmd_t::_MHW_CMD_T(CMD)>>(               \
            new std::pair<_MHW_CMD_PAR_T(CMD), typename cmd_t::_MHW_CMD_T(CMD)>())

#define _MHW_CMD_SET_DECL_OVERRIDE(CMD) __MHW_CMD_SET_DECL(CMD) override

#define _MHW_CMDSET_GETCMDPARAMS_AND_CALLBASE(CMD)           \
    MHW_FUNCTION_ENTER;                                      \
    const auto &params = this->__MHW_CMD_PAR_M(CMD)->first;  \
    auto       &cmd    = this->__MHW_CMD_PAR_M(CMD)->second; \
    MHW_CHK_STATUS_RETURN(base_t::__MHW_CMD_SET_F(CMD)())

// DWORD location of a command field
#define _MHW_CMD_DW_LOCATION(field) \
    static_cast<uint32_t>((reinterpret_cast<uint32_t *>(&(cmd.field)) - reinterpret_cast<uint32_t *>(&cmd)))

#define _MHW_CMD_ASSIGN_FIELD(dw, field, value) cmd.dw.field = (value)

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

class Impl
{
protected:
    Impl(PMOS_INTERFACE osItf)
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_NO_STATUS_RETURN(osItf);

        m_osItf = osItf;
        if (m_osItf->bUsesGfxAddress)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
        }
        else
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
        }
    }

    virtual ~Impl() = default;

protected:
    MOS_STATUS(*AddResourceToCmd)
    (PMOS_INTERFACE osItf, PMOS_COMMAND_BUFFER cmdBuf, PMHW_RESOURCE_PARAMS params) = nullptr;

    PMOS_INTERFACE      m_osItf           = nullptr;
    PMOS_COMMAND_BUFFER m_currentCmdBuf   = nullptr;
    PMHW_BATCH_BUFFER   m_currentBatchBuf = nullptr;

#if MHW_HWCMDPARSER_ENABLED
    std::shared_ptr<HwcmdParser> m_hwcmdParser       = mhw::HwcmdParser::GetInstance();
    bool                         m_parseFieldsLayout = m_hwcmdParser->ParseFieldsLayoutEn();
#endif  // MHW_HWCMDPARSER_ENABLED
};
}  // namespace mhw

#endif  // __MHW_IMPL_H__
