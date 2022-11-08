/*
* Copyright (c) 2017-2021, Intel Corporation
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
//! \file        linux_media_skuwa.h
//! \brief
//!

#ifndef __LINUX_MEDIA_SKUWA_H__
#define __LINUX_MEDIA_SKUWA_H__

#include <stdint.h>
#include <stdbool.h>
#include <string>
#include <map>
#include "media_class_trace.h"

class MediaFeatureTable
{
public:
    typedef std::string SkuKey;
    typedef std::map<SkuKey, uint8_t> MediaMap;
    typedef MediaMap::iterator MapIterator;
public:
    //!
    //! \brief Constructor/Deconstructor
    //!
    MediaFeatureTable() { mediaMap = nullptr; }
    ~MediaFeatureTable()
    {
        if (mediaMap) {
            mediaMap->clear();
            delete mediaMap;
        }
        mediaMap = nullptr;
    }

    //!
    //! \brief Copy Constructor
    //!
    MediaFeatureTable(MediaFeatureTable &mediaTable)
    {
        if (mediaMap)
            mediaMap->clear();
        else
            mediaMap = new MediaMap;

        if (mediaTable.mediaMap && mediaMap)
        {
            *mediaMap = *(mediaTable.mediaMap);
        }
    }

    //!
    //! \brief    read the attribute for the given sku field.
    //!
    //! \param    [in] ftrKey
    //!           the required FtrKey of MediaFeatureTable
    //!
    //! \return   bool. true indicates that the attribute is enabled
    //!           false indicates that the attribute is disabled

    bool MediaReadSku(SkuKey ftrKey)
    {
        if (mediaMap == nullptr)
        {
            mediaMap = new MediaMap;
            if (mediaMap == nullptr)
                return false;
        }

        MapIterator item = mediaMap->find(ftrKey);
        if (item == mediaMap->end())
        {
            // false is returned if it is not found
            return false;
        }
        return item->second;
    }

    //!
    //! \brief    write/update the attribute for the given sku field.
    //!
    //! \param    [in] ftrKey
    //!           the required FtrKey of MediaFeatureTable
    //! \param    [in] ftrValue
    //!           the attribute for the given sku field

    void   MediaWriteSku(SkuKey ftrKey, uint8_t ftrValue)
    {
        if (mediaMap == nullptr)
        {
            mediaMap = new MediaMap;
            if (mediaMap == nullptr)
                return;
        }

        (*mediaMap)[ftrKey] = ftrValue;
        return;
    }

    //!
    //! \brief   clear all the elements
    //!
    void reset()
    {
        if (mediaMap)
        {
            mediaMap->clear();
            delete mediaMap;
        }
        mediaMap = nullptr;
    }

    //!
    //! \brief Copy= operator
    //!
    inline MediaFeatureTable& operator=(MediaFeatureTable &mediaTable)
    {
        if (mediaMap)
            mediaMap->clear();
        else
            mediaMap = new MediaMap;

        if (mediaTable.mediaMap && mediaMap)
        {
            *mediaMap = *(mediaTable.mediaMap);
        }

        return *this;
    }

    MediaMap *GetMediaSku()
    {
        return mediaMap;
    }

protected:
    MediaMap *mediaMap = nullptr;

MEDIA_CLASS_DEFINE_END(MediaFeatureTable)
};

class MediaWaTable
{
public:
    typedef std::string WaKey;
    typedef std::map<WaKey, uint8_t> MediaMap;
    typedef MediaMap::iterator MapIterator;
public:
    //!
    //! \brief Constructor
    //!
    MediaWaTable() { mediaMap = nullptr; }
    ~MediaWaTable()
    {
        if (mediaMap) {
            mediaMap->clear();
            delete mediaMap;
        }
        mediaMap = nullptr;
    }

    //!
    //! \brief Copy Constructor
    //!
    MediaWaTable(MediaWaTable &mediaTable)
    {
        if (mediaMap)
            mediaMap->clear();
        else
            mediaMap = new MediaMap;

        if (mediaTable.mediaMap && mediaMap)
            *mediaMap = *(mediaTable.mediaMap);
    }

    //!
    //! \brief    read the attribute for the given sku field.
    //!
    //! \param    [in] waKey
    //!           the required FtrKey of MediaWaTable
    //!
    //! \return   bool. true indicates that the attribute is enabled
    //!           false indicates that the attribute is disabled

    bool MediaReadWa(WaKey waKey)
    {
        if (mediaMap == nullptr)
        {
            mediaMap = new MediaMap;
            if (mediaMap == nullptr)
                return false;
        }

        MapIterator item = mediaMap->find(waKey);
        if (item == mediaMap->end())
        {
            // false is returned if it is not found
            return false;
        }
        return item->second;
    }

    //!
    //! \brief    write/update the attribute for the given wa field.
    //!
    //! \param    [in] waKey
    //!           the required FtrKey of MediaWaTable
    //! \param    [in] waValue
    //!           the attribute for the given wa field

    void   MediaWriteWa(WaKey waKey, uint8_t waValue)
    {
        if (mediaMap == nullptr)
        {
            mediaMap = new MediaMap;
            if (mediaMap == nullptr)
                return;
        }

        (*mediaMap)[waKey] = waValue;
        return;
    }

    //!
    //! \brief   clear all the elements
    //!
    void reset()
    {
        if (mediaMap)
        {
            mediaMap->clear();
            delete mediaMap;
        }
        mediaMap = nullptr;
    }

    //!
    //! \brief Copy= operator
    //!
    inline MediaWaTable& operator=(MediaWaTable &other)
    {
        if (mediaMap)
            mediaMap->clear();
        else
            mediaMap = new MediaMap;

        if (other.mediaMap && mediaMap)
        {
            *mediaMap = *(other.mediaMap);
        }

        return *this;
    }

    MediaMap *GetMediaWa()
    {
        return mediaMap;
    }

protected:
    MediaMap *mediaMap = nullptr;
    MEDIA_CLASS_DEFINE_END(MediaWaTable)
};

inline void MediaWriteSku(MediaFeatureTable *skuTable, const char *ftrKey, uint8_t value)
{
    skuTable->MediaWriteSku(ftrKey, value);
}

inline void MediaWriteWa(MediaWaTable *waTable, const char *waKey, uint8_t value)
{
    waTable->MediaWriteWa(waKey, value);
}

inline bool MediaReadSku(MediaFeatureTable *skuTable, const char *ftrKey)
{
    return skuTable->MediaReadSku(ftrKey);
}

inline bool MediaReadWa(MediaWaTable *waTable, const char *waKey)
{
    return waTable->MediaReadWa(waKey);
}

#define MEDIA_IS_SKU(s, f) MediaReadSku(s, #f)
#define MEDIA_WR_SKU(s, f, v) MediaWriteSku(s, #f, v)
#define MEDIA_IS_WA(s, w)  MediaReadWa(s, #w)
#define MEDIA_WR_WA(s, w, v) MediaWriteWa(s, #w, v)

#endif //__LINUX_MEDIA_SKUWA_H__
