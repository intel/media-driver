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
//! \file     media_capstable_specific.cpp
//! \brief    implemantation of media caps table class on specific os
//!

#include "media_capstable_specific.h"
#include "media_libva_common.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "hwinfo_linux.h"
#include "linux_system_info.h"
#include "media_libva_caps_factory.h"
#include "caps_register_specific.h"

MediaCapsTableSpecific::MediaCapsTableSpecific(HwDeviceInfo &deviceInfo)
{
    m_plt.ipVersion = deviceInfo.ipVersion;
    m_plt.usRevId   = deviceInfo.usRevId;

    MediaCapsTable::Iterator capsIter;
    if(GetCapsTablePlatform(m_plt, capsIter))
    {
        m_profileMap  = capsIter->second.profileMap;
        m_imgTbl      = capsIter->second.imgTbl;
    }
    else
    {
        DDI_ASSERTMESSAGE("unknown platform with usRevId=%d, ipVersion=%d\n", (int)m_plt.usRevId, (int)m_plt.ipVersion);
    }
}

VAStatus MediaCapsTableSpecific::Init()
{
    DDI_FUNCTION_ENTER();

    for (auto profileMapIter: *m_profileMap)
    {
        auto profile = profileMapIter.first;
        for(auto entrypointMapIter: *profileMapIter.second)
        {
            auto entrypoint     = entrypointMapIter.first;
            auto entrypointData = entrypointMapIter.second;
            auto attriblist     = entrypointData->attribList;
            auto componentData  = entrypointData->configDataList;
            int32_t numAttribList = attriblist->size();

            for(int i = 0; i < componentData->size(); i++)
            {
                auto configData = componentData->at(i);
                m_configList.emplace_back(profile, entrypoint, attriblist->data(), numAttribList, configData);
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaCapsTableSpecific::QueryConfigProfiles(
    VAProfile *profileList,
    int32_t   *profilesNum)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(profileList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(profilesNum, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    int i = 0;

    if(m_profileMap->size() <= 0)
    {
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    for (auto it = m_profileMap->begin(); it!=m_profileMap->end(); ++it)
    {
        profileList[i++] = (VAProfile)(it->first);
    }
    *profilesNum = m_profileMap->size();
    return VA_STATUS_SUCCESS;
}

EntrypointMap* MediaCapsTableSpecific::QueryConfigEntrypointsMap(
    VAProfile      profile)
{
    DDI_FUNCTION_ENTER();

    int i = 0;

    if(m_profileMap->find(profile) == m_profileMap->end())
    {
        return nullptr;
    }

    return m_profileMap->at(profile);
}

AttribList* MediaCapsTableSpecific::QuerySupportedAttrib(
    VAProfile     profile,
    VAEntrypoint  entrypoint)
{
    DDI_FUNCTION_ENTER();

    if(m_profileMap->find(profile) == m_profileMap->end()                             ||
       m_profileMap->at(profile)->find(entrypoint) == m_profileMap->at(profile)->end())
    {
        return nullptr;
    }

    return m_profileMap->at(profile)->at(entrypoint)->attribList;
}

ConfigList* MediaCapsTableSpecific::GetConfigList()
{
    DDI_FUNCTION_ENTER();

    return &m_configList;
}

ConfigLinux* MediaCapsTableSpecific::QueryConfigItemFromIndex(
    VAConfigID     configId)
{
    DDI_FUNCTION_ENTER();

    if (configId >= m_configList.size())
    {
        return nullptr;
    }

    return &m_configList[configId];
}

VAStatus MediaCapsTableSpecific::CreateConfig(
    VAProfile       profile,
    VAEntrypoint    entrypoint,
    VAConfigAttrib  *attribList,
    int32_t         numAttribs,
    VAConfigID      *configId)
{
    DDI_FUNCTION_ENTER();

    DDI_CHK_NULL(attribList, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    DDI_CHK_NULL(configId,   "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

    bool sameConfig = false;
    for (auto configItem : m_configList)
    {
        // check profile, entrypoint here, check attribute in caps_next
        // check component specific and return configId in component function
        if (configItem.profile    == profile      &&
            configItem.entrypoint == entrypoint)
        {
            sameConfig = true;
            break;
        }
    }

    return sameConfig ? VA_STATUS_SUCCESS : VA_STATUS_ERROR_INVALID_VALUE;
}

VAStatus MediaCapsTableSpecific::DestroyConfig(VAConfigID configId)
{
    DDI_FUNCTION_ENTER();

    if(configId < m_configList.size())
    {
        return VA_STATUS_SUCCESS;
    }

    return VA_STATUS_ERROR_INVALID_CONFIG;
}

ImgTable* MediaCapsTableSpecific::GetImgTable()
{
    DDI_FUNCTION_ENTER();

    return m_imgTbl;
}

uint32_t MediaCapsTableSpecific::GetImageFormatsMaxNum()
{
    DDI_FUNCTION_ENTER();

    return m_imgTbl->size();
}

ProfileSurfaceAttribInfo* MediaCapsTableSpecific::QuerySurfaceAttributesFromConfigId(
    VAConfigID                 configId)
{
    DDI_FUNCTION_ENTER();

    ConfigLinux*  configItem = nullptr;
    configItem = QueryConfigItemFromIndex(configId);
    DDI_CHK_NULL(configItem, "Invalid config id!", nullptr);

    VAProfile    profile = configItem->profile;
    VAEntrypoint entrypoint = configItem->entrypoint;

    uint32_t i = 0;

    if (m_profileMap->find(profile) == m_profileMap->end() ||
        m_profileMap->at(profile)->find(entrypoint) == m_profileMap->at(profile)->end())
    {
        return nullptr;
    }

    return m_profileMap->at(profile)->at(entrypoint)->surfaceAttrib;
}
