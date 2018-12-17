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
#include "gpu_cmd_factory.h"

#ifndef _FULL_OPEN_SOURCE

#include "gpu_cmd_hcp_ind_obj_base_addr.h"
#include "gpu_cmd_hcp_pipe_buf_addr.h"
#include "gpu_cmd_mfx_bsp_buf_base_addr.h"
#include "gpu_cmd_mfx_ind_obj_base_addr.h"
#include "gpu_cmd_mfx_pipe_buf_addr.h"

using namespace std;

#define PUSH(CMD_TYPE) gpuCmds.push_back(make_shared<CMD_TYPE>())
#define GPU_CMD_FACTORY_CREATE(factory) (new factory())

class GpuCmdFactoryEncodeHevcDualPipe : public GpuCmdFactory
{
    void CreateGpuCmds(vector<pcmditf_t> &gpuCmds, Platform_t platform) const override
    {
        switch(platform)
        {
        case igfxSKLAKE:
            break;
        case igfxBROXTON:
            break;
        case igfxBROADWELL:
            break;
        case igfxCANNONLAKE:
            PUSH(GpuCmdHcpPipeBufAddrG10);
            break;
        default:
            return;
        }
    }
};

class GpuCmdFactoryDecodeHEVCLong : public GpuCmdFactory
{
    void CreateGpuCmds(vector<pcmditf_t> &gpuCmds, Platform_t platform) const override
    {
        switch(platform)
        {
        case igfxSKLAKE:
            break;
        case igfxBROXTON:
            break;
        case igfxBROADWELL:
            break;
        case igfxCANNONLAKE:
            PUSH(GpuCmdHcpPipeBufAddrG10);
            PUSH(GpuCmdHcpIndObjBaseAddrG10);
            break;
        default:
            return;
        }
    }
};

class GpuCmdFactoryEncodeAvcDualPipe : public GpuCmdFactory
{
    void CreateGpuCmds(vector<pcmditf_t> &gpuCmds, Platform_t platform) const override
    {
        switch(platform)
        {
        case igfxSKLAKE:
            //PUSH(GpuCmdMfxPipeBufAddrG9Skl);
            //PUSH(GpuCmdMfxIndObjBaseAddrG9Skl);
            //PUSH(GpuCmdMfxBspBufBaseAddrG9Skl);
            break;
        case igfxBROXTON:
            //PUSH(GpuCmdMfxPipeBufAddrG9Bxt);
            //PUSH(GpuCmdMfxIndObjBaseAddrG9Bxt);
            //PUSH(GpuCmdMfxBspBufBaseAddrG9Bxt);
            break;
        case igfxBROADWELL:
            break;
        case igfxCANNONLAKE:
            //PUSH(GpuCmdMfxPipeBufAddrG10);
            //PUSH(GpuCmdMfxIndObjBaseAddrG10);
            //PUSH(GpuCmdMfxBspBufBaseAddrG10);
            break;
        default:
            return;
        }
    }
};

class GpuCmdFactoryDecodeAVCLong : public GpuCmdFactory
{
    void CreateGpuCmds(vector<pcmditf_t> &gpuCmds, Platform_t platform) const override
    {
        switch(platform)
        {
        case igfxSKLAKE:
            //PUSH(GpuCmdMfxPipeBufAddrG9Skl);
            //PUSH(GpuCmdMfxIndObjBaseAddrG9Skl);
            //PUSH(GpuCmdMfxBspBufBaseAddrG9Skl);
            break;
        case igfxBROXTON:
            //PUSH(GpuCmdMfxPipeBufAddrG9Bxt);
            //PUSH(GpuCmdMfxIndObjBaseAddrG9Bxt);
            //PUSH(GpuCmdMfxBspBufBaseAddrG9Bxt);
            break;
        case igfxBROADWELL:
            break;
        case igfxCANNONLAKE:
            //PUSH(GpuCmdMfxPipeBufAddrG10);
            //PUSH(GpuCmdMfxIndObjBaseAddrG10);
            //PUSH(GpuCmdMfxBspBufBaseAddrG10);
            break;
        default:
            return;
        }
    }
};

#else // not _FULL_OPEN_SOURCE

#define GPU_CMD_FACTORY_CREATE(factory) nullptr

#endif // _FULL_OPEN_SOURCE

GpuCmdFactory *g_gpuCmdFactoryEncodeHevcDualPipe = GPU_CMD_FACTORY_CREATE(GpuCmdFactoryEncodeHevcDualPipe);
GpuCmdFactory *g_gpuCmdFactoryDecodeHEVCLong     = GPU_CMD_FACTORY_CREATE(GpuCmdFactoryDecodeHEVCLong);
GpuCmdFactory *g_gpuCmdFactoryEncodeAvcDualPipe  = GPU_CMD_FACTORY_CREATE(GpuCmdFactoryEncodeAvcDualPipe);
GpuCmdFactory *g_gpuCmdFactoryDecodeAVCLong      = GPU_CMD_FACTORY_CREATE(GpuCmdFactoryDecodeAVCLong);
