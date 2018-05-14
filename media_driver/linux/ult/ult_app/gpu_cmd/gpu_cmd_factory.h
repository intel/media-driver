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

class GpuCmdFactory
{
public:

    using pcmditf_t = GpuCmdInterface::pcmditf_t;

    virtual void CreateGpuCmds(std::vector<pcmditf_t> &gpuCmds, Platform_t platform) const = 0;

    virtual ~GpuCmdFactory() { }
};

extern GpuCmdFactory *g_gpuCmdFactoryEncodeHevcDualPipe;
extern GpuCmdFactory *g_gpuCmdFactoryDecodeHEVCLong;
extern GpuCmdFactory *g_gpuCmdFactoryEncodeAvcDualPipe;
extern GpuCmdFactory *g_gpuCmdFactoryDecodeAVCLong;

#endif // __GPU_CMD_FACTORY_H__
