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
#pragma  once

#include "driver_loader.h"
#include<string>
#include<string.h>
#include <map>
#include <memory>

using std::shared_ptr;

#define DEC_FRAME_NUM 3

const FeatureID TEST_Intel_Decode_HEVC = {VAProfileHEVCMain, VAEntrypointVLD};
const FeatureID TEST_Intel_Decode_AVC  = {VAProfileH264Main, VAEntrypointVLD};

class DecBufHEVC
{
public:
    DecBufHEVC();

    uint32_t GetPpsSize() { return sizeof(VAPictureParameterBufferHEVC); };
    uint32_t GetSlcSize() { return sizeof(VASliceParameterBufferHEVC); };
    size_t GetBsSize() { return bitstream.size(); };

    VAPictureParameterBufferHEVC* GetPpsBuf() { return pps.get(); };
    VASliceParameterBufferHEVC* GetSlcBuf() { return slc.get(); };
    uint8_t* GetBsBuf() { return &bitstream[0]; };

private:
    shared_ptr<VAPictureParameterBufferHEVC> pps;
    shared_ptr<VASliceParameterBufferHEVC> slc;
    vector<uint8_t> bitstream;
};

class DecBufAVC
{
public:
    DecBufAVC();

    uint32_t GetPpsSize() { return sizeof(VAPictureParameterBufferH264); };
    uint32_t GetSlcSize() { return sizeof(VASliceParameterBufferH264); };
    size_t GetBsSize() { return bitstream.size(); };

    VAPictureParameterBufferH264* GetPpsBuf() { return pps.get(); };
    VASliceParameterBufferH264* GetSlcBuf() { return slc.get(); };
    uint8_t* GetBsBuf() { return &bitstream[0]; };

private:
    shared_ptr<VAPictureParameterBufferH264> pps;
    shared_ptr<VASliceParameterBufferH264> slc;
    vector<uint8_t> bitstream;
};

class DecTestData
{
public:
    DecTestData() {};
    virtual ~DecTestData() {};
    FeatureID GetFeatureID() { return featureId; };
    uint32_t GetWidth() { return picWidth; };
    uint32_t GetHeight() { return picHeight; };
    vector<vector<CompBufConif>> & GetCompBuffers() { return compBufs; };
    vector<VASurfaceID> & GetResources() { return resources; };
    vector <VAConfigAttrib> & GetConfAttrib() {return ConfAttrib; };
    virtual void UpdateCompBuffers(int frameId) { };
    int num_frames;
protected:
    FeatureID featureId;
    uint32_t picWidth;
    uint32_t picHeight;
    uint32_t SurfacesNum;
    vector<vector<CompBufConif>> compBufs;
    vector<VASurfaceID> resources;
    vector <VAConfigAttrib> ConfAttrib;
};

class DecTestDataHEVC : public DecTestData
{
public:
    //DecTestDataHEVC(FeatureID testFeatureID);
    DecTestDataHEVC(FeatureID testFeatureID, bool bInShortFormat);
    void UpdateCompBuffers(int frameId) override;
    struct DecFrameDataHEVC
    {
        vector<uint8_t> picParam;
        vector<uint8_t> slcParam;
        vector<uint8_t> bsData;
    };

protected:
    void InitCompBuffers();
    bool bShortFormat;
    vector<DecFrameDataHEVC> frameArray;
    vector<DecFrameDataHEVC> frameArrayLong;
    shared_ptr<DecBufHEVC> pBufs = nullptr;
};
class DecTestDataHEVCLong : public DecTestDataHEVC{
    public:
    DecTestDataHEVCLong(FeatureID testFeatureID):DecTestDataHEVC(testFeatureID, false){};
};
class DecTestDataHEVCShort:public DecTestDataHEVC{
public:
    DecTestDataHEVCShort(FeatureID testFeatureID):DecTestDataHEVC(testFeatureID, true){};
};

class DecTestDataAVC : public DecTestData
{
public:
    //DecTestDataAVC(FeatureID testFeatureID);
    DecTestDataAVC(FeatureID testFeatureID, bool bInShortFormat);
    void UpdateCompBuffers(int frameId) override;
    struct DecFrameDataAVC
    {
        vector<uint8_t> picParam;
        vector<uint8_t> slcParam;
        vector<uint8_t> bsData;
    };

protected:
    void InitCompBuffers();
    bool bShortFormat;
    vector<DecFrameDataAVC> frameArray;
    vector<DecFrameDataAVC> frameArrayLong;
    shared_ptr<DecBufAVC> pBufs = nullptr;
};

class DecTestDataAVCLong:public DecTestDataAVC{
public:
    DecTestDataAVCLong(FeatureID testFeatureID):DecTestDataAVC(testFeatureID, false){};
protected:
    void InitCompBuffers();
};
class DecTestDataAVCShort:public DecTestDataAVC{
public:
    DecTestDataAVCShort(FeatureID testFeatureID):DecTestDataAVC(testFeatureID, true){};
protected:
    void InitCompBuffers(){};
};

class DecTestDataFactory
{
public:
    static DecTestData *GetDecTestData(const std::string &description)
    {
        if (description == "HEVC-Long")
            return new DecTestDataHEVCLong(TEST_Intel_Decode_HEVC);
        if (description == "AVC-Long")
            return new DecTestDataAVCLong(TEST_Intel_Decode_AVC);
        return NULL;
    }
};

