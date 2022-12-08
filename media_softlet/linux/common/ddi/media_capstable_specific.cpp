/*
* Copyright (c) 2021-2022, Intel Corporation
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
#include "media_libva.h"
#include "hwinfo_linux.h"
#include "linux_system_info.h"
#include "media_libva_caps_factory.h"
#include "ddi_cp_caps_interface.h"

bool operator<(const ComponentInfo &lhs, const ComponentInfo &rhs)
{
    return memcmp(&lhs, &rhs, sizeof(ComponentInfo)) < 0;
}

MediaCapsTableSpecific::~MediaCapsTableSpecific()
{
    if(m_cpCaps)
    {
        MOS_Delete(m_cpCaps);
        m_cpCaps = nullptr;
    }
}

MediaCapsTableSpecific::MediaCapsTableSpecific(HwDeviceInfo &deviceInfo)
{
    m_plt.ipVersion = deviceInfo.ipVersion;
    m_plt.usRevId   = deviceInfo.usRevId;

    MediaCapsTable::Iterator capsIter;
    if(GetCapsTablePlatform(m_plt, capsIter))
    {
        m_profileMap  = const_cast<ProfileMap *>(capsIter->second.profileMap);
        m_imgTbl      = const_cast<ImgTable *>(capsIter->second.imgTbl);
    }
    else
    {
        DDI_ASSERTMESSAGE("unknown platform with usRevId=%d, ipVersion=%d\n", (int)m_plt.usRevId, (int)m_plt.ipVersion);
    }
}

VAStatus MediaCapsTableSpecific::Init(DDI_MEDIA_CONTEXT *mediaCtx)
{
    DDI_FUNC_ENTER;

    if (m_profileMap == nullptr)
    {
        DDI_ASSERTMESSAGE("Invalid parameter for profileMap");
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    m_cpCaps = CreateDdiCpCaps();
    DDI_CHK_NULL(m_cpCaps, "CP caps fail with null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);
    
    if (m_cpCaps->InitProfileMap(mediaCtx, m_profileMap) != VA_STATUS_SUCCESS)
    {
        DDI_ASSERTMESSAGE("Init CP caps failed");
        MOS_Delete(m_cpCaps);
        m_cpCaps = nullptr;
        return VA_STATUS_ERROR_INVALID_PARAMETER;
    }

    for (auto profileMapIter: *m_profileMap)
    {
        auto profile = profileMapIter.first;
        for(auto entrypointMapIter: *profileMapIter.second)
        {
            auto entrypoint     = entrypointMapIter.first;
            auto entrypointData = entrypointMapIter.second;
            DDI_CHK_NULL(entrypointData, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

            auto attriblist     = entrypointData->attribList;
            DDI_CHK_NULL(attriblist, "Null pointer", VA_STATUS_ERROR_INVALID_PARAMETER);

            auto componentData  = entrypointData->configDataList;
            int32_t numAttribList = attriblist->size();
            if(componentData && componentData->size() != 0)
            {
                for(int i = 0; i < componentData->size(); i++)
                {
                    auto configData = componentData->at(i);
                    m_configList.emplace_back(profile, entrypoint, const_cast<VAConfigAttrib*>(attriblist->data()), numAttribList, configData);
                }
            }
            else
            {
                ComponentData configData = {};
                m_configList.emplace_back(profile, entrypoint, const_cast<VAConfigAttrib*>(attriblist->data()), numAttribList, configData);
            }
        }
    }

    return VA_STATUS_SUCCESS;
}

VAStatus MediaCapsTableSpecific::QueryConfigProfiles(
    VAProfile *profileList,
    int32_t   *profilesNum)
{
    DDI_FUNC_ENTER;

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
    DDI_FUNC_ENTER;

    int i = 0;

    if(m_profileMap->find(profile) == m_profileMap->end())
    {
        return nullptr;
    }

    return const_cast<EntrypointMap*>(m_profileMap->at(profile));
}

AttribList* MediaCapsTableSpecific::QuerySupportedAttrib(
    VAProfile     profile,
    VAEntrypoint  entrypoint)
{
    DDI_FUNC_ENTER;

    if(m_profileMap->find(profile) == m_profileMap->end()                             ||
       m_profileMap->at(profile)->find(entrypoint) == m_profileMap->at(profile)->end())
    {
        return nullptr;
    }

    return const_cast<AttribList*>(m_profileMap->at(profile)->at(entrypoint)->attribList);
}

ConfigList* MediaCapsTableSpecific::GetConfigList()
{
    DDI_FUNC_ENTER;

    return &m_configList;
}

ConfigLinux* MediaCapsTableSpecific::QueryConfigItemFromIndex(
    VAConfigID     configId)
{
    DDI_FUNC_ENTER;

    if (!IS_VALID_CONFIG_ID(configId))
    {
        DDI_ASSERTMESSAGE("Invalid config ID");
        return nullptr;
    }

    if(IsDecConfigId(configId) && REMOVE_CONFIG_ID_DEC_OFFSET(configId) < m_configList.size())
    {
        return &m_configList[REMOVE_CONFIG_ID_DEC_OFFSET(configId)];
    }
    else if(IsEncConfigId(configId) && REMOVE_CONFIG_ID_ENC_OFFSET(configId) < m_configList.size())
    {
        return &m_configList[REMOVE_CONFIG_ID_ENC_OFFSET(configId)];
    }
    else if(IsVpConfigId(configId) && REMOVE_CONFIG_ID_VP_OFFSET(configId) < m_configList.size())
    {
        return &m_configList[REMOVE_CONFIG_ID_VP_OFFSET(configId)];
    }
    else if((m_cpCaps != nullptr) && (m_cpCaps->IsCpConfigId(configId)))
    {
        uint32_t index = m_cpCaps->GetCpConfigId(configId);
        DDI_CHK_CONDITION((index >=  m_configList.size()), "Invalid config ID", nullptr)

        return &m_configList[index];
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid config ID");
        return nullptr;
    }
}

VAStatus MediaCapsTableSpecific::CreateConfig(
    VAProfile       profile,
    VAEntrypoint    entrypoint,
    VAConfigAttrib  *attribList,
    int32_t         numAttribs,
    VAConfigID      *configId)
{
    DDI_FUNC_ENTER;

    DDI_UNUSED(attribList);
    DDI_UNUSED(numAttribs);
    DDI_UNUSED(configId);

    VAStatus ret = VA_STATUS_ERROR_UNSUPPORTED_PROFILE;
    for (auto configItem : m_configList)
    {
        // check profile, entrypoint here
        if (configItem.profile == profile)
        {
            ret = VA_STATUS_ERROR_UNSUPPORTED_ENTRYPOINT;
            if(configItem.entrypoint == entrypoint)
            {
                ret = VA_STATUS_SUCCESS;
                break;
            }
        }
    }

    return ret;
}

bool MediaCapsTableSpecific::IsDecConfigId(VAConfigID configId)
{
    VAConfigID curConfigId = REMOVE_CONFIG_ID_OFFSET(configId);

    return ((curConfigId >= DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE) &&
            (curConfigId <= DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_MAX));
}

bool MediaCapsTableSpecific::IsEncConfigId(VAConfigID configId)
{
    VAConfigID curConfigId = REMOVE_CONFIG_ID_OFFSET(configId);

    return ((curConfigId >= DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE) &&
            (curConfigId <= DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_MAX));
}

bool MediaCapsTableSpecific::IsVpConfigId(VAConfigID configId)
{
    VAConfigID curConfigId = REMOVE_CONFIG_ID_OFFSET(configId);

    return ((curConfigId >= DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE) &&
            (curConfigId <= DDI_VP_GEN_CONFIG_ATTRIBUTES_MAX));
}

VAStatus MediaCapsTableSpecific::DestroyConfig(VAConfigID configId)
{
    uint32_t index  = 0;
    VAStatus status = VA_STATUS_SUCCESS;
    DDI_FUNC_ENTER;

    if(!IS_VALID_CONFIG_ID(configId))
    {
        DDI_ASSERTMESSAGE("Invalid config ID");
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    if(IsDecConfigId(configId))
    {
        index = REMOVE_CONFIG_ID_DEC_OFFSET(configId);
    }
    else if(IsEncConfigId(configId))
    {
        index = REMOVE_CONFIG_ID_ENC_OFFSET(configId);
    }
    else if(IsVpConfigId(configId))
    {
        index = REMOVE_CONFIG_ID_VP_OFFSET(configId);
    }
    else if( (m_cpCaps != nullptr) && (m_cpCaps->IsCpConfigId(configId)) )
    {
        index = m_cpCaps->GetCpConfigId(configId);
    }
    else
    {
        DDI_ASSERTMESSAGE("Invalid config ID");
        return VA_STATUS_ERROR_INVALID_CONFIG;
    }

    if(index < m_configList.size())
    {
        DDI_NORMALMESSAGE("Succeed Destroy config ID");
        status = VA_STATUS_SUCCESS;
    }
    else
    {
        DDI_NORMALMESSAGE("Invalid config ID");
        status = VA_STATUS_ERROR_INVALID_CONFIG;
    }

    return status;
}

ImgTable* MediaCapsTableSpecific::GetImgTable()
{
    DDI_FUNC_ENTER;

    return m_imgTbl;
}

uint32_t MediaCapsTableSpecific::GetImageFormatsMaxNum()
{
    DDI_FUNC_ENTER;

    return m_imgTbl->size();
}

ProfileSurfaceAttribInfo* MediaCapsTableSpecific::QuerySurfaceAttributesFromConfigId(
    VAConfigID                 configId)
{
    DDI_FUNC_ENTER;

    if (!IS_VALID_CONFIG_ID(configId))
    {
        DDI_ASSERTMESSAGE("Invalid config ID");
        return nullptr;
    }  

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

    return const_cast<ProfileSurfaceAttribInfo*>(m_profileMap->at(profile)->at(entrypoint)->surfaceAttrib);
}