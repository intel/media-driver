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
#ifndef __TEST_DATA_ENCODE_H__
#define __TEST_DATA_ENCODE_H__

#include <map>
#include <memory>
#include <string>
#include <string.h>
#include "driver_loader.h"

#define ENC_FRAME_NUM 3

const FeatureID TEST_Intel_Encode_HEVC  = { VAProfileHEVCMain    , VAEntrypointEncSlice  , };
const FeatureID TEST_Intel_Encode_AVC   = { VAProfileH264Main    , VAEntrypointEncSlice  , };
const FeatureID TEST_Intel_Encode_MPEG2 = { VAProfileMPEG2Main   , VAEntrypointEncSlice  , };
const FeatureID TEST_Intel_Encode_JPEG  = { VAProfileJPEGBaseline, VAEntrypointEncPicture, };

class HevcEncBufs
{
public:

    HevcEncBufs();

    uint32_t GetSpsSize() { return sizeof(VAEncSequenceParameterBufferHEVC); }

    uint32_t GetPpsSize() { return sizeof(VAEncPictureParameterBufferHEVC); }

    uint32_t GetSlcSize() { return sizeof(VAEncSliceParameterBufferHEVC); }

    VAEncSequenceParameterBufferHEVC *GetSpsBuf() { return m_sps.get(); }

    VAEncPictureParameterBufferHEVC *GetPpsBuf() { return m_pps.get(); }

    VAEncSliceParameterBufferHEVC *GetSlcBuf() { return m_slc.get(); }

private:

    std::shared_ptr<VAEncSequenceParameterBufferHEVC> m_sps;
    std::shared_ptr<VAEncPictureParameterBufferHEVC>  m_pps;
    std::shared_ptr<VAEncSliceParameterBufferHEVC>    m_slc;
};

class AvcEncBufs
{
public:

    AvcEncBufs();

    uint32_t GetSpsSize() { return sizeof(VAEncSequenceParameterBufferH264); }

    uint32_t GetPpsSize() { return sizeof(VAEncPictureParameterBufferH264); }

    uint32_t GetSlcSize() { return sizeof(VAEncSliceParameterBufferH264); }

    VAEncSequenceParameterBufferH264 *GetSpsBuf() { return m_sps.get(); }

    VAEncPictureParameterBufferH264 *GetPpsBuf() { return m_pps.get(); }

    VAEncSliceParameterBufferH264 *GetSlcBuf() { return m_slc.get(); }

private:

    std::shared_ptr<VAEncSequenceParameterBufferH264> m_sps;
    std::shared_ptr<VAEncPictureParameterBufferH264>  m_pps;
    std::shared_ptr<VAEncSliceParameterBufferH264>    m_slc;
};

class EncTestData
{
public:

    virtual ~EncTestData() { }

    FeatureID GetFeatureID() { return m_featureId; }

    uint32_t GetWidth() { return m_picWidth; }

    uint32_t GetHeight() { return m_picHeight; }

    std::vector<std::vector<CompBufConif>> &GetCompBuffers() { return m_compBufs; }

    std::vector<VASurfaceID> &GetResources() { return m_resources; }

    std::vector<VAConfigAttrib> &GetConfAttrib() { return m_confAttrib; }

    std::vector<VASurfaceAttrib> &GetSurfAttrib() { return m_surfAttrib; }

    virtual void UpdateCompBuffers(int frameId) { }

public:

    int                                    m_num_frames;

protected:

    FeatureID                              m_featureId;
    uint32_t                               m_picWidth;
    uint32_t                               m_picHeight;
    uint32_t                               m_surfacesNum;
    std::vector<std::vector<CompBufConif>> m_compBufs;
    std::vector<VASurfaceID>               m_resources;
    std::vector<VAConfigAttrib>            m_confAttrib;
    std::vector<VASurfaceAttrib>           m_surfAttrib;
};

class EncTestDataHEVC : public EncTestData
{
public:

    EncTestDataHEVC(FeatureID testFeatureID);

    void UpdateCompBuffers(int frameId) override;

    struct EncFrameDataHEVC
    {
        std::vector<uint8_t> spsData;
        std::vector<uint8_t> ppsData;
        std::vector<uint8_t> sliceData;
    };

public:

    const std::vector<uint8_t> m_headerData = { 0x00, 0x00, 0x00, 0x01 };

protected:

    void InitCompBuffers();

protected:

    std::vector<EncFrameDataHEVC>    m_frameArray;
    VAEncPackedHeaderParameterBuffer m_packedsps;
    VAEncPackedHeaderParameterBuffer m_packedpps;
    VAEncPackedHeaderParameterBuffer m_packedsh;
    std::shared_ptr<HevcEncBufs>     m_pBufs = nullptr;
};

class EncTestDataAVC: public EncTestData
{
public:

    EncTestDataAVC(FeatureID testFeatureID);

    void UpdateCompBuffers(int frameId) override;

    struct EncFrameDataAVC
    {
        std::vector<uint8_t> spsData;
        std::vector<uint8_t> ppsData;
        std::vector<uint8_t> sliceData;
    };

public:

    const std::vector<uint8_t> m_headerData = { 0x00, 0x00, 0x00, 0x01 };

protected:

    void InitCompBuffers();

protected:

    std::vector<EncFrameDataAVC>     m_frameArray;
    VAEncPackedHeaderParameterBuffer m_packedsps;
    VAEncPackedHeaderParameterBuffer m_packedpps;
    VAEncPackedHeaderParameterBuffer m_packedsh;
    std::shared_ptr<AvcEncBufs>      m_pBufs = nullptr;
};

class EncTestDataFactory
{
public:

    static EncTestData *GetEncTestData(const std::string &description)
    {
        if (description == "HEVC-DualPipe")
        {
            return new EncTestDataHEVC(TEST_Intel_Encode_HEVC);
        }
        if (description == "AVC-DualPipe")
        {
            return new EncTestDataAVC(TEST_Intel_Encode_AVC);
        }

        return nullptr;
    }
};

#endif // __TEST_DATA_ENCODE_H__
