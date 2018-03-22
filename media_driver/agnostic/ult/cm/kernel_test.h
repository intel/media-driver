/*
* Copyright (c) 2017, Intel Corporation
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

#include "cm_test.h"

using CMRT_UMD::MockDevice;
static uint8_t COMMOM_ISA_CODE[]
= {0x43, 0x49, 0x53, 0x41, 0x03, 0x06, 0x01, 0x00, 0x06, 0x6b, 0x65, 0x72, 0x6e,
   0x65, 0x6c, 0x2d, 0x00, 0x00, 0x00, 0x59, 0x01, 0x00, 0x00, 0x35, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x05, 0x86, 0x01, 0x00, 0x00, 0x18, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x6b, 0x65, 0x72,
   0x6e, 0x65, 0x6c, 0x00, 0x6e, 0x75, 0x6c, 0x6c, 0x00, 0x74, 0x68, 0x72, 0x65,
   0x61, 0x64, 0x5f, 0x78, 0x00, 0x74, 0x68, 0x72, 0x65, 0x61, 0x64, 0x5f, 0x79,
   0x00, 0x67, 0x72, 0x6f, 0x75, 0x70, 0x5f, 0x69, 0x64, 0x5f, 0x78, 0x00, 0x67,
   0x72, 0x6f, 0x75, 0x70, 0x5f, 0x69, 0x64, 0x5f, 0x79, 0x00, 0x67, 0x72, 0x6f,
   0x75, 0x70, 0x5f, 0x69, 0x64, 0x5f, 0x7a, 0x00, 0x74, 0x73, 0x63, 0x00, 0x72,
   0x30, 0x00, 0x61, 0x72, 0x67, 0x00, 0x72, 0x65, 0x74, 0x76, 0x61, 0x6c, 0x00,
   0x73, 0x70, 0x00, 0x66, 0x70, 0x00, 0x68, 0x77, 0x5f, 0x69, 0x64, 0x00, 0x73,
   0x72, 0x30, 0x00, 0x63, 0x72, 0x30, 0x00, 0x63, 0x65, 0x30, 0x00, 0x64, 0x62,
   0x67, 0x30, 0x00, 0x63, 0x6f, 0x6c, 0x6f, 0x72, 0x00, 0x54, 0x30, 0x00, 0x54,
   0x31, 0x00, 0x54, 0x32, 0x00, 0x54, 0x33, 0x00, 0x54, 0x32, 0x35, 0x32, 0x00,
   0x54, 0x32, 0x35, 0x35, 0x00, 0x53, 0x33, 0x31, 0x00, 0x56, 0x33, 0x32, 0x00,
   0x56, 0x33, 0x33, 0x00, 0x6b, 0x65, 0x72, 0x6e, 0x65, 0x6c, 0x5f, 0x42, 0x42,
   0x5f, 0x30, 0x5f, 0x31, 0x00, 0x41, 0x73, 0x6d, 0x4e, 0x61, 0x6d, 0x65, 0x00,
   0x4e, 0x6f, 0x42, 0x61, 0x72, 0x72, 0x69, 0x65, 0x72, 0x00, 0x54, 0x61, 0x72,
   0x67, 0x65, 0x74, 0x00, 0x64, 0x3a, 0x5c, 0x65, 0x6d, 0x70, 0x74, 0x79, 0x5f,
   0x67, 0x65, 0x6e, 0x78, 0x2e, 0x63, 0x70, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,
   0x02, 0x00, 0x00, 0x00, 0x1a, 0x00, 0x00, 0x00, 0x21, 0x01, 0x00, 0x00, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x27, 0x01, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
   0x00, 0x1c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
   0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x00, 0x04, 0x00, 0x00, 0x21, 0x00,
   0x00, 0x00, 0x24, 0x00, 0x04, 0x00, 0x11, 0x00, 0x00, 0x00, 0x48, 0x01, 0x00,
   0x00, 0x03, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x10, 0x65, 0x6d, 0x70, 0x74, 0x79,
   0x5f, 0x67, 0x65, 0x6e, 0x78, 0x5f, 0x30, 0x2e, 0x61, 0x73, 0x6d, 0x1e, 0x00,
   0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x01, 0x00, 0x30, 0x00, 0x00, 0x51,
   0x20, 0x00, 0x00, 0x00, 0x52, 0x03, 0x00, 0x00, 0x00, 0x34, 0x00, 0x00, 0x00,
   0x01, 0x4d, 0x00, 0x20, 0x07, 0x7f, 0x00, 0x00, 0x31, 0x00, 0x00, 0x07, 0x00,
   0x02, 0x00, 0x20, 0xe0, 0x0f, 0x00, 0x06, 0x10, 0x00, 0x00, 0x82};

class KernelTest: public CmTest
{
public:
    KernelTest(): m_program(nullptr), m_kernel(nullptr) {}

    ~KernelTest() {}

    int32_t LoadDestroyProgram(uint8_t *isa_code,
                               uint32_t size,
                               bool fix_isa,
                               const char *options)
    {
        if (fix_isa)
        {
            FixIsaBinary(isa_code);
        }
        if (nullptr == options)
        {
            options = "nojitter";
        }
        int32_t result = m_mockDevice->LoadProgram(isa_code, size, m_program,
                                                   options);
        if (result != CM_SUCCESS)
        {
            return result;
        }
        return m_mockDevice->DestroyProgram(m_program);
    }//================================================

    int32_t LoadDestroyProgram(uint8_t *isa_code, uint32_t size)
    {
        if (nullptr == isa_code || 0 == size)
        {
            return LoadDestroyProgram(isa_code, size, false, nullptr);
        }
        return LoadDestroyProgram(isa_code, size, true, nullptr);
    }//==========================================================

    int32_t LoadDestroyProgram(uint8_t *isa_code,
                               uint32_t size,
                               const char *options)
    { return LoadDestroyProgram(isa_code, size, true, options); }
    //===========================================================

    int32_t CreateKernel(const char *kernel_name)
    {
        if (nullptr == kernel_name)
        {
            return m_mockDevice->CreateKernel(nullptr, "kernel", m_kernel,
                                             nullptr);
        }
        int32_t result = CreateKernelFromCommonIsa(kernel_name);
        if (CM_SUCCESS == result)
        {
            uint32_t spill_size = 0;
            int32_t temp_result = m_kernel->QuerySpillSize(spill_size);
            EXPECT_EQ(CM_FAILURE, temp_result);
        }
        DestroyKernel();
        return result;
    }//===============

    int32_t SetArgument(uint32_t index,
                        size_t size,
                        const void *value)
    {
        int32_t result = CreateKernelFromCommonIsa("kernel");
        EXPECT_EQ(CM_SUCCESS, result);

        result = m_kernel->SetKernelArg(index, size, value);
        DestroyKernel();
        return result;
    }//===============

private:
    bool FixIsaBinary(uint8_t *isa_code);

    int32_t CreateKernelFromCommonIsa(const char *kernel_name)
    {
        FixIsaBinary(COMMOM_ISA_CODE);
        int32_t result = m_mockDevice->LoadProgram(COMMOM_ISA_CODE,
                                                   sizeof(COMMOM_ISA_CODE),
                                                   m_program, "nojitter");
        EXPECT_EQ(CM_SUCCESS, result);
        return m_mockDevice->CreateKernel(m_program, kernel_name, m_kernel,
                                          nullptr);
    }//============================================

    int32_t DestroyKernel()
    {
        int32_t result = CM_SUCCESS;
        if (nullptr != m_kernel)
        {
            result = m_mockDevice->DestroyKernel(m_kernel);
            EXPECT_EQ(CM_SUCCESS, result);
        }
        if (nullptr != m_program)
        {
            result = m_mockDevice->DestroyProgram(m_program);
            EXPECT_EQ(CM_SUCCESS, result);
        }
        return result;
    }//===============

    CMRT_UMD::CmProgram *m_program;
    CMRT_UMD::CmKernel *m_kernel;
};//=============================

TEST_F(KernelTest, LoadDestroyProgram)
{
    RunEach<int32_t>(CM_INVALID_COMMON_ISA,
            [this]() { return LoadDestroyProgram(nullptr, 0); });

    uint8_t *isa_code = COMMOM_ISA_CODE;
    RunEach<int32_t>(CM_INVALID_COMMON_ISA,
                     [this, isa_code]() { return LoadDestroyProgram(isa_code,
                                                                    0); });

    uint32_t size = sizeof(COMMOM_ISA_CODE);
    RunEach<int32_t>(CM_SUCCESS,
                     [this, isa_code, size]()
                     { return LoadDestroyProgram(isa_code, size - 1); });
    RunEach<int32_t>(CM_SUCCESS,
                     [this, isa_code, size]()
                     { return LoadDestroyProgram(isa_code, size); });

    char options[CM_MAX_OPTION_SIZE_IN_BYTE + 1];
    for (int i = 0; i < CM_MAX_OPTION_SIZE_IN_BYTE; ++i)
    {
        options[i] = '0';
    }
    options[CM_MAX_OPTION_SIZE_IN_BYTE] = 0;
    char *options_ptr = options;  // Uses a pointer instead if an array name.
    auto LoadWithOption
            = [this, isa_code, options_ptr]()
            { return LoadDestroyProgram(isa_code, sizeof(COMMOM_ISA_CODE),
                                          options_ptr); };
    RunEach<int32_t>(CM_INVALID_ARG_VALUE, LoadWithOption);

    return;
}//========

TEST_F(KernelTest, LoadWrongIsa)
{
    const uint32_t CODE_SIZE = sizeof(COMMOM_ISA_CODE);
    uint8_t wrong_isa_code[CODE_SIZE];
    memcpy(wrong_isa_code, COMMOM_ISA_CODE, CODE_SIZE);
    wrong_isa_code[32] = 0xff;
    uint8_t *wrong_isa_code_ptr = wrong_isa_code;

    auto LoadWrongIsa
            = [this, wrong_isa_code_ptr, CODE_SIZE]()
            { return LoadDestroyProgram(wrong_isa_code_ptr, CODE_SIZE, false,
                                        nullptr); };
    RunEach<int32_t>(CM_INVALID_GENX_BINARY, LoadWrongIsa);
    return;
}//========

TEST_F(KernelTest, CreateKernel)
{
    RunEach<int32_t>(CM_FAILURE,
            [this]() { return CreateKernel("wrong_name"); });

    RunEach<int32_t>(CM_NULL_POINTER,
            [this]() { return CreateKernel(nullptr); });
    return;
}//========

TEST_F(KernelTest, SetArgument)
{
    int arg0_value = 10;
    void *arg0_ptr = &arg0_ptr;

    RunEach<int32_t>(CM_SUCCESS,
                     [this, arg0_ptr]() { return SetArgument(0, sizeof(int),
                                                    arg0_ptr); });
    RunEach<int32_t>(CM_INVALID_ARG_INDEX,
                     [this, arg0_ptr]() { return SetArgument(2, sizeof(int),
                                                    arg0_ptr); });
    RunEach<int32_t>(CM_INVALID_ARG_SIZE,
                     [this, arg0_ptr]() { return SetArgument(0, sizeof(int) + 1,
                                                    arg0_ptr); });
    RunEach<int32_t>(CM_INVALID_ARG_SIZE,
                     [this, arg0_ptr]() { return SetArgument(0, sizeof(int) - 1,
                                                    arg0_ptr); });
    RunEach<int32_t>(CM_INVALID_ARG_SIZE,
                     [this, arg0_ptr]()
                     { return SetArgument(0, 0, arg0_ptr); });
    RunEach<int32_t>(CM_INVALID_ARG_VALUE,
                     [this, arg0_ptr]() { return SetArgument(0, sizeof(int),
                                                    nullptr); });
    return;    
}//========
