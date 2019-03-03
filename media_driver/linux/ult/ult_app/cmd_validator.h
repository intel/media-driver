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
#ifndef __CMD_VALIDATOR_H__
#define __CMD_VALIDATOR_H__

#include "driver_loader.h"
#include "gpu_cmd_factory.h"

class CmdValidator
{
public:

    using pcmditf_t = GpuCmdInterface::pcmditf_t;

    static CmdValidator *GetInstance();

    static void GpuCmdsValidationInit(const GpuCmdFactory *cmdFactory, Platform_t platform);

    void CreateGpuCmds(const GpuCmdFactory *cmdFactory, Platform_t platform)
    {
        if (cmdFactory)
        {
            cmdFactory->CreateGpuCmds(m_gpuCmds, platform);
        }
    }

    void Reset()
    {
        m_gpuCmds.clear();
    }

    void Validate(const PMOS_COMMAND_BUFFER pCmdBuffer) const;

private:

    static CmdValidator *m_instance;

    std::vector<pcmditf_t> m_gpuCmds;
};

#endif // __CMD_VALIDATOR_H__
