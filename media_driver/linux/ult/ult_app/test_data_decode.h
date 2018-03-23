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
#ifndef __TEST_DATA_DECODE_H__
#define __TEST_DATA_DECODE_H__

#include <memory>
#include <map>
#include <string>
#include <string.h>
#include "driver_loader.h"

#define DEC_FRAME_NUM 3

const FeatureID TEST_Intel_Decode_HEVC = { VAProfileHEVCMain, VAEntrypointVLD, };
const FeatureID TEST_Intel_Decode_AVC  = { VAProfileH264Main, VAEntrypointVLD, };

class DecBufHEVC
{
public:

    DecBufHEVC();

    uint32_t GetPpsSize() { return sizeof(VAPictureParameterBufferHEVC); }

    uint32_t GetSlcSize() { return sizeof(VASliceParameterBufferHEVC); }

    size_t GetBsSize() { return m_bitstream.size(); }

    VAPictureParameterBufferHEVC *GetPpsBuf() { return m_pps.get(); }

    VASliceParameterBufferHEVC *GetSlcBuf() { return m_slc.get(); }

    uint8_t *GetBsBuf() { return &m_bitstream[0]; }

private:

    std::shared_ptr<VAPictureParameterBufferHEVC> m_pps;
    std::shared_ptr<VASliceParameterBufferHEVC>   m_slc;
    std::vector<uint8_t>                          m_bitstream;
};

class DecBufAVC
{
public:

    DecBufAVC();

    uint32_t GetPpsSize() { return sizeof(VAPictureParameterBufferH264); }

    uint32_t GetSlcSize() { return sizeof(VASliceParameterBufferH264); }

    size_t GetBsSize() { return m_bitstream.size(); }

    VAPictureParameterBufferH264 *GetPpsBuf() { return m_pps.get(); }

    VASliceParameterBufferH264 *GetSlcBuf() { return m_slc.get(); }

    uint8_t *GetBsBuf() { return &m_bitstream[0]; }

private:

    std::shared_ptr<VAPictureParameterBufferH264> m_pps;
    std::shared_ptr<VASliceParameterBufferH264>   m_slc;
    std::vector<uint8_t>                          m_bitstream;
};

class DecTestData
{
public:

    virtual ~DecTestData() { }

    FeatureID GetFeatureID() { return m_featureId; }

    uint32_t GetWidth() { return m_picWidth; }

    uint32_t GetHeight() { return m_picHeight; }

    std::vector<std::vector<CompBufConif>> &GetCompBuffers() { return m_compBufs; }

    std::vector<VASurfaceID> &GetResources() { return m_resources; }

    std::vector<VAConfigAttrib> &GetConfAttrib() {return m_confAttrib; }

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
};

class DecTestDataHEVC : public DecTestData
{
public:

    DecTestDataHEVC(FeatureID testFeatureID, bool bInShortFormat);

    void UpdateCompBuffers(int frameId) override;

    struct DecFrameDataHEVC
    {
        std::vector<uint8_t> picParam;
        std::vector<uint8_t> slcParam;
        std::vector<uint8_t> bsData;
    };

protected:

    void InitCompBuffers();

protected:

    bool                          m_bShortFormat;
    std::vector<DecFrameDataHEVC> m_frameArray;
    std::vector<DecFrameDataHEVC> m_frameArrayLong;
    std::shared_ptr<DecBufHEVC>   m_pBufs = nullptr;
};

class DecTestDataHEVCLong : public DecTestDataHEVC
{
public:

    DecTestDataHEVCLong(FeatureID testFeatureID) : DecTestDataHEVC(testFeatureID, false) { }
};

class DecTestDataHEVCShort:public DecTestDataHEVC
{
public:

    DecTestDataHEVCShort(FeatureID testFeatureID) : DecTestDataHEVC(testFeatureID, true) { }
};

class DecTestDataAVC : public DecTestData
{
public:

    DecTestDataAVC(FeatureID testFeatureID, bool bInShortFormat);

    void UpdateCompBuffers(int frameId) override;

    struct DecFrameDataAVC
    {
        std::vector<uint8_t> picParam;
        std::vector<uint8_t> slcParam;
        std::vector<uint8_t> bsData;
    };

protected:

    void                         InitCompBuffers();

protected:

    bool                         m_bShortFormat;
    std::vector<DecFrameDataAVC> m_frameArray;
    std::vector<DecFrameDataAVC> m_frameArrayLong;
    std::shared_ptr<DecBufAVC>   m_pBufs = nullptr;
};

class DecTestDataAVCLong : public DecTestDataAVC
{
public:

    DecTestDataAVCLong(FeatureID testFeatureID) : DecTestDataAVC(testFeatureID, false) { }

protected:

    void InitCompBuffers();
};

class DecTestDataAVCShort : public DecTestDataAVC
{
public:
 
    DecTestDataAVCShort(FeatureID testFeatureID) : DecTestDataAVC(testFeatureID, true) { }

protected:

    void InitCompBuffers() { }
};

class DecTestDataFactory
{
public:

    static DecTestData *GetDecTestData(const std::string &description)
    {
        if (description == "HEVC-Long")
        {
            return new DecTestDataHEVCLong(TEST_Intel_Decode_HEVC);
        }
        if (description == "AVC-Long")
        {
            return new DecTestDataAVCLong(TEST_Intel_Decode_AVC);
        }

        return nullptr;
    }
};

#endif // __TEST_DATA_DECODE_H__
