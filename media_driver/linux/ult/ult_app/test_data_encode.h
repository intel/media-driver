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
#include <memory>

using std::shared_ptr;

#define ENC_FRAME_NUM 3

const FeatureID TEST_Intel_Encode_HEVC  = {VAProfileHEVCMain, VAEntrypointEncSlice};
const FeatureID TEST_Intel_Encode_AVC   = {VAProfileH264Main, VAEntrypointEncSlice};
const FeatureID TEST_Intel_Encode_MPEG2 = {VAProfileMPEG2Main, VAEntrypointEncSlice};
const FeatureID TEST_Intel_Encode_JPEG  = {VAProfileJPEGBaseline, VAEntrypointEncPicture};

class HevcEncBufs
{
public:
    HevcEncBufs();

    uint32_t GetSpsSize() { return sizeof(VAEncSequenceParameterBufferHEVC); };
    uint32_t GetPpsSize() { return sizeof(VAEncPictureParameterBufferHEVC); };
    uint32_t GetSlcSize() { return sizeof(VAEncSliceParameterBufferHEVC); };

    VAEncSequenceParameterBufferHEVC* GetSpsBuf() { return sps.get(); };
    VAEncPictureParameterBufferHEVC* GetPpsBuf() { return pps.get(); }
    VAEncSliceParameterBufferHEVC* GetSlcBuf() { return slc.get(); };

private:
    shared_ptr<VAEncSequenceParameterBufferHEVC> sps;
    shared_ptr<VAEncPictureParameterBufferHEVC> pps;
    shared_ptr<VAEncSliceParameterBufferHEVC> slc;
};

class AvcEncBufs
{
public:
    AvcEncBufs();

    uint32_t GetSpsSize() { return sizeof(VAEncSequenceParameterBufferH264); };
    uint32_t GetPpsSize() { return sizeof(VAEncPictureParameterBufferH264); };
    uint32_t GetSlcSize() { return sizeof(VAEncSliceParameterBufferH264); };

    VAEncSequenceParameterBufferH264* GetSpsBuf() { return sps.get(); };
    VAEncPictureParameterBufferH264* GetPpsBuf() { return pps.get(); };
    VAEncSliceParameterBufferH264* GetSlcBuf() { return slc.get(); };

private:
    shared_ptr<VAEncSequenceParameterBufferH264> sps;
    shared_ptr<VAEncPictureParameterBufferH264> pps;
    shared_ptr<VAEncSliceParameterBufferH264> slc;
};

class EncTestData
{
public:
    EncTestData(){};
    virtual ~EncTestData(){};
    FeatureID GetFeatureID() { return featureId; };
    uint32_t GetWidth() { return picWidth; };
    uint32_t GetHeight() { return picHeight; };
    vector<vector<CompBufConif>> & GetCompBuffers() { return compBufs; };
    vector<VASurfaceID> & GetResources() { return resources; };
    vector <VAConfigAttrib> & GetConfAttrib() { return ConfAttrib; };
    vector <VASurfaceAttrib>& GetSurfAttrib() { return SurfAttrib; };

    virtual void UpdateCompBuffers(int frameId){};
    int num_frames;
protected:
    FeatureID featureId;
    uint32_t picWidth;
    uint32_t picHeight;
    uint32_t SurfacesNum;
    vector<vector<CompBufConif>> compBufs;
    vector<VASurfaceID> resources;
    vector <VAConfigAttrib> ConfAttrib;
    vector <VASurfaceAttrib>SurfAttrib;
};

class EncTestDataHEVC : public EncTestData
{
public:
    EncTestDataHEVC(){};
    EncTestDataHEVC(FeatureID testFeatureID);
    void UpdateCompBuffers(int frameId) override;

    struct EncFrameDataHEVC
    {
        vector<uint8_t> spsData;
        vector<uint8_t> ppsData;
        vector<uint8_t> sliceData;
    };
    const vector<uint8_t> headerData={0x00,0x00,0x00,0x01};
protected:
    void InitCompBuffers();
    vector<EncFrameDataHEVC> frameArray;
    VAEncPackedHeaderParameterBuffer packedsps;
    VAEncPackedHeaderParameterBuffer packedpps;
    VAEncPackedHeaderParameterBuffer packedsh;
    shared_ptr<HevcEncBufs> pBufs = nullptr;
};
class EncTestDataAVC: public EncTestDataHEVC
{
public:
    //EncTestDataAVC(FeatureID testFeatureID):EncTestDataHEVC(testFeatureID){};
    EncTestDataAVC(){};
    EncTestDataAVC(FeatureID testFeatureID);
    void UpdateCompBuffers(int frameId) override;

protected:
    void InitCompBuffers();
    shared_ptr<AvcEncBufs> pBufs = nullptr;
};

class EncTestDataFactory
{
public:
    static EncTestData *GetEncTestData(const std::string &description)
    {
        if (description == "HEVC-DualPipe")
            return new EncTestDataHEVC(TEST_Intel_Encode_HEVC);
        if (description == "AVC-DualPipe")
            return new EncTestDataAVC(TEST_Intel_Encode_AVC);
        return NULL;
    }
};

