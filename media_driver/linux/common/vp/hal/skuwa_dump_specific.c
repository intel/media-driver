/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     skuwa_dump_specific.c
//! 
//!

#if (_DEBUG || _RELEASE_INTERNAL) && (!WDDM_LINUX)
#include <stdio.h>
#include "vphal.h"
#include "mhw_vebox.h"
#include "mos_os.h"
#include "vphal_debug.h"
#include "ctype.h"   

MOS_STATUS VphalParameterDumper::SkuWa_DumpToXML(
    MEDIA_FEATURE_TABLE             *skuTable,
    MEDIA_WA_TABLE                  *waTable)
{
    char                            sPath[MAX_PATH] = {0}, sOsPath[MAX_PATH] = {0};
    MOS_STATUS                      eStatus;
    char*                           pcOutContents;
    std::string                     SkuWatableFileName = "skuwatable_dump";
    VPHAL_DBG_PARAMS_DUMP_SPEC*     pParamsDumpSpec    = &m_dumpSpec;
    bool                            enableSkuWaDump    = false;

    eStatus         = MOS_STATUS_SUCCESS;
    pcOutContents   = nullptr;
    enableSkuWaDump = m_dumpSpec.enableSkuWaDump;

    std::map<std::string, uint8_t>::iterator iter;
    MediaFeatureTable::MediaMap *            mediaSkuMap = skuTable->GetMediaSku();
    MediaWaTable::MediaMap *                 mediaWaMap  = waTable->GetMediaWa();
    VPHAL_DEBUG_CHK_NULL(pParamsDumpSpec);

    if (!enableSkuWaDump)
        goto finish;

    VPHAL_DEBUG_CHK_NULL(skuTable);
    VPHAL_DEBUG_CHK_NULL(waTable);

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(true, &pcOutContents, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\n"));

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<ID>%d</ID>\n", MosUtilities::MosGetPid()));

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<!-- SkuTable infomation -->\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<SkuTable>\n"));

    VPHAL_DEBUG_CHK_NULL(mediaSkuMap);

    for (iter = mediaSkuMap->begin(); iter != mediaSkuMap->end(); iter++)
    {
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<%s> %d </%s>\n", iter->first.c_str(), unsigned(iter->second), iter->first.c_str()));
    }
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t</SkuTable>\n"));

    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<!-- WaTable information -->\n"));
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t<WaTable>\n"));

    VPHAL_DEBUG_CHK_NULL(mediaWaMap);

    for (iter = mediaWaMap->begin(); iter != mediaWaMap->end(); iter++)
    {
        VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t\t<%s> %d </%s>\n", iter->first.c_str(), unsigned(iter->second), iter->first.c_str()));
    }
    VPHAL_DEBUG_CHK_STATUS(VphalDumperTool::AppendString(false, &pcOutContents, "\t</WaTable>\n"));

    MOS_SecureStringPrint(sPath, MAX_PATH, MAX_PATH, "%s%c%s.xml", pParamsDumpSpec->outFileLocation, MOS_DIR_SEPERATOR, SkuWatableFileName.c_str());
    VphalDumperTool::GetOsFilePath(sPath, sOsPath);
    VPHAL_DEBUG_CHK_STATUS(MosUtilities::MosWriteFileFromPtr(sOsPath, pcOutContents, strlen(pcOutContents)));

finish:
    if (pcOutContents)
    {
        MOS_FreeMemory(pcOutContents);
        pcOutContents = nullptr;
    }

    return eStatus;
}

#endif  // (_DEBUG || _RELEASE_INTERNAL)