/*
* Copyright (c) 2017-2022, Intel Corporation
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
//! \file     mos_util_user_interface.h
//! \brief    Common MOS util user feature key service across different platform
//! \details  Common MOS util user feature key service across different platform
//!
#ifndef __MOS_UTIL_USER_INTERFACE_H__
#define __MOS_UTIL_USER_INTERFACE_H__

#include "mos_utilities.h"

using UserFeatureValueMapType = std::map<uint32_t, PMOS_USER_FEATURE_VALUE>;

class MosUtilUserInterface
{
public:
    MosUtilUserInterface() = default;
    virtual ~MosUtilUserInterface() = default;

    //!
    //! \brief    Add a user feature key to the m_userFeatureKeyMap
    //! \details  Each component call MosUtilities::MosDeclareUserFeatureKeysFromDescFields to add their specifc user key vlaue to the m_userFeatureKeyMap
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS AddEntry(uint32_t keyId, PMOS_USER_FEATURE_VALUE userFeatureKey);

    //!
    //! \brief    Del a user feature key from the m_userFeatureKeyMap
    //! \details  Each component call MosUtilities::MosDestroyUserFeatureKeysFromDescFields to delete their regostered user key from the m_userFeatureKeyMap
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS DelEntry(uint32_t keyId);

    //!
    //! \brief    Get a user feature key from the m_userFeatureKeyMap
    //! \details  Get a user feature key from the m_userFeatureKeyMap
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static PMOS_USER_FEATURE_VALUE GetValue(uint32_t keyId);

    //!
    //! \brief    Get user feature key map
    //! \details  Get user feature key map
    //! \return   UserFeatureValueMapType
    //!           Returns the map of user feature keys.
    //!
    static UserFeatureValueMapType& GetUserFeatureKeyMap()
    {
        return m_userFeatureKeyMap;
    }

private:
    static std::map<uint32_t, PMOS_USER_FEATURE_VALUE>  m_userFeatureKeyMap;
    static MosMutex                                     m_mosMutex;
};


#endif // __MOS_UTIL_USER_INTERFACE_H__
