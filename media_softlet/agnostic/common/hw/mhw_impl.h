/*
* Copyright (c) 2021-2022, Intel Corporation
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
#include "mhw_utilities.h"
#include "media_class_trace.h"

//   [Macro Prefixes]                 |   [Macro Suffixes]
//   No Prefix: for external use      |   _T   : type
//   _        : for internal use      |   _F   : function name
//   __       : intermediate macros   |   _M   : class member name
//              only used by other    |   _DECL: declaration
//              macros                |   _DEF : definition

#define __MHW_CMD_T(CMD) CMD##_CMD  // MHW command type

// MHW command parameters and data
#define __MHW_CMDINFO_T(CMD) std::pair<_MHW_PAR_T(CMD), typename cmd_t::__MHW_CMD_T(CMD)>

#define __MHW_CMDINFO_M(CMD) m_##CMD##_Info

#define __MHW_GETPAR_DEF(CMD)                     \
    __MHW_GETPAR_DECL(CMD) override               \
    {                                             \
        return this->__MHW_CMDINFO_M(CMD)->first; \
    }

#define __MHW_GETSIZE_DEF(CMD)                           \
    __MHW_GETSIZE_DECL(CMD) override                     \
    {                                                    \
        return sizeof(typename cmd_t::__MHW_CMD_T(CMD)); \
    }

#if MHW_HWCMDPARSER_ENABLED
#define MHW_HWCMDPARSER_INITCMDNAME(CMD) this->m_currentCmdName = #CMD
#else
#define MHW_HWCMDPARSER_INITCMDNAME(CMD)
#endif

#define __MHW_ADDCMD_DEF(CMD)                                             \
    __MHW_ADDCMD_DECL(CMD) override                                       \
    {                                                                     \
        MHW_FUNCTION_ENTER;                                               \
        MHW_HWCMDPARSER_INITCMDNAME(CMD);                                 \
        return this->AddCmd(cmdBuf,                                       \
            batchBuf,                                                     \
            this->__MHW_CMDINFO_M(CMD)->second,                           \
            [=]() -> MOS_STATUS { return this->__MHW_SETCMD_F(CMD)(); }); \
    }

#if __cplusplus < 201402L
#define __MHW_CMDINFO_DEF(CMD) std::unique_ptr<__MHW_CMDINFO_T(CMD)> \
    __MHW_CMDINFO_M(CMD) = std::unique_ptr<__MHW_CMDINFO_T(CMD)>(new __MHW_CMDINFO_T(CMD)())
#else
#define __MHW_CMDINFO_DEF(CMD) std::unique_ptr<__MHW_CMDINFO_T(CMD)> \
    __MHW_CMDINFO_M(CMD) = std::make_unique<__MHW_CMDINFO_T(CMD)>()
#endif

#define _MHW_CMD_ALL_DEF_FOR_IMPL(CMD) \
public:                                \
    __MHW_GETPAR_DEF(CMD);             \
    __MHW_GETSIZE_DEF(CMD);            \
    __MHW_ADDCMD_DEF(CMD)              \
protected:                             \
    __MHW_CMDINFO_DEF(CMD)

#define _MHW_SETCMD_OVERRIDE_DECL(CMD) __MHW_SETCMD_DECL(CMD) override

#define _MHW_SETCMD_CALLBASE(CMD)                            \
    MHW_FUNCTION_ENTER;                                      \
    const auto &params = this->__MHW_CMDINFO_M(CMD)->first;  \
    auto &      cmd    = this->__MHW_CMDINFO_M(CMD)->second; \
    MHW_CHK_STATUS_RETURN(base_t::__MHW_SETCMD_F(CMD)())

// DWORD location of a command field
#define _MHW_CMD_DW_LOCATION(field) \
    static_cast<uint32_t>((reinterpret_cast<uint32_t *>(&(cmd.field)) - reinterpret_cast<uint32_t *>(&cmd)))

#define _MHW_CMD_ASSIGN_FIELD(dw, field, value) cmd.dw.field = (value)
#define PATCH_LIST_COMMAND(x)  (x##_NUMBER_OF_ADDRESSES)

namespace mhw
{
class Impl
{
protected:
#if !_MEDIA_RESERVED
    template <typename T, typename = void>
    struct HasExtSettings : std::false_type
    {
    };

    template <typename T>
    struct HasExtSettings<
        T,
        decltype(static_cast<T *>(nullptr)->extSettings, void())>
        : std::true_type
    {
    };

    template <
        typename P,
        typename std::enable_if<HasExtSettings<P>::value, bool>::type = true>
    MOS_STATUS ApplyExtSettings(const P &params, uint32_t *cmd)
    {
        for (const auto &func : params.extSettings)
        {
            MHW_CHK_STATUS_RETURN(func(cmd));
        }
        return MOS_STATUS_SUCCESS;
    }

    template <
        typename P,
        typename std::enable_if<!HasExtSettings<P>::value, bool>::type = true>
    MOS_STATUS ApplyExtSettings(const P &, uint32_t *)
    {
        return MOS_STATUS_SUCCESS;
    }
#endif  // !_MEDIA_RESERVED

    static int32_t Clip3(int32_t x, int32_t y, int32_t z)
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

    static bool MmcEnabled(MOS_MEMCOMP_STATE state)
    {
        return state == MOS_MEMCOMP_RC || state == MOS_MEMCOMP_MC;
    }

    static bool MmcRcEnabled(MOS_MEMCOMP_STATE state)
    {
        return state == MOS_MEMCOMP_RC;
    }

    static uint32_t GetHwTileType(MOS_TILE_TYPE tileType, MOS_TILE_MODE_GMM tileModeGMM, bool gmmTileEnabled)
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

    static uint16_t Fp32_to_Fp16(float value)
    {
        //FP32 sign 1 bit, exponent 8 bits, fraction 23 bits
        //FP16 sign 1 bit, exponent 5 bits, fraction 10 bits
        uint32_t bits     = *((uint32_t *)&value);
        uint16_t sign     = (bits >> 31) & 0x1;
        uint16_t exponent = (bits >> 23) & 0xFF;
        uint32_t fraction = bits & 0x7FFFFF;
        int16_t  exp_fp16;
        if (exponent == 0xFF)
        {
            exp_fp16 = 0x1F;
        }
        else if (exponent == 0)
        {
            exp_fp16 = 0;
        }
        else
        {
            exp_fp16 = exponent - 127 + 15;
        }
        uint16_t fraction_fp16 = (fraction + 0x1000) >> 13;
        uint16_t result        = (sign << 15) | (exp_fp16 << 10) | fraction_fp16;
        if (0 == sign)
        {
            result = MOS_MIN(result, 0x7bff);  //Infinity:0x7c00
        }
        else
        {
            result = MOS_MIN(result, 0xfbff);  //-Infinity:0xfc00
        }
        return result;
    }

    Impl(PMOS_INTERFACE osItf)
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_NO_STATUS_RETURN(osItf);

        m_osItf          = osItf;
        m_userSettingPtr = osItf->pfnGetUserSettingInstance(osItf);
        if (m_osItf->bUsesGfxAddress)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
        }
        else
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
        }
    }

    virtual ~Impl()
    {
        MHW_FUNCTION_ENTER;
    }

    template <typename Cmd, typename CmdSetting>
    MOS_STATUS AddCmd(PMOS_COMMAND_BUFFER cmdBuf,
        PMHW_BATCH_BUFFER                 batchBuf,
        Cmd &                             cmd,
        const CmdSetting &                setting)
    {
        this->m_currentCmdBuf   = cmdBuf;
        this->m_currentBatchBuf = batchBuf;

        // set MHW cmd
        cmd = {};
        MHW_CHK_STATUS_RETURN(setting());

        // call MHW cmd parser
    #if MHW_HWCMDPARSER_ENABLED
        auto instance = mhw::HwcmdParser::GetInstance();
        if (instance)
        {
            instance->ParseCmd(this->m_currentCmdName,
                reinterpret_cast<uint32_t *>(&cmd),
                sizeof(cmd) / sizeof(uint32_t));
        }
    #endif

        // add cmd to cmd buffer
        return Mhw_AddCommandCmdOrBB(m_osItf, cmdBuf, batchBuf, &cmd, sizeof(cmd));
    }

protected:
    MOS_STATUS(*AddResourceToCmd)
    (PMOS_INTERFACE osItf, PMOS_COMMAND_BUFFER cmdBuf, PMHW_RESOURCE_PARAMS params) = nullptr;

    PMOS_INTERFACE              m_osItf           = nullptr;
    MediaUserSettingSharedPtr   m_userSettingPtr  = nullptr;
    PMOS_COMMAND_BUFFER         m_currentCmdBuf   = nullptr;
    PMHW_BATCH_BUFFER           m_currentBatchBuf = nullptr;

#if MHW_HWCMDPARSER_ENABLED
    std::string m_currentCmdName;
#endif  // MHW_HWCMDPARSER_ENABLED
MEDIA_CLASS_DEFINE_END(mhw__Impl)
};
}  // namespace mhw

#endif  // __MHW_IMPL_H__
