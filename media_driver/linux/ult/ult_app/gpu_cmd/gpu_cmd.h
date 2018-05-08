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
#ifndef __GPU_CMD_H__
#define __GPU_CMD_H__

#include <memory>
#include <vector>
#include "devconfig.h"
#include "gtest/gtest.h"

class GpuCmdInterface
{
public:

    using pcmditf_t = std::shared_ptr<GpuCmdInterface>;

    virtual int32_t GetOpCode() const = 0;

    virtual void Validate(const void *p) const = 0;

    virtual ~GpuCmdInterface() { }
};

template<typename _CmdType>
class GpuCmd : public GpuCmdInterface
{
public:

    using cmd_t = _CmdType;

    static void ExpectEqWithMask(uint32_t src, uint32_t dst, uint32_t msk = 0xffffffff)
    {
        EXPECT_EQ(src & msk, dst & msk);
    }

    static void ExpectEqOrZeroWithMask(uint32_t src, uint32_t dst, uint32_t msk = 0xffffffff)
    {
        if (dst & msk)
        {
            ExpectEqWithMask(src, dst, msk);
        }
    }

    static void CacheCheck1(uint32_t src, uint32_t dst)
    {
        ExpectEqWithMask(src, dst, 0x7f);
    }

    static void CacheCheck2(uint32_t src, uint32_t dst)
    {
        ExpectEqOrZeroWithMask(src, dst, 0x7f);
    }

    int32_t GetOpCode() const override
    {
        return m_pCmd->DW0.Value;
    }

    void Validate(const void *p) const override
    {
        TEST_COUT << "Validating \"" << typeid(cmd_t).name() << "\"\n";
        
        auto pCmd = static_cast<const cmd_t *>(p);

        ValidateCachePolicy(pCmd);
    }

protected:

    virtual void ValidateCachePolicy(const cmd_t *pCmd) const = 0;

protected:

    std::shared_ptr<cmd_t> m_pCmd = std::make_shared<cmd_t>();
};

#endif // __GPU_CMD_H__
