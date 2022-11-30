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
//! \file     media_capstable_specific.h
//! \brief    Header file of media caps table class on specific os
//!

#ifndef __MEDIA_CAPSTABLE_LINUX_H__
#define __MEDIA_CAPSTABLE_LINUX_H__

#include <vector>
#include <map>
#include <set>

#include "va/va.h"
#include "va/va_drmcommon.h"
#include "capstable_data_linux_definition.h"
#include "media_capstable.h"

//!
//! \class  ConfigInfo
//! \brief  Component info to create specific component
//!
struct ComponentInfo
{
    VAProfile       profile       = VAProfileNone;
    VAEntrypoint    entrypoint    = VAEntrypointVLD;
    ComponentInfo(
        VAProfile p,
        VAEntrypoint e
        ) : profile(p), entrypoint(e) {};
    ComponentInfo(){};
};

bool operator<(const ComponentInfo &lhs, const ComponentInfo &rhs);

//!
//! \class  ConfigLinux
//! \brief  Config for Linux caps
//!
struct ConfigLinux
{
    VAProfile       profile       = VAProfileNone;
    VAEntrypoint    entrypoint    = VAEntrypointVLD;
    VAConfigAttrib  *attribList   = nullptr;
    int32_t         numAttribs    = 0;
    ComponentData   componentData = {};

    ConfigLinux(
        VAProfile      p,
        VAEntrypoint   e,
        VAConfigAttrib *a,
        int32_t        n,
        ComponentData  c) : profile(p), entrypoint(e), attribList(a), numAttribs(n), componentData(c){}
    ConfigLinux(){}
};

typedef std::vector<ConfigLinux> ConfigList;

#define CONFIG_ATTRIB_NONE 0x00000000

// This offset is for cap fallback enabling, can be removed when all refactor done
#define CONFIG_ID_OFFSET 10000
#define ADD_CONFIG_ID_OFFSET(x) ((x) + CONFIG_ID_OFFSET)
#define ADD_CONFIG_ID_DEC_OFFSET(x) (ADD_CONFIG_ID_OFFSET(x) + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE)
#define ADD_CONFIG_ID_ENC_OFFSET(x) (ADD_CONFIG_ID_OFFSET(x) + DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE)
#define ADD_CONFIG_ID_VP_OFFSET(x) (ADD_CONFIG_ID_OFFSET(x) + DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE)
#define REMOVE_CONFIG_ID_OFFSET(x) ((x) - CONFIG_ID_OFFSET)
#define REMOVE_CONFIG_ID_DEC_OFFSET(x) (REMOVE_CONFIG_ID_OFFSET(x) - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_DEC_BASE)
#define REMOVE_CONFIG_ID_ENC_OFFSET(x) (REMOVE_CONFIG_ID_OFFSET(x) - DDI_CODEC_GEN_CONFIG_ATTRIBUTES_ENC_BASE)
#define REMOVE_CONFIG_ID_VP_OFFSET(x) (REMOVE_CONFIG_ID_OFFSET(x) - DDI_VP_GEN_CONFIG_ATTRIBUTES_BASE)
#define IS_VALID_CONFIG_ID(x) ((x) >= CONFIG_ID_OFFSET)

class DdiCpCapsInterface;
//!
//! \class  MediaLibvaCaps
//! \brief  Media libva caps
//!
class MediaCapsTableSpecific : public MediaCapsTable<CapsData>
{
private:
    PlatformInfo  m_plt;
    ProfileMap    *m_profileMap = nullptr;
    ImgTable      *m_imgTbl     = nullptr;
    DdiCpCapsInterface *m_cpCaps = nullptr;

public:
    //!
    //! \brief  Store config
    //!
    ConfigList m_configList = {};

    //!
    //! \brief    Constructor
    //!
    MediaCapsTableSpecific(HwDeviceInfo &deviceInfo);

    //!
    //! \brief    Destructor
    //!
    ~MediaCapsTableSpecific();

