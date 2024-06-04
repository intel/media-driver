/*
* Copyright (c) 2020, Intel Corporation
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
#ifndef __VP_PACKET_SHARED_CONTEXT_H__
#define __VP_PACKET_SHARED_CONTEXT_H__

namespace vp
{
struct VP_PACKET_SHARED_CONTEXT
{
    bool isVeboxFirstFrame = true;

    struct
    {
        bool     bTgneFirstFrame            = true;
        bool     bTgneEnable                = false;
        bool     previousFallback           = false;
        uint32_t tgneFrame                  = 0;
        uint32_t lumaStadTh                 = 3200;
        uint32_t chromaStadTh               = 1600;
        uint32_t dw4X4TGNEThCnt             = 576;
        uint32_t dwBSDThreshold             = 480;
        uint32_t dwHistoryInit              = 32;
        uint32_t globalNoiseLevel_Temporal  = 0;  //!< Global Temporal Noise Level for Y
        uint32_t globalNoiseLevelU_Temporal = 0;  //!< Global Temporal Noise Level for U
        uint32_t globalNoiseLevelV_Temporal = 0;  //!< Global Temporal Noise Level for V
        uint32_t globalNoiseLevel_Spatial   = 0;  //!< Global Spatial Noise Level for Y
        uint32_t globalNoiseLevelU_Spatial  = 0;  //!< Global Spatial Noise Level for U
        uint32_t globalNoiseLevelV_Spatial  = 0;  //!< Global Spatial Noise Level for V
    } tgneParams;

    struct
    {
        uint16_t         QP                  = 0;
        uint16_t         Strength            = 0;
        VPHAL_HVSDN_MODE Mode                = HVSDENOISE_AUTO_BDRATE;
        uint16_t         Format              = 0;
        uint16_t         TgneEnable          = 0;
        uint16_t         FirstFrame          = 0;
        uint16_t         TgneFirstFrame      = 0;
        uint16_t         Fallback            = 0;
        uint16_t         EnableChroma        = 0;
        uint16_t         EnableTemporalGNE   = 0;
        uint16_t         Width               = 0;
        uint16_t         Height              = 0;
        uint32_t         Sgne_Level          = 0;
        uint32_t         Sgne_LevelU         = 0;
        uint32_t         Sgne_LevelV         = 0;
        uint32_t         Sgne_Count          = 0;
        uint32_t         Sgne_CountU         = 0;
        uint32_t         Sgne_CountV         = 0;
        int32_t          PrevNslvTemporal    = 0;
        int32_t          PrevNslvTemporalU   = 0;
        int32_t          PrevNslvTemporalV   = 0;
        uint32_t         dwGlobalNoiseLevel  = 0;  //!< Global Noise Level for Y
        uint32_t         dwGlobalNoiseLevelU = 0;  //!< Global Noise Level for U
        uint32_t         dwGlobalNoiseLevelV = 0;  //!< Global Noise Level for V
        bool             hVSAutoBdrateEnable     = false;
        bool             hVSAutoSubjectiveEnable = false;
        bool             hVSfallback             = false;
    } hvsParams;
    virtual ~VP_PACKET_SHARED_CONTEXT(){};
};
};

#endif // !__VP_PACKET_SHARED_CONTEXT_H__