/*
* Copyright (c) 2014-2021, Intel Corporation
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
//! \file     mhw_mi_linux.cpp
//! \brief    MHW interface for MI and generic flush commands across all engines
//! \details  Impelements the functionalities common across all platforms for MHW_MI
//!

#include "mhw_mi.h"

void MhwMiInterface::GetWatchdogThreshold(PMOS_INTERFACE osInterface)
{
    char* mediaResetThreshold = getenv("INTEL_MEDIA_RESET_TH");
    if(mediaResetThreshold)
    {
        long int threshold = strtol(mediaResetThreshold, nullptr, 0);
        if(threshold > 0 )
        {
            MediaResetParam.watchdogCountThreshold = threshold;
            return;
        }
    }

    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
#if (_DEBUG || _RELEASE_INTERNAL)
    // User feature config of watchdog timer threshold
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_MEDIA_RESET_TH_ID,
        &userFeatureData,
        osInterface->pOsContext);
    if (userFeatureData.u32Data != 0)
    {
        MediaResetParam.watchdogCountThreshold = userFeatureData.u32Data;
    }
#endif
}