    //!
    //! \brief    Init configlist
    //!
    //! \param    [in] mediaCtx
    //!           media context
    //!
    VAStatus Init(DDI_MEDIA_CONTEXT *mediaCtx);

    //!
    //! \brief    Get configlist, this is for component createConfig
    //!
    ConfigList* GetConfigList();

    //!
    //! \brief    Get Image Table
    //!
    //! \return   ImgTable
    //!
    ImgTable* GetImgTable();

    //!
    //! \brief    Get Supported Attrib Value
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //!
    //! \return   AttribList*
    //!           nullptr if query failed
    //!
    AttribList* QuerySupportedAttrib(
        VAProfile      profile,
        VAEntrypoint   entrypoint);

    //!
    //! \brief    Get specific config item
    //!
    //! \param    [in] configId
    //!           config list index
    //!
    //! \return   ConfigLinux
    //!           nullptr if invalid index
    //!
    ConfigLinux* QueryConfigItemFromIndex(
        VAConfigID    configId);

    //!
    //! \brief    Create a configuration
    //! \details  It passes in the attribute list that specifies the attributes it
    //!           cares about, with the rest taking default values.
    //!
    //! \param    [in] profile
    //!           VA profile
    //!
    //! \param    [in] entrypoint
    //!           VA entrypoint
    //!
    //! \param    [in] attribList
    //!           Pointer to VAConfigAttrib array that specifies the attributes
    //!
    //! \param    [in] numAttribs
    //!           Number of VAConfigAttrib in the array attribList
    //!
    //! \param    [out] configId
    //!           Pointer to returned VAConfigID if success
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus CreateConfig(
        VAProfile       profile,
        VAEntrypoint    entrypoint,
        VAConfigAttrib  *attribList,
        int32_t         numAttribs,
        VAConfigID      *configId);

    //!
    //! \brief    Check if the configID is a valid decode config
    //!
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   True if the configID is a valid decode config, otherwise false
    //!
    bool IsDecConfigId(VAConfigID configId);

    //!
    //! \brief    Check if the configID is a valid encode config
    //!
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   True if the configID is a valid encode config, otherwise false
    //!
    bool IsEncConfigId(VAConfigID configId);

    //!
    //! \brief    Check if the configID is a valid vp config
    //!
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   True if the configID is a valid vp config, otherwise false
    //!
    bool IsVpConfigId(VAConfigID configId);

    //!
    //! \brief    Destory the VAConfigID
    //!
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if succeed
    //!           VA_STATUS_ERROR_INVALID_CONFIG if the conifgId is invalid
    //!
    VAStatus DestroyConfig(VAConfigID configId);

    //!
    //! \brief    Query EntrypointsMap
    //!
    //! \param    [in] configId
    //!
    //! \return   EntrypointMap*
    //!           nullptr if invalid profile
    //!
    EntrypointMap* QueryConfigEntrypointsMap(
        VAProfile      profile);

    //!
    //! \brief    Query supported profiles
    //!
    //! \param    [in] profileList
    //!           Pointer to VAProfile array that can hold at least vaMaxNumProfile() entries
    //!
    //! \param    [out] numProfiles
    //!           Pointer to int32_t. It returns the actual number of supported profiles.
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    VAStatus QueryConfigProfiles(
        VAProfile *profileList,
        int32_t   *profilesNum);

    //!
    //! \brief    Query SurfaceAttributes From ConfigId
    //!
    //! \param    [in] configId
    //!           Supported surface attrib
    //!
    //! \return   ProfileSurfaceAttribInfo*
    //!           nullptr if invalid configid
    //!
    ProfileSurfaceAttribInfo* QuerySurfaceAttributesFromConfigId(
        VAConfigID                configId);

    //!
    //! \brief    Return the maxinum number of supported image formats for current platform ipVersion
    //!
    //! \return   The maxinum number of supported image formats for current platform ipVersion
    //!
    uint32_t GetImageFormatsMaxNum();
MEDIA_CLASS_DEFINE_END(MediaCapsTableSpecific)
};


#endif //__MEDIA_CAPSTABLE_LINUX_H__
