/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_libva_apo_desicion.cpp
//! \brief    libva apo desicion.
//!

#include "media_libva_apo_decision.h"
#include "mos_os_specific.h"
#include "hwinfo_linux.h"
#include "media_libva_util.h"

bool MediaLibvaApoDecision::InitDdiApoState(int32_t devicefd, MediaUserSettingSharedPtr userSettingPtr)
{
    DDI_FUNCTION_ENTER();

    bool apoMosEnabled = SetupApoMosSwitch(devicefd, userSettingPtr);
    bool apoDdiEnabled = SetupApoDdiSwitch(devicefd, userSettingPtr);
    if (!apoMosEnabled || !apoDdiEnabled)
    {
        return false;
    }

    // if media solo supported, use default path
    bool mediaSoloEnabled = SetupMediaSoloSwitch();
    if (mediaSoloEnabled)
    {
        if (!apoDdiEnabled)
        {
            MOS_OS_CRITICALMESSAGE("DDI is not apo path! Pls use APO DDI!");
        }
        if (!apoMosEnabled)
        {
            MOS_OS_CRITICALMESSAGE("MOS is not apo path! Pls use APO MOS!");
        }
        return apoDdiEnabled && apoMosEnabled;
    }

    PRODUCT_FAMILY eProductFamily = IGFX_UNKNOWN;
    HWInfo_GetGfxProductFamily(devicefd, eProductFamily);

    if (eProductFamily < IGFX_METEORLAKE)
    {
        apoDdiEnabled = false;
    }

    return apoDdiEnabled && apoMosEnabled;
}