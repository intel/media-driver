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
#ifndef __GPU_CMD_FACTORY_H__
#define __GPU_CMD_FACTORY_H__

#include <gpu_cmd.h>
#ifndef _FULL_OPEN_SOURCE
#include "gpu_cmd_hcp_ind_obj_base_addr.h"
#include "gpu_cmd_hcp_pipe_buf_addr.h"
#include "gpu_cmd_mfx_bsp_buf_base_addr.h"
#include "gpu_cmd_mfx_ind_obj_base_addr.h"
#include "gpu_cmd_mfx_pipe_buf_addr.h"
#endif // _FULL_OPEN_SOURCE

class GpuCmdFactory
{
public:

    virtual void CreateGpuCmds(std::vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const = 0;

    virtual ~GpuCmdFactory() { }
};

#ifndef _FULL_OPEN_SOURCE

class GpuCmdFactoryEncodeHevcDualPipe : public GpuCmdFactory
{
    void CreateGpuCmds(std::vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const override;
};

class GpuCmdFactoryDecodeHEVCLong : public GpuCmdFactory
{
    void CreateGpuCmds(std::vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const override;
};

class GpuCmdFactoryEncodeAvcDualPipe : public GpuCmdFactory
{
    void CreateGpuCmds(std::vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const override;
};

class GpuCmdFactoryDecodeAVCLong : public GpuCmdFactory
{
    void CreateGpuCmds(std::vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const override;
};

extern GpuCmdFactory *g_gpuCmdFactoryEncodeHevcDualPipe;
extern GpuCmdFactory *g_gpuCmdFactoryDecodeHEVCLong;
extern GpuCmdFactory *g_gpuCmdFactoryEncodeAvcDualPipe;
extern GpuCmdFactory *g_gpuCmdFactoryDecodeAVCLong;

#endif // _FULL_OPEN_SOURCE

#endif // __GPU_CMD_FACTORY_H__
