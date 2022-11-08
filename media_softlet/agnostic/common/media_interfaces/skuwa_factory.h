/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     skuwa_factory.h
//! \brief    Defines one template class for the management of SKU/WA/SystemInfo
//!

#ifndef _SKUWA_FACTORY_H_
#define _SKUWA_FACTORY_H_

#include <map>
#include <vector>
#include <utility>
#include <cstdarg>
#include <cstdint>
#include "media_class_trace.h"
//!
//! \class    DeviceInfoFactory
//! \brief    Device info factory
//!
template <class T>
class DeviceInfoFactory
{
public:
    typedef T*  Type;
    typedef uint32_t                 KeyType;
    typedef std::map<KeyType, Type>  Creators;
    typedef typename Creators::iterator iterator;

    //!
    //! \brief    register one value with key.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of uint32_t
    //! \param    [in] value
    //!           T *, the type of info related with key
    //!
    //!
    //! \return   true is returned if it is successfully registerted with @param key
    //!           false is returned if @param key is already registered and doesn't register
    //!
    static bool RegisterDevice(KeyType key, Type value)
    {
        std::pair<iterator, bool> result =
            GetCreators().insert(std::make_pair(key, value));

        return result.second;
    }

    //!
    //! \brief    Get the pointer of DeviceInfo registered with key.
    //!
    //! \param    [in] key
    //!           KeyType, the type alias of std::string.
    //!
    //! \return   the pointer of DeviceInfo is returned if @param key is found.
    //!           nullptr is returned if @param key is not found
    //!
    static Type LookupDevice(KeyType key)
    {
        Creators &creators = GetCreators();
        iterator  creator  = creators.find(key);
        if (creator != creators.end())
            return creator->second;

        return nullptr;
    }

private:

    //!
    //! \brief    obtain the static pair table of param@ key and callback function
    //! \details  obtain the static pair table that is registerted with key and callback function
    //!

    //! \return   return the static pair table about the param @key and callback function
    static Creators &GetCreators()
    {
        static Creators creators;

        return creators;
    }
MEDIA_CLASS_DEFINE_END(DeviceInfoFactory)
};

#endif /*  _SKUWA_FACTORY_H_ */
