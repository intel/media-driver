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
#ifndef __DDI_TEST_DECODE_H__
#define __DDI_TEST_DECODE_H__

#include "driver_loader.h"
#include "gtest/gtest.h"
#include "memory_leak_detector.h"
#include "test_data_caps.h"
#include "test_data_decode.h"

class DecodeTestConfig
{
public:

    DecodeTestConfig();

    bool IsDecTestEnabled(DeviceConfig platform, FeatureID featureId);

private:

    std::map<DeviceConfig, std::vector<FeatureID>, MapFeatureIDComparer> m_mapPlatformFeatureID;
};

class MediaDecodeDdiTest : public testing::Test
{
protected:

    virtual void SetUp() { }

    virtual void TearDown() { }

    void DecodeExecute(DecTestData *pDecData, Platform_t platform);

    void ExectueDecodeTest(DecTestData *pDecData);

protected:

    DriverDllLoader    m_driverLoader;
    DecTestDataFactory m_decDataFactory;
    DecodeTestConfig   m_decTestCfg;
};

#endif // __DDI_TEST_DECODE_H__
