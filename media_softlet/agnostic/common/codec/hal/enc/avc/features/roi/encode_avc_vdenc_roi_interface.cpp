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
//!
//! \file     encode_avc_vdenc_roi_interface.cpp
//! \brief    implementation of basic ROI features of AVC VDENC

#include "encode_avc_vdenc_roi_interface.h"

#include "mos_defs.h"
#include "media_avc_feature_defs.h"
#include "encode_avc_vdenc_feature_manager.h"

namespace encode
{

AvcVdencRoiInterface::AvcVdencRoiInterface(
    MediaFeatureManager *featureManager,
    EncodeAllocator *allocator,
    CodechalHwInterfaceNext *hwInterface,
    void *constSettings,
    SupportedModes &supportedModes) :
    MediaFeature(constSettings),
    m_allocator(allocator),
    m_hwInterface(hwInterface),
    m_supportedModes(supportedModes)
{
    m_featureManager = featureManager;
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_featureManager);

    m_basicFeature = dynamic_cast<AvcBasicFeature*>(m_featureManager->GetFeature(AvcFeatureIDs::basicFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_basicFeature);

    m_brcFeature = dynamic_cast<AvcEncodeBRC*>(m_featureManager->GetFeature(AvcFeatureIDs::avcBrcFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_brcFeature);

    m_vdencStreamInFeature = dynamic_cast<AvcVdencStreamInFeature*>(m_featureManager->GetFeature(AvcFeatureIDs::avcVdencStreamInFeature));
    ENCODE_CHK_NULL_NO_STATUS_RETURN(m_vdencStreamInFeature);
}

MOS_STATUS AvcVdencRoiInterface::Init(void *setting)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(setting);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencRoiInterface::Update(void *params)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_NULL_RETURN(params);
    m_picParam = m_basicFeature->m_picParam;

    if (m_basicFeature->m_mbQpDataEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(IsModesSupported(m_supportedModes.MBQP_ForceQP | m_supportedModes.MBQP_DeltaQP, "MBQP"));
        if (m_picParam->NumROI || m_picParam->NumDirtyROI)
        {
            ENCODE_ASSERTMESSAGE("MBQP feature is not compatible with ROI/DirtyROI\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        ENCODE_CHK_STATUS_RETURN(SetupMBQP());
    }
    else if (m_basicFeature->m_brcAdaptiveRegionBoostEnabled)
    {
        ENCODE_CHK_STATUS_RETURN(IsModesSupported(m_supportedModes.ArbROI, "ArbROI"));
        if (m_picParam->NumROI || m_picParam->NumDirtyROI)
        {
            ENCODE_ASSERTMESSAGE("ROI/DirtyROI is not supported with ArbROI\n");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        ENCODE_CHK_STATUS_RETURN(SetupArbROI());
    }
    else if (m_picParam->NumROI)
    {
        ENCODE_CHK_STATUS_RETURN(IsModesSupported(m_supportedModes.ROI_Native | m_supportedModes.ROI_NonNative, "ROI"));

        m_picParam->bNativeROI = ProcessRoiDeltaQp();
        ENCODE_CHK_STATUS_RETURN(SetupROI());
    }
    else if (m_picParam->NumDirtyROI)
    {
        ENCODE_CHK_STATUS_RETURN(IsModesSupported(m_supportedModes.DirtyROI, "DirtyROI"));
        ENCODE_CHK_STATUS_RETURN(SetupDirtyROI());
    }

    if (m_picParam->ForceSkip.Enable && (m_basicFeature->m_pictureCodingType != I_TYPE))
    {
        m_picParam->ForceSkip.Enable = 1;
        ENCODE_CHK_STATUS_RETURN(SetupForceSkip());
    }
    else
        m_picParam->ForceSkip.Enable = 0;

    return MOS_STATUS_SUCCESS;
}

bool AvcVdencRoiInterface::ProcessRoiDeltaQp()
{
    ENCODE_FUNC_CALL();

    // Intialize ROIDistinctDeltaQp to be min expected delta qp, setting to -128
    // Check if forceQp is needed or not
    // forceQp is enabled if there are greater than 3 distinct delta qps or if the deltaqp is beyond range (-8, 7)
    for (auto k = 0; k < m_maxNumRoi; k++)
    {
        m_picParam->ROIDistinctDeltaQp[k] = -128;
    }

    int32_t numQp = 0;
    for (int32_t i = 0; i < m_picParam->NumROI; i++)
    {
        bool dqpNew = true;

        //Get distinct delta Qps among all ROI regions, index 0 having the lowest delta qp
        int32_t k = numQp - 1;
        for (; k >= 0; k--)
        {
            if (m_picParam->ROI[i].PriorityLevelOrDQp == m_picParam->ROIDistinctDeltaQp[k] ||
                m_picParam->ROI[i].PriorityLevelOrDQp == 0)
            {
                dqpNew = false;
                break;
            }
            else if (m_picParam->ROI[i].PriorityLevelOrDQp < m_picParam->ROIDistinctDeltaQp[k])
            {
                continue;
            }
            else
            {
                break;
            }
        }

        if (dqpNew)
        {
            for (int32_t j = numQp - 1; (j >= k + 1 && j >= 0); j--)
            {
                m_picParam->ROIDistinctDeltaQp[j + 1] = m_picParam->ROIDistinctDeltaQp[j];
            }
            m_picParam->ROIDistinctDeltaQp[k + 1] = m_picParam->ROI[i].PriorityLevelOrDQp;
            numQp++;
        }
    }

    //Set the ROI DeltaQp to zero for remaining array elements
    for (auto k = numQp; k < m_maxNumRoi; k++)
    {
        m_picParam->ROIDistinctDeltaQp[k] = 0;
    }

    m_picParam->NumROIDistinctDeltaQp = (int8_t)numQp;
    
    int32_t lastROIIdx = MOS_CLAMP_MIN_MAX(numQp - 1, 0, m_maxNumRoi - 1);

#if _MEDIA_RESERVED
    return (!(numQp > m_maxNumNativeRoi || m_picParam->ROIDistinctDeltaQp[0] < -8 || m_picParam->ROIDistinctDeltaQp[lastROIIdx] > 7));
#else
    bool bIsNativeROI = (!(numQp > m_maxNumNativeRoi || m_picParam->ROIDistinctDeltaQp[0] < -8 || m_picParam->ROIDistinctDeltaQp[lastROIIdx] > 7));
    bool bIsNativeROIAllowed = !m_brcFeature->IsVdencBrcEnabled() || m_brcFeature->IsMbBrcEnabled(); // BRC native ROI require MBBRC on

    // return whether is native ROI or not
    return bIsNativeROI && bIsNativeROIAllowed;
#endif  // _MEDIA_RESERVED
}

MOS_STATUS AvcVdencRoiInterface::SetupROI()
{
    ENCODE_FUNC_CALL();
    ENCODE_ASSERTMESSAGE("ROI feature is not supported");
    return MOS_STATUS_INVALID_PARAMETER;
}

MOS_STATUS AvcVdencRoiInterface::SetupDirtyROI()
{
    ENCODE_FUNC_CALL();
    ENCODE_ASSERTMESSAGE("DirtyROI feature is not supported");
    return MOS_STATUS_INVALID_PARAMETER;
}

MOS_STATUS AvcVdencRoiInterface::SetupMBQP()
{
    ENCODE_FUNC_CALL();
    ENCODE_ASSERTMESSAGE("MBQP feature is not supported");
    return MOS_STATUS_INVALID_PARAMETER;
}

MOS_STATUS AvcVdencRoiInterface::SetupForceSkip()
{
    ENCODE_FUNC_CALL();
    ENCODE_ASSERTMESSAGE("ForceSkip feature is not supported");
    return MOS_STATUS_INVALID_PARAMETER;
}

MOS_STATUS AvcVdencRoiInterface::SetupArbROI()
{
    ENCODE_FUNC_CALL();
    ENCODE_ASSERTMESSAGE("ARB feature is not supported");
    return MOS_STATUS_INVALID_PARAMETER;
}

MOS_STATUS AvcVdencRoiInterface::IsModesSupported(uint32_t modes, std::string featureName)
{
    if (modes == 0)
    {
        ENCODE_ASSERTMESSAGE("%s feature is not supported", featureName.c_str());
        return MOS_STATUS_INVALID_PARAMETER;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AvcVdencRoiInterface::GetDeltaQPIndex(uint32_t maxNumRoi, int32_t dqp, int32_t& dqpIdx)
{
    ENCODE_FUNC_CALL();

    dqpIdx = -1;
    for (uint32_t j = 0; j < maxNumRoi; j++)
    {
        if (m_picParam->ROIDistinctDeltaQp[j] == dqp)
        {
            dqpIdx = j;
            break;
        }
    }
    if (dqpIdx == -1)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}
}
