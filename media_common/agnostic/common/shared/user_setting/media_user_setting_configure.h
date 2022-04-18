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
//! \file     media_user_setting_configure.h
//! \brief    The interface of media user setting configure.
//!

#ifndef __MEDIA_USER_SETTING_CONFIGURE__H__
#define __MEDIA_USER_SETTING_CONFIGURE__H__

#include <string>
#include "mos_utilities.h"
#include "media_user_setting_value.h"
#include "media_user_setting_definition.h"

namespace MediaUserSetting {

//!
//! The media user setting group
//! Device - for regkeys which are touched per device
//! Sequence - for regkeys which are touched per video sequence
//! Frame - for regkeys which are touched per frame
//! MaxCount - is used to configure size of Configure::m_definitions array
//! Note: you must not assign any numeric values to the enum items, except for
//! the device being set to 0
//!
enum Group
{
    Device = 0,
    Sequence,
    Frame,
    MaxCount
};

namespace Internal {

class Configure
{
public:
    //!
    //! \brief    Constructor
    //!
    Configure();

    //!
    //! \brief    Constructor
    //!
    Configure(MOS_USER_FEATURE_KEY_PATH_INFO *keyPathInfo);

    //!
    //! \brief    Destructor
    //!
    ~Configure();

    //!
    //! \brief    Register user setting item
    //! \param    [in] itemName
    //!           Name of the item
    //! \param    [in] group
    //!           Group of the item
    //! \param    [in] defaultValue
    //!           The default value of the item
    //! \param    [in] isReportKey
    //!           Whether this item can be reported
    //! \param    [in] debugOnly
    //!           Whether this item is only for debug/release-internal
    //! \param    [in] useCustomPath
    //!           Specifiy a read path
    //! \param    [in] customPath
    //!           The specified read path
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error, otherwise will return failed reason
    //!
    MOS_STATUS Register(
        const std::string &itemName,
        const Group &group,
        const Value &defaultValue,
        bool isReportKey,
        bool debugOnly,
        bool useCustomPath,
        const std::string &customPath,
        bool statePath);

    //!
    //! \brief    Read value of specific item
    //! \param    [out] value
    //!           The return value of the item
    //! \param    [in] itemName
    //!           Name of the item
    //! \param    [in] group
    //!           Group of the item
    //! \param    [in] mosContext
    //!           The pointer of mos context
    //! \param    [in] customValue
    //!           The custom value when failed
    //! \param    [in] useCustomValue
    //!           Whether use costom value when failed
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error,MOS_STATUS_USER_FEATURE_KEY_OPEN_FAILED if user setting is not set, otherwise will return specific failed reason
    //!
    MOS_STATUS Read(Value &value,
        const std::string &itemName,
        const Group &group,
        const Value &customValue,
        bool useCustomValue = false,
        uint32_t option = MEDIA_USER_SETTING_INTERNAL);

    //!
    //! \brief    Write value to specific item
    //! \param    [in] itemName
    //!           Name of the item
    //! \param    [in] value
    //!           The value write to specific item
    //! \param    [in] group
    //!           Group of the item
    //! \param    [in] mosContext
    //!           The pointer of mos context
    //! \param    [in] isForReport
    //!           This call is for reporting a item value or modify the value of the item
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if no error, otherwise will return failed reason
    //!
    MOS_STATUS Write(
        const std::string &itemName,
        const Value &value,
        const Group &group,
        bool isForReport,
        uint32_t option = MEDIA_USER_SETTING_INTERNAL);

    //!
    //! \brief    Get the path of the key
    //! \return   std::string
    //!           the path
    //!
    std::string GetPath(
        std::shared_ptr<Definition> def,
        uint32_t                    option,
        bool                        bReport);

    //!
    //! \brief    Get the report path of the internal key
    //! \return   std::string
    //!           the path
    //!
    std::string GetInteranlReportPath(
        std::shared_ptr<Definition> def);

    //!
    //! \brief    Get the read path of the internal key
    //! \return   std::string
    //!           the path
    //!
    std::string GetInteranlReadPath(
        std::shared_ptr<Definition> def);

    //!
    //! \brief    Get the path of the internal key
    //! \return   std::string
    //!           the path
    //!
    std::string GetInternalPath(
        std::shared_ptr<Definition> def,
        bool                        bReport);

    //!
    //! \brief    Get the path of the external key
    //! \return   std::string
    //!           the path
    //!
    std::string GetExternalPath(uint32_t option);

    //!
    //! \brief    Check whether definition of specific item name exist in all groups
    //! \param    [in] itemName
    //!           Item name
    //! \return   bool
    //!           true if exist, otherwise false
    //!
    inline bool IsDefinitionExist(const std::string &itemName)
    {
        bool ret = false;
        for (auto defs : m_definitions)
        {
            auto it = defs.find(MakeHash(itemName));
            if (it != defs.end())
            {
                ret = true;
                break;
            }
        }
        return ret;
    }

    //!
    //! \brief    Get media user setting definitions of specific group
    //! \param    [in] group
    //!           Group of the item
    //! \return   Media user setting definitions
    //!           Definitions of specific group, return definitions of device group if failed
    //!
    inline Definitions &GetDefinitions(const Group &group)
    {
        if (group < Group::Device || group >= Group::MaxCount)
        {
            return m_definitions[Group::Device];
        }

        return m_definitions[group];
    }
protected:

    //!
    //! \brief    Get hash value of specific string
    //! \param    [in] str
    //!           Input string
    //! \return   size_t
    //!           Hash value
    //!
    size_t MakeHash(const std::string &str)
    {
        std::hash<std::string> HashFunc;
        return HashFunc(str);
    }

protected:
    MosMutex m_mutexLock = {}; //!< mutex for protecting definitions
    Definitions m_definitions[Group::MaxCount]{}; //!< definitions of media user setting
    bool m_isDebugMode = false; //!< whether in debug/release-internal mode
    RegBufferMap m_regBufferMap{};
    MOS_USER_FEATURE_KEY_PATH_INFO *m_keyPathInfo = nullptr;

    static const UFKEY_NEXT m_rootKey;
    static const char *m_configPath;
    static const char *m_reportPath;
    static const std::map<uint32_t, const char *> m_pathOption;
};

}}
#endif