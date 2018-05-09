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

using namespace std;

void GpuCmdFactoryEncodeHevcDualPipe::CreateGpuCmds(vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const
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
        gpuCmds.push_back(make_shared<GpuCmdHcpPipeBufAddrG10>());
        break;
    default:
        return;
    }
}

void GpuCmdFactoryDecodeHEVCLong::CreateGpuCmds(vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const
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
        gpuCmds.push_back(make_shared<GpuCmdHcpPipeBufAddrG10>());
        gpuCmds.push_back(make_shared<GpuCmdHcpIndObjBaseAddrG10>());
        break;
    default:
        return;
    }
}

void GpuCmdFactoryEncodeAvcDualPipe::CreateGpuCmds(vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const
{
    switch(platform)
    {
    case igfxSKLAKE:
        //gpuCmds.push_back(make_shared<GpuCmdMfxPipeBufAddrG9Skl>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxIndObjBaseAddrG9Skl>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxBspBufBaseAddrG9Skl>());
        break;
    case igfxBROXTON:
        //gpuCmds.push_back(make_shared<GpuCmdMfxPipeBufAddrG9Bxt>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxIndObjBaseAddrG9Bxt>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxBspBufBaseAddrG9Bxt>());
        break;
    case igfxBROADWELL:
        break;
    case igfxCANNONLAKE:
        //gpuCmds.push_back(make_shared<GpuCmdMfxPipeBufAddrG10>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxIndObjBaseAddrG10>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxBspBufBaseAddrG10>());
        break;
    default:
        return;
    }
}

void GpuCmdFactoryDecodeAVCLong::CreateGpuCmds(vector<GpuCmdInterface::pcmditf_t> &gpuCmds, Platform_t platform) const
{
    switch(platform)
    {
    case igfxSKLAKE:
        //gpuCmds.push_back(make_shared<GpuCmdMfxPipeBufAddrG9Skl>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxIndObjBaseAddrG9Skl>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxBspBufBaseAddrG9Skl>());
        break;
    case igfxBROXTON:
        //gpuCmds.push_back(make_shared<GpuCmdMfxPipeBufAddrG9Bxt>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxIndObjBaseAddrG9Bxt>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxBspBufBaseAddrG9Bxt>());
        break;
    case igfxBROADWELL:
        break;
    case igfxCANNONLAKE:
        //gpuCmds.push_back(make_shared<GpuCmdMfxPipeBufAddrG10>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxIndObjBaseAddrG10>());
        //gpuCmds.push_back(make_shared<GpuCmdMfxBspBufBaseAddrG10>());
        break;
    default:
        return;
    }
}

GpuCmdFactory *g_gpuCmdFactoryEncodeHevcDualPipe = new GpuCmdFactoryEncodeHevcDualPipe();
GpuCmdFactory *g_gpuCmdFactoryDecodeHEVCLong     = new GpuCmdFactoryDecodeHEVCLong();
GpuCmdFactory *g_gpuCmdFactoryEncodeAvcDualPipe  = new GpuCmdFactoryEncodeAvcDualPipe();
GpuCmdFactory *g_gpuCmdFactoryDecodeAVCLong      = new GpuCmdFactoryDecodeAVCLong();
