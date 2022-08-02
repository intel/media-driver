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
//! \file     media_user_setting_definition.h
//! \brief    Media user setting item definition
//!

#ifndef __MEDIA_USER_SETTING_DEFINITION__H__
#define __MEDIA_USER_SETTING_DEFINITION__H__

#include <string>
#include <map>
#include <memory>
#include <iosfwd>
#include "mos_defs_specific.h"
#include "media_user_setting_value.h"

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

class Definition
{
public:
    //!
    //! \brief    Constructor
    //! \param    [in] itemName
    //!           Name of the item
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
    Definition(const std::string &itemName,
               const Value &defaultValue,
               bool isReportKey,
               bool debugOnly,
               bool useCustomPath,
               const std::string &subPath,
               UFKEY_NEXT  rootKey,
               bool        statePath);

    //!
    //! \brief    Constructor
    //! \param    [in] Definition
    //!           Reference of definition
    Definition(const Definition& def);

    //!
    //! \brief    Destructor
    //!
    virtual ~Definition();

    Definition& operator=(const Definition& def);

    //!
    //! \brief    Get the item name of the definition
    //! \return   std::string
    //!           the item name
    //!
    std::string ItemName() const { return m_itemName; }

    //!
    //! \brief    Get the sub path of the definition
    //! \return   std::string
    //!           the custom path
    //!
    std::string &ItemEnvName() { return m_itemEnvName; }

    //!
    //! \brief    Get the debug flag
    //! \return   bool
    //!           debug flag
    //!
    bool IsDebugOnly() const { return m_debugOnly; }

    //!
    //! \brief    Get default value of the user setting item
    //! \return   Value
    //!           default value
    //!
    Value DefaultValue() const { return m_defaultValue; }

    //!
    //! \brief    Get report flag
    //! \return   bool
    //!           report flag
    //!
    bool IsReportKey() const { return m_isReportKey; }

    //!
    //! \brief    Get report flag
    //! \return   bool
    //!           report flag
    //!
    bool UseCustomPath() const { return m_useCustomePath; }

    //!
    //! \brief    Get the sub path of the definition
    //! \return   std::string
    //!           the custom path
    //!
    std::string GetSubPath() const { return m_subPath; }

    //!
    //! \brief    Get the custom path of the definition
    //! \return   std::string
    //!           the custom path
    //!
    UFKEY_NEXT GetRootKey() const { return m_rootKey; }

    //!
    //! \brief    Get the custom path of the definition
    //! \return   std::string
    //!           the custom path
    //!
    bool UseStatePath() const { return m_statePath; }
private:
    //!
    //! \brief    Set the values of definition
    //! \param    [in] Definition
    //!           Reference of definition
    void SetData(const Definition& def);

private:
    std::string m_itemName{};   //!< Item name
    std::string m_itemEnvName{};             //!< Item name
    Value m_defaultValue {};    //!< Default value
    bool m_isReportKey = false; //!< This item value can be reported
    bool m_debugOnly = false;   //!< Whether the item is only enabled in debug/release-internal mode
    bool m_useCustomePath = false;   //!< Whether the item read from a specific path
    std::string m_subPath{};    //!< custome path is a relative path, it could be null
    UFKEY_NEXT m_rootKey{};    //!< root key
    bool m_statePath      = true;    //!< Whether the item read from a specific path
};

using Definitions = std::map<std::size_t, std::shared_ptr<Definition>>;

}
}
#endif