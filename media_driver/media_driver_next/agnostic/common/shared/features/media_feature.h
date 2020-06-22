/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_feature.h
//! \brief    Defines the common interface for media feature
//!
#ifndef __MEDIA_FEATURE_H__
#define __MEDIA_FEATURE_H__
#include "mos_os.h"
#include "media_feature_manager.h"
#include "media_utils.h"

class MediaFeatureManager;

class MediaFeature
{
public:
    MediaFeature(void *constSettings) : m_constSettings(constSettings){};
    MediaFeature() {};
    virtual ~MediaFeature() { }

    //!
    //! \brief  Init parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *settings);

    //!
    //! \brief  Update parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params);

protected:

    //! \brief  Allocate feature related resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateResources() { return MOS_STATUS_SUCCESS; };

    //! \brief  set feature refer to const settings.
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetConstSettings(){ return MOS_STATUS_SUCCESS; };

    //! \brief  Check whether the feather is enabled
    //! \return bool
    //!         true if enabled, otherwise false
    //!
    bool IsEnabled() const { return m_enabled; }

    bool               m_enabled = false;

    MediaFeatureManager *m_featureManager = nullptr;
    void                *m_constSettings = nullptr;
};

#endif  // !__ENCODE_FEATURE_H__
