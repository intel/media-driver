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
//! \file     capstable_data_linux_definition.h
//! \brief    This file defines the data structure for MediaCapsTableLinux.
//!

#ifndef __CAPSTABLE_DATA_LINUX_DEFINITION_H__
#define __CAPSTABLE_DATA_LINUX_DEFINITION_H__

#include "va/va.h"
#include "media_libva_common.h"
#include "media_libva_util.h"
#include "media_libva.h"
#include "media_libva_vp.h"

#include "hwinfo_linux.h"
#include "linux_system_info.h"
#include "media_ddi_decode_const.h"
#include "media_ddi_encode_const.h"
#include "media_libva_caps_factory.h"

//!
//! \struct  VASurfaceAttribInfo
//! \brief  parameters to construct VA Surface Attribute
//!
struct VASurfaceAttribInfo
{
    VASurfaceAttribType type1;
    VAGenericValueType  type2;
    uint32_t            flags;
    int32_t             value;
};

//!
//! \struct ComponentData
//! \brief  component specific data for ConfigLinux
//!
struct ComponentData
{
    union
    {
        struct // Encode usage
        {
            uint32_t rcMode;      //!< RateControl Mode
            uint32_t feiFunction; //!< Fei Function
        };
        struct // Decode usage
        {
            uint32_t sliceMode;   //!< Decode slice mode
            uint32_t encryptType; //!< Decode entrypoint Type
            uint32_t processType; //!< Decode processing Type
        };
    } data;
};

typedef std::vector<VASurfaceAttribInfo>  ProfileSurfaceAttribInfo;
typedef std::vector<ComponentData>        ConfigDataList;
typedef std::vector<VAConfigAttrib>       AttribList;

struct EntrypointData
{
    AttribList                *attribList;
    ConfigDataList            *configDataList;
    ProfileSurfaceAttribInfo  *surfaceAttrib;
};

typedef std::map<VAEntrypoint, EntrypointData*>   EntrypointMap;
typedef std::map<VAProfile,    EntrypointMap*>    ProfileMap;
typedef std::map<uint32_t,     VAImageFormat*>    ImgTable;

struct CapsData
{
    ProfileMap   *profileMap;
    ImgTable     *imgTbl;
};

#endif //__CAPSTABLE_DATA_LINUX_DEFINITION_H__
