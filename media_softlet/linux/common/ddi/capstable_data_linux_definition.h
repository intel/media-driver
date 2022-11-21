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
//! \file     capstable_data_linux_definition.h
//! \brief    This file defines the data structure for MediaCapsTableLinux.
//!

#ifndef __CAPSTABLE_DATA_LINUX_DEFINITION_H__
#define __CAPSTABLE_DATA_LINUX_DEFINITION_H__

#include "va/va.h"
#include "media_libva_util_next.h"

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
    uint32_t            flags;
    VAGenericValue      value;
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
        struct // Cp usage
        {
            uint32_t sessionMode;   //!< Session mode
            uint32_t sessionType;   //!< Session type
            uint32_t algorithm;     //!< Algorithm
            uint32_t blockSize;     //!< Block size
            uint32_t counterMode;   //!< Counter mode
            uint32_t sampleType;    //!< Sample type
            uint32_t usage;         //!< Usage
        };
    } data;

    ComponentData()
    {
        data.sessionMode = data.sessionType = data.algorithm = data.blockSize = data.counterMode = data.sampleType = data.usage = 0;
    }
    ComponentData(uint32_t rcMode, uint32_t feiFunction)
    {
        data.rcMode = rcMode; data.feiFunction = feiFunction;
    }
    ComponentData(uint32_t sessionMode, uint32_t sessionType, uint32_t algorithm, uint32_t blockSize, uint32_t counterMode, uint32_t sampleType, uint32_t usage)
    {
        data.sessionMode = sessionMode; data.sessionType = sessionType; data.algorithm = algorithm; data.blockSize = blockSize;
        data.counterMode = counterMode; data.sampleType  = sampleType;  data.usage     = usage;
    }
    ComponentData(uint32_t sliceMode, uint32_t encryptType, uint32_t processType)
    {
        data.sliceMode = sliceMode; data.encryptType = encryptType; data.processType = processType;
    }
};

typedef std::vector<VASurfaceAttribInfo>  ProfileSurfaceAttribInfo;
typedef std::vector<ComponentData>        ConfigDataList;
typedef std::vector<VAConfigAttrib>       AttribList;

struct EntrypointData
{
    const AttribList                *attribList;
    const ConfigDataList            *configDataList;
    const ProfileSurfaceAttribInfo  *surfaceAttrib;
};

typedef std::map<const VAEntrypoint, const EntrypointData*>   EntrypointMap;
typedef std::map<const VAProfile,    const EntrypointMap*>    ProfileMap;
typedef std::map<const uint32_t,     const VAImageFormat*>    ImgTable;

struct CapsData
{
    const ProfileMap   *profileMap;
    const ImgTable     *imgTbl;
};

#endif //__CAPSTABLE_DATA_LINUX_DEFINITION_H__
