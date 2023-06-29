/*
* Copyright (c) 2018, Intel Corporation
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
#ifndef __GPU_CMD_MFX_BSP_BUF_BASE_ADDR_H__
#define __GPU_CMD_MFX_BSP_BUF_BASE_ADDR_H__

#include "gpu_cmd.h"
#include "mhw_vdbox_mfx_hwcmd_g9_bxt.h"
#include "mhw_vdbox_mfx_hwcmd_g9_skl.h"

template<typename _CmdType>
class GpuCmdMfxBspBufBaseAddr : public GpuCmd<_CmdType>
{
public:

    using typename GpuCmd<_CmdType>::cmd_t;
    using GpuCmd<_CmdType>::CacheCheck1;
    using GpuCmd<_CmdType>::CacheCheck2;

protected:

    using GpuCmd<_CmdType>::m_pCmd;

    void ValidateCachePolicy(const cmd_t *pCmd) const override
    {
        CacheCheck2(m_pCmd->DW3.Value, pCmd->DW3.Value);
        CacheCheck2(m_pCmd->DW6.Value, pCmd->DW6.Value);
        CacheCheck2(m_pCmd->DW9.Value, pCmd->DW9.Value);
    }
};

class GpuCmdMfxBspBufBaseAddrG9Bxt : public GpuCmdMfxBspBufBaseAddr<mhw_vdbox_mfx_g9_bxt::MFX_BSP_BUF_BASE_ADDR_STATE_CMD>
{
public:

    GpuCmdMfxBspBufBaseAddrG9Bxt()
    {
        InitCachePolicy();
    }
    
protected:

    void InitCachePolicy();
};

class GpuCmdMfxBspBufBaseAddrG9Skl : public GpuCmdMfxBspBufBaseAddr<mhw_vdbox_mfx_g9_skl::MFX_BSP_BUF_BASE_ADDR_STATE_CMD>
{
public:

    GpuCmdMfxBspBufBaseAddrG9Skl()
    {
        InitCachePolicy();
    }
    
protected:

    void InitCachePolicy();
};

#endif //__GPU_CMD_MFX_BSP_BUF_BASE_ADDR_H__
