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
#pragma once

#include "driver_loader.h"
#include<string>
#include<string.h>
#include <map>

using std::string;

struct MapFeatureIDComparer
{
    bool operator()(const DeviceConfig &Left, const DeviceConfig &Right) const
    {
        return memcmp(&Left, &Right, sizeof(Right)) < 0;
    }
};

class CapsTestData
{
public:
    CapsTestData();
    ~CapsTestData();
    vector<FeatureID>& GetRefFeatureIDTable(DeviceConfig platform)
    {
        return mapPlatformRefFeatureIDs[platform];
    };
private:
    void InitRefFeatureIDMap();

private:
    map<DeviceConfig, vector<FeatureID>, MapFeatureIDComparer> mapPlatformRefFeatureIDs;
};

