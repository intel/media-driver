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
//! \file        mos_user_setting.h
//! \brief       This file defines the user settings of mos
//! \details  Common OS debug across different platform
//!

#ifndef _MOS_USER_SETTING_
#define _MOS_USER_SETTING_

#include "mos_defs.h"
#include "media_user_setting.h"

class MosUserSetting
{
public:
    //!
    //! \brief    Init the mos user settings
    //! \details  declare the common user settings
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS InitMosUserSetting(MediaUserSettingSharedPtr userSettingPtr);

    //!
    //! \brief    Init user settings
    //! \details  declare the ddi user settings
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS InitMosUserSettingSpecific(MediaUserSettingSharedPtr userSettingPtr);

private:
    //!
    //! \brief    Init the mos user settings of mos message
    //! \details  declare the common user settings
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS InitMosCommonUserSetting(MediaUserSettingSharedPtr userSettingPtr);

#if MOS_MESSAGES_ENABLED
    //!
    //! \brief    Init the mos user settings of mos message
    //! \details  declare the common user settings
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS InitMosMessageUserSetting(MediaUserSettingSharedPtr userSettingPtr);
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    Init the mos user settings of debug
    //! \details  declare the common user settings for debug
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS InitUserSettingForDebug(MediaUserSettingSharedPtr userSettingPtr);
#endif

    MEDIA_CLASS_DEFINE_END(MosUserSetting)
};
#endif //_MOS_USER_SETTING_
